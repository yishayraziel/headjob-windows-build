#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "BinaryData.h"
class Processor;
class Panel: public juce::AudioProcessorEditor, private juce::Timer, public juce::LookAndFeel_V4 {
public:
 Panel(Processor&);
 ~Panel() override {stopTimer();setLookAndFeel(nullptr);}
 void paint(juce::Graphics&) override;
 void resized() override;
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
private:
 void timerCallback() override {repaint();}
 Processor& proc;
 juce::Image panel;
 std::array<juce::Image,4> knobFrames;
 std::array<float,4> knobAngles{{-100,-100,-100,-100}};
 juce::ToggleButton power,autoGainButton;
 juce::Slider outputGain;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputLink;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoLink;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerLink;
 std::array<juce::Slider,3> knobs;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,3> links;
};
class Processor:public juce::AudioProcessor {
public:
 juce::AudioProcessorValueTreeState state;
 Engine engine; juce::AudioBuffer<double> scratch;
 std::array<std::atomic<float>,2> meter{{0,0}};
 Processor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"HEADJOB",layout()) {}
 static juce::AudioProcessorValueTreeState::ParameterLayout layout() {
  juce::AudioProcessorValueTreeState::ParameterLayout l;
  for(auto id:{"triode","pentode","tape"}) l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},juce::String(id).toUpperCase(),juce::NormalisableRange<float>(0,10,0.01f),0));
  l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
  l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"outputGain",1},"Output Gain",juce::NormalisableRange<float>(-24,12,0.1f),0));
  l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"autoGain",1},"Auto Gain",false));return l;
 }
 const juce::String getName() const override{return "HEADJOB";}
 bool acceptsMidi()const override{return false;} bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 0.1;}
 int getNumPrograms()override{return 1;} int getCurrentProgram()override{return 0;}
 void setCurrentProgram(int)override{} const juce::String getProgramName(int)override{return "Default";} void changeProgramName(int,const juce::String&)override{}
 bool isBusesLayoutSupported(const BusesLayout& l)const override{return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 void prepareToPlay(double sr,int block)override{capacity=juce::jmax(1,block);engine.prepare(sr,capacity);scratch.setSize(2,capacity);setLatencySamples(engine.getLatency());decay=(float)std::exp(-1.0/(sr*0.3));}
 void releaseResources()override{}
 juce::AudioProcessorParameter* getBypassParameter()const override{return state.getParameter("bypass");}
 void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)override {run(b,false);}
 void processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&)override{run(b,true);}
 void run(juce::AudioBuffer<float>& b,bool bypassed){
  juce::ScopedNoDenormals nd;
  std::array<float,3> a{state.getRawParameterValue("triode")->load(),state.getRawParameterValue("pentode")->load(),state.getRawParameterValue("tape")->load()};
  for(int start=0;start<b.getNumSamples();start+=capacity){int count=juce::jmin(capacity,b.getNumSamples()-start);double* ptrs[]{scratch.getWritePointer(0),scratch.getWritePointer(1)};juce::AudioBuffer<double> view(ptrs,2,count);
   for(int c=0;c<2;++c)for(int i=0;i<count;++i)view.setSample(c,i,b.getSample(c,start+i));
   engine.process(view,a,bypassed||state.getRawParameterValue("bypass")->load()>0.5f,state.getRawParameterValue("outputGain")->load(),state.getRawParameterValue("autoGain")->load()>0.5f);
   for(int c=0;c<2;++c){float peak=meter[c].load();for(int i=0;i<count;++i){float v=(float)view.getSample(c,i);b.setSample(c,start+i,v);peak=juce::jmax(std::abs(v),peak*decay);}meter[c].store(peak);}
  }
 }
 bool hasEditor()const override{return true;} juce::AudioProcessorEditor* createEditor()override{return new Panel(*this);}
 void getStateInformation(juce::MemoryBlock& b)override{auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
 void setStateInformation(const void* d,int n)override{auto x=getXmlFromBinary(d,n);if(x&&x->hasTagName("HEADJOB"))state.replaceState(juce::ValueTree::fromXml(*x));}
private:int capacity=512;float decay=0.9999f;
};
Panel::Panel(Processor& p):AudioProcessorEditor(p),proc(p){
 panel=juce::ImageCache::getFromMemory(BinaryData::HEADJOBPanel_png,BinaryData::HEADJOBPanel_pngSize);
 setSize(1162,304);setLookAndFeel(this);
 const char* ids[]{"triode","pentode","tape"};
 for(int i=0;i<3;++i){auto& s=knobs[i];s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s.setName(ids[i]);s.setTitle(juce::String(ids[i]).toUpperCase());s.setDoubleClickReturnValue(true,0);s.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);addAndMakeVisible(s);links[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],s);}
 power.setButtonText("Bypass");power.setTitle("Bypass");power.setAlpha(0.0f);power.setMouseCursor(juce::MouseCursor::PointingHandCursor);addAndMakeVisible(power);
 powerLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",power);
 outputGain.setName("outputGain");outputGain.setTitle("Output Gain");outputGain.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);outputGain.setTextBoxStyle(juce::Slider::TextBoxRight,false,70,22);outputGain.setTextValueSuffix(" dB");outputGain.setDoubleClickReturnValue(true,0);outputGain.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff008f87));outputGain.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xff163f3c));outputGain.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xffd4dcda));addAndMakeVisible(outputGain);
 outputLink=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"outputGain",outputGain);
 autoGainButton.setButtonText("AUTO GAIN");autoGainButton.setTitle("Auto Gain");autoGainButton.setColour(juce::ToggleButton::textColourId,juce::Colour(0xff173e3b));autoGainButton.setColour(juce::ToggleButton::tickColourId,juce::Colour(0xff008d7e));addAndMakeVisible(autoGainButton);
 autoLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"autoGain",autoGainButton);
 startTimerHz(30);
}
void Panel::resized(){
 outputGain.setBounds(490,237,140,60);autoGainButton.setBounds(683,253,145,28);
 const auto transform=juce::AffineTransform::translation(-56.0f,-149.0f).scaled(getWidth()/1936.0f,230.0f/382.0f);
 const int centres[]{902,1252,1596};
 for(int i=0;i<3;++i)knobs[i].setBounds(juce::Rectangle<float>((float)centres[i]-83,202,166,166).transformedBy(transform).toNearestInt());
 power.setBounds(juce::Rectangle<float>(568,211,92,201).transformedBy(transform).toNearestInt());
}
void Panel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float v,float start,float end,juce::Slider& slider){
 const float cx=x+w*0.5f,cy=y+h*0.5f;
 const float radius=juce::jmin(w,h)*(76.0f/166.0f);
 int index=&slider==&outputGain?3:0;for(int i=0;i<3;++i)if(&slider==&knobs[i])index=i;
 const float angle=start+v*(end-start);
 if(!knobFrames[index].isValid() || std::abs(knobAngles[index]-angle)>0.0001f){
  constexpr int size=256;
  auto image=juce::Image(juce::Image::ARGB,size,size,true);
  juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
  const float dx=std::sin(angle),dy=-std::cos(angle);
  // A geometric height field rotates the raised grip only. Normals are lit
  // in panel coordinates, so the light source never rotates with the control.
  auto height=[&](float xx,float yy){
   const float rr=xx*xx+yy*yy;
   const float along=xx*dx+yy*dy,cross=xx*dy-yy*dx;
   float base=0.10f*std::sqrt(juce::jmax(0.0f,1-rr));
   float handle=0.23f*std::exp(-std::pow(cross/0.21f,4)-std::pow(along/0.74f,8));
   return base+handle;
  };
  for(int py=0;py<size;++py)for(int px=0;px<size;++px){
   const float xx=(px+0.5f-size*0.5f)/(size*0.5f),yy=(py+0.5f-size*0.5f)/(size*0.5f);
   const float r=std::sqrt(xx*xx+yy*yy);if(r>=1)continue;
   const float eps=0.004f;
   float nx=-(height(xx+eps,yy)-height(xx-eps,yy))/(2*eps);
   float ny=-(height(xx,yy+eps)-height(xx,yy-eps))/(2*eps),nz=1;
   if(r>0.88f){
    // Fixed cylindrical rim with fine radial machining grooves.
    float slope=1.6f+0.55f*std::sin(std::atan2(yy,xx)*56);
    nx=xx*slope;ny=yy*slope;nz=0.65f;
   }
   float len=std::sqrt(nx*nx+ny*ny+nz*nz);nx/=len;ny/=len;nz/=len;
   const float diffuse=juce::jmax(0.0f,-0.45f*nx-0.55f*ny+0.70f*nz);
   const float spec=std::pow(juce::jmax(0.0f,-0.25f*nx-0.31f*ny+0.917f*nz),46);
   float illum=0.25f+0.79f*diffuse;
   if(r>0.975f)illum*=0.48f;
   float red=0.025f*illum+0.66f*spec,green=0.54f*illum+0.58f*spec,blue=0.51f*illum+0.57f*spec;
   float along=xx*dx+yy*dy,cross=xx*dy-yy*dx;
   // Inlaid pointer dot moves with the grip, while its illumination stays fixed.
   float dot=std::sqrt(cross*cross+(along-0.72f)*(along-0.72f));
   if(dot<0.032f){red=0.85f;green=0.97f;blue=0.94f;}
   pixels.setPixelColour(px,py,juce::Colour::fromFloatRGBA(juce::jlimit(0.0f,1.0f,red),juce::jlimit(0.0f,1.0f,green),juce::jlimit(0.0f,1.0f,blue),juce::jlimit(0.0f,1.0f,(1-r)*size*0.5f)));
  }
  knobFrames[index]=image;knobAngles[index]=angle;
 }
 g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
 g.drawImage(knobFrames[index],juce::Rectangle<float>(cx-radius,cy-radius,2*radius,2*radius));
}
void Panel::paint(juce::Graphics& g){
 g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
 g.drawImage(panel,0,0,getWidth(),230,56,149,1936,382);
 for(int xx=0;xx<getWidth();xx+=300)for(int yy=230;yy<304;yy+=55){int ww=juce::jmin(300,getWidth()-xx),hh=juce::jmin(55,304-yy);g.drawImage(panel,xx,yy,ww,hh,260,414,ww,hh);}
 g.setColour(juce::Colour(0xff778b87));g.drawHorizontalLine(231,0,(float)getWidth());
 g.setColour(juce::Colour(0xff173e3b));g.setFont(juce::FontOptions(14));g.drawText("OUTPUT",411,254,76,22,juce::Justification::centredRight);
 juce::Graphics::ScopedSaveState save(g);
 g.addTransform(juce::AffineTransform::translation(-56.0f,-149.0f).scaled(getWidth()/1936.0f,230.0f/382.0f));
 const bool off=proc.state.getRawParameterValue("bypass")->load()>0.5f;
 if(off){
  // Dim the photographed power lamp without replacing its polished bezel.
  g.setColour(juce::Colour(0xff123417).withAlpha(0.87f));g.fillEllipse(583,222,61,61);
  // Flip the photographed switch assembly to preserve its metallic detail.
  juce::Graphics::ScopedSaveState toggleState(g);
  juce::Path toggleClip;toggleClip.addEllipse(578,337,72,72);g.reduceClipRegion(toggleClip);
  g.addTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi,614,373));
  g.drawImage(panel,578,337,72,72,578,337,72,72);
 }
 const int ledX[]{806,838,870,903,935,967,1000,1032,1065,1097,1129,1162,1195,1227,1259,1292,1324,1357,1390,1422,1456,1488,1521,1553,1586,1619,1652,1684,1716};
 for(int c=0;c<2;++c){
  const float level=proc.meter[c].load();const float db=juce::Decibels::gainToDecibels(level,-60.0f);
  for(int i=0;i<29;++i)if(level<0.000001f || db < -60.0f+i*60.0f/28.0f){
   // Darken individual original LEDs; lit cells retain the reference's glow.
   const float xx=(float)ledX[i],yy=c?483.0f:455.0f;
   g.setColour(juce::Colour(0xff04100e).withAlpha(0.96f));g.fillRoundedRectangle(xx-2,yy-2,27,20,1.5f);
  }
 }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}
