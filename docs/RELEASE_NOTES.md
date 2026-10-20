First Windows x64 VST3 prototype of Jerzy Clone Collection, with an EHX-inspired original vector GUI.

Unzip and copy the entire `JerzyCloneCollection.vst3` directory to `C:\Program Files\Common Files\VST3`, then rescan effects in your DAW.

Validation of the downloadable MinGW build: official Steinberg validator under Wine 47 passed / 0 failed; 89 DSP scenarios; 10 processor/state scenarios; actual DLL GUI attachment, knobs, reset, bypass and Filter Matrix checked. Sources, validation report and GUI screenshot are included in the ZIP.

The models are behavioural prototypes, not hardware-calibrated circuit reconstructions. Some revision profiles share DSP settings. Stereo Mistress Inspired is an interpretation without a circuit schematic. Native Windows/FL Studio compatibility remains to be confirmed; Wine tests do not establish it. See docs/MODELLING.md and docs/VALIDATION.md.
