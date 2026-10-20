#include "Dsp.h"
#include <iostream>
#include <stdexcept>
using namespace jerzy;
void check(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
int main()try{
 int checks=0;
 for(double fs:{8000.,44100.,48000.,96000.,192000.,384000.})for(size_t model=0;model<circuits.size();++model){
  Engine e;auto p=defaults();p[Model]=double(model)/(circuits.size()-1);p[Color]=1;p[Depth]=1;p[Stereo]=1;e.reset(fs,p);double energy=0;
  for(int i=0;i<int(fs*.15);++i){auto y=e.process(i==0?.8:0,0,p);check(std::isfinite(y[0])&&std::isfinite(y[1])&&std::abs(y[0])<8,"unstable circuit");energy+=y[0]*y[0];check(y[1]==0,"stereo crosstalk");}
  check(energy>.01,"no impulse output");++checks;
 }
 for(int id:{Mix,Bypass}){auto p=defaults();p[id]=id==Mix?0:1;Engine e;e.reset(48000,p);for(int i=0;i<12000;++i){double x=.5*sin(i*.1);auto y=e.process(x,-x,p);check(std::abs(y[0]-x)<1e-12&&std::abs(y[1]+x)<1e-12,"dry/bypass transparency");}++checks;}
 {auto p=defaults();p[Mix]=1;p[Stereo]=1;Engine e;e.reset(48000,p);double difference=0,energy=0;for(int i=0;i<48000;++i){double x=.3*sin(2*pi*440*i/48000.);auto y=e.process(x,x,p);difference+=std::abs(y[0]-y[1]);energy+=y[0]*y[0];}check(difference>10&&energy>1,"wet/stereo modulation missing");++checks;}
 {auto p=defaults();p[Filter]=1;Engine e;e.reset(48000,p);for(int i=0;i<1000;++i)e.process(.1,.1,p);check(e.getPhase()==0,"filter matrix does not freeze clock");++checks;}
 {Engine e;e.reset(48000);auto p=defaults();for(int i=0;i<100000;++i){p[Model]=(i/2000%14)/13.;p[Color]=1;auto y=e.process(.2,.1,p);check(std::isfinite(y[0])&&std::abs(y[0])<8,"switching instability");}++checks;}
 // Every division and model: one cycle is the declared number of quarter-note beats.
 for(size_t model=0;model<circuits.size();++model)for(size_t div=0;div<divisions.size();++div)for(double bpm:{90.,140.}){
  auto p=defaults();p[Model]=double(model)/13.;p[Sync]=1;p[Division]=double(div)/17.;
  Engine e;e.reset(48000,p);for(int i=0;i<240;++i)e.process(0,0,p,bpm);
  double expected=std::fmod(240*bpm/(60*48000*divisions[div].beats),1.);
  check(std::abs(e.getPhase()-expected)<1e-10,"incorrect tempo division period");
  e.process(0,0,p,bpm,-3.125);expected=-3.125/divisions[div].beats;expected-=std::floor(expected);
  check(std::abs(e.getPhase()-expected)<1e-10,"host position alignment including negative preroll");
  auto frozen=e.getPhase();p[Filter]=1;e.process(0,0,p,bpm,99.75);check(e.getPhase()==frozen,"Filter Matrix must freeze sync");
  ++checks;
 }
 {auto p=defaults();p[Sync]=1;Engine e;e.reset(48000,p);e.process(0,0,p,NAN);check(std::isfinite(e.getPhase()),"missing tempo fallback");++checks;}
 std::cout<<checks<<" DSP scenarios passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
