#include "PluginProcessor.h"
#include <cmath>
namespace {
constexpr std::array<float,6> attacks {0.1f,0.3f,1.f,3.f,10.f,30.f};
constexpr std::array<float,6> releases {0.1f,0.2f,0.4f,0.8f,1.6f,-1.f};
constexpr std::array<float,6> ratios {1.5f,2.f,3.f,4.f,5.f,10.f};
constexpr std::array<float,6> hpfs {0.f,30.f,60.f,105.f,125.f,185.f};
float coef(float seconds,double fs) {return std::exp(-1.f/(std::max(0.00001f,seconds)*static_cast<float>(fs)));}
}
juce::AudioProcessorValueTreeState::ParameterLayout GSeriesLabProcessor::makeParameters() {
 using namespace juce;
 std::vector<std::unique_ptr<RangedAudioParameter>> p;
 p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"threshold",1},"Threshold",NormalisableRange<float>(-20.f,20.f,0.1f),0.f));
 p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"makeup",1},"Makeup",NormalisableRange<float>(-12.f,20.f,0.1f),0.f));
 p.push_back(std::make_unique<AudioParameterChoice>(ParameterID{"attack",1},"Attack",StringArray{"0.1 ms","0.3 ms","1 ms","3 ms","10 ms","30 ms"},5));
 p.push_back(std::make_unique<AudioParameterChoice>(ParameterID{"release",1},"Release",StringArray{"0.1 s","0.2 s","0.4 s","0.8 s","1.6 s","Auto"},5));
 p.push_back(std::make_unique<AudioParameterChoice>(ParameterID{"ratio",1},"Ratio",StringArray{"1.5:1","2:1","3:1","4:1","5:1","10:1"},3));
 p.push_back(std::make_unique<AudioParameterChoice>(ParameterID{"hpf",1},"SC HPF",StringArray{"Off","30 Hz","60 Hz","105 Hz","125 Hz","185 Hz"},0));
 p.push_back(std::make_unique<AudioParameterBool>(ParameterID{"enabled",1},"Compressor In",true));
 p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{"drive",1},"Experimental Drive",NormalisableRange<float>(0.f,1.f,0.01f),0.f));
 return {p.begin(),p.end()};
}
GSeriesLabProcessor::GSeriesLabProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMS",makeParameters()) {}
void GSeriesLabProcessor::prepareToPlay(double sampleRate,int) {fs=sampleRate;envelope=slowEnvelope=0.f;hpX.fill(0.f);hpY.fill(0.f);gainReductionDb.store(0.f);}
void GSeriesLabProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
 juce::ScopedNoDenormals noDenormals;
 const int channels=std::min(2,b.getNumChannels());
 if(channels==0) return;
 auto get=[this](const char* id){return state.getRawParameterValue(id)->load();};
 const bool enabled=get("enabled")>=0.5f;
 const float threshold=get("threshold"),makeup=get("makeup"),drive=get("drive");
 const int a=juce::jlimit(0,5,(int)get("attack")),r=juce::jlimit(0,5,(int)get("release")),q=juce::jlimit(0,5,(int)get("ratio")),h=juce::jlimit(0,5,(int)get("hpf"));
 const float atk=coef(attacks[a]*0.001f,fs),rel=coef(r==5?0.1f:releases[r],fs),slow=coef(1.6f,fs);
 const float hp=hpfs[h]>0.f?std::exp(-2.f*juce::MathConstants<float>::pi*hpfs[h]/(float)fs):0.f;
 const float makeupLinear=juce::Decibels::decibelsToGain(makeup);
 float lastGR=0.f;
 for(int i=0;i<b.getNumSamples();++i) {
  float dominant=0.f;
  for(int c=0;c<channels;++c) {
   float x=b.getSample(c,i),filtered=x;
   if(h>0) { filtered=hp*(hpY[c]+x-hpX[c]); hpX[c]=x;hpY[c]=filtered; }
   dominant=std::max(dominant,std::abs(filtered));
  }
  if(enabled) {
   // Prototype detector: stereo dominant peak; not a measured SSL circuit emulation.
   const float tc=dominant>envelope?atk:rel;
   envelope=tc*envelope+(1.f-tc)*dominant;
   slowEnvelope=slow*slowEnvelope+(1.f-slow)*dominant;
   float level=juce::Decibels::gainToDecibels(std::max(envelope,1.e-9f));
   if(r==5) level=juce::Decibels::gainToDecibels(std::max(0.65f*envelope+0.35f*slowEnvelope,1.e-9f));
   lastGR=std::max(0.f,(level-threshold)*(1.f-1.f/ratios[q]));
  } else lastGR=0.f;
  const float gain=juce::Decibels::decibelsToGain(-lastGR)*makeupLinear;
  for(int c=0;c<channels;++c) {
   float y=b.getSample(c,i)*gain;
   // Optional experimental coloration. NOT calibrated to SSL/THAT 2181 hardware.
   if(drive>0.f) { const float k=1.f+drive*3.f; y=std::tanh(k*y)/std::tanh(k); }
   b.setSample(c,i,y);
  }
 }
 gainReductionDb.store(lastGR,std::memory_order_relaxed);
}
juce::AudioProcessorEditor* GSeriesLabProcessor::createEditor() {return new juce::GenericAudioProcessorEditor(*this);}
void GSeriesLabProcessor::getStateInformation(juce::MemoryBlock& m) {auto xml=state.copyState().createXml();copyXmlToBinary(*xml,m);}
void GSeriesLabProcessor::setStateInformation(const void* d,int n) {if(auto xml=getXmlFromBinary(d,n)) if(xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new GSeriesLabProcessor();}
