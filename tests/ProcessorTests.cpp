#include "Plugin.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace jerzy;
static void check(bool c, const char* m) { if (!c) throw std::runtime_error(m); }
static void rewind(MemoryStream& s) { s.seek(0, IBStream::kIBSeekSet, nullptr); }
static void state(MemoryStream& s, const Values& values) {
    IBStreamer writer(&s, kLittleEndian);
    writer.writeInt32(0x4A434831); writer.writeInt32(2);
    for (auto v : values) writer.writeDouble(v);
    rewind(s);
}
int main() try {
    auto p = std::make_unique<Processor>();
    check(p->initialize(nullptr) == kResultOk, "processor initialization");
    Controller c; check(c.initialize(nullptr) == kResultOk, "controller initialization");
    int passed = 0;
    check(c.getParameterCount() == NumParams, "parameter count");
    for (int i = 0; i < NumParams; ++i) {
        ParameterInfo info{}; check(c.getParameterInfo(i, info) == kResultOk && info.id == unsigned(i), "stable parameter IDs");
    }
    ++passed;
    auto v=defaults();v[Bypass]=1;v[Model]=1;v[Depth]=.8;v[Stereo]=.7;v[Sync]=1;v[Division]=10./17.;
    MemoryStream initial; state(initial, v);
    check(p->setState(&initial) == kResultOk, "restore processor");
    MemoryStream saved; check(p->getState(&saved) == kResultOk, "save processor"); rewind(saved);
    check(c.setComponentState(&saved) == kResultOk, "restore controller");
    for (int i = 0; i < NumParams; ++i) check(std::abs(c.getParamNormalized(i) - v[i]) < 1e-12, "all controls persist");
    ++passed;
    MemoryStream bad; auto invalid = v; invalid[Depth] = NAN; state(bad, invalid);
    check(p->setState(&bad) == kResultFalse, "NaN state must be rejected");
    MemoryStream unchanged; p->getState(&unchanged);
    check(saved.getSize() == unchanged.getSize() && std::memcmp(saved.getData(), unchanged.getData(), saved.getSize()) == 0, "invalid state cannot partially modify controls");
    ++passed;
    MemoryStream truncated(saved.getData(), saved.getSize() - 1);
    check(p->setState(&truncated) == kResultFalse, "truncated state must be rejected"); ++passed;
    MemoryStream legacy;IBStreamer old(&legacy,kLittleEndian);
    old.writeInt32(0x4A434831);old.writeInt32(1);for(int i=0;i<9;++i)old.writeDouble(v[i]);rewind(legacy);
    check(p->setState(&legacy)==kResultOk,"load v0.1.0 state");
    MemoryStream migrated;p->getState(&migrated);rewind(migrated);c.setComponentState(&migrated);
    for(int i=0;i<9;++i)check(c.getParamNormalized(i)==v[i],"legacy controls unchanged");
    check(c.getParamNormalized(Sync)==0&&c.getParamNormalized(Division)==defaults()[Division],"legacy state defaults to Free");
    rewind(initial);check(p->setState(&initial)==kResultOk,"restore current state");++passed;
    for(int id:{Model,Stereo,Sync,Division}) {
        String128 text{};double parsed=0;
        check(c.getParamStringByValue(id,1,text)==kResultTrue&&c.getParamValueByString(id,text,parsed)==kResultTrue&&parsed==1,"parameter text roundtrip");
    }
    ++passed;
    ProcessSetup setup{}; setup.sampleRate = 48000; setup.maxSamplesPerBlock = 128; setup.symbolicSampleSize = kSample32; setup.processMode = kRealtime;
    check(p->setupProcessing(setup) == kResultOk, "setup processing");
    SpeakerArrangement mono = SpeakerArr::kMono;
    check(p->setBusArrangements(&mono, 1, &mono, 1) == kResultOk, "mono support");
    auto stereo = SpeakerArr::kStereo;
    check(p->setBusArrangements(&mono, 1, &stereo, 1) == kResultOk, "mono input to stereo output support");
    check(p->setBusArrangements(&stereo,1,&mono,1)==kResultFalse,"reject unsupported stereo downmix layout");
    check(p->setBusArrangements(&stereo, 1, &stereo, 1) == kResultOk, "stereo support"); ++passed;
    check(p->setActive(true) == kResultOk, "activation");
    std::array<float, 128> left{}, right{}, outL{}, outR{};
    left.fill(.25f); right.fill(-.25f);
    float* input[]{left.data(), right.data()}; float* output[]{outL.data(), outR.data()};
    AudioBusBuffers in{}, out{}; in.numChannels = out.numChannels = 2; in.channelBuffers32 = input; out.channelBuffers32 = output;
    ProcessData data{}; data.symbolicSampleSize = kSample32; data.numSamples = 128; data.numInputs = data.numOutputs = 1; data.inputs = &in; data.outputs = &out;
    for (int block = 0; block < 40; ++block) check(p->process(data) == kResultOk, "render");
    check(std::abs(outL[100] - .25f) < 1e-6 && std::abs(outR[100] + .25f) < 1e-6, "bypass output after latency"); ++passed;
    ParameterChanges changes; int32 index = 0;
    auto* queue = changes.addParameterData(Depth, index); queue->addPoint(17, .9, index);
    data.inputParameterChanges = &changes; p->process(data); data.inputParameterChanges = nullptr;
    MemoryStream automated; p->getState(&automated); rewind(automated); c.setComponentState(&automated);
    check(std::abs(c.getParamNormalized(Depth) - .9) < 1e-12, "automation is saved in project state"); ++passed;
    // Real concurrent project restore against audio: catches data races in shared controls.
    std::thread restore([&] {
        for (int i = 0; i < 500; ++i) { MemoryStream s; auto x = defaults(); x[Depth] = (i % 101) / 100.; state(s, x); p->setState(&s); }
    });
    bool finite = true;
    for (int i = 0; i < 200; ++i) {
        p->process(data);
        for (auto x : outL) finite = finite && std::isfinite(x) && std::abs(x) < 8;
    }
    restore.join(); check(finite, "concurrent restore produces finite audio"); ++passed;
    data.numSamples = 0; data.numOutputs = data.numInputs = 0; data.inputs = data.outputs = nullptr;
    check(p->process(data) == kResultOk, "zero sample parameter flush"); ++passed;
    p->setActive(false);
    // Exercise actual host tempo, sample-offset division automation and transport jumps.
    for(int model=0;model<int(circuits.size());++model){
        auto patch=defaults();patch[Model]=model/13.;patch[Sync]=1;patch[Division]=7./17.;patch[Mix]=1;
        MemoryStream st;state(st,patch);p->setState(&st);p->setActive(true);
        Engine reference;reference.reset(48000,patch);
        data.numSamples=128;data.numInputs=data.numOutputs=1;data.inputs=&in;data.outputs=&out;
        ProcessContext context{};context.state=ProcessContext::kPlaying|ProcessContext::kTempoValid|ProcessContext::kProjectTimeMusicValid;
        context.sampleRate=48000;data.processContext=&context;
        for(int block=0;block<12;++block){
            context.tempo=block<6?90:140;context.projectTimeMusic=(block<8?3.25:21.75)+block*.1;
            ParameterChanges divChange;int32 idx=0;
            if(block==4){auto* q=divChange.addParameterData(Division,idx);q->addPoint(37,11./17.,idx);data.inputParameterChanges=&divChange;}
            for(int i=0;i<128;++i)left[i]=float(.2*std::sin((block*128+i)*.11));
            p->process(data);
            for(int i=0;i<128;++i){
                if(block==4&&i==37)patch[Division]=11./17.;
                auto expected=reference.process(left[i],right[i],patch,context.tempo,context.projectTimeMusic+i*context.tempo/(60.*48000));
                check(std::abs(outL[i]-expected[0])<1e-6,"host BPM/grid and division automation audio");
            }
            data.inputParameterChanges=nullptr;
        }
        p->setActive(false);++passed;
    }
    p->terminate(); c.terminate();
    std::cout << passed << " processor/state checks passed\n";
    return 0;
} catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
