#include "Plugin.h"
#include "pluginterfaces/base/ipluginbase.h"
#include <windows.h>
#include <windowsx.h>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace jerzy;
static void check(bool c, const char* m) { if (!c) throw std::runtime_error(m); }
static void pump() { MSG m{}; while (PeekMessage(&m, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&m); DispatchMessage(&m); } }
static void click(HWND h, int x, int y) {
    SendMessage(h, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    SendMessage(h, WM_LBUTTONUP, 0, MAKELPARAM(x, y)); pump();
}
static void screenshot(HWND h, const char* path) {
    constexpr int w = 760, height = 620;
    HDC dc = GetDC(h), memory = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, w, height); auto previous = SelectObject(memory, bmp);
    RedrawWindow(h, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    check(BitBlt(memory, 0, 0, w, height, dc, 0, 0, SRCCOPY) != 0, "capture editor");
    SelectObject(memory, previous);
    BITMAPINFO info{}; info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); info.bmiHeader.biWidth = w;
    info.bmiHeader.biHeight = height; info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    std::vector<unsigned char> pixels(w * height * 4);
    check(GetDIBits(dc, bmp, 0, height, pixels.data(), &info, DIB_RGB_COLORS) != 0, "read editor pixels");
    BITMAPFILEHEADER header{}; header.bfType = 0x4d42; header.bfOffBits = sizeof(header) + sizeof(BITMAPINFOHEADER);
    header.bfSize = header.bfOffBits + static_cast<DWORD>(pixels.size());
    FILE* file = std::fopen(path, "wb"); check(file != nullptr, "write screenshot");
    std::fwrite(&header, sizeof(header), 1, file); std::fwrite(&info.bmiHeader, sizeof(info.bmiHeader), 1, file);
    std::fwrite(pixels.data(), 1, pixels.size(), file); std::fclose(file);
    DeleteObject(bmp); DeleteDC(memory); ReleaseDC(h, dc);
}
int main(int argc, char** argv) try {
    check(argc >= 2, "usage: editor_smoke plugin.vst3 [screenshot.bmp]");
    HMODULE module = LoadLibraryA(argv[1]); check(module != nullptr, "load Windows VST3 DLL");
    auto init = reinterpret_cast<bool(*)()>(GetProcAddress(module, "InitDll"));
    auto exit = reinterpret_cast<bool(*)()>(GetProcAddress(module, "ExitDll"));
    auto factoryFunction = reinterpret_cast<IPluginFactory*(*)()>(GetProcAddress(module, "GetPluginFactory"));
    check(init && exit && factoryFunction && init(), "module entry points");
    auto* factory = factoryFunction(); IEditController* c = nullptr;
    check(factory->createInstance(controllerId, IEditController::iid, reinterpret_cast<void**>(&c)) == kResultOk && c, "instantiate real DLL controller");
    check(c->initialize(nullptr) == kResultOk, "initialize GUI controller");
    auto* view = c->createView(ViewType::kEditor); check(view, "custom pedal editor exists");
    HWND parent = CreateWindowExA(0, "STATIC", "Jerzy Clone Collection GUI test", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        0, 0, 800, 680, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    check(parent, "create test host window");
    check(view->attached(parent, kPlatformTypeHWND) == kResultOk, "attach custom GUI"); pump();
    HWND child = FindWindowExA(parent, nullptr, "JerzyCloneCollectionEditor010", nullptr); check(child, "editor child window");
    SendMessage(child,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(160,191));
    SendMessage(child,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(160,131));
    SendMessage(child,WM_LBUTTONUP,0,MAKELPARAM(160,131));
    check(c->getParamNormalized(Rate)>.5,"rate knob drag");
    click(child,380,534);check(c->getParamNormalized(Bypass)==1,"footswitch bypass");
    click(child,380,534);check(c->getParamNormalized(Bypass)==0,"footswitch enable");
    click(child,160,505);check(c->getParamNormalized(Filter)==1,"filter matrix toggle");
    SendMessage(child,WM_RBUTTONUP,0,MAKELPARAM(160,191));
    check(c->getParamNormalized(Rate)==defaults()[Rate],"right click reset");
    for(int i=0;i<NumParams;++i){c->setParamNormalized(i,defaults()[i]);}
    if (argc >= 3) screenshot(child, argv[2]);
    check(view->removed() == kResultOk, "detach GUI"); view->release(); DestroyWindow(parent);
    c->terminate(); c->release(); factory->release(); check(exit(), "unload module"); FreeLibrary(module);
    std::cout << "Windows GUI attachment, knob, reset, bypass and filter checks passed\n"; return 0;
} catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
