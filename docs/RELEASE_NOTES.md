Jerzy Clone Collection 0.1.1 — host tempo sync for every model.

Added FREE/BPM SYNC and 18 musical divisions, including dotted and triplet values, from four bars to 1/64. In Sync, Rate edits the division. While playing, the LFO follows VST3 host tempo and musical position, including tempo changes and transport jumps. Filter Matrix still freezes modulation. Both new parameters support host automation and project-state persistence.

Existing parameter IDs and plugin identity are unchanged. v0.1.0 projects load with Sync off. Free-mode audio was compared against v0.1.0 and is bit-identical for all 14 models in the regression probe. The sound models and existing controls were not redesigned.

Install: copy the complete `JerzyCloneCollection.vst3` folder into `C:\Program Files\Common Files\VST3`, replacing the previous version while the DAW is closed, then rescan.

594 DSP scenarios, 25 processor/state scenarios, and the actual Windows GUI tested under Wine. Official Steinberg validator: 47 passed / 0 failed. Native FL Studio testing remains unverified. Circuit models retain the prototype limitations documented in MODELLING.md.
