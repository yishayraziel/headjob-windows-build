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
  for(auto& s:amount) { s.reset(rate,0.025); s.setCurrentAndTargetValue(0); }
  powerCoeff=std::exp(-1.0/(sr*0.4)); autoCoeff=std::exp(-1.0/(sr*0.25)); inputPower=outputPower=0;autoLevel=1;
  manualGain.reset(sr,0.025);manualGain.setCurrentAndTargetValue(1);
  gainEnable.reset(sr,0.015);gainEnable.setCurrentAndTargetValue(1);
  bypass.reset(sr,0.015); bypass.setCurrentAndTargetValue(0);
  dry.setSize(2,block); const int latency=getLatency();
  delay.assign((size_t)latency+1,{0,0}); pos=0; low={}; de={}; deSat={}; deLin={}; prevX={}; prevY={};
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
   const double p=std::pow(amount[1].getNextValue(),1.5);
   const double a=std::pow(amount[2].getNextValue(),1.5);
   for(int c=0;c<2;++c) {
    double x=up.getSample(c,(int)i);
    const double triDrive=0.45+0.75*t,bias=0.50;
    const double tri=(std::tanh(triDrive*x+bias)-std::tanh(bias))/(triDrive*(1-std::pow(std::tanh(bias),2)));
    x+=triodeMix*t*(tri-x);
    // The untouched component preserves transient slope and crest factor.
    const double pentDrive=1.50+1.00*p,pentMix=juce::jmin(0.9,pentodeMix*p*(0.25+0.10*p));
    x+=pentMix*(std::tanh(pentDrive*x)/pentDrive-x);
    // Tape, linear part: exactly the 0.4.1 response, including its gentle lift of the highs.
    low[c]+=alpha*(x-low[c]);
    const double tapeMix=a*(0.20+0.18*a),tapeDrive=0.85+1.00*a;
    constexpr double emphasis=0.85;
    const double pre=x+emphasis*(x-low[c]);
    const double linear=(pre+emphasis*(1-alpha)*de[c])/(1+emphasis*(1-alpha));de[c]=linear;
    // Tape, nonlinear part: stronger record emphasis, so highs saturate and compress before lows.
    // Only the difference the curve makes is added, which leaves the linear response above untouched.
    constexpr double nlEmphasis=1.8;
    const double nlPre=x+nlEmphasis*(x-low[c]);
    const double nlSat=std::tanh(tapeDrive*nlPre)/tapeDrive;
    const double nlOut=(nlSat+nlEmphasis*(1-alpha)*deSat[c])/(1+nlEmphasis*(1-alpha));deSat[c]=nlOut;
    const double nlLin=(nlPre+nlEmphasis*(1-alpha)*deLin[c])/(1+nlEmphasis*(1-alpha));deLin[c]=nlLin;
    x+=tapeMix*(linear-x)+tapeHarmonics*tapeMix*(nlOut-nlLin);
    const double hp=x-prevX[c]+dcR*prevY[c];prevX[c]=x;prevY[c]=hp;
    const double active=juce::jlimit(0.0,1.0,(t+p+a)*20.0);
    // Restrained makeup replaces the previous large amount-dependent boost.
    up.setSample(c,(int)i,(x+active*(hp-x))*(1+0.03*t+0.02*p+0.02*a));
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
 size_t pos=0; double rate=192000,alpha=0,dcR=0;
 std::array<double,2> low{},de{},deSat{},deLin{},prevX{},prevY{};
 // 0.7: the 0.4.1 structure with each stage's harmonics raised.
 static constexpr double triodeMix=0.22*2.7,pentodeMix=3.0,tapeHarmonics=1.2;
};
