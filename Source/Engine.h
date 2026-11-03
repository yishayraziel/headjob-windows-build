#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
// Original approximation; not Crane Song's proprietary transfer functions.
class Engine {
public:
 void prepare(double sr, int block) {
  os=std::make_unique<juce::dsp::Oversampling<double>>(2,2,juce::dsp::Oversampling<double>::filterHalfBandPolyphaseIIR,true,true);
  os->initProcessing((size_t)block); os->reset();
  rate=sr*4; alpha=1-std::exp(-2*juce::MathConstants<double>::pi*1800/rate);
  dcR=std::exp(-2*juce::MathConstants<double>::pi*8/rate);
  bumpA=1-std::exp(-2*juce::MathConstants<double>::pi*600/rate);bumpB=1-std::exp(-2*juce::MathConstants<double>::pi*45/rate);
  for(auto& s:amount) { s.reset(rate,0.025); s.setCurrentAndTargetValue(0); }
  powerCoeff=std::exp(-1.0/(sr*0.4)); autoCoeff=std::exp(-1.0/(sr*0.25)); inputPower=outputPower=0;autoLevel=1;
  manualGain.reset(sr,0.025);manualGain.setCurrentAndTargetValue(1);
  gainEnable.reset(sr,0.015);gainEnable.setCurrentAndTargetValue(1);
  bypass.reset(sr,0.015); bypass.setCurrentAndTargetValue(0);
  dry.setSize(2,block); const int latency=getLatency();
  delay.assign((size_t)latency+1,{0,0}); pos=0; low={}; satPrev={}; replay1={}; bumpHigh={}; bumpLow={}; prevX={}; prevY={};
 }
 int getLatency() const {return os ? (int)std::lround(os->getLatencyInSamples()) : 0;}
 void process(juce::AudioBuffer<double>& b, const std::array<float,3>& knobs,bool off,float outputDb=0,bool autoGain=false) {
  int n=b.getNumSamples();
  for(int c=0;c<2;++c) for(int i=0;i<n;++i) dry.setSample(c,i,b.getSample(c,i));
  for(int k=0;k<3;++k) amount[k].setTargetValue(juce::jlimit(0.0,1.0,knobs[k]/10.0));
  auto up=os->processSamplesUp(juce::dsp::AudioBlock<double>(b));
  for(size_t i=0;i<up.getNumSamples();++i) {
   // More usable low/mid travel and a restrained parallel nonlinear residual.
   const double t=std::pow(amount[0].getNextValue(),1.5);
   const double pentKnob=amount[1].getNextValue();
   const double p=juce::jmin(1.0,pentKnob/pentodeSaturationPoint);
   const double pentAbove=juce::jmax(0.0,(pentKnob-pentodeSaturationPoint)/(1-pentodeSaturationPoint));
   const double a=std::pow(amount[2].getNextValue(),1.5);
   for(int c=0;c<2;++c) {
    double x=up.getSample(c,(int)i);
    const double triDrive=0.45+0.75*t,bias=0.50;
    const double tri=(std::tanh(triDrive*x+bias)-std::tanh(bias))/(triDrive*(1-std::pow(std::tanh(bias),2)));
    x+=0.80*t*(tri-x);
    // Pentode: flat, memoryless, third-harmonic curve. At 14:00 a -6 dBFS sine reaches its flat top;
    // past 14:00 that point keeps falling, to -12 dBFS at full. A little dry signal keeps the transient edge.
    const double pentDrive=(1.50+1.50*p)*(1+pentAbove);
    const double pentU=juce::jlimit(-1.5,1.5,pentDrive*x);
    x+=0.85*p*p*((pentU-(4.0/27.0)*pentU*pentU*pentU)/pentDrive-x);
    // Tape: odd harmonics that grow with level. Record emphasis before the curve and its exact inverse after it
    // keep quiet signals flat, while highs reach the curve first and compress sooner, and peaks get squashed.
    constexpr double emphasis=1.8;
    low[c]+=alpha*(x-low[c]);
    const double pre=x+emphasis*(x-low[c]);
    const double tapeDrive=0.60+1.10*a;
    const double sat=std::tanh(tapeDrive*pre)/tapeDrive;
    const double b=emphasis*(1-alpha),pole=1-alpha;
    const double replay=(sat-pole*satPrev[c]+(pole+b)*replay1[c])/(1+b);satPrev[c]=sat;replay1[c]=replay;
    x+=juce::jmin(1.0,1.3*a)*(replay-x);
    // Bottom and midrange fatten as the process goes up.
    bumpHigh[c]+=bumpA*(x-bumpHigh[c]);
    bumpLow[c]+=bumpB*(bumpHigh[c]-bumpLow[c]);
    x+=0.20*a*(bumpHigh[c]-bumpLow[c]);
    const double hp=x-prevX[c]+dcR*prevY[c];prevX[c]=x;prevY[c]=hp;
    const double active=juce::jlimit(0.0,1.0,(t+p+a)*20.0);
    // Restrained makeup replaces the previous large amount-dependent boost.
    up.setSample(c,(int)i,(x+active*(hp-x))*(1+0.03*t+0.05*p*p+0.03*a));
   }
  }
  auto down=juce::dsp::AudioBlock<double>(b); os->processSamplesDown(down);
  bypass.setTargetValue(off || (knobs[0]==0 && knobs[1]==0 && knobs[2]==0) ? 0.0:1.0);
  manualGain.setTargetValue(juce::Decibels::decibelsToGain((double)juce::jlimit(-24.0f,12.0f,outputDb)));
  gainEnable.setTargetValue(off?0.0:1.0);
  for(int i=0;i<n;++i) {
   double mix=bypass.getNextValue();auto read=(pos+1)%delay.size();
   double inEnergy=0,outEnergy=0,processed[2]{};
   for(int c=0;c<2;++c){delay[pos][c]=dry.getSample(c,i);double clean=delay[read][c];processed[c]=clean+mix*(b.getSample(c,i)-clean);inEnergy+=clean*clean*0.5;outEnergy+=processed[c]*processed[c]*0.5;}
   pos=read;
   inputPower=powerCoeff*inputPower+(1-powerCoeff)*inEnergy;
   outputPower=powerCoeff*outputPower+(1-powerCoeff)*outEnergy;
   double target=1;
   if(autoGain && !off){target=autoLevel;if(inputPower>1e-10 && outputPower>1e-12)target=juce::jlimit(0.0630957344,1.0,std::sqrt(inputPower/outputPower));}
   autoLevel=autoCoeff*autoLevel+(1-autoCoeff)*target;
   double enabled=gainEnable.getNextValue();double gain=1+enabled*(manualGain.getNextValue()*autoLevel-1);
   for(int c=0;c<2;++c)b.setSample(c,i,processed[c]*gain);
  }
 }
private:
 std::unique_ptr<juce::dsp::Oversampling<double>> os;
 std::array<juce::SmoothedValue<double>,3> amount;
 juce::SmoothedValue<double> bypass,manualGain,gainEnable;
 double powerCoeff=0,autoCoeff=0,inputPower=0,outputPower=0,autoLevel=1;
 juce::AudioBuffer<double> dry;
 std::vector<std::array<double,2>> delay;
 size_t pos=0; double rate=192000,alpha=0,dcR=0,bumpA=0,bumpB=0;
 // Knob 7.22 of 10 is the 14:00 position on the panel's 270 degree travel.
 static constexpr double pentodeSaturationPoint=7.22/10.0;
 std::array<double,2> low{},satPrev{},replay1{},bumpHigh{},bumpLow{},prevX{},prevY{};
};
