#include "PluginProcessor.h"
#include <cstring>
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>
#include <iterator>

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
        activeFilterType = 0;
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
        const int oscillatorMode = juce::jlimit(0, 3, (int)value("oscMode"));
        const float fmAmount = value("fmAmount");
        const float syncRatio = value("syncRatio");
        const int filterType = juce::jlimit(0, 2, (int)value("filterType"));
        const auto importedTable = processor->getImportedWavetable();

        if (filterType != activeFilterType) {
            const auto type = filterType == 1
                ? juce::dsp::StateVariableTPTFilterType::bandpass
                : (filterType == 2 ? juce::dsp::StateVariableTPTFilterType::highpass
                                   : juce::dsp::StateVariableTPTFilterType::lowpass);
            voiceFilter.setType(type);
            activeFilterType = filterType;
        }
        const float baseMix = value("mix");
        const float baseWTPosition = value("wtPosition");
        const float baseCutoff = value("cutoff");
        const float baseResonance = value("res");
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
        std::array<float, 4> routePeaks{};

        for (int i = 0; i < count; ++i) {
            const int blockIndex = start + i;
            const float lfo1 = blockIndex < (int)processor->lfo1Block.size()
                ? processor->lfo1Block[(size_t)blockIndex] : 0.0f;
            const float lfo2 = blockIndex < (int)processor->lfo2Block.size()
                ? processor->lfo2Block[(size_t)blockIndex] : 0.0f;
            const float lfo3 = blockIndex < (int)processor->lfo3Block.size()
                ? processor->lfo3Block[(size_t)blockIndex] : 0.0f;
            const float pitchBend = blockIndex < (int)processor->pitchBendBlock.size()
                ? processor->pitchBendBlock[(size_t)blockIndex] : 0.0f;
            const float aftertouch = blockIndex < (int)processor->aftertouchBlock.size()
                ? processor->aftertouchBlock[(size_t)blockIndex] : 0.0f;

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

            const float sources[10] = {
                0.0f, lfo1, lfo2, vel, env, filterEnv, modWheel, lfo3, pitchBend, aftertouch
            };
            float cutoffOctaves = filterEnv * filtEnvAmount * 4.0f;
            float mixMod = 0.0f;
            float positionMod = 0.0f;
            float pitchCents = 0.0f;
            float panMod = 0.0f;
            float resonanceMod = 0.0f;
            float volumeMod = 0.0f;
            float fxChorusMod = 0.0f, fxDelayMod = 0.0f, fxReverbMod = 0.0f;
            bool hasFxModulation = false;

            for (int route = 0; route < 4; ++route) {
                const int source = sourceIds[route];
                const int target = targetIds[route];
                if (source <= 0 || source >= 10 || target <= 0 || target >= 11)
                    continue;

                const float mod = sources[source] * routeAmounts[route];
                routePeaks[(size_t)route] = juce::jmax(routePeaks[(size_t)route], std::abs(mod));
                switch (target) {
                    case 1: cutoffOctaves += mod * 4.0f; break;
                    case 2: mixMod += mod; break;
                    case 3: positionMod += mod; break;
                    case 4: pitchCents += mod * 1200.0f; break;
                    case 5: panMod += mod; break;
                    case 6: resonanceMod += mod * 0.45f; break;
                    case 7: volumeMod += mod; break;
                    case 8: fxChorusMod += mod; hasFxModulation = true; break;
                    case 9: fxDelayMod += mod; hasFxModulation = true; break;
                    case 10: fxReverbMod += mod; hasFxModulation = true; break;
                    default: break;
                }
            }

            const float currentMix = juce::jlimit(0.0f, 1.0f, baseMix + mixMod);
            const float position = juce::jlimit(0.0f, 1.0f, baseWTPosition + positionMod);
            const float maxCutoff = juce::jmin(19000.0f, (float)sr * 0.45f);
            const float cutoffHz = juce::jlimit(25.0f, maxCutoff,
                baseCutoff * std::pow(2.0f, juce::jlimit(-7.0f, 7.0f, cutoffOctaves)));
            const float resonance = juce::jlimit(0.1f, 1.0f,
                baseResonance + resonanceMod);

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
                const float oscA = wave(phaseA, wave1, table, position, importedTable.get());
                const float oscB = wave(phaseB, wave2, table, position, importedTable.get());
                float sample = (1.0f - currentMix) * oscA + currentMix * oscB;

                if (oscillatorMode == 1) {
                    // Phase modulation FM: oscillator 2 modulates oscillator 1's phase.
                    const float fmCarrier = wave(phaseA + oscB * fmAmount, wave1, table,
                                                 position, importedTable.get());
                    sample = (1.0f - currentMix) * fmCarrier + currentMix * oscB;
                } else if (oscillatorMode == 2) {
                    // Bipolar ring modulation creates sum and difference sidebands.
                    sample = oscA * oscB * 2.0f;
                }

                sample += sub * std::sin((float)phase1[(size_t)voice] * 0.5f);
                const float pan = juce::jlimit(-0.95f, 0.95f,
                    normalized * unisonSpread * 0.85f + panMod * 0.85f);

                left += sample * std::sqrt(0.5f * (1.0f - pan)) * vel * env;
                right += sample * std::sqrt(0.5f * (1.0f + pan)) * vel * env;

                const double incrementA = juce::MathConstants<double>::twoPi * hz * ratio1 / sr;
                const double slaveRatio = oscillatorMode == 3 ? syncRatio : 1.0f;
                const double incrementB = juce::MathConstants<double>::twoPi * hz * ratio2 * slaveRatio / sr;
                const double nextPhaseA = phase1[(size_t)voice] + incrementA;
                const bool masterWrapped = nextPhaseA >= juce::MathConstants<double>::twoPi;
                phase1[(size_t)voice] = std::fmod(nextPhaseA, juce::MathConstants<double>::twoPi);
                if (oscillatorMode == 3 && masterWrapped)
                    phase2[(size_t)voice] = 0.0;
                else
                    phase2[(size_t)voice] += incrementB;
                phase2[(size_t)voice] = std::fmod(phase2[(size_t)voice], juce::MathConstants<double>::twoPi);
            }

            const float gain = 0.22f / std::sqrt((float)unisonVoices);
            const float modulationGain = juce::jlimit(0.0f, 2.0f, 1.0f + volumeMod);
            left *= gain * modulationGain;
            right *= gain * modulationGain;

            if (hasFxModulation && blockIndex < (int)processor->fxModWeightBlock.size()) {
                const float weight = juce::jmax(0.001f, env);
                processor->fxChorusModBlock[(size_t)blockIndex] += fxChorusMod * weight;
                processor->fxDelayModBlock[(size_t)blockIndex] += fxDelayMod * weight;
                processor->fxReverbModBlock[(size_t)blockIndex] += fxReverbMod * weight;
                processor->fxModWeightBlock[(size_t)blockIndex] += weight;
            }

            // Recalculate filter coefficients every four samples to reduce per-voice DSP cost.
            if ((i & 3) == 0) {
                voiceFilter.setCutoffFrequency(cutoffHz);
                voiceFilter.setResonance(resonance);
            }
            left = voiceFilter.processSample(0, left);
            right = voiceFilter.processSample(1, right);

            buffer.addSample(0, blockIndex, left);
            if (buffer.getNumChannels() > 1)
                buffer.addSample(1, blockIndex, right);
        }

        for (int route = 0; route < 4; ++route) {
            auto& meter = processor->modRouteActivity[(size_t)route];
            float previous = meter.load(std::memory_order_relaxed);
            while (routePeaks[(size_t)route] > previous
                   && !meter.compare_exchange_weak(previous, routePeaks[(size_t)route],
                                                   std::memory_order_relaxed)) {}
        }
    }

    void setProcessor(AuroraProcessor* p) { processor = p; }

private:
    float wave(float phase, int type, int table, float position,
               const std::vector<float>* importedTable) {
        // Oscillator phases are stored in radians; convert to a normalized cycle.
        const float phaseCycles = phase / juce::MathConstants<float>::twoPi;
        const float x = phaseCycles - std::floor(phaseCycles);
        if (table == 6 && importedTable != nullptr && importedTable->size() >= 64u * 2048u) {
            constexpr int frameSize = 2048;
            constexpr int frameCount = 64;
            const float framePosition = juce::jlimit(0.0f, 1.0f, position) * (frameCount - 1);
            const int frameA = (int)framePosition;
            const int frameB = juce::jmin(frameCount - 1, frameA + 1);
            const float frameBlend = framePosition - (float)frameA;
            const float samplePosition = x * (float)frameSize;
            const int sampleA = ((int)samplePosition) % frameSize;
            const int sampleB = (sampleA + 1) % frameSize;
            const float sampleBlend = samplePosition - std::floor(samplePosition);
            const auto at = [importedTable, frameSize](int frame, int index) {
                return (*importedTable)[(size_t)(frame * frameSize + index)];
            };
            const float a = juce::jmap(sampleBlend, at(frameA, sampleA), at(frameA, sampleB));
            const float b = juce::jmap(sampleBlend, at(frameB, sampleA), at(frameB, sampleB));
            return juce::jmap(frameBlend, a, b);
        }
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
    int activeFilterType = 0;
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
    choice("filterType", "Filter Type", { "Low Pass", "Band Pass", "High Pass" }, 0);
    choice("oscMode", "Oscillator Mode", { "Classic", "FM", "Ring Mod", "Hard Sync" }, 0);
    f("fmAmount", "FM Amount", 0.0f, 12.0f, 2.5f);
    f("syncRatio", "Hard Sync Ratio", 1.0f, 8.0f, 2.0f);

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
    f("lfo3Rate", "LFO 3 Rate", 0.02f, 20.0f, 0.50f, 0.35f);
    choice("lfo1Shape", "LFO 1 Shape", { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" }, 0);
    choice("lfo2Shape", "LFO 2 Shape", { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" }, 1);
    choice("lfo3Shape", "LFO 3 Shape", { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" }, 0);

    // Keep the first seven source indices stable for projects saved before 0.7.0.
    const juce::StringArray sources = { "Off", "LFO 1", "LFO 2", "Velocity", "Amp Env",
        "Filter Env", "Mod Wheel", "LFO 3", "Pitch Bend", "Aftertouch" };
    const juce::StringArray targets = { "Off", "Cutoff", "Osc Mix", "WT Position", "Pitch",
        "Pan", "Resonance", "Volume", "FX Chorus", "FX Delay", "FX Reverb" };
    for (int route = 1; route <= 4; ++route) {
        const auto suffix = juce::String(route);
        choice(("mod" + suffix + "Source").toRawUTF8(), ("Mod " + suffix + " Source").toRawUTF8(), sources, 0);
        choice(("mod" + suffix + "Target").toRawUTF8(), ("Mod " + suffix + " Target").toRawUTF8(), targets, 0);
        f(("mod" + suffix + "Amount").toRawUTF8(), ("Mod " + suffix + " Amount").toRawUTF8(), -1.0f, 1.0f, 0.0f);
    }

    choice("seqMode", "Sequencer Mode", { "Off", "Arpeggiator", "Step Sequencer" }, 0);
    choice("arpStyle", "Arpeggiator Style", { "Up", "Down", "Up / Down", "Random" }, 0);
    choice("seqRate", "Sequencer Rate", { "1/4", "1/8", "1/16", "1/32", "1/8T" }, 2);
    layout.add(std::make_unique<juce::AudioParameterInt>("seqOctaves", "Arpeggiator Octaves", 1, 4, 1));
    f("seqGate", "Sequencer Gate", 10.0f, 100.0f, 82.0f);
    f("seqSwing", "Sequencer Swing", 0.0f, 60.0f, 0.0f);
    choice("seqDirection", "Sequencer Direction", { "Forward", "Reverse", "Ping-Pong", "Random" }, 0);
    choice("seqLength", "Pattern Length", { "4", "8", "16", "32" }, 2);
    choice("seqPage", "Pattern Page", { "Steps 1-16", "Steps 17-32" }, 0);
    choice("seqPatternSlot", "User Pattern Slot", { "01", "02", "03", "04", "05", "06", "07", "08" }, 0);
    const int defaultStepPitches[8] = { 0, 7, 12, 7, 0, 7, 12, 7 };
    for (int step = 0; step < 32; ++step) {
        const auto suffix = juce::String(step);
        const auto stepName = "Step " + juce::String(step + 1) + " ";
        layout.add(std::make_unique<juce::AudioParameterBool>(
            ("stepOn" + suffix).toRawUTF8(), (stepName + "On").toRawUTF8(), true));
        layout.add(std::make_unique<juce::AudioParameterInt>(
            ("stepPitch" + suffix).toRawUTF8(), (stepName + "Pitch").toRawUTF8(),
            -12, 12, defaultStepPitches[step % 8]));
        f(("stepVelocity" + suffix).toRawUTF8(), (stepName + "Velocity").toRawUTF8(), 1.0f, 127.0f, 127.0f);
        f(("stepGate" + suffix).toRawUTF8(), (stepName + "Gate").toRawUTF8(), 10.0f, 100.0f, 100.0f);
        layout.add(std::make_unique<juce::AudioParameterBool>(
            ("stepAccent" + suffix).toRawUTF8(), (stepName + "Accent").toRawUTF8(), false));
    }

    f("master", "Master", 0.0f, 1.0f, 0.8f);
    f("drive", "Drive", 0.0f, 1.0f, 0.0f);
    f("width", "Stereo Width", 0.0f, 1.0f, 0.5f);
    choice("wtTable", "Wavetable", { "Basic", "Harmonics", "Triangle Morph", "Digital", "Octave", "Fold", "Imported WAV" }, 0);
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

void AuroraProcessor::prepareToPlay(double sr, int maximumBlockSize) {
    sampleRate = sr;
    synth.setCurrentPlaybackSampleRate(sr);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<Voice*>(synth.getVoice(i)))
            voice->prepare(sr);

    const auto storageSize = (size_t)juce::jmax(8192, maximumBlockSize + 1);
    lfo1Block.resize(storageSize);
    lfo2Block.resize(storageSize);
    lfo3Block.resize(storageSize);
    pitchBendBlock.resize(storageSize);
    aftertouchBlock.resize(storageSize);
    fxChorusModBlock.resize(storageSize);
    fxDelayModBlock.resize(storageSize);
    fxReverbModBlock.resize(storageSize);
    fxModWeightBlock.resize(storageSize);
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
    if ((int)lfo3Block.size() < samples) lfo3Block.resize((size_t)samples);
    if ((int)pitchBendBlock.size() < samples) pitchBendBlock.resize((size_t)samples);
    if ((int)aftertouchBlock.size() < samples) aftertouchBlock.resize((size_t)samples);
    if ((int)fxChorusModBlock.size() < samples) fxChorusModBlock.resize((size_t)samples);
    if ((int)fxDelayModBlock.size() < samples) fxDelayModBlock.resize((size_t)samples);
    if ((int)fxReverbModBlock.size() < samples) fxReverbModBlock.resize((size_t)samples);
    if ((int)fxModWeightBlock.size() < samples) fxModWeightBlock.resize((size_t)samples);

    const float rate1 = value("lfo1Rate");
    const float rate2 = value("lfo2Rate");
    const float rate3 = value("lfo3Rate");
    const int shape1 = (int)value("lfo1Shape");
    const int shape2 = (int)value("lfo2Shape");
    const int shape3 = (int)value("lfo3Shape");
    const double phaseStep1 = rate1 / sampleRate;
    const double phaseStep2 = rate2 / sampleRate;
    const double phaseStep3 = rate3 / sampleRate;

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
        lfo3Block[(size_t)i] = lfoShape(shape3, (float)lfo3Phase, lfo3Hold);
        lfo1Phase += phaseStep1;
        lfo2Phase += phaseStep2;
        lfo3Phase += phaseStep3;
        if (lfo1Phase >= 1.0) {
            lfo1Phase -= std::floor(lfo1Phase);
            lfo1Hold = nextRandom();
        }
        if (lfo2Phase >= 1.0) {
            lfo2Phase -= std::floor(lfo2Phase);
            lfo2Hold = nextRandom();
        }
        if (lfo3Phase >= 1.0) {
            lfo3Phase -= std::floor(lfo3Phase);
            lfo3Hold = nextRandom();
        }
    }

    // Build sample-accurate expression lanes from incoming MIDI.
    std::fill(fxChorusModBlock.begin(), fxChorusModBlock.begin() + samples, 0.0f);
    std::fill(fxDelayModBlock.begin(), fxDelayModBlock.begin() + samples, 0.0f);
    std::fill(fxReverbModBlock.begin(), fxReverbModBlock.begin() + samples, 0.0f);
    std::fill(fxModWeightBlock.begin(), fxModWeightBlock.begin() + samples, 0.0f);
    for (auto& meter : modRouteActivity)
        meter.store(0.0f, std::memory_order_relaxed);

    std::fill(pitchBendBlock.begin(), pitchBendBlock.begin() + samples, pitchBendValue);
    std::fill(aftertouchBlock.begin(), aftertouchBlock.begin() + samples, aftertouchValue);
    for (const auto metadata : midi) {
        const auto message = metadata.getMessage();
        const int position = juce::jlimit(0, juce::jmax(0, samples - 1), metadata.samplePosition);
        if (message.isPitchWheel())
            pitchBendValue = ((float)message.getPitchWheelValue() - 8192.0f) / 8192.0f;
        else if (message.isChannelPressure())
            aftertouchValue = (float)message.getChannelPressureValue() / 127.0f;
        else if (message.isAftertouch())
            aftertouchValue = (float)message.getAfterTouchValue() / 127.0f;
        else
            continue;

        for (int sample = position; sample < samples; ++sample) {
            pitchBendBlock[(size_t)sample] = pitchBendValue;
            aftertouchBlock[(size_t)sample] = aftertouchValue;
        }
    }

    const int sequencerMode = (int)value("seqMode");
    if (sequencerMode == 0) {
        if (sequencerWasRunning && sequenceNoteActive && sequenceCurrentNote >= 0)
            midi.addEvent(juce::MidiMessage::noteOff(1, sequenceCurrentNote), 0);
        sequencerWasRunning = false;
        sequenceNoteActive = false;
        sequenceCurrentNote = -1;
        sequenceCurrentStep.store(-1, std::memory_order_relaxed);
        heldNoteVelocities.fill(0.0f);
        synth.renderNextBlock(buffer, midi, 0, samples);
    } else {
        sequencerWasRunning = true;
        if (sequencerMode != 2)
            sequenceCurrentStep.store(-1, std::memory_order_relaxed);
        juce::MidiBuffer sequencedMidi;
        auto inputIterator = midi.begin();
        const auto inputEnd = midi.end();
        juce::MidiMessage inputMessage;
        int nextInputPosition = 0;
        auto loadNextInput = [&]() {
            if (inputIterator == inputEnd)
                return false;
            const auto metadata = *inputIterator;
            inputMessage = metadata.getMessage();
            nextInputPosition = metadata.samplePosition;
            ++inputIterator;
            return true;
        };
        bool hasInput = loadNextInput();

        double bpm = 120.0;
        if (auto* hostPlayHead = getPlayHead()) {
            const auto position = hostPlayHead->getPosition();
            if (position.hasValue()) {
                const auto hostBpm = position->getBpm();
                if (hostBpm.hasValue() && *hostBpm > 1.0)
                    bpm = *hostBpm;
            }
        }

        const int rateIndex = juce::jlimit(0, 4, (int)value("seqRate"));
        const double beatMultipliers[5] = { 1.0, 0.5, 0.25, 0.125, 1.0 / 3.0 };
        const double baseStepSamples = (60.0 / bpm) * sampleRate * beatMultipliers[rateIndex];
        const float gate = juce::jlimit(0.10f, 1.0f, value("seqGate") * 0.01f);
        const float swing = juce::jlimit(0.0f, 0.60f, value("seqSwing") * 0.01f);
        const int octaveCount = juce::jlimit(1, 4, (int)value("seqOctaves"));
        const int arpStyle = juce::jlimit(0, 3, (int)value("arpStyle"));
        const int patternLengths[4] = { 4, 8, 16, 32 };
        const int patternLength = patternLengths[juce::jlimit(0, 3, (int)value("seqLength"))];
        const int seqDirection = juce::jlimit(0, 3, (int)value("seqDirection"));

        auto stopSequencedNote = [&](int samplePosition) {
            if (sequenceNoteActive && sequenceCurrentNote >= 0) {
                sequencedMidi.addEvent(juce::MidiMessage::noteOff(1, sequenceCurrentNote), samplePosition);
                sequenceNoteActive = false;
                sequenceCurrentNote = -1;
            }
        };

        for (int sample = 0; sample < samples; ++sample) {
            while (hasInput && nextInputPosition <= sample) {
                const bool hadHeldNotes = std::any_of(heldNoteVelocities.begin(), heldNoteVelocities.end(),
                                                       [](float velocity) { return velocity > 0.0f; });
                if (inputMessage.isNoteOn()) {
                    heldNoteVelocities[(size_t)inputMessage.getNoteNumber()] = inputMessage.getFloatVelocity();
                    if (!hadHeldNotes) {
                        sequenceStepIndex = 0;
                        sequenceSamplesUntilStep = 0.0;
                        arpDirection = 1;
                    }
                } else if (inputMessage.isNoteOff()) {
                    heldNoteVelocities[(size_t)inputMessage.getNoteNumber()] = 0.0f;
                } else if (inputMessage.isAllNotesOff() || inputMessage.isAllSoundOff()) {
                    heldNoteVelocities.fill(0.0f);
                } else {
                    sequencedMidi.addEvent(inputMessage, sample);
                }
                hasInput = loadNextInput();
            }

            const int heldCount = (int)std::count_if(heldNoteVelocities.begin(), heldNoteVelocities.end(),
                                                     [](float velocity) { return velocity > 0.0f; });
            if (heldCount == 0) {
                stopSequencedNote(sample);
                sequenceSamplesUntilStep = 0.0;
                sequenceCurrentStep.store(-1, std::memory_order_relaxed);
                continue;
            }

            if (sequenceNoteActive && sequenceSamplesUntilOff <= 0.0)
                stopSequencedNote(sample);

            if (sequenceSamplesUntilStep <= 0.0) {
                stopSequencedNote(sample);
                int note = -1;
                float velocity = 0.85f;

                if (sequencerMode == 1) {
                    std::array<int, 128> notes{};
                    std::array<float, 128> velocities{};
                    int count = 0;
                    for (int midiNote = 0; midiNote < 128; ++midiNote) {
                        const float noteVelocity = heldNoteVelocities[(size_t)midiNote];
                        if (noteVelocity > 0.0f) {
                            notes[(size_t)count] = midiNote;
                            velocities[(size_t)count] = noteVelocity;
                            ++count;
                        }
                    }
                    const int totalNotes = count * octaveCount;
                    int sequenceIndex = 0;
                    if (arpStyle == 3) {
                        randomSeed ^= randomSeed << 13;
                        randomSeed ^= randomSeed >> 17;
                        randomSeed ^= randomSeed << 5;
                        sequenceIndex = (int)(randomSeed % (uint32_t)totalNotes);
                    } else if (arpStyle == 1) {
                        sequenceIndex = totalNotes - 1 - (sequenceStepIndex % totalNotes);
                    } else if (arpStyle == 2) {
                        const int cycleLength = juce::jmax(1, totalNotes * 2 - 2);
                        const int cycle = sequenceStepIndex % cycleLength;
                        sequenceIndex = cycle < totalNotes ? cycle : cycleLength - cycle;
                    } else {
                        sequenceIndex = sequenceStepIndex % totalNotes;
                    }
                    const int octave = sequenceIndex / count;
                    const int noteIndex = sequenceIndex % count;
                    note = notes[(size_t)noteIndex] + octave * 12;
                    velocity = velocities[(size_t)noteIndex];
                } else {
                    const int root = (int)std::distance(heldNoteVelocities.begin(),
                        std::find_if(heldNoteVelocities.begin(), heldNoteVelocities.end(),
                                     [](float v) { return v > 0.0f; }));
                    int step = 0;
                    const int cyclePosition = sequenceStepIndex % patternLength;
                    if (seqDirection == 1) {
                        step = patternLength - 1 - cyclePosition;
                    } else if (seqDirection == 2) {
                        const int cycleLength = juce::jmax(1, patternLength * 2 - 2);
                        const int position = sequenceStepIndex % cycleLength;
                        step = position < patternLength ? position : cycleLength - position;
                    } else if (seqDirection == 3) {
                        randomSeed ^= randomSeed << 13;
                        randomSeed ^= randomSeed >> 17;
                        randomSeed ^= randomSeed << 5;
                        step = (int)(randomSeed % (uint32_t)patternLength);
                    } else {
                        step = cyclePosition;
                    }

                    sequenceCurrentStep.store(step, std::memory_order_relaxed);
                    const auto suffix = juce::String(step);
                    if (value(("stepOn" + suffix).toRawUTF8()) > 0.5f) {
                        const int pitch = (int)value(("stepPitch" + suffix).toRawUTF8());
                        const float stepVelocity = juce::jlimit(1.0f, 127.0f,
                            value(("stepVelocity" + suffix).toRawUTF8())) / 127.0f;
                        const float stepGate = juce::jlimit(10.0f, 100.0f,
                            value(("stepGate" + suffix).toRawUTF8())) * 0.01f;
                        const bool accent = value(("stepAccent" + suffix).toRawUTF8()) > 0.5f;
                        const float accentGain = accent ? 1.25f : 1.0f;
                        note = root + pitch;
                        velocity = heldNoteVelocities[(size_t)root] * stepVelocity * accentGain;
                        sequenceSamplesUntilOff = baseStepSamples * gate * stepGate;
                    }
                }

                if (note >= 0) {
                    note = juce::jlimit(0, 127, note);
                    velocity = juce::jlimit(0.01f, 1.0f, velocity);
                    sequencedMidi.addEvent(juce::MidiMessage::noteOn(1, note, velocity), sample);
                    sequenceCurrentNote = note;
                    sequenceNoteActive = true;
                    if (sequencerMode == 1)
                        sequenceSamplesUntilOff = baseStepSamples * gate;
                }

                const double swingMultiplier = (sequenceStepIndex & 1) == 0
                    ? (1.0 + swing * 0.5) : (1.0 - swing * 0.5);
                sequenceSamplesUntilStep = baseStepSamples * swingMultiplier;
                ++sequenceStepIndex;
            }

            sequenceSamplesUntilStep -= 1.0;
            if (sequenceNoteActive)
                sequenceSamplesUntilOff -= 1.0;
        }

        synth.renderNextBlock(buffer, sequencedMidi, 0, samples);
    }

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
        const float fxWeight = fxModWeightBlock[(size_t)i];
        const float chorusMod = fxWeight > 0.0001f
            ? fxChorusModBlock[(size_t)i] / fxWeight : 0.0f;
        const float delayMod = fxWeight > 0.0001f
            ? fxDelayModBlock[(size_t)i] / fxWeight : 0.0f;
        const float chorus = juce::jlimit(0.0f, 1.0f, value("fxChorus") + chorusMod * 0.5f);
        const float flange = value("fxFlange");
        const float delay = juce::jlimit(0.0f, 1.0f, value("fxDelay") + delayMod * 0.5f);
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
    float reverbModAverage = 0.0f;
    int reverbModSamples = 0;
    for (int i = 0; i < samples; ++i) {
        const float weight = fxModWeightBlock[(size_t)i];
        if (weight > 0.0001f) {
            reverbModAverage += fxReverbModBlock[(size_t)i] / weight;
            ++reverbModSamples;
        }
    }
    if (reverbModSamples > 0)
        reverbModAverage /= (float)reverbModSamples;
    const float modulatedReverb = juce::jlimit(0.0f, 1.0f,
        value("fxReverb") + reverbModAverage * 0.5f);
    reverbParameters.wetLevel = modulatedReverb * 0.35f;
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

void AuroraProcessor::saveUserPattern(int slot) {
    slot = juce::jlimit(0, 7, slot);
    auto& pattern = userPatterns[(size_t)slot];
    pattern.direction = (int)params.getRawParameterValue("seqDirection")->load();
    pattern.length = (int)params.getRawParameterValue("seqLength")->load();
    pattern.rate = (int)params.getRawParameterValue("seqRate")->load();
    pattern.globalGate = params.getRawParameterValue("seqGate")->load();
    pattern.swing = params.getRawParameterValue("seqSwing")->load();

    for (int step = 0; step < 32; ++step) {
        const auto suffix = juce::String(step);
        auto& data = pattern.steps[(size_t)step];
        data.enabled = params.getRawParameterValue(("stepOn" + suffix).toRawUTF8())->load() > 0.5f;
        data.pitch = (int)params.getRawParameterValue(("stepPitch" + suffix).toRawUTF8())->load();
        data.velocity = params.getRawParameterValue(("stepVelocity" + suffix).toRawUTF8())->load();
        data.gate = params.getRawParameterValue(("stepGate" + suffix).toRawUTF8())->load();
        data.accent = params.getRawParameterValue(("stepAccent" + suffix).toRawUTF8())->load() > 0.5f;
    }
    userPatternValid[(size_t)slot] = true;
}

bool AuroraProcessor::loadUserPattern(int slot) {
    slot = juce::jlimit(0, 7, slot);
    if (!userPatternValid[(size_t)slot])
        return false;

    auto setParameter = [this](const juce::String& id, float valueToSet) {
        if (auto* parameter = params.getParameter(id)) {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(valueToSet));
            parameter->endChangeGesture();
        }
    };

    const auto& pattern = userPatterns[(size_t)slot];
    setParameter("seqDirection", (float)pattern.direction);
    setParameter("seqLength", (float)pattern.length);
    setParameter("seqRate", (float)pattern.rate);
    setParameter("seqGate", pattern.globalGate);
    setParameter("seqSwing", pattern.swing);

    for (int step = 0; step < 32; ++step) {
        const auto suffix = juce::String(step);
        const auto& data = pattern.steps[(size_t)step];
        setParameter("stepOn" + suffix, data.enabled ? 1.0f : 0.0f);
        setParameter("stepPitch" + suffix, (float)data.pitch);
        setParameter("stepVelocity" + suffix, data.velocity);
        setParameter("stepGate" + suffix, data.gate);
        setParameter("stepAccent" + suffix, data.accent ? 1.0f : 0.0f);
    }
    return true;
}

bool AuroraProcessor::loadWavetableFromFile(const juce::File& file, juce::String& error) {
    if (!file.existsAsFile()) {
        error = "File not found";
        return false;
    }

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
    if (reader == nullptr || reader->lengthInSamples < 64 || reader->numChannels < 1) {
        error = "Unsupported or empty audio file";
        return false;
    }

    constexpr int frameCount = 64;
    constexpr int frameSize = 2048;
    const int sourceLength = (int)juce::jmin<int64_t>(4 * 1024 * 1024, reader->lengthInSamples);
    juce::AudioBuffer<float> source(1, sourceLength);
    source.clear();
    if (!reader->read(&source, 0, sourceLength, 0, true, false)) {
        error = "Could not decode audio data";
        return false;
    }

    std::vector<float> table((size_t)frameCount * (size_t)frameSize, 0.0f);
    for (int frame = 0; frame < frameCount; ++frame) {
        const int start = (int)((int64_t)frame * sourceLength / frameCount);
        const int end = (int)((int64_t)(frame + 1) * sourceLength / frameCount);
        const int length = juce::jmax(1, end - start);
        float peak = 0.0f;

        for (int i = 0; i < frameSize; ++i) {
            const float position = (float)i * (float)length / (float)frameSize;
            const int indexA = juce::jlimit(0, length - 1, (int)position);
            const int indexB = (indexA + 1) % length;
            const float blend = position - std::floor(position);
            const float a = source.getSample(0, start + indexA);
            const float b = source.getSample(0, start + indexB);
            const float sample = juce::jmap(blend, a, b);
            table[(size_t)frame * (size_t)frameSize + (size_t)i] = sample;
            peak = juce::jmax(peak, std::abs(sample));
        }

        if (peak > 0.00001f) {
            const float gain = 0.95f / peak;
            for (int i = 0; i < frameSize; ++i)
                table[(size_t)frame * (size_t)frameSize + (size_t)i] *= gain;
        }
    }

    auto imported = std::make_shared<const std::vector<float>>(std::move(table));
    std::atomic_store_explicit(&importedWavetable, imported, std::memory_order_release);
    importedWavetableName = file.getFileName();
    error.clear();
    return true;
}

void AuroraProcessor::clearImportedWavetable() {
    std::atomic_store_explicit(&importedWavetable,
                               std::shared_ptr<const std::vector<float>>{},
                               std::memory_order_release);
    importedWavetableName.clear();
}

void AuroraProcessor::editImportedWavetableRange(int frame, int firstSample, int lastSample,
                                                   float firstValue, float lastValue) {
    constexpr int frameCount = 64;
    constexpr int frameSize = 2048;
    frame = juce::jlimit(0, frameCount - 1, frame);
    firstSample = juce::jlimit(0, frameSize - 1, firstSample);
    lastSample = juce::jlimit(0, frameSize - 1, lastSample);
    firstValue = juce::jlimit(-1.0f, 1.0f, firstValue);
    lastValue = juce::jlimit(-1.0f, 1.0f, lastValue);

    auto current = getImportedWavetable();
    std::vector<float> edited;
    if (current && current->size() == (size_t)(frameCount * frameSize)) {
        edited = *current;
    } else {
        edited.resize((size_t)frameCount * (size_t)frameSize);
        for (int f = 0; f < frameCount; ++f)
            for (int i = 0; i < frameSize; ++i)
                edited[(size_t)f * (size_t)frameSize + (size_t)i] =
                    std::sin((float)i * juce::MathConstants<float>::twoPi / (float)frameSize);
    }

    int start = firstSample;
    int end = lastSample;
    float startValue = firstValue;
    float endValue = lastValue;
    if (start > end) {
        std::swap(start, end);
        std::swap(startValue, endValue);
    }

    const float denominator = (float)juce::jmax(1, end - start);
    for (int i = start; i <= end; ++i) {
        const float blend = (float)(i - start) / denominator;
        edited[(size_t)frame * (size_t)frameSize + (size_t)i] =
            juce::jmap(blend, startValue, endValue);
    }

    auto updated = std::make_shared<const std::vector<float>>(std::move(edited));
    std::atomic_store_explicit(&importedWavetable, updated, std::memory_order_release);
    if (importedWavetableName.isEmpty())
        importedWavetableName = "Drawn Wavetable";
}

float AuroraProcessor::getImportedWavetableSample(float phase, float position) const noexcept {
    const auto table = getImportedWavetable();
    if (!table || table->size() < 64u * 2048u)
        return std::sin(phase * juce::MathConstants<float>::twoPi);

    constexpr int frameSize = 2048;
    constexpr int frameCount = 64;
    const float cycle = phase - std::floor(phase);
    const float framePosition = juce::jlimit(0.0f, 1.0f, position) * (frameCount - 1);
    const int frameA = (int)framePosition;
    const int frameB = juce::jmin(frameCount - 1, frameA + 1);
    const float frameBlend = framePosition - (float)frameA;
    const float samplePosition = cycle * (float)frameSize;
    const int sampleA = ((int)samplePosition) % frameSize;
    const int sampleB = (sampleA + 1) % frameSize;
    const float sampleBlend = samplePosition - std::floor(samplePosition);
    const auto at = [&table, frameSize](int frame, int index) {
        return (*table)[(size_t)(frame * frameSize + index)];
    };
    const float a = juce::jmap(sampleBlend, at(frameA, sampleA), at(frameA, sampleB));
    const float b = juce::jmap(sampleBlend, at(frameB, sampleA), at(frameB, sampleB));
    return juce::jmap(frameBlend, a, b);
}

void AuroraProcessor::getStateInformation(juce::MemoryBlock& destination) {
    auto xml = params.copyState().createXml();
    auto* bank = xml->createNewChildElement("USER_PATTERNS");
    for (int slot = 0; slot < 8; ++slot) {
        if (!userPatternValid[(size_t)slot])
            continue;
        const auto& patternData = userPatterns[(size_t)slot];
        auto* pattern = bank->createNewChildElement("PATTERN");
        pattern->setAttribute("slot", slot);
        pattern->setAttribute("direction", patternData.direction);
        pattern->setAttribute("length", patternData.length);
        pattern->setAttribute("rate", patternData.rate);
        pattern->setAttribute("globalGate", (double)patternData.globalGate);
        pattern->setAttribute("swing", (double)patternData.swing);
        for (int step = 0; step < 32; ++step) {
            const auto& data = patternData.steps[(size_t)step];
            auto* element = pattern->createNewChildElement("STEP");
            element->setAttribute("index", step);
            element->setAttribute("on", data.enabled ? 1 : 0);
            element->setAttribute("pitch", data.pitch);
            element->setAttribute("velocity", (double)data.velocity);
            element->setAttribute("gate", (double)data.gate);
            element->setAttribute("accent", data.accent ? 1 : 0);
        }
    }
    if (const auto imported = getImportedWavetable(); imported && !imported->empty()) {
        auto* wavetable = xml->createNewChildElement("IMPORTED_WAVETABLE");
        wavetable->setAttribute("name", importedWavetableName);
        const auto encoded = juce::Base64::toBase64(imported->data(), imported->size() * sizeof(float));
        wavetable->addTextElement(encoded);
    }
    copyXmlToBinary(*xml, destination);
}

void AuroraProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(params.state.getType())) {
        userPatternValid.fill(false);
        clearImportedWavetable();
        if (auto* wavetable = xml->getChildByName("IMPORTED_WAVETABLE")) {
            juce::MemoryOutputStream decoded(0, true);
            if (juce::Base64::convertFromBase64(decoded, wavetable->getAllSubText())
                && decoded.getDataSize() == (size_t)(64 * 2048 * (int)sizeof(float))) {
                std::vector<float> importedSamples(64u * 2048u);
                std::memcpy(importedSamples.data(), decoded.getData(), decoded.getDataSize());
                auto imported = std::make_shared<const std::vector<float>>(std::move(importedSamples));
                std::atomic_store_explicit(&importedWavetable, imported, std::memory_order_release);
                importedWavetableName = wavetable->getStringAttribute("name", "Imported WAV");
            }
            xml->removeChildElement(wavetable, true);
        }
        if (auto* bank = xml->getChildByName("USER_PATTERNS")) {
            for (auto* pattern = bank->getFirstChildElement(); pattern != nullptr; pattern = pattern->getNextElement()) {
                if (!pattern->hasTagName("PATTERN"))
                    continue;
                const int slot = pattern->getIntAttribute("slot", -1);
                if (slot < 0 || slot >= 8)
                    continue;
                auto& targetPattern = userPatterns[(size_t)slot];
                targetPattern.direction = pattern->getIntAttribute("direction", 0);
                targetPattern.length = pattern->getIntAttribute("length", 2);
                targetPattern.rate = pattern->getIntAttribute("rate", 2);
                targetPattern.globalGate = (float)pattern->getDoubleAttribute("globalGate", 82.0);
                targetPattern.swing = (float)pattern->getDoubleAttribute("swing", 0.0);
                for (auto* element = pattern->getFirstChildElement(); element != nullptr; element = element->getNextElement()) {
                    if (!element->hasTagName("STEP"))
                        continue;
                    const int step = element->getIntAttribute("index", -1);
                    if (step < 0 || step >= 32)
                        continue;
                    auto& target = targetPattern.steps[(size_t)step];
                    target.enabled = element->getIntAttribute("on", 1) != 0;
                    target.pitch = element->getIntAttribute("pitch", 0);
                    target.velocity = (float)element->getDoubleAttribute("velocity", 127.0);
                    target.gate = (float)element->getDoubleAttribute("gate", 100.0);
                    target.accent = element->getIntAttribute("accent", 0) != 0;
                }
                userPatternValid[(size_t)slot] = true;
            }
            // Keep the custom pattern bank out of APVTS's parameter tree.
            xml->removeChildElement(bank, true);
        }
        params.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorEditor* AuroraProcessor::createEditor() {
    return new AuroraEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AuroraProcessor();
}
