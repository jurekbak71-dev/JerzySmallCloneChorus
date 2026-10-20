# Jerzy Clone Collection 0.1.0

Pierwszy prototyp Windows x64 VST3: chorus, vibrato i flanger z panelem inspirowanym wyglądem klasycznych kostek EHX. Własna grafika wektorowa i oznaczenia Jerzy Audio. Brak powiązania z Electro-Harmonix.

## Pobierz

[Windows x64 VST3 — ZIP v0.1.0](https://github.com/jurekbak71-dev/JerzySmallCloneChorus/releases/download/v0.1.0/JerzyCloneCollection-Win64.zip)

[GitHub Releases](https://github.com/jurekbak71-dev/JerzySmallCloneChorus/releases) · [Podgląd GUI](docs/gui.png) · [Sumy SHA-256](releases/v0.1.0/SHA256SUMS.txt)

## Instalacja w Windows / FL Studio

1. Rozpakuj `JerzyCloneCollection-Win64.zip`.
2. Skopiuj cały folder `JerzyCloneCollection.vst3` do `C:\Program Files\Common Files\VST3`.
3. W FL Studio otwórz Plugin Manager i uruchom ponowne wyszukiwanie.
4. Dodaj **Jerzy Clone Collection** jako efekt na mikserze.

Zachowaj strukturę `Contents/x86_64-win`. To VST3, nie VST2 DLL.

## Obsługa

- Kliknij nazwę obwodu na górze, aby wybrać profil.
- RATE: 0,05–8 Hz. DEPTH / RANGE: głębokość modulacji.
- CHORUS / VIBRATO: 0% dry, 50% chorus/flanger, 100% wet/vibrato.
- COLOR / FEEDBACK: sprzężenie w profilach flangera; w trzech profilach chorusowych nieaktywne.
- OUTPUT: −12 do +12 dB, środek = 0 dB.
- STEREO WIDTH: opcjonalne rozszerzenie z przeciwną fazą modulacji prawego kanału. 0% zachowuje wspólną modulację. Nie jest to model przesłanej płytki stereo Clone Theory.
- FILTER MATRIX: zatrzymuje LFO w aktualnej fazie. W profilach flangera Range zmienia statyczny czas opóźnienia. To uproszczenie oryginalnego przełącznika.
- Metalowy footswitch: bypass z wygładzeniem, dioda pokazuje aktywację.
- Gałki: ruch pionowy, Shift = dokładnie, prawy klik = reset, kółko myszy = krok.

Mono, mono → stereo, stereo; 32/64-bit float; 8–384 kHz. Automatyka z offsetami próbek, zapis stanu wszystkich dziewięciu parametrów. Brak dodatkowej raportowanej latencji; samo opóźnienie modulowanego toru wet jest częścią efektu.

## Status odwzorowania

**To prototyp behawioralny do odsłuchu, nie ukończona emulacja element po elemencie.** Profile wykorzystują modulowane opóźnienie `N/(2*fclock)`, filtry, preemfazę, saturację i sprzężenie. Nie symulują fizycznego przenoszenia ładunku, przecieku zegara, rozrzutu tranzystorów ani szumu BBD. Częstotliwości zegara, zakresy, headroom i charakterystyki filtrów wymagają pomiarów referencyjnych. Nie wszystkie wartości pochodzą ze schematów.

14 pozycji wyboru obejmuje Small Clone, Clone Theory SAD/MN, Electric Mistress V2–V6, Deluxe V1–V5 oraz Stereo Mistress Inspired. **Nie są to 14 niezależnie zweryfikowanych rekonstrukcji.** Niektóre rewizje współdzielą identyczne ustawienia DSP (Deluxe V1/V2 oraz V4/V5); zachowują oddzielne nazwy do dalszej kalibracji. Stereo Mistress jest autorską interpretacją zdjęcia, bez schematu. Szczegóły: [docs/MODELLING.md](docs/MODELLING.md).

## Budowanie

SDK Steinberga 3.8.1, commit `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96` z submodułami; CMake ≥3.25, C++17.

Pobranie przypiętego SDK:

```sh
git clone https://github.com/steinbergmedia/vst3sdk.git C:/dependencies/vst3sdk
git -C C:/dependencies/vst3sdk checkout 3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96
git -C C:/dependencies/vst3sdk submodule update --init --recursive
```

Windows, terminal deweloperski Visual Studio 2022:

```powershell
cmake -S . -B build -A x64 -DVST3_SDK_ROOT=C:/dependencies/vst3sdk
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Linux / przygotowane środowisko chmurowe:

```sh
bash tools/build_cloud.sh
```

Wykorzystuj istniejący checkout. Zadanie chmurowe jest izolowane; nie twórz worktree. Natywny Linux służy do testów DSP/hosta; własny edytor jest dostępny na Windows. Cross-kompilacja nie zmienia plików SDK. Pomija wyłącznie nieużywany pomocnik DataExchange SDK, którego `aligned_alloc` nie jest dostępny w MSVCRT.

Walidacja konkretnego wydania: [docs/VALIDATION.md](docs/VALIDATION.md).
