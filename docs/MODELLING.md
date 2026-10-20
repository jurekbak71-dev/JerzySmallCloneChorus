# Zakres modelu 0.1.0

Materiały użytkownika: Small Clone + depth mod (21.02.2019), Clone Theory EH1317 SAD1024A, Clone Theory MN3007 (Scruffie rev 1.0 i layouty IvIark), Electric Mistress V2–V6 i Deluxe V1–V5 (Ralf Metzger), fabryczny EC1000, fotografie Small Clone EH4600 i Stereo Electric Mistress. Oryginalne rysunki nie są dołączane do dystrybucji.

Kod to nowa, uproszczona implementacja DSP. Schematy wskazują architekturę, lecz bieżący DSP nie rozwiązuje ich sieci elektrycznych. Nie należy przedstawiać wyniku jako pomierzonej rekonstrukcji urządzeń.

| Rodzina | Bieżąca reprezentacja | Braki do odwzorowania |
|---|---|---|
| Small Clone | sinusoidalna modulacja krótkiego opóźnienia, 1024 efektywne stopnie, mix dry/wet | rzeczywisty kształt LFO, dwupozycyjny Depth, filtry aktywne i charakterystyka zegara |
| Clone Theory SAD/MN | dwa profile barwy z 1024 efektywnymi stopniami; chorus/vibrato mix | topologia dwóch sekcji SAD, kalibracja MN, przełącznik Vib/Flange i Edge, położenie domieszki dry |
| Electric Mistress 18 V V2–V5 | 512 efektywnych stopni (równoległe sekcje SAD), trójkątny LFO, feedback | pomiary częstotliwości zegara, zakresu Range, tolerancji i filtrów poszczególnych rewizji |
| Electric Mistress V6 9 V | osobny profil barwy i zakresu | dokładne przeliczenie układu LM324/LM311 i headroomu |
| Deluxe V1–V3 SAD | filtracja i feedback odmienne od zwykłego Mistress | pełna aktywna pre/deemfaza, ustawienia trymrów i kalibracja każdej rewizji |
| Deluxe V4/V5 RD5106A | 1024-stopniowy profil i inna filtracja | zegar, charakter RD5106A, tor mieszania i filtry |
| Stereo Mistress Inspired | dwukanałowa modulacja | brak schematu; nie odwzorowuje rzeczywistej topologii tego urządzenia |

Wszystkie wartości w tablicy `circuits` są na razie założeniami strojenia. Nie skalibrowano ich względem pomiarów strony Mistress Mystery Page. Część sąsiednich profili brzmi podobnie lub identycznie. Nie wprowadzano sztucznych różnic między zasilaniem Deluxe V4/V5 do czystego DSP.

Filtry: uproszczona preemfaza, dwukaskadowy filtr dolnoprzepustowy i usuwanie DC. Saturacja tanh to aproksymacja headroomu, nie model tranzystora. LFO nie odtwarza jeszcze wszystkich asymetrii układów analogowych. Brak oversamplingu: przy mocnym przesterowaniu możliwy aliasing.

Przełączanie modelu wygasza efekt przez 12 ms, czyści linię delay i przywraca efekt przez 12 ms. Chroni przed starym ogonem po zmianie obwodu. Nie jest to fizyczne zachowanie kostki. Bypass płynnie wraca do dry; stan linii dalej pracuje.

Szerokość stereo, płynny mix, output i ciągła głębokość Small Clone są rozszerzeniami wtyczki. Brak dodatkowego szumu i przydźwięku. Brak powiązania z producentem urządzeń. Nazwy identyfikują źródła inspiracji; panel nie zawiera logo EHX.

Schemat Scruffie ma adnotację „NOT FOR COMMERCIAL USE”. Ten prototyp nie stanowi ustalenia warunków komercyjnego wykorzystania materiałów źródłowych. Przed dystrybucją komercyjną trzeba osobno wyjaśnić ich zakres.
