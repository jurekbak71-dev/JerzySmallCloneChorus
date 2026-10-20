# Validation — 0.1.0

Current cloud build, 2026-10-20. Native GNU C++ 14; Windows x64 MinGW GCC 14, static C++ runtime; official Steinberg SDK 3.8.1 pinned in README.

- Native Linux: DSP suite, 89 scenarios (14 profiles × 6 sample rates plus dry, bypass, stereo, Filter Matrix and model switching).
- Native Linux: 10 processor/state scenarios (persistence, invalid/truncated state rejection, bus layouts, bypass, automation, concurrent restore, zero-sample flush).
- Native Linux VST3: Steinberg validator, 47 passed / 0 failed.
- Windows DSP under Wine 10: 89 scenarios passed.
- Windows processor/state under Wine 10: 10 scenarios passed.
- Windows GUI under Wine/Xvfb: actual VST3 loaded; editor attached/detached; rate drag, right-click reset, bypass on/off and Filter Matrix passed. `gui.png` captures the actual editor.

Final Windows x64 bundle: official Steinberg validator under Wine, **47 passed / 0 failed**, exit status 0. The earlier flat DLL invocation failed only the bundle-structure check; final delivery uses the validated folder structure. PE imports contain only GDI32, KERNEL32, msvcrt, ole32 and USER32. No Wine or zlib files are required by the delivered plugin. Windows and FL Studio native testing, audio-device latency, and comparison against hardware remain untested. Wine GUI results do not establish native DAW compatibility. Numerical stability tests do not establish authenticity of the analogue models; see MODELLING.md.
