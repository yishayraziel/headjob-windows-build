// Renders the same material through two installed plug-ins and measures what is left after subtracting them.
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <random>
using Knobs=std::array<float,3>;
static juce::AudioBuffer<float> render(juce::AudioPluginFormatManager& m,const juce::PluginDescription& type,Knobs k,float inputDb){
 juce::String error;auto p=m.createPluginInstance(type,48000,512,error);if(!p){std::cout<<"LOAD FAILED "<<error<<"\n";std::exit(2);}
 p->enableAllBuses();p->setPlayConfigDetails(2,2,48000,512);p->prepareToPlay(48000,512);
 const char* names[]{"TRIODE","PENTODE","TAPE"};
 for(auto* q:p->getParameters())for(int i=0;i<3;++i)if(q->getName(32).equalsIgnoreCase(names[i]))q->setValueNotifyingHost(k[(size_t)i]/10.0f);
 std::mt19937 r(7);std::normal_distribution<float> d(0,1);float lp=0;
 juce::AudioBuffer<float> out(2,96000),block(2,512);juce::MidiBuffer midi;const float g=juce::Decibels::decibelsToGain(inputDb);int w=0;
 for(int s=0;s<120000;s+=512){for(int i=0;i<512;++i){int n=s+i;lp+=0.08f*(d(r)-lp);float x=g*(0.6f*(float)std::sin(0.0407*n)+0.3f*(float)std::sin(0.53*n)+0.5f*lp);block.setSample(0,i,x);block.setSample(1,i,x);}
  p->processBlock(block,midi);if(s>=24000&&w+512<=96000){out.copyFrom(0,w,block,0,0,512);out.copyFrom(1,w,block,1,0,512);w+=512;}}
 p->releaseResources();return out;
}
int main(int,char** argv){juce::ScopedJuceInitialiser_GUI gui;juce::AudioPluginFormatManager m;m.addDefaultFormats();
 auto find=[&](juce::String path){juce::OwnedArray<juce::PluginDescription> t;for(auto* f:m.getFormats())if(f->fileMightContainThisPluginType(path)){f->findAllTypesForFile(t,path);if(!t.isEmpty())break;}if(t.isEmpty()){std::cout<<"NOT FOUND "<<path<<"\n";std::exit(3);}return *t[0];};
 auto a=find(argv[1]),b=find(argv[2]);std::cout<<a.name<<" "<<a.version<<" vs "<<b.name<<" "<<b.version<<"\n";
 for(auto* d:{&a,&b})std::cout<<"desc "<<d->name<<" format "<<d->pluginFormatName<<" isInstrument "<<(int)d->isInstrument<<" category ["<<d->category<<"] in "<<d->numInputChannels<<" out "<<d->numOutputChannels<<"\n";
 double worst=-1000;
 for(Knobs k:{Knobs{0,0,0},Knobs{5,0,0},Knobs{0,5,0},Knobs{0,0,5},Knobs{7.2f,0,0},Knobs{0,7.2f,0},Knobs{0,0,7.2f},Knobs{5,5,5},Knobs{10,10,10}})for(float level:{-18.f,-6.f}){
  auto x=render(m,a,k,level),y=render(m,b,k,level);double s=0,d=0,sx=0,sy=0;for(int i=0;i<90000;++i){double u=x.getSample(0,i),v=y.getSample(0,i);s+=u*u;d+=(u-v)*(u-v);sx+=u*u;sy+=v*v;}
  worst=juce::jmax(worst,10*std::log10((d+1e-30)/(s+1e-30)));
  std::cout<<"knobs "<<k[0]<<" "<<k[1]<<" "<<k[2]<<" input "<<level<<": residual "<<10*std::log10((d+1e-30)/(s+1e-30))<<" dB, level difference "<<10*std::log10((sx+1e-30)/(sy+1e-30))<<" dB\n";}
 std::cout<<"WORST residual over all 18 cases "<<worst<<" dB\n";
 return 0;}
