#include "Plugin.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/main/pluginfactory.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

namespace jerzy {
using namespace Steinberg;
using namespace Steinberg::Vst;
constexpr int32 stateMagic = 0x4A434831;
static bool readState(IBStream* stream, Values& v) {
    if (!stream) return false;
    IBStreamer s(stream, kLittleEndian);
    int32 magic = 0, version = 0;
    if (!s.readInt32(magic) || !s.readInt32(version) || magic != stateMagic || (version != 1 && version != 2)) return false;
    v = defaults();
    for (int i=0;i<(version==1?9:NumParams);++i)
        if (!s.readDouble(v[i]) || !std::isfinite(v[i]) || v[i] < 0 || v[i] > 1) return false;
    return true;
}
Processor::Processor() {
    setControllerClass(controllerId);
    auto initial = defaults();
    for (int i = 0; i < NumParams; ++i) values[i].store(initial[i]);
    processContextRequirements.needTempo().needProjectTimeMusic().needTransportState();
}
tresult PLUGIN_API Processor::initialize(FUnknown* host) {
    auto result = AudioEffect::initialize(host);
    if (result != kResultOk) return result;
    addAudioInput(STR16("Input"), SpeakerArr::kStereo);
    addAudioOutput(STR16("Output"), SpeakerArr::kStereo);
    return kResultOk;
}
tresult PLUGIN_API Processor::setBusArrangements(SpeakerArrangement* ins, int32 ni, SpeakerArrangement* outs, int32 no) {
    if (ni != 1 || no != 1 || !ins || !outs ||
        (ins[0] != SpeakerArr::kMono && ins[0] != SpeakerArr::kStereo) ||
        (outs[0] != SpeakerArr::kMono && outs[0] != SpeakerArr::kStereo) ||
        (ins[0] == SpeakerArr::kStereo && outs[0] == SpeakerArr::kMono)) return kResultFalse;
    getAudioInput(0)->setArrangement(ins[0]);
    getAudioOutput(0)->setArrangement(outs[0]);
    return kResultOk;
}
tresult PLUGIN_API Processor::canProcessSampleSize(int32 size) {
    return size == kSample32 || size == kSample64 ? kResultTrue : kResultFalse;
}
tresult PLUGIN_API Processor::setupProcessing(ProcessSetup& setup) {
    if (!std::isfinite(setup.sampleRate) || setup.sampleRate < 8000 || setup.sampleRate > 384000 || setup.maxSamplesPerBlock < 0)
        return kResultFalse;
    return AudioEffect::setupProcessing(setup);
}
tresult PLUGIN_API Processor::setActive(TBool active) {
    if (active) {
        lastTempo = 120;
        Values p{}; for (int i = 0; i < NumParams; ++i) p[i] = values[i].load(std::memory_order_relaxed);
        engine.reset(processSetup.sampleRate, p);
    }
    return AudioEffect::setActive(active);
}
tresult PLUGIN_API Processor::setState(IBStream* stream) {
    Values p{}; if (!readState(stream, p)) return kResultFalse;
    for (int i = 0; i < NumParams; ++i) values[i].store(p[i], std::memory_order_relaxed);
    return kResultOk;
}
tresult PLUGIN_API Processor::getState(IBStream* stream) {
    if (!stream) return kResultFalse;
    IBStreamer s(stream, kLittleEndian);
    if (!s.writeInt32(stateMagic) || !s.writeInt32(2)) return kResultFalse;
    for (auto& value : values) if (!s.writeDouble(value.load(std::memory_order_relaxed))) return kResultFalse;
    return kResultOk;
}
tresult PLUGIN_API Processor::process(ProcessData& data) {
    if (data.numSamples < 0 || (data.symbolicSampleSize != kSample32 && data.symbolicSampleSize != kSample64)) return kResultFalse;
    Values p{}; for (int i = 0; i < NumParams; ++i) p[i] = values[i].load(std::memory_order_relaxed);
    constexpr int maxQueues = 64;
    std::array<int32, maxQueues> point{};
    const int queues = data.inputParameterChanges ? std::min(maxQueues, data.inputParameterChanges->getParameterCount()) : 0;
    auto apply = [&](int sample) {
        for (int q = 0; q < queues; ++q) {
            auto* queue = data.inputParameterChanges->getParameterData(q);
            if (!queue || queue->getParameterId() >= NumParams) continue;
            int32 offset = 0; ParamValue value = 0;
            while (point[q] < queue->getPointCount() && queue->getPoint(point[q], offset, value) == kResultTrue) {
                if (data.numSamples != 0 && offset > sample) break;
                p[queue->getParameterId()] = clean(value);
                values[queue->getParameterId()].store(clean(value), std::memory_order_relaxed);
                ++point[q];
            }
        }
    };
    if (!data.numSamples) { apply(0); return kResultOk; }
    if (data.numOutputs != 1 || !data.outputs || data.outputs[0].numChannels < 1 || data.outputs[0].numChannels > 2)
        return kResultOk;
    auto& out = data.outputs[0];
    auto* in = data.numInputs > 0 && data.inputs ? &data.inputs[0] : nullptr;
    if(data.processContext && (data.processContext->state&ProcessContext::kTempoValid) &&
       std::isfinite(data.processContext->tempo) && data.processContext->tempo>0)
        lastTempo=data.processContext->tempo;
    const bool grid=data.processContext && (data.processContext->state&ProcessContext::kProjectTimeMusicValid) &&
        (data.processContext->state&ProcessContext::kPlaying) && std::isfinite(data.processContext->projectTimeMusic);
    const double beatStep=lastTempo/(60*processSetup.sampleRate);
    out.silenceFlags = 0;
    for (int32 i = 0; i < data.numSamples; ++i) {
        apply(i);
        std::array<double, 2> input{};
        for (int c = 0; c < 2; ++c) {
            int channel = in && in->numChannels > 0 ? std::min(c, in->numChannels - 1) : 0;
            if (in && in->numChannels > 0 && !(in->silenceFlags & (uint64(1) << channel))) {
                if (data.symbolicSampleSize == kSample32 && in->channelBuffers32 && in->channelBuffers32[channel]) input[c] = in->channelBuffers32[channel][i];
                if (data.symbolicSampleSize == kSample64 && in->channelBuffers64 && in->channelBuffers64[channel]) input[c] = in->channelBuffers64[channel][i];
            }
        }
        auto y = engine.process(input[0], input[1], p, lastTempo, grid?data.processContext->projectTimeMusic+i*beatStep:NAN);
        if(out.numChannels==1) y[0]=(y[0]+y[1])*.5;
        for (int c = 0; c < out.numChannels; ++c) {
            if (data.symbolicSampleSize == kSample32 && out.channelBuffers32 && out.channelBuffers32[c]) out.channelBuffers32[c][i] = static_cast<float>(y[c]);
            if (data.symbolicSampleSize == kSample64 && out.channelBuffers64 && out.channelBuffers64[c]) out.channelBuffers64[c][i] = y[c];
        }
    }
    return kResultOk;
}
tresult PLUGIN_API Controller::initialize(FUnknown* host) {
    auto result = EditControllerEx1::initialize(host);
    if (result != kResultOk) return result;
    addUnit(new Unit(STR16("Clone Collection"), kRootUnitId, kNoParentUnitId));
    const TChar* titles[]{STR16("Rate"),STR16("Depth / Range"),STR16("Chorus / Vibrato Mix"),STR16("Color / Feedback"),STR16("Output"),STR16("Circuit"),STR16("Stereo Width"),STR16("Filter Matrix"),STR16("Bypass"),STR16("Tempo Sync"),STR16("Beat Division")};
    auto initial = defaults();
    for (int i = 0; i < NumParams; ++i) {
        int flags = ParameterInfo::kCanAutomate;
        if (i == Bypass) flags |= ParameterInfo::kIsBypass;
        const int steps = i == Model ? int(circuits.size()-1) : i == Division ? int(divisions.size()-1) : i == Filter || i == Bypass || i == Sync ? 1 : 0;
        parameters.addParameter(titles[i], nullptr, steps, initial[i], flags, i);
    }
    return kResultOk;
}
tresult PLUGIN_API Controller::setComponentState(IBStream* stream) {
    Values p{}; if (!readState(stream, p)) return kResultFalse;
    for (int i = 0; i < NumParams; ++i) setParamNormalized(i, p[i]);
    return kResultOk;
}
IPlugView* PLUGIN_API Controller::createView(FIDString type) {
    return type && std::strcmp(type, ViewType::kEditor) == 0 ? createEditor(this) : nullptr;
}
tresult PLUGIN_API Controller::getParamStringByValue(ParamID id, ParamValue v, String128 text) {
    char buffer[128]{}; v = clean(v);
    switch (id) {
    case Rate: std::snprintf(buffer,sizeof(buffer),"%.2f Hz",rateHz(v)); break;
    case Level: std::snprintf(buffer,sizeof(buffer),"%+.1f dB",24*v-12); break;
    case Model: std::snprintf(buffer,sizeof(buffer),"%s",circuits[modelIndex(v)].name); break;
    case Division: std::snprintf(buffer,sizeof(buffer),"%s",divisions[divisionIndex(v)].label); break;
    case Bypass: case Filter: case Sync: std::snprintf(buffer,sizeof(buffer),"%s",v>=.5?"On":"Off"); break;
    default: std::snprintf(buffer,sizeof(buffer),"%.0f %%",100*v); break;
    }
    UString(text, 128).fromAscii(buffer);
    return kResultTrue;
}
tresult PLUGIN_API Controller::getParamValueByString(ParamID id, TChar* input, ParamValue& v) {
    if (!input || id >= NumParams) return kResultFalse;
    char text[128]{};
    for (int i = 0; i < 127 && input[i]; ++i) text[i] = char(input[i]);
    const std::string label(text);
    if(id==Model){for(size_t i=0;i<circuits.size();++i)if(label==circuits[i].name){v=double(i)/(circuits.size()-1);return kResultTrue;}return kResultFalse;}
    if(id==Division){for(size_t i=0;i<divisions.size();++i)if(label==divisions[i].label){v=double(i)/(divisions.size()-1);return kResultTrue;}return kResultFalse;}
    if(id==Filter||id==Bypass||id==Sync){if(label=="On"||label=="1")v=1;else if(label=="Off"||label=="0")v=0;else return kResultFalse;return kResultTrue;}
    char* end=nullptr; double n=std::strtod(text,&end); if(end==text||!std::isfinite(n))return kResultFalse;
    if(id==Rate){if(n<=0)return kResultFalse;v=clean(std::log(n/.05)/std::log(160.));}
    else if(id==Level)v=clean((n+12)/24);else v=clean(n/100);
    return kResultTrue;
}
}
using namespace Steinberg;
using namespace Steinberg::Vst;
bool InitModule() { return true; }
bool DeinitModule() { return true; }
BEGIN_FACTORY_DEF("Jerzy Audio", "https://github.com/jurekbak71-dev/JerzySmallCloneChorus", "")
DEF_CLASS2(INLINE_UID_FROM_FUID(jerzy::processorId), PClassInfo::kManyInstances, kVstAudioEffectClass,
    "Jerzy Clone Collection", Vst::kDistributable, "Fx|Modulation", JERZY_VERSION, kVstVersionString, jerzy::Processor::create)
DEF_CLASS2(INLINE_UID_FROM_FUID(jerzy::controllerId), PClassInfo::kManyInstances, kVstComponentControllerClass,
    "Jerzy Clone Collection Controller", 0, "", JERZY_VERSION, kVstVersionString, jerzy::Controller::create)
END_FACTORY
