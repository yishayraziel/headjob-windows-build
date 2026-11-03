// Loads the installed plug-in through a real host path and measures what each knob does to the audio.
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
static juce::AudioBuffer<float> render(juce::AudioPluginFormatManager& manager,const juce::PluginDescription& type,const juce::String& knob,float setting){
 juce::String error;auto plugin=manager.createPluginInstance(type,48000,512,error);
 if(plugin==nullptr){std::cout<<"LOAD FAILED: "<<error<<"\n";std::exit(2);}
 plugin->enableAllBuses();plugin->setPlayConfigDetails(2,2,48000,512);plugin->prepareToPlay(48000,512);
 bool found=knob.isEmpty();
 for(auto* p:plugin->getParameters())if(p->getName(32).equalsIgnoreCase(knob)){p->beginChangeGesture();p->setValueNotifyingHost(setting);p->endChangeGesture();found=true;}
 if(!found){std::cout<<"PARAMETER NOT FOUND: "<<knob<<"\n";for(auto* p:plugin->getParameters())std::cout<<" has: "<<p->getName(32)<<"\n";std::exit(3);}
 juce::AudioBuffer<float> all(2,48000),block(2,512);juce::MidiBuffer midi;int written=0;
 for(int s=0;s<72000;s+=512){for(int c=0;c<2;++c)for(int i=0;i<512;++i)block.setSample(c,i,0.5f*(float)std::sin(2*juce::MathConstants<double>::pi*997*(s+i)/48000));
  plugin->processBlock(block,midi);if(s>=24000&&written+512<=48000){all.copyFrom(0,written,block,0,0,512);all.copyFrom(1,written,block,1,0,512);written+=512;}}
 plugin->releaseResources();return all;
}
int main(int argc,char** argv){
 if(argc<2)return 1;juce::ScopedJuceInitialiser_GUI gui;juce::AudioPluginFormatManager manager;manager.addDefaultFormats();
 const juce::String target(argv[1]);juce::OwnedArray<juce::PluginDescription> types;
 for(auto* format:manager.getFormats())if(format->fileMightContainThisPluginType(target)||target.startsWith("AudioUnit:")){format->findAllTypesForFile(types,target);if(!types.isEmpty())break;}
 if(types.isEmpty()){std::cout<<"NO PLUG-IN FOUND IN "<<target<<"\n";return 4;}
 std::cout<<"Loaded "<<types[0]->name<<" "<<types[0]->version<<" ("<<types[0]->pluginFormatName<<")\n";
 auto base=render(manager,*types[0],{},0);int failures=0;double total=0;for(int i=0;i<47000;++i)total+=base.getSample(0,i)*base.getSample(0,i);
 for(auto knob:{"TRIODE","PENTODE","TAPE"}){auto moved=render(manager,*types[0],knob,0.722f);double diff=0;for(int i=0;i<47000;++i){double d=moved.getSample(0,i)-base.getSample(0,i);diff+=d*d;}
  const double db=10*std::log10((diff+1e-20)/(total+1e-20));std::cout<<knob<<" at 14:00 changes the audio by "<<db<<" dB relative to the signal\n";if(db<-60)++failures;}
 return failures?5:0;
}
