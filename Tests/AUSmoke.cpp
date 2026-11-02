#include <initializer_list>
#include <cstring>

#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <cmath>
#include <cstdio>
static OSStatus input(void*,AudioUnitRenderActionFlags*,const AudioTimeStamp* ts,UInt32,UInt32 n,AudioBufferList* b){for(UInt32 c=0;c<b->mNumberBuffers;++c)for(UInt32 i=0;i<n;++i)((float*)b->mBuffers[c].mData)[i]=0.25f*std::sin(6.28318530718*997*(ts->mSampleTime+i)/48000);return noErr;}
int main(int argc,char**argv){if(argc<2)return 1;auto url=CFURLCreateFromFileSystemRepresentation(nullptr,(const UInt8*)argv[1],strlen(argv[1]),true);auto bundle=CFBundleCreate(nullptr,url);if(!bundle||!CFBundleLoadExecutable(bundle))return 2;auto factory=(AudioComponentFactoryFunction)CFBundleGetFunctionPointerForName(bundle,CFSTR("HEADJOB_RefinedAUFactory"));if(!factory)return 3;AudioComponentDescription d{'aufx','HjR4','Hjob',0,0};auto* iface=(AudioComponentPlugInInterface*)factory(&d);if(!iface)return 3;void* unit=iface;OSStatus err=iface->Open(unit,nullptr);if(err){printf("open=%d\n",(int)err);return 4;}
auto set=(OSStatus(*)(void*,AudioUnitPropertyID,AudioUnitScope,AudioUnitElement,const void*,UInt32))iface->Lookup(kAudioUnitSetPropertySelect);
auto init=(OSStatus(*)(void*))iface->Lookup(kAudioUnitInitializeSelect);
auto render=(OSStatus(*)(void*,AudioUnitRenderActionFlags*,const AudioTimeStamp*,UInt32,UInt32,AudioBufferList*))iface->Lookup(kAudioUnitRenderSelect);
AudioStreamBasicDescription fmt{};fmt.mSampleRate=48000;fmt.mFormatID=kAudioFormatLinearPCM;fmt.mFormatFlags=kAudioFormatFlagsNativeFloatPacked|kAudioFormatFlagIsNonInterleaved;fmt.mBytesPerPacket=fmt.mBytesPerFrame=sizeof(float);fmt.mFramesPerPacket=1;fmt.mChannelsPerFrame=2;fmt.mBitsPerChannel=32;
for(auto scope:{kAudioUnitScope_Input,kAudioUnitScope_Output})if(set(unit,kAudioUnitProperty_StreamFormat,scope,0,&fmt,sizeof(fmt)))return 5;
AURenderCallbackStruct cb{input,nullptr};if(set(unit,kAudioUnitProperty_SetRenderCallback,kAudioUnitScope_Input,0,&cb,sizeof(cb)))return 6;
if(init(unit))return 7;
struct {UInt32 count;AudioBuffer buffers[2];} storage{};auto* b=(AudioBufferList*)&storage;float left[256]{},right[256]{};b->mNumberBuffers=2;b->mBuffers[0]={1,sizeof(left),left};b->mBuffers[1]={1,sizeof(right),right};double peak=0;
for(int block=0;block<100;++block){AudioTimeStamp ts{};ts.mSampleTime=block*256;ts.mFlags=kAudioTimeStampSampleTimeValid;AudioUnitRenderActionFlags flags=0;err=render(unit,&flags,&ts,0,256,b);if(err){printf("render=%d\n",(int)err);return 8;}for(auto v:left){if(!std::isfinite(v))return 9;peak=std::fmax(peak,std::abs(v));}}
printf("AU bundle factory/load/initialize/stereo render PASS peak=%g\n",peak);auto uninit=(OSStatus(*)(void*))iface->Lookup(kAudioUnitUninitializeSelect);uninit(unit);iface->Close(unit);return peak>0?0:10;}
