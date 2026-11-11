#include "Plugin.cpp"
#include <iostream>
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI gui;
 if(argc>1 && juce::String(argv[1]).startsWith("--preview")) {Processor p; if(juce::String(argv[1])=="--preview-demo"){p.state.getParameter("triode")->setValueNotifyingHost(0.32f);p.state.getParameter("pentode")->setValueNotifyingHost(0.87f);p.state.getParameter("tape")->setValueNotifyingHost(0.63f);p.meter[0].store(0.72f);p.meter[1].store(0.5f);}if(juce::String(argv[1])=="--preview-low"){p.state.getParameter("triode")->setValueNotifyingHost(0);p.state.getParameter("pentode")->setValueNotifyingHost(0.25f);p.state.getParameter("tape")->setValueNotifyingHost(0.5f);}if(juce::String(argv[1])=="--preview-high"){p.state.getParameter("triode")->setValueNotifyingHost(0.5f);p.state.getParameter("pentode")->setValueNotifyingHost(0.75f);p.state.getParameter("tape")->setValueNotifyingHost(1);}if(juce::String(argv[1])=="--preview-bypass")p.state.getParameter("bypass")->setValueNotifyingHost(1);auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor()); if(juce::String(argv[1])=="--preview-small")editor->setSize(813,213);if(juce::String(argv[1])=="--preview-large")editor->setSize(1743,456);auto img=editor->createComponentSnapshot(editor->getLocalBounds(),true,2);juce::File f{juce::String(argv[2])};auto stream=f.createOutputStream();return juce::PNGImageFormat().writeImageToStream(img,*stream)?0:10;}
 {Processor p;p.prepareToPlay(48000,256);p.state.getParameter("triode")->setValueNotifyingHost(0.73f);juce::MemoryBlock data;p.getStateInformation(data);Processor q;q.setStateInformation(data.getData(),(int)data.getSize());if(std::abs(q.state.getRawParameterValue("triode")->load()-7.3f)>0.001)return 5;std::cout<<"DAW state roundtrip passed\n";}
 {Processor p;auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());int sliders=0,buttons=0;for(auto* child:editor->getChildren()){if(auto* slider=dynamic_cast<juce::Slider*>(child)){slider->setValue(4.25,juce::sendNotificationSync);++sliders;}if(auto* button=dynamic_cast<juce::ToggleButton*>(child)){button->setToggleState(true,juce::sendNotificationSync);++buttons;}}if(sliders!=4||buttons!=2||p.state.getRawParameterValue("bypass")->load()!=1||std::abs(p.state.getRawParameterValue("tape")->load()-4.25f)>0.001)return 11;std::cout<<"Photographic UI controls and bypass attachment passed\n";}
 {
  for(bool automatic:{false,true}){Engine e;e.prepare(48000,256);juce::AudioBuffer<double>b(2,256);double input=0,output=0,stereoError=0;
   for(int block=0;block<1600;++block){for(int i=0;i<256;++i){double x=0.03*std::sin(2*juce::MathConstants<double>::pi*1000*(block*256+i)/48000);b.setSample(0,i,x);b.setSample(1,i,x);}e.process(b,{10,0,0},false,0,automatic);if(block>1400)for(int i=0;i<256;++i){input+=0.03*0.03*0.5;output+=b.getSample(0,i)*b.getSample(0,i);stereoError=std::max(stereoError,std::abs(b.getSample(0,i)-b.getSample(1,i)));}}
   double db=10*std::log10(output/input);std::cout<<"Auto Gain="<<automatic<<" steady RMS delta dB="<<db<<" stereo error="<<stereoError<<"\n";if(stereoError>1e-12 || (automatic && std::abs(db)>0.15) || (!automatic && db<0.1))return 12;
  }
  Engine e;e.prepare(48000,256);juce::AudioBuffer<double>b(2,256);double energy=0;for(int k=0;k<100;++k){for(int c=0;c<2;++c)for(int i=0;i<256;++i)b.setSample(c,i,0.1);e.process(b,{0,0,0},false,6,false);if(k==99)energy=b.getSample(0,255);}if(std::abs(energy-0.1*std::pow(10.0,6.0/20))>1e-10)return 13;
  for(int k=0;k<100;++k){for(int c=0;c<2;++c)for(int i=0;i<256;++i)b.setSample(c,i,0.1);e.process(b,{0,0,0},true,12,true);}if(std::abs(b.getSample(0,255)-0.1)>1e-10)return 14;
  Processor p;p.state.getParameter("outputGain")->setValueNotifyingHost(0.5f);p.state.getParameter("autoGain")->setValueNotifyingHost(1);juce::MemoryBlock data;p.getStateInformation(data);Processor q;q.setStateInformation(data.getData(),(int)data.getSize());if(q.state.getRawParameterValue("autoGain")->load()!=1||std::abs(q.state.getRawParameterValue("outputGain")->load()+6)>0.001)return 15;
  std::cout<<"Manual gain, host bypass and new parameter state passed\n";
 }
 {Processor p;auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
 if(!editor->isResizable())return 16;
 for(int width:{813,1162,1743,2324}){int height=juce::roundToInt(width*304.0/1162.0);editor->setSize(width,height);int controlCount=0;for(auto* child:editor->getChildren())if(dynamic_cast<juce::Slider*>(child)||dynamic_cast<juce::ToggleButton*>(child)){++controlCount;auto bounds=child->getBoundsInParent();if(!editor->getLocalBounds().expanded(1).contains(bounds))return 17;if(std::abs(child->getTransform().getScaleFactor()-width/1162.0f)>0.001)return 18;}if(controlCount!=6)return 19;}
 juce::MemoryBlock saved;p.getStateInformation(saved);Processor q;q.setStateInformation(saved.getData(),(int)saved.getSize());auto reopened=std::unique_ptr<juce::AudioProcessorEditor>(q.createEditor());if(reopened->getWidth()!=2324)return 20;
 std::cout<<"Resize 70/100/150/200 percent, control bounds and DAW size recall passed\n";
 }
 for(int stage=0;stage<3;++stage){Engine e;e.prepare(48000,256);juce::AudioBuffer<double>b(2,256);std::array<float,3>a{};a[stage]=10;double re[6]{},im[6]{};int n=48000;
 for(int start=0;start<n+48000;start+=256){int count=std::min(256,n+48000-start);double* ptr[]{b.getWritePointer(0),b.getWritePointer(1)};juce::AudioBuffer<double>v(ptr,2,count);for(int i=0;i<count;++i)for(int c=0;c<2;++c)v.setSample(c,i,0.5*std::sin(2*juce::MathConstants<double>::pi*1000*(start+i)/48000));e.process(v,a,false);if(start>=48000)for(int i=0;i<count;++i)for(int h=1;h<=5;++h){double angle=2*juce::MathConstants<double>::pi*1000*h*(start+i)/48000;re[h]+=v.getSample(0,i)*std::cos(angle);im[h]+=v.getSample(0,i)*std::sin(angle);}}
 double h2=std::hypot(re[2],im[2]),h3=std::hypot(re[3],im[3]);std::cout<<"stage="<<stage<<" H2="<<h2<<" H3="<<h3<<"\n";if(stage==0?h2<h3:h3<h2*10)return 6;
 }

 {// 0.7: each stage clearly audible at 14:00, Pentode and Tape distinct, the 0.4.1 high lift kept
  auto sine=[](int stage,float knob,double db,double hz){Engine e;e.prepare(48000,256);juce::AudioBuffer<double>b(2,256);std::array<float,3>a{};a[stage]=knob;const double amp=std::pow(10.0,db/20),w=2*juce::MathConstants<double>::pi*hz/48000;double re[4]{},im[4]{},count=0;
   for(int s=0;s<96000;s+=256){for(int i=0;i<256;++i)for(int c=0;c<2;++c)b.setSample(c,i,amp*std::sin(w*(s+i)));e.process(b,a,false);if(s>=48000)for(int i=0;i<256;++i){double x=b.getSample(0,i),wn=1-std::cos(2*juce::MathConstants<double>::pi*(s+i-48000)/48000.0);count+=wn;for(int h:{1,2,3}){re[h]+=wn*x*std::cos(w*h*(s+i));im[h]+=wn*x*std::sin(w*h*(s+i));}}}
   const double f=std::hypot(re[1],im[1]);return std::array<double,3>{20*std::log10(2*f/count/amp),100*std::hypot(re[2],im[2])/f,100*std::hypot(re[3],im[3])/f};};
  constexpr float twoOClock=7.22f;
  auto tri=sine(0,twoOClock,-12,997),pent=sine(1,twoOClock,-12,997),tape=sine(2,twoOClock,-12,997),tapeHigh=sine(2,twoOClock,-12,3000),tapeQuietHigh=sine(2,twoOClock,-30,6000),tapeLoudHigh=sine(2,twoOClock,-6,6000);
  std::cout<<"14:00 at -12 dBFS: Triode H2 "<<tri[1]<<"%, Pentode H3 "<<pent[2]<<"%, Tape H3 1 kHz "<<tape[2]<<"% 3 kHz "<<tapeHigh[2]<<"%, Tape 6 kHz quiet "<<tapeQuietHigh[0]<<" dB loud "<<tapeLoudHigh[0]<<" dB\n";
  if(tri[1]<1.5||pent[2]<1.0)return 21;                                   // audible at 14:00
  if(tape[2]>0.8*pent[2]||tapeHigh[2]<1.5*tape[2])return 22;              // Tape is lighter in the mids and works the highs
  if(tapeQuietHigh[0]<1.0||tapeQuietHigh[0]-tapeLoudHigh[0]<2.0)return 23; // high lift kept, loud highs compress
 }

 for(double sr:{44100.,48000.,96000.,192000.}) {
  Engine e;e.prepare(sr,256);juce::AudioBuffer<double>b(2,256);double peak=0;
  for(int block=0;block<300;++block){for(int i=0;i<256;++i){double x=0.5*std::sin(2*juce::MathConstants<double>::pi*997*(block*256+i)/sr);b.setSample(0,i,x);b.setSample(1,i,-x);}e.process(b,{10,10,10},block>150);for(int c=0;c<2;++c)for(int i=0;i<256;++i){double x=b.getSample(c,i);if(!std::isfinite(x))return 1;peak=std::max(peak,std::abs(x));}}
  if(peak>4)return 2;std::cout<<"finite/stable/bypass sr="<<sr<<" latency="<<e.getLatency()<<" peak="<<peak<<"\n";
 }
 Engine e;e.prepare(48000,256);juce::AudioBuffer<double>b(2,256);std::vector<double> history;double error=0;
 for(int k=0;k<30;++k){for(int i=0;i<256;++i){double x=std::sin((k*256+i)*0.123)*0.5;history.push_back(x);b.setSample(0,i,x);b.setSample(1,i,0);}e.process(b,{0,0,0},false);for(int i=0;i<256;++i){int j=k*256+i-e.getLatency();double ref=j>=0?history[(size_t)j]:0;error=std::max(error,std::abs(ref-b.getSample(0,i)));if(b.getSample(1,i)!=0)return 3;}}
 std::cout<<"zero controls null error="<<error<<"\n";return error<1e-12?0:4;
}
