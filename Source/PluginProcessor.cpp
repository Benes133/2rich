#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

class AuroraSound final : public juce::SynthesiserSound {
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class AuroraProcessor::Voice final : public juce::SynthesiserVoice {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return dynamic_cast<AuroraSound*>(sound) != nullptr;
    }

    void prepare(double sampleRate) {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = 4096;
        spec.numChannels = 2;
        voiceFilter.prepare(spec);
        voiceFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        voiceFilter.reset();
    }

    void startNote(int note, float velocity, juce::SynthesiserSound*, int) override {
        hz = juce::MidiMessage::getMidiNoteInHertz(note);
        vel = velocity;
        for (auto& phase : phase1) phase = 0.0;
        for (auto& phase : phase2) phase = 0.0;
        env = 0.0f;
        filterEnv = 0.0f;
        stage = 1;
        filterStage = 1;
        voiceFilter.reset();
    }

    void stopNote(float, bool allowTailOff) override {
        if (allowTailOff) {
            stage = 4;
            filterStage = 4;
        } else {
            clearCurrentNote();
            env = 0.0f;
            filterEnv = 0.0f;
            voiceFilter.reset();
        }
    }

    void pitchWheelMoved(int) override {}

    void controllerMoved(int controller, int value) override {
        if (controller == 1)
            modWheel = (float)value / 127.0f;
    }

    void renderNextBlock(juce::AudioBuffer<float>& buffer, int start, int count) override {
        const double sr = getSampleRate();
        if (sr <= 0.0 || processor == nullptr)
            return;

        auto value = [this](const char* id) {
            return processor->params.getRawParameterValue(id)->load();
        };

        const int wave1 = (int)value("wave1");
        const int wave2 = (int)value("wave2");
        const int table = (int)value("wtTable");
        const float baseMix = value("mix");
        const float detune = value("detune");
        const float sub = value("sub");
        const int unisonVoices = juce::jlimit(1, 7, (int)value("unison"));
        const float unisonDetune = value("uniDetune");
        const float unisonSpread = value("uniSpread");

        const float ampAttack = juce::jmax(0.001f, value("attack"));
        const float ampDecay = juce::jmax(0.01f, value("decay"));
        const float ampSustain = value("sustain");
        const float ampRelease = juce::jmax(0.01f, value("release"));

        const float filtAttack = juce::jmax(0.001f, value("filterAttack"));
        const float filtDecay = juce::jmax(0.01f, value("filterDecay"));
        const float filtSustain = value("filterSustain");
        const float filtRelease = juce::jmax(0.01f, value("filterRelease"));
        const float filtEnvAmount = value("filterEnvAmount");

        const int sourceIds[4] = {
            (int)value("mod1Source"), (int)value("mod2Source"),
            (int)value("mod3Source"), (int)value("mod4Source")
        };
        const int targetIds[4] = {
            (int)value("mod1Target"), (int)value("mod2Target"),
            (int)value("mod3Target"), (int)value("mod4Target")
        };
        const float routeAmounts[4] = {
            value("mod1Amount"), value("mod2Amount"),
            value("mod3Amount"), value("mod4Amount")
        };

        for (int i = 0; i < count; ++i) {
            const int blockIndex = start + i;
            const float lfo1 = blockIndex < (int)processor->lfo1Block.size()
                ? processor->lfo1Block[(size_t)blockIndex] : 0.0f;
            const float lfo2 = blockIndex < (int)processor->lfo2Block.size()
                ? processor->lfo2Block[(size_t)blockIndex] : 0.0f;

            if (stage == 1) {
                env += 1.0f / (float)(sr * ampAttack);
                if (env >= 1.0f) { env = 1.0f; stage = 2; }
            } else if (stage == 2) {
                env -= (1.0f - ampSustain) / (float)(sr * ampDecay);
                if (env <= ampSustain) { env = ampSustain; stage = 3; }
            } else if (stage == 4) {
                env -= 1.0f / (float)(sr * ampRelease);
                if (env <= 0.0f) {
                    env = 0.0f;
                    filterEnv = 0.0f;
                    clearCurrentNote();
                    break;
                }
            }

            if (filterStage == 1) {
                filterEnv += 1.0f / (float)(sr * filtAttack);
                if (filterEnv >= 1.0f) { filterEnv = 1.0f; filterStage = 2; }
            } else if (filterStage == 2) {
                filterEnv -= (1.0f - filtSustain) / (float)(sr * filtDecay);
                if (filterEnv <= filtSustain) { filterEnv = filtSustain; filterStage = 3; }
            } else if (filterStage == 4) {
                filterEnv -= 1.0f / (float)(sr * filtRelease);
                if (filterEnv <= 0.0f) { filterEnv = 0.0f; filterStage = 0; }
            }

            const float sources[7] = {
                0.0f, lfo1, lfo2, vel, env, filterEnv, modWheel
            };
            float cutoffOctaves = filterEnv * filtEnvAmount * 4.0f;
            float mixMod = 0.0f;
            float positionMod = 0.0f;
            float pitchCents = 0.0f;
            float panMod = 0.0f;
            float resonanceMod = 0.0f;

            for (int route = 0; route < 4; ++route) {
                const int source = sourceIds[route];
                const int target = targetIds[route];
                if (source <= 0 || source >= 7 || target <= 0 || target >= 7)
                    continue;

                const float mod = sources[source] * routeAmounts[route];
                switch (target) {
                    case 1: cutoffOctaves += mod * 4.0f; break;
                    case 2: mixMod += mod; break;
                    case 3: positionMod += mod; break;
                    case 4: pitchCents += mod * 1200.0f; break;
                    case 5: panMod += mod; break;
                    case 6: resonanceMod += mod * 0.45f; break;
                    default: break;
                }
            }

            const float currentMix = juce::jlimit(0.0f, 1.0f, baseMix + mixMod);
            const float position = juce::jlimit(0.0f, 1.0f, value("wtPosition") + positionMod);
            const float cutoffHz = juce::jlimit(25.0f, 19000.0f,
                value("cutoff") * std::pow(2.0f, juce::jlimit(-7.0f, 7.0f, cutoffOctaves)));
            const float resonance = juce::jlimit(0.1f, 1.0f,
                value("res") + resonanceMod);

            float left = 0.0f;
            float right = 0.0f;

            for (int voice = 0; voice < unisonVoices; ++voice) {
                const float normalized = unisonVoices == 1 ? 0.0f
                    : ((float)voice / (float)(unisonVoices - 1) - 0.5f) * 2.0f;
                const double cents = detune + pitchCents + normalized * unisonDetune;
                const double ratio1 = std::pow(2.0, cents / 1200.0);
                const double ratio2 = std::pow(2.0, (cents + normalized * 4.0f) / 1200.0);
                const float phaseA = (float)(phase1[(size_t)voice] + normalized * 0.13f);
                const float phaseB = (float)(phase2[(size_t)voice] + normalized * 0.19f);
                const float oscA = wave(phaseA, wave1, table, position);
                const float oscB = wave(phaseB, wave2, table, position);
                const float sample = (1.0f - currentMix) * oscA + currentMix * oscB
                    + sub * std::sin((float)phase1[(size_t)voice] * 0.5f);
                const float pan = juce::jlimit(-0.95f, 0.95f,
                    normalized * unisonSpread * 0.85f + panMod * 0.85f);

                left += sample * std::sqrt(0.5f * (1.0f - pan)) * vel * env;
                right += sample * std::sqrt(0.5f * (1.0f + pan)) * vel * env;

                phase1[(size_t)voice] += juce::MathConstants<double>::twoPi * hz * ratio1 / sr;
                phase2[(size_t)voice] += juce::MathConstants<double>::twoPi * hz * ratio2 / sr;
                phase1[(size_t)voice] = std::fmod(phase1[(size_t)voice], juce::MathConstants<double>::twoPi);
                phase2[(size_t)voice] = std::fmod(phase2[(size_t)voice], juce::MathConstants<double>::twoPi);
            }

            const float gain = 0.22f / std::sqrt((float)unisonVoices);
            left *= gain;
            right *= gain;

            voiceFilter.setCutoffFrequency(cutoffHz);
            voiceFilter.setResonance(resonance);
            left = voiceFilter.processSample(0, left);
            right = voiceFilter.processSample(1, right);

            buffer.addSample(0, blockIndex, left);
            if (buffer.getNumChannels() > 1)
                buffer.addSample(1, blockIndex, right);
        }
    }

    void setProcessor(AuroraProcessor* p) { processor = p; }

private:
    float wave(float phase, int type, int table, float position) {
        const float x = phase - std::floor(phase);
        auto basic = [&](int shape) {
            switch (shape) {
                case 1: return x < 0.5f ? 1.0f : -1.0f;
                case 2: return 1.0f - 4.0f * std::abs(x - 0.5f);
                case 3: return std::sin(x * juce::MathConstants<float>::twoPi);
                default: return 2.0f * x - 1.0f;
            }
        };

        const float saw = basic(0);
        const float square = basic(1);
        const float triangle = basic(2);
        const float sine = basic(3);
        float first = saw, second = sine;
        switch (table) {
            case 1: first = square; second = triangle; break;
            case 2: first = triangle; second = sine; break;
            case 3: first = saw; second = square; break;
            case 4: first = sine; second = std::sin(x * juce::MathConstants<float>::twoPi * 2.0f); break;
            case 5: first = saw; second = std::tanh(saw * 3.0f); break;
            default: first = basic(type); second = basic(type == 0 ? 3 : 0); break;
        }
        return juce::jmap(position, first, second);
    }

    AuroraProcessor* processor = nullptr;
    juce::dsp::StateVariableTPTFilter<float> voiceFilter;
    double hz = 440.0;
    std::array<double, 7> phase1{}, phase2{};
    float vel = 0.0f, env = 0.0f, filterEnv = 0.0f, modWheel = 0.0f;
    int stage = 0, filterStage = 0;
};

AuroraProcessor::AuroraProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      params(*this, nullptr, "PARAMS", createLayout()) {
    for (int i = 0; i < 16; ++i) {
        auto* voice = new Voice();
        voice->setProcessor(this);
        synth.addVoice(voice);
    }
    synth.addSound(new AuroraSound());
}

AuroraProcessor::APVTS::ParameterLayout AuroraProcessor::createLayout() {
    APVTS::ParameterLayout layout;
    auto choice = [&](const char* id, const char* name, juce::StringArray values, int def) {
        layout.add(std::make_unique<juce::AudioParameterChoice>(id, name, values, def));
    };
    auto f = [&](const char* id, const char* name, float low, float high, float def, float skew = 1.0f) {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            id, name, juce::NormalisableRange<float>(low, high, 0.001f, skew), def));
    };

    choice("wave1", "OSC 1 Wave", { "Saw", "Square", "Triangle", "Sine" }, 0);
    choice("wave2", "OSC 2 Wave", { "Saw", "Square", "Triangle", "Sine" }, 2);
    f("mix", "OSC Mix", 0.0f, 1.0f, 0.35f);
    f("detune", "Detune", -50.0f, 50.0f, 7.0f);
    f("sub", "Sub Level", 0.0f, 1.0f, 0.2f);
    f("cutoff", "Cutoff", 40.0f, 18000.0f, 12000.0f, 0.35f);
    f("res", "Resonance", 0.1f, 1.0f, 0.2f);

    f("attack", "Attack", 0.001f, 4.0f, 0.01f, 0.4f);
    f("decay", "Decay", 0.01f, 4.0f, 0.2f, 0.4f);
    f("sustain", "Sustain", 0.0f, 1.0f, 0.75f);
    f("release", "Release", 0.01f, 5.0f, 0.3f, 0.4f);

    f("filterAttack", "Filter Env Attack", 0.001f, 4.0f, 0.01f, 0.4f);
    f("filterDecay", "Filter Env Decay", 0.01f, 5.0f, 0.25f, 0.4f);
    f("filterSustain", "Filter Env Sustain", 0.0f, 1.0f, 0.0f);
    f("filterRelease", "Filter Env Release", 0.01f, 5.0f, 0.25f, 0.4f);
    f("filterEnvAmount", "Filter Envelope Amount", 0.0f, 1.0f, 0.0f);

    f("lfo1Rate", "LFO 1 Rate", 0.02f, 20.0f, 1.0f, 0.35f);
    f("lfo2Rate", "LFO 2 Rate", 0.02f, 20.0f, 0.25f, 0.35f);
    choice("lfo1Shape", "LFO 1 Shape", { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" }, 0);
    choice("lfo2Shape", "LFO 2 Shape", { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" }, 1);

    const juce::StringArray sources = { "Off", "LFO 1", "LFO 2", "Velocity", "Amp Env", "Filter Env", "Mod Wheel" };
    const juce::StringArray targets = { "Off", "Cutoff", "Osc Mix", "WT Position", "Pitch", "Pan", "Resonance" };
    for (int route = 1; route <= 4; ++route) {
        const auto suffix = juce::String(route);
        choice(("mod" + suffix + "Source").toRawUTF8(), ("Mod " + suffix + " Source").toRawUTF8(), sources, 0);
        choice(("mod" + suffix + "Target").toRawUTF8(), ("Mod " + suffix + " Target").toRawUTF8(), targets, 0);
        f(("mod" + suffix + "Amount").toRawUTF8(), ("Mod " + suffix + " Amount").toRawUTF8(), -1.0f, 1.0f, 0.0f);
    }

    f("master", "Master", 0.0f, 1.0f, 0.8f);
    f("drive", "Drive", 0.0f, 1.0f, 0.0f);
    f("width", "Stereo Width", 0.0f, 1.0f, 0.5f);
    choice("wtTable", "Wavetable", { "Basic", "Harmonics", "Triangle Morph", "Digital", "Octave", "Fold" }, 0);
    f("wtPosition", "WT Position", 0.0f, 1.0f, 0.0f);
    layout.add(std::make_unique<juce::AudioParameterInt>("unison", "Unison Voices", 1, 7, 1));
    f("uniDetune", "Unison Detune", 0.0f, 50.0f, 12.0f);
    f("uniSpread", "Unison Spread", 0.0f, 1.0f, 0.65f);

    f("fxEq", "EQ Amount", 0.0f, 1.0f, 0.0f);
    f("fxDist", "FX Distortion", 0.0f, 1.0f, 0.0f);
    f("fxChorus", "Chorus", 0.0f, 1.0f, 0.0f);
    f("fxFlange", "Flanger", 0.0f, 1.0f, 0.0f);
    f("fxDelay", "Delay", 0.0f, 1.0f, 0.0f);
    f("fxReverb", "Reverb", 0.0f, 1.0f, 0.0f);
    f("fxLow", "EQ Low", -12.0f, 12.0f, 0.0f);
    f("fxMid", "EQ Mid", -12.0f, 12.0f, 0.0f);
    f("fxHigh", "EQ High", -12.0f, 12.0f, 0.0f);
    return layout;
}

void AuroraProcessor::prepareToPlay(double sr, int blockSize) {
    sampleRate = sr;
    synth.setCurrentPlaybackSampleRate(sr);
    filter.prepare({ sr, (juce::uint32)juce::jmax(1, blockSize), 2 });
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<Voice*>(synth.getVoice(i)))
            voice->prepare(sr);

    const auto storageSize = (size_t)juce::jmax(8192, blockSize + 1);
    lfo1Block.resize(storageSize);
    lfo2Block.resize(storageSize);
    for (auto& channel : delayBuffer)
        channel.assign((size_t)juce::jmax(1, (int)sr * 2), 0.0f);
    delayWrite = 0;

    juce::dsp::Reverb::Parameters reverbParameters;
    reverbParameters.roomSize = 0.45f;
    reverbParameters.damping = 0.55f;
    reverbParameters.wetLevel = 0.0f;
    reverbParameters.dryLevel = 1.0f;
    reverb.setParameters(reverbParameters);
    reverb.reset();
}

bool AuroraProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AuroraProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    auto value = [this](const char* id) {
        return params.getRawParameterValue(id)->load();
    };

    const int samples = buffer.getNumSamples();
    if ((int)lfo1Block.size() < samples) lfo1Block.resize((size_t)samples);
    if ((int)lfo2Block.size() < samples) lfo2Block.resize((size_t)samples);

    const float rate1 = value("lfo1Rate");
    const float rate2 = value("lfo2Rate");
    const int shape1 = (int)value("lfo1Shape");
    const int shape2 = (int)value("lfo2Shape");
    const double phaseStep1 = rate1 / sampleRate;
    const double phaseStep2 = rate2 / sampleRate;

    auto nextRandom = [this]() {
        randomSeed ^= randomSeed << 13;
        randomSeed ^= randomSeed >> 17;
        randomSeed ^= randomSeed << 5;
        return (float)(randomSeed & 0x00ffffffu) / 8388607.5f - 1.0f;
    };
    auto lfoShape = [](int shape, float phase, float held) {
        switch (shape) {
            case 1: return 1.0f - 4.0f * std::abs(phase - 0.5f);
            case 2: return 2.0f * phase - 1.0f;
            case 3: return phase < 0.5f ? 1.0f : -1.0f;
            case 4: return held;
            default: return std::sin(phase * juce::MathConstants<float>::twoPi);
        }
    };

    for (int i = 0; i < samples; ++i) {
        lfo1Block[(size_t)i] = lfoShape(shape1, (float)lfo1Phase, lfo1Hold);
        lfo2Block[(size_t)i] = lfoShape(shape2, (float)lfo2Phase, lfo2Hold);
        lfo1Phase += phaseStep1;
        lfo2Phase += phaseStep2;
        if (lfo1Phase >= 1.0) {
            lfo1Phase -= std::floor(lfo1Phase);
            lfo1Hold = nextRandom();
        }
        if (lfo2Phase >= 1.0) {
            lfo2Phase -= std::floor(lfo2Phase);
            lfo2Hold = nextRandom();
        }
    }

    synth.renderNextBlock(buffer, midi, 0, samples);

    const float drive = juce::jmax(value("drive"), value("fxDist"));
    if (drive > 0.001f) {
        const float gain = 1.0f + drive * 15.0f;
        const float normalizer = std::tanh(gain);
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            for (int i = 0; i < samples; ++i)
                buffer.setSample(channel, i,
                    std::tanh(buffer.getSample(channel, i) * gain) / normalizer);
    }

    const float low = value("fxLow");
    const float mid = value("fxMid");
    const float high = value("fxHigh");
    const float eq = value("fxEq");
    float lowLeft = 0.0f, lowRight = 0.0f;
    const float coeff = std::exp(-juce::MathConstants<float>::twoPi * 220.0f / (float)sampleRate);

    for (int i = 0; i < samples; ++i) {
        float left = buffer.getSample(0, i);
        float right = buffer.getSample(1, i);
        lowLeft = (1.0f - coeff) * left + coeff * lowLeft;
        lowRight = (1.0f - coeff) * right + coeff * lowRight;
        const float highLeft = left - lowLeft;
        const float highRight = right - lowRight;
        left += eq * (lowLeft * (std::pow(10.0f, low / 20.0f) - 1.0f)
                    + highLeft * (std::pow(10.0f, high / 20.0f) - 1.0f));
        right += eq * (lowRight * (std::pow(10.0f, low / 20.0f) - 1.0f)
                     + highRight * (std::pow(10.0f, high / 20.0f) - 1.0f));
        const float midGain = std::pow(10.0f, mid / 20.0f);
        left += (left - (lowLeft + highLeft)) * (midGain - 1.0f) * eq;
        right += (right - (lowRight + highRight)) * (midGain - 1.0f) * eq;

        const int delaySize = (int)delayBuffer[0].size();
        const float lfo = 0.5f + 0.5f * std::sin(fxLfoPhase);
        const float baseDelay = (float)sampleRate * (0.12f + 0.18f * lfo * value("fxFlange"));
        const float delayMs = (float)sampleRate * (0.22f + 0.18f * value("fxDelay"));
        const int tap = juce::jlimit(1, delaySize - 1, (int)(baseDelay + delayMs));
        const int read = (delayWrite - tap + delaySize) % delaySize;
        const float delayedLeft = delayBuffer[0][(size_t)read];
        const float delayedRight = delayBuffer[1][(size_t)read];
        const float chorus = value("fxChorus");
        const float flange = value("fxFlange");
        const float delay = value("fxDelay");
        delayBuffer[0][(size_t)delayWrite] = left + delayedLeft * (delay * 0.35f + chorus * 0.16f + flange * 0.45f);
        delayBuffer[1][(size_t)delayWrite] = right + delayedRight * (delay * 0.35f + chorus * 0.16f + flange * 0.45f);
        left += delayedLeft * (delay * 0.28f + chorus * 0.18f + flange * 0.45f);
        right += delayedRight * (delay * 0.28f + chorus * 0.18f + flange * 0.45f);
        buffer.setSample(0, i, left);
        buffer.setSample(1, i, right);
        delayWrite = (delayWrite + 1) % delaySize;
        fxLfoPhase += juce::MathConstants<float>::twoPi * 0.23f / (float)sampleRate;
        if (fxLfoPhase > juce::MathConstants<float>::twoPi)
            fxLfoPhase -= juce::MathConstants<float>::twoPi;
    }

    juce::dsp::Reverb::Parameters reverbParameters;
    reverbParameters.roomSize = 0.55f;
    reverbParameters.damping = 0.48f;
    reverbParameters.wetLevel = value("fxReverb") * 0.35f;
    reverbParameters.dryLevel = 1.0f;
    reverbParameters.width = 1.0f;
    reverb.setParameters(reverbParameters);
    juce::dsp::AudioBlock<float> reverbBlock(buffer);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(reverbBlock));

    buffer.applyGain(value("master"));
    const float width = value("width");
    for (int i = 0; i < samples; ++i) {
        const float midSample = (buffer.getSample(0, i) + buffer.getSample(1, i)) * 0.5f;
        const float sideSample = (buffer.getSample(0, i) - buffer.getSample(1, i)) * 0.5f * width * 2.0f;
        buffer.setSample(0, i, midSample + sideSample);
        buffer.setSample(1, i, midSample - sideSample);
    }
}

void AuroraProcessor::getStateInformation(juce::MemoryBlock& destination) {
    auto xml = params.copyState().createXml();
    copyXmlToBinary(*xml, destination);
}

void AuroraProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(params.state.getType()))
        params.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* AuroraProcessor::createEditor() {
    return new AuroraEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AuroraProcessor();
}
