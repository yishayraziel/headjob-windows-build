#include "Engine.h"
#include "LegacyEngine.h"
#include <iostream>
#include <iomanip>
struct Result {double thd, gain,crest;};
template<class E> Result measure(int stage,float setting,double db){
 E engine;engine.prepare(48000,256);juce::AudioBuffer<double> b(2,256);std::array<float,3> a{};a[stage]=setting;
 double re[10]{},im[10]{},power=0,peak=0;const double amp=std::pow(10.0,db/20);
 for(int start=0;start<96000;start+=256){for(int i=0;i<256;++i)for(int c=0;c<2;++c)b.setSample(c,i,amp*std::sin(2*juce::MathConstants<double>::pi*997*(start+i)/48000));engine.process(b,a,false);
 for(int i=0;i<256;++i)if(start+i>=48000){double x=b.getSample(0,i);power+=x*x;peak=std::max(peak,std::abs(x));for(int h=1;h<=9;++h){double phase=2*juce::MathConstants<double>::pi*997*h*(start+i)/48000;re[h]+=x*std::cos(phase);im[h]+=x*std::sin(phase);}}}
 double fundamental=re[1]*re[1]+im[1]*im[1],harmonics=0;for(int h=2;h<=9;++h)harmonics+=re[h]*re[h]+im[h]*im[h];double rms=std::sqrt(power/48000);
 return {100*std::sqrt(harmonics/fundamental),20*std::log10(rms/(amp/std::sqrt(2.0))),20*std::log10(peak/rms)};
}
int main(){std::cout<<std::setprecision(8)<<"stage,knob,input_peak_dbfs,old_thd_percent,new_thd_percent,old_gain_db,new_gain_db,old_crest_db,new_crest_db\n";const char* names[]{"Triode","Pentode","Tape"};for(int stage=0;stage<3;++stage)for(float knob:{2.5f,5.f,7.5f,10.f})for(double level:{-18.,-12.,-6.}){auto old=measure<LegacyEngine>(stage,knob,level),now=measure<Engine>(stage,knob,level);std::cout<<names[stage]<<','<<knob<<','<<level<<','<<old.thd<<','<<now.thd<<','<<old.gain<<','<<now.gain<<','<<old.crest<<','<<now.crest<<'\n';}return 0;}
