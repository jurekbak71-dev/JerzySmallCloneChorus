# Validation — 0.1.1

Cloud validation, 2026-10-20. GNU C++ 14 / MinGW GCC 14, Steinberg SDK 3.8.1.

- Linux CTest: DSP and processor/state suites passed.
- 594 DSP scenarios: previous coverage plus all 14 models × 18 divisions × 90/140 BPM, negative preroll alignment, Filter Matrix freeze and missing-tempo fallback.
- 25 processor/state scenarios: previous coverage, legacy nine-parameter state migration, new parameter persistence and actual VST3 ProcessContext tempo/grid rendering for every model, including sample-offset division automation, tempo changes and transport jumps.
- Free-mode output: bit-identical against the pre-sync DSP for all 14 models over 48,000 samples per model.
- Windows x64 DSP and processor/state executables under Wine: 594 and 25 scenarios passed.
- Windows actual VST3 GUI under Wine/Xvfb: attachment, knobs, reset, bypass, Filter Matrix, BPM Sync/Free toggle; Rate edits division in Sync without changing the saved Free rate. Screenshot is from the actual editor.
- Linux official Steinberg validator: 47 passed / 0 failed.
- Windows complete bundle official Steinberg validator under Wine: 47 passed / 0 failed, exit status 0.

Native Windows/FL Studio testing and hardware calibration remain unverified. The existing native MSVC CI had a build failure before this tempo-sync change; it is not used as evidence of a passing native Windows build. Numerical tests and Wine do not establish native DAW compatibility or analogue authenticity.
