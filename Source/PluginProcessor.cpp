#include "PluginProcessor.h"
#include "PluginEditor.h"

class AuroraSound final : public juce::SynthesiserSound {
public:
 bool appliesToNote(int) override { return true; }
 bool appliesToChannel(int) override { return true; }
};
class AuroraProcessor::Voice final : public juce::SynthesiserVoice {
public:
 bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<juce::SynthesiserSound*>(s)!=nullptr; }
 void startNote(int note,float velocity,juce::SynthesiserSound*,int) override {
  hz=juce::MidiMessage::getMidiNoteInHertz(note); vel=velocity; phase1=phase2=0; env=0; stage=1;
 }
 void stopNote(float,bool tail) override { if(tail) stage=4; else {clearCurrentNote(); env=0;} }
 void pitchWheelMoved(int) override {} void controllerMoved(int,int) override {}
 void renderNextBlock(juce::AudioBuffer<float>& b,int start,int count) override {
  const auto sr=getSampleRate(); if(sr<=0) return;
  auto* p1=processor->params.getRawParameterValue("wave1");
  auto* p2=processor->params.getRawParameterValue("wave2");
  const int w1=(int)p1->load(),w2=(int)p2->load();
  const float mix=processor->params.getRawParameterValue("mix")->load();
  const float det=processor->params.getRawParameterValue("detune")->load();
  const float attack=processor->params.getRawParameterValue("attack")->load();
  const float decay=processor->params.getRawParameterValue("decay")->load();
  const float sustain=processor->params.getRawParameterValue("sustain")->load();
  const float release=processor->params.getRawParameterValue("release")->load();
  const float sub=processor->params.getRawParameterValue("sub")->load();
  for(int i=0;i<count;++i){
   if(stage==1){env+=1.0f/(float)(sr*attack); if(env>=1){env=1;stage=2;}}
   else if(stage==2){env-= (1.0f-sustain)/(float)(sr*decay); if(env<=sustain){env=sustain;stage=3;}}
   else if(stage==4){env-=1.0f/(float)(sr*release); if(env<=0){env=0;clearCurrentNote();break;}}
   auto s1=wave(phase1,w1),s2=wave(phase2,w2);
   const float sample=((1-mix)*s1+mix*s2+sub*std::sin((float)phase1*0.5f))*vel*env*0.22f;
   phase1+=juce::MathConstants<double>::twoPi*hz/sr;
   phase2+=juce::MathConstants<double>::twoPi*hz*std::pow(2.0,det/1200.0)/sr;
   phase1=std::fmod(phase1,juce::MathConstants<double>::twoPi); phase2=std::fmod(phase2,juce::MathConstants<double>::twoPi);
   for(int ch=0;ch<b.getNumChannels();++ch)b.addSample(ch,start+i,sample);
  }
 }
 void setProcessor(AuroraProcessor* p){processor=p;}
private:
 float wave(double p,int type){
  const float x=(float)(p/juce::MathConstants<double>::twoPi);
  switch(type){case 1:return x<0.5f?1.f:-1.f;case 2:return 1.f-4.f*std::abs(x-.5f);case 3:return std::sin((float)p);default:return 2.f*x-1.f;}
 }
 AuroraProcessor* processor=nullptr; double hz=440,phase1=0,phase2=0; float vel=0,env=0; int stage=0;
};
AuroraProcessor::AuroraProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMS",createLayout()){
 for(int i=0;i<16;++i){auto* v=new Voice();v->setProcessor(this);synth.addVoice(v);}
 synth.addSound(new AuroraSound());
}
AuroraProcessor::APVTS::ParameterLayout AuroraProcessor::createLayout(){
 APVTS::ParameterLayout p;
 p.add(std::make_unique<juce::AudioParameterChoice>("wave1","OSC 1 Wave",juce::StringArray{"Saw","Square","Triangle","Sine"},0));
 p.add(std::make_unique<juce::AudioParameterChoice>("wave2","OSC 2 Wave",juce::StringArray{"Saw","Square","Triangle","Sine"},2));
 p.add(std::make_unique<juce::AudioParameterFloat>("mix","OSC Mix",juce::NormalisableRange<float>(0,1,.001f),.35f));
 p.add(std::make_unique<juce::AudioParameterFloat>("detune","Detune",juce::NormalisableRange<float>(-50,50,.1f),7.f));
 p.add(std::make_unique<juce::AudioParameterFloat>("sub","Sub Level",juce::NormalisableRange<float>(0,1,.001f),.2f));
 p.add(std::make_unique<juce::AudioParameterFloat>("cutoff","Cutoff",juce::NormalisableRange<float>(40,18000,1,.35f),12000.f));
 p.add(std::make_unique<juce::AudioParameterFloat>("res","Resonance",juce::NormalisableRange<float>(.1f,1.f,.001f),.2f));
 p.add(std::make_unique<juce::AudioParameterFloat>("attack","Attack",juce::NormalisableRange<float>(.001f,4.f,.001f,.4f),.01f));
 p.add(std::make_unique<juce::AudioParameterFloat>("decay","Decay",juce::NormalisableRange<float>(.01f,4.f,.001f,.4f),.2f));
 p.add(std::make_unique<juce::AudioParameterFloat>("sustain","Sustain",juce::NormalisableRange<float>(0,1,.001f),.75f));
 p.add(std::make_unique<juce::AudioParameterFloat>("release","Release",juce::NormalisableRange<float>(.01f,5.f,.001f,.4f),.3f));
 p.add(std::make_unique<juce::AudioParameterFloat>("master","Master",juce::NormalisableRange<float>(0.f,1.f,.001f),.8f));
 p.add(std::make_unique<juce::AudioParameterFloat>("drive","Drive",juce::NormalisableRange<float>(0.f,1.f,.001f),0.f));
 p.add(std::make_unique<juce::AudioParameterFloat>("width","Stereo Width",juce::NormalisableRange<float>(0.f,1.f,.001f),.5f));
 return p;
}
void AuroraProcessor::prepareToPlay(double sr,int){synth.setCurrentPlaybackSampleRate(sr);filter.prepare({sr,512,2});filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);}
bool AuroraProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void AuroraProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& m){
 juce::ScopedNoDenormals no; b.clear();synth.renderNextBlock(b,m,0,b.getNumSamples());
 filter.setCutoffFrequency(params.getRawParameterValue("cutoff")->load());filter.setResonance(params.getRawParameterValue("res")->load());
 const float drive = params.getRawParameterValue("drive")->load();
 if (drive > 0.001f) {
  const float gain = 1.0f + drive * 12.0f;
  for (int ch=0; ch<b.getNumChannels(); ++ch)
   for (int i=0; i<b.getNumSamples(); ++i)
    b.setSample(ch,i,std::tanh(b.getSample(ch,i)*gain)/std::tanh(gain));
 }
 juce::dsp::AudioBlock<float> block(b);filter.process(juce::dsp::ProcessContextReplacing<float>(block));
 const float master = params.getRawParameterValue("master")->load();
 b.applyGain(master);
 if (b.getNumChannels() > 1) {
  const float width = params.getRawParameterValue("width")->load();
  for (int i=0;i<b.getNumSamples();++i) {
   const float mid=(b.getSample(0,i)+b.getSample(1,i))*0.5f;
   const float side=(b.getSample(0,i)-b.getSample(1,i))*0.5f*width*2.0f;
   b.setSample(0,i,mid+side); b.setSample(1,i,mid-side);
  }
 }
}
void AuroraProcessor::getStateInformation(juce::MemoryBlock& d){auto x=params.copyState().createXml();copyXmlToBinary(*x,d);}
void AuroraProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(params.state.getType()))params.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* AuroraProcessor::createEditor(){return new AuroraEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AuroraProcessor();}