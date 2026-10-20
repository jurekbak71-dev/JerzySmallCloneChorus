#pragma once
#include "Dsp.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include <atomic>
namespace jerzy {
inline const Steinberg::FUID processorId(0x7D3E7421, 0x6C284B71, 0xB32AFC08, 0xC4012901);
inline const Steinberg::FUID controllerId(0xA789D0B3, 0x912F4D01, 0x985CEB70, 0x72514409);
class Processor final : public Steinberg::Vst::AudioEffect {
    std::array<std::atomic<double>, NumParams> values;
    Engine engine;
public:
    Processor();
    static Steinberg::FUnknown* create(void*) { return static_cast<Steinberg::Vst::IAudioProcessor*>(new Processor); }
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown*) override;
    Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement*, Steinberg::int32, Steinberg::Vst::SpeakerArrangement*, Steinberg::int32) override;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32) override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup&) override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData&) override;
    Steinberg::uint32 PLUGIN_API getLatencySamples() override { return 0; }
    Steinberg::uint32 PLUGIN_API getTailSamples() override { return static_cast<Steinberg::uint32>(processSetup.sampleRate * 5.); }
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream*) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream*) override;
};
class Controller final : public Steinberg::Vst::EditControllerEx1 {
public:
    static Steinberg::FUnknown* create(void*) { return static_cast<Steinberg::Vst::IEditController*>(new Controller); }
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown*) override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream*) override;
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString) override;
    Steinberg::tresult PLUGIN_API getParamStringByValue(Steinberg::Vst::ParamID, Steinberg::Vst::ParamValue, Steinberg::Vst::String128) override;
    Steinberg::tresult PLUGIN_API getParamValueByString(Steinberg::Vst::ParamID, Steinberg::Vst::TChar*, Steinberg::Vst::ParamValue&) override;
};
Steinberg::IPlugView* createEditor(Controller*);
}
