// Black-box measurement of an installed effect plug-in, the way a host sees it: parameters,
// their value texts, and the response to sine tests. Reads nothing from the plug-in's files.
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <fstream>
#include <map>
#if JUCE_MAC
 #include <CoreFoundation/CoreFoundation.h>
#endif
using juce::String;
static void pump(){
#if JUCE_MAC
 for(int i=0;i<5;++i)CFRunLoopRunInMode(kCFRunLoopDefaultMode,0.01,false);
#endif
}
static constexpr double sr=48000;static constexpr int block=512;
struct Tone{double gainDb,h[8];};
static Tone measure(juce::AudioPluginInstance& p,double freq,double levelDb,double seconds=0.6){
 // settle, then analyse an integer number of cycles with a Hann window
 juce::AudioBuffer<float> b(2,block);juce::MidiBuffer midi;const double amp=juce::Decibels::decibelsToGain(levelDb),w=2*juce::MathConstants<double>::pi*freq/sr;
 const int settle=(int)(sr*0.4),length=(int)(sr*seconds);double re[8]{},im[8]{},norm=0;long n=0;
 for(int s=0;s<settle+length;s+=block){for(int i=0;i<block;++i){float x=(float)(amp*std::sin(w*(double)(n+i)));b.setSample(0,i,x);b.setSample(1,i,x);}
  p.processBlock(b,midi);
  for(int i=0;i<block;++i){long k=n+i;if(k<settle||k>=settle+length)continue;double y=b.getSample(0,i),hw=0.5-0.5*std::cos(2*juce::MathConstants<double>::pi*(double)(k-settle)/length);norm+=hw;
   for(int h=1;h<8;++h){if(freq*h>20000)continue;re[h]+=hw*y*std::cos(w*h*(double)k);im[h]+=hw*y*std::sin(w*h*(double)k);}}
  n+=block;}
 Tone t{};const double f=std::hypot(re[1],im[1]);t.gainDb=20*std::log10(2*f/norm/amp+1e-30);for(int h=2;h<8;++h)t.h[h]=100*std::hypot(re[h],im[h])/(f+1e-30);return t;
}
struct Knob{juce::AudioProcessorParameter* p=nullptr;std::vector<std::pair<float,String>> texts;};
static Knob find(juce::AudioPluginInstance& p,const String& part){
 for(auto* q:p.getParameters())if(q->getName(64).containsIgnoreCase(part)){Knob k{q,{}};for(int i=0;i<=400;++i){float v=i/400.0f;k.texts.push_back({v,q->getText(v,64)});}return k;}
 return {};
}
static void setNumber(Knob& k,double target){ // nearest value whose text reads as target
 float best=0;double err=1e9;for(auto& [v,t]:k.texts){double e=std::abs(t.retainCharacters("-0123456789.").getDoubleValue()-target);if(e<err){err=e;best=v;}}
 k.p->setValueNotifyingHost(best);pump();
}
static std::vector<std::pair<float,String>> choices(Knob& k){
 std::vector<std::pair<float,String>> out;std::map<String,std::vector<float>> seen;for(auto& [v,t]:k.texts)seen[t].push_back(v);
 for(auto& [t,vs]:seen)out.push_back({vs[vs.size()/2],t});std::sort(out.begin(),out.end());return out;
}
int main(int argc,char** argv){
 if(argc<3){std::cout<<"usage: PluginProbe <plugin path> <output dir> [process parameter name] [type parameter name]\n";return 1;}
 juce::ScopedJuceInitialiser_GUI gui;juce::AudioPluginFormatManager m;
 m.addDefaultFormats();
 const String path(argv[1]);juce::File out(juce::File::getCurrentWorkingDirectory().getChildFile(argv[2]));out.createDirectory();
 juce::OwnedArray<juce::PluginDescription> types;for(auto* f:m.getFormats()){f->findAllTypesForFile(types,path);if(!types.isEmpty())break;}
 if(types.isEmpty()){std::cout<<"NO PLUG-IN FOUND IN "<<path<<"\n";return 2;}
 String error;auto p=m.createPluginInstance(*types[0],sr,block,error);if(!p){std::cout<<"LOAD FAILED: "<<error<<"\n";return 3;}
 p->enableAllBuses();p->setPlayConfigDetails(2,2,sr,block);p->prepareToPlay(sr,block);pump();
 std::cout<<"Loaded "<<types[0]->name<<" "<<types[0]->version<<" ("<<types[0]->pluginFormatName<<"), latency "<<p->getLatencySamples()<<"\n";
 {std::ofstream f(out.getChildFile("parameters.csv").getFullPathName().toStdString());f<<"index,name,default,text_at_default,steps\n";int i=0;
  for(auto* q:p->getParameters())f<<i++<<",\""<<q->getName(64)<<"\","<<q->getDefaultValue()<<",\""<<q->getText(q->getDefaultValue(),64)<<"\","<<q->getNumSteps()<<"\n";}
 {auto t=measure(*p,997,-12);std::cout<<"Default settings, 997 Hz at -12 dBFS: gain "<<t.gainDb<<" dB, H2 "<<t.h[2]<<"%, H3 "<<t.h[3]<<"%\n";}
 Knob input=find(*p,"Input"),process=find(*p,argc>3?argv[3]:"Process"),output=find(*p,"Output"),bright=find(*p,"Bright"),type=find(*p,argc>4?argv[4]:"Type");
 if(!process.p||!type.p){std::cout<<"Could not find Process/Type parameters, see parameters.csv\n";return 4;}
 if(input.p)setNumber(input,0);if(output.p)setNumber(output,0);
 auto types_=choices(type);auto brights=bright.p?choices(bright):std::vector<std::pair<float,String>>{{0,"-"}};
 std::cout<<"Types:";for(auto& t:types_)std::cout<<" "<<t.second;std::cout<<"\nBrightness:";for(auto& t:brights)std::cout<<" "<<t.second;std::cout<<"\n";
 std::ofstream h(out.getChildFile("harmonics.csv").getFullPathName().toStdString());h<<"type,brightness,process,freq,level_dbfs,gain_db,h2,h3,h4,h5,h6,h7\n";
 std::ofstream r(out.getChildFile("response.csv").getFullPathName().toStdString());r<<"type,brightness,process,level_dbfs,freq,gain_db\n";
 const double third[]={20,31.5,50,80,125,200,315,500,800,1250,2000,3150,5000,8000,12500,16000,20000};
 for(auto& [tv,tname]:types_)for(auto& [bv,bname]:brights){
  type.p->setValueNotifyingHost(tv);if(bright.p)bright.p->setValueNotifyingHost(bv);pump();
  for(double proc:{0.0,25.0,50.0,75.0,100.0}){setNumber(process,proc);
   for(double level:{-30.0,-18.0,-12.0,-6.0,-3.0}){auto t=measure(*p,997,level);h<<tname<<","<<bname<<","<<proc<<",997,"<<level<<","<<t.gainDb;for(int k=2;k<8;++k)h<<","<<t.h[k];h<<"\n";}
   for(double fr:{60.0,300.0,3000.0,8000.0})for(double level:{-30.0,-6.0}){auto t=measure(*p,fr,level);h<<tname<<","<<bname<<","<<proc<<","<<fr<<","<<level<<","<<t.gainDb;for(int k=2;k<8;++k)h<<","<<t.h[k];h<<"\n";}
   if(proc==50||proc==100)for(double level:{-40.0,-12.0})for(double fr:third){auto t=measure(*p,fr,level,0.3);r<<tname<<","<<bname<<","<<proc<<","<<level<<","<<fr<<","<<t.gainDb<<"\n";}
  }
  std::cout<<"measured "<<tname<<" / "<<bname<<"\n"<<std::flush;h.flush();r.flush();
 }
 p->releaseResources();std::cout<<"PROBE_DONE\n";return 0;
}
