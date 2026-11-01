#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

class AuroraSound final : public juce::SynthesiserSound {
public: bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;}
};
class AuroraProcessor::Voice final : public juce::SynthesiserVoice {
public:
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<AuroraSound*>(s)!=nullptr;}
 void startNote(int note,float velocity,juce::SynthesiserSound*,int) override{hz=juce::MidiMessage::getMidiNoteInHertz(note);vel=velocity;for(auto& ph:phase1)ph=0;for(auto& ph:phase2)ph=0;env=0;stage=1;}
 void stopNote(float,bool tail) override{if(tail)stage=4;else{clearCurrentNote();env=0;}}
 void pitchWheelMoved(int) override{} void controllerMoved(int,int) override{}
 void renderNextBlock(juce::AudioBuffer<float>& b,int start,int count) override{
  const double sr=getSampleRate();if(sr<=0||processor==nullptr)return;
  auto v=[this](const char* id){return processor->params.getRawParameterValue(id)->load();};
  const int w1=(int)v("wave1"),w2=(int)v("wave2"),table=(int)v("wtTable");
  const float mix=v("mix"),det=v("detune"),a=v("attack"),d=v("decay"),s=v("sustain"),r=v("release"),sub=v("sub");
  const int voices=juce::jlimit(1,7,(int)v("unison"));const float uniDet=v("uniDetune"),spread=v("uniSpread"),position=v("wtPosition");
  for(int i=0;i<count;++i){
   if(stage==1){env+=1.0f/(float)(sr*a);if(env>=1){env=1;stage=2;}}
   else if(stage==2){env-=(1.0f-s)/(float)(sr*d);if(env<=s){env=s;stage=3;}}
   else if(stage==4){env-=1.0f/(float)(sr*r);if(env<=0){env=0;clearCurrentNote();break;}}
   float left=0,right=0;
   for(int u=0;u<voices;++u){
    const float norm=voices==1?0.0f:((float)u/(voices-1)-.5f)*2.0f;
    const double ratio=std::pow(2.0,(det+norm*uniDet)/1200.0);
    const double ratio2=std::pow(2.0,(det+norm*uniDet+norm*4.0f)/1200.0);
    const float p1=(float)(phase1+norm*.13),p2=(float)(phase2+norm*.19);
    float x1=wave(p1,w1,table,position),x2=wave(p2,w2,table,position);
    const float sample=((1.0f-mix)*x1+mix*x2+sub*std::sin((float)phase1*.5f));
    const float pan=norm*spread*.85f;
    left+=sample*std::sqrt(.5f*(1.0f-pan))*vel*env;
    right+=sample*std::sqrt(.5f*(1.0f+pan))*vel*env;
    phase1[u]+=juce::MathConstants<double>::twoPi*hz*ratio/sr;
     phase2[u]+=juce::MathConstants<double>::twoPi*hz*ratio2/sr;
     phase1[u]=std::fmod(phase1[u],juce::MathConstants<double>::twoPi);
     phase2[u]=std::fmod(phase2[u],juce::MathConstants<double>::twoPi);
   }
   const float gain=.22f/std::sqrt((float)voices);
   b.addSample(0,start+i,left*gain);if(b.getNumChannels()>1)b.addSample(1,start+i,right*gain);
   
  }
 }
 void setProcessor(AuroraProcessor* p){processor=p;}
private:
 float wave(float phase,int type,int table,float pos){
  const float x=phase-std::floor(phase);
  auto basic=[&](int t){switch(t){case 1:return x<.5f?1.0f:-1.0f;case 2:return 1.0f-4.0f*std::abs(x-.5f);case 3:return std::sin(x*juce::MathConstants<float>::twoPi);default:return 2.0f*x-1.0f;}};
  const float sine=basic(3),saw=basic(0),square=basic(1),tri=basic(2);
  float a=saw,b=sine;
  switch(table){case 1:a=square;b=tri;break;case 2:a=tri;b=sine;break;case 3:a=saw;b=square;break;case 4:a=sine;b=std::sin(x*juce::MathConstants<float>::twoPi*2.0f);break;case 5:a=saw;b=std::tanh(saw*3.0f);break;default:a=basic(type);b=basic(type==0?3:0);break;}
  return juce::jmap(pos,a,b);
 }
 AuroraProcessor* processor=nullptr;double hz=440;std::array<double,7> phase1{},phase2{};float vel=0,env=0;int stage=0;
};
AuroraProcessor::AuroraProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMS",createLayout()){
 for(int i=0;i<16;++i){auto* v=new Voice();v->setProcessor(this);synth.addVoice(v);}synth.addSound(new AuroraSound());
}
AuroraProcessor::APVTS::ParameterLayout AuroraProcessor::createLayout(){
 APVTS::ParameterLayout p;
 auto choice=[&](const char* id,const char* name,juce::StringArray values,int def){p.add(std::make_unique<juce::AudioParameterChoice>(id,name,values,def));};
 auto f=[&](const char* id,const char* name,float lo,float hi,float def,float skew=1.0f){p.add(std::make_unique<juce::AudioParameterFloat>(id,name,juce::NormalisableRange<float>(lo,hi,.001f,skew),def));};
 choice("wave1","OSC 1 Wave",{"Saw","Square","Triangle","Sine"},0);choice("wave2","OSC 2 Wave",{"Saw","Square","Triangle","Sine"},2);
 f("mix","OSC Mix",0,1,.35f);f("detune","Detune",-50,50,7);f("sub","Sub Level",0,1,.2f);
 f("cutoff","Cutoff",40,18000,12000,.35f);f("res","Resonance",.1f,1,.2f);
 f("attack","Attack",.001f,4,.01f,.4f);f("decay","Decay",.01f,4,.2f,.4f);f("sustain","Sustain",0,1,.75f);f("release","Release",.01f,5,.3f,.4f);
 f("master","Master",0,1,.8f);f("drive","Drive",0,1,0);f("width","Stereo Width",0,1,.5f);
 choice("wtTable","Wavetable",{"Basic","Harmonics","Triangle Morph","Digital","Octave","Fold"},0);
 f("wtPosition","WT Position",0,1,0);
 f("unison","Unison Voices",1,7,1);f("uniDetune","Unison Detune",0,50,12);f("uniSpread","Unison Spread",0,1,.65f);
 f("fxEq","EQ Amount",0,1,0);f("fxDist","FX Distortion",0,1,0);f("fxChorus","Chorus",0,1,0);f("fxFlange","Flanger",0,1,0);
 f("fxDelay","Delay",0,1,0);f("fxReverb","Reverb",0,1,0);f("fxLow","EQ Low", -12,12,0);f("fxMid","EQ Mid",-12,12,0);f("fxHigh","EQ High",-12,12,0);
 return p;
}
void AuroraProcessor::prepareToPlay(double sr,int block){
 sampleRate=sr;synth.setCurrentPlaybackSampleRate(sr);filter.prepare({sr,(juce::uint32)juce::jmax(1,block),2});filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
 for(auto& ch:delayBuffer)ch.assign((size_t)juce::jmax(1,(int)sr*2),0.0f);delayWrite=0;
 juce::dsp::Reverb::Parameters rp;rp.roomSize=.45f;rp.damping=.55f;rp.wetLevel=.0f;rp.dryLevel=1.0f;reverb.setParameters(rp);reverb.reset();
}
bool AuroraProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void AuroraProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& m){
 juce::ScopedNoDenormals no;b.clear();synth.renderNextBlock(b,m,0,b.getNumSamples());
 auto v=[this](const char* id){return params.getRawParameterValue(id)->load();};
 filter.setCutoffFrequency(v("cutoff"));filter.setResonance(v("res"));
 juce::dsp::AudioBlock<float> block(b);filter.process(juce::dsp::ProcessContextReplacing<float>(block));
 const float drive=juce::jmax(v("drive"),v("fxDist"));if(drive>.001f){const float gain=1+drive*15;for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i)b.setSample(c,i,std::tanh(b.getSample(c,i)*gain)/std::tanh(gain));}
 const float low=v("fxLow"),mid=v("fxMid"),high=v("fxHigh"),eq=v("fxEq");
 float lpL=0,lpR=0,hpL=0,hpR=0;const float coeff=std::exp(-juce::MathConstants<float>::twoPi*220.0f/(float)sampleRate);
 for(int i=0;i<b.getNumSamples();++i){
  float l=b.getSample(0,i),r=b.getSample(1,i);
  lpL=(1-coeff)*l+coeff*lpL;lpR=(1-coeff)*r+coeff*lpR;
  const float loL=lpL,loR=lpR;const float hiL=l-loL,hiR=r-loR;
  const float eqGain=eq;
  l+=eqGain*(loL*(std::pow(10.0f,low/20.0f)-1.0f)+hiL*(std::pow(10.0f,high/20.0f)-1.0f));
  r+=eqGain*(loR*(std::pow(10.0f,low/20.0f)-1.0f)+hiR*(std::pow(10.0f,high/20.0f)-1.0f));
  const float midGain=std::pow(10.0f,mid/20.0f);l+=(l-(loL+hiL))*(midGain-1)*eqGain;r+=(r-(loR+hiR))*(midGain-1)*eqGain;
  const int size=(int)delayBuffer[0].size();const float lfo=.5f+.5f*std::sin(fxLfoPhase);
  const float baseDelay=(float)sampleRate*(.12f+.18f*lfo*v("fxFlange"));
  const float delayMs=(float)sampleRate*(.22f+.18f*v("fxDelay"));
  const int tap=juce::jlimit(1,size-1,(int)(baseDelay+delayMs));
  const int read=(delayWrite-tap+size)%size;
  const float dl=delayBuffer[0][(size_t)read],dr=delayBuffer[1][(size_t)read];
  const float chorus=v("fxChorus"),flange=v("fxFlange"),delay=v("fxDelay");
  delayBuffer[0][(size_t)delayWrite]=l+(dl*(delay*.35f+chorus*.16f+flange*.45f));
  delayBuffer[1][(size_t)delayWrite]=r+(dr*(delay*.35f+chorus*.16f+flange*.45f));
  l+=dl*(delay*.28f+chorus*.18f+flange*.45f);r+=dr*(delay*.28f+chorus*.18f+flange*.45f);
  b.setSample(0,i,l);b.setSample(1,i,r);delayWrite=(delayWrite+1)%size;
  fxLfoPhase+=juce::MathConstants<float>::twoPi*0.23f/(float)sampleRate;if(fxLfoPhase>juce::MathConstants<float>::twoPi)fxLfoPhase-=juce::MathConstants<float>::twoPi;
 }
 juce::dsp::Reverb::Parameters rp;rp.roomSize=.55f;rp.damping=.48f;rp.wetLevel=v("fxReverb")*.35f;rp.dryLevel=1.0f;rp.width=1.0f;reverb.setParameters(rp);
  juce::dsp::AudioBlock<float> reverbBlock(b);
  reverb.process(juce::dsp::ProcessContextReplacing<float>(reverbBlock));
 const float master=v("master");b.applyGain(master);
 const float width=v("width");for(int i=0;i<b.getNumSamples();++i){const float mm=(b.getSample(0,i)+b.getSample(1,i))*.5f,ss=(b.getSample(0,i)-b.getSample(1,i))*.5f*width*2;b.setSample(0,i,mm+ss);b.setSample(1,i,mm-ss);}
}
void AuroraProcessor::getStateInformation(juce::MemoryBlock& d){auto x=params.copyState().createXml();copyXmlToBinary(*x,d);}
void AuroraProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(params.state.getType()))params.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* AuroraProcessor::createEditor(){return new AuroraEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AuroraProcessor();}
