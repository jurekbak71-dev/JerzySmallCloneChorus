#include "Plugin.h"
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include "public.sdk/source/common/pluginview.h"
#include <cstdio>
#include <cstring>
namespace jerzy {
using namespace Steinberg;
class PedalEditor final:public CPluginView {
 Controller* controller; HWND window=nullptr; HINSTANCE instance=nullptr;
 int drag=-1,startY=0;double startValue=0;
 static constexpr int width=760,height=620;
 double value(int p){return controller->getParamNormalized(p);}
 void change(int id,double v,bool gesture=true){if(gesture)controller->beginEdit(id);v=clean(v);if(id==Division)v=double(divisionIndex(v))/(divisions.size()-1);controller->setParamNormalized(id,v);controller->performEdit(id,v);if(gesture)controller->endEdit(id);InvalidateRect(window,nullptr,FALSE);}
 static void fill(HDC dc,int x,int y,int w,int h,COLORREF c,int radius=0){auto b=CreateSolidBrush(c);auto old=SelectObject(dc,b);auto pen=SelectObject(dc,GetStockObject(NULL_PEN));if(radius)RoundRect(dc,x,y,x+w,y+h,radius,radius);else Rectangle(dc,x,y,x+w,y+h);SelectObject(dc,pen);SelectObject(dc,old);DeleteObject(b);}
 static void circle(HDC dc,int x,int y,int r,COLORREF c){auto b=CreateSolidBrush(c);auto old=SelectObject(dc,b);auto pen=SelectObject(dc,GetStockObject(NULL_PEN));Ellipse(dc,x-r,y-r,x+r,y+r);SelectObject(dc,pen);SelectObject(dc,old);DeleteObject(b);}
 static void line(HDC dc,int x,int y,int xx,int yy,COLORREF c,int w=1){auto p=CreatePen(PS_SOLID,w,c);auto old=SelectObject(dc,p);MoveToEx(dc,x,y,nullptr);LineTo(dc,xx,yy);SelectObject(dc,old);DeleteObject(p);}
 static void text(HDC dc,int x,int y,int w,const char* s,int size,COLORREF color=RGB(243,240,229),bool bold=true){auto font=CreateFontA(size,0,0,0,bold?FW_BOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Segoe UI");auto old=SelectObject(dc,font);SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);RECT rect{x,y,x+w,y+size+7};DrawTextA(dc,s,-1,&rect,DT_CENTER|DT_SINGLELINE|DT_VCENTER);SelectObject(dc,old);DeleteObject(font);}
 void knob(HDC dc,int id,int x,int y,const char* label,COLORREF accent){
  for(int j=0;j<=10;++j){double a=(-225+j*27)*pi/180;line(dc,x+int(54*cos(a)),y+int(54*sin(a)),x+int(59*cos(a)),y+int(59*sin(a)),RGB(161,161,157),2);}
  circle(dc,x+2,y+5,47,RGB(5,5,6));circle(dc,x,y,46,RGB(114,116,115));circle(dc,x,y,44,RGB(25,27,29));
  for(int j=0;j<36;++j){double a=j*2*pi/36;line(dc,x+int(39*cos(a)),y+int(39*sin(a)),x+int(43*cos(a)),y+int(43*sin(a)),RGB(57,59,61),2);}
  circle(dc,x,y,36,RGB(39,41,43));circle(dc,x-2,y-3,32,RGB(44,46,48));
  double angle=(-225+270*value(id==Rate&&value(Sync)>=.5?Division:id))*pi/180;line(dc,x+int(18*cos(angle)),y+int(18*sin(angle)),x+int(33*cos(angle)),y+int(33*sin(angle)),RGB(244,240,224),3);
  text(dc,x-75,y+65,150,label,15);
  Vst::String128 display{};int shown=id==Rate&&value(Sync)>=.5?Division:id;controller->getParamStringByValue(shown,value(shown),display);char ascii[128]{};for(int i=0;i<127&&display[i];++i)ascii[i]=char(display[i]);text(dc,x-75,y+85,150,ascii,13,accent,false);
 }
 void paint(){PAINTSTRUCT ps;auto target=BeginPaint(window,&ps);auto dc=CreateCompatibleDC(target);auto bmp=CreateCompatibleBitmap(target,width,height);auto previous=SelectObject(dc,bmp);
  int model=modelIndex(value(Model)); bool clone=model<3; COLORREF accent=clone?RGB(172,114,232):RGB(238,99,156);
  fill(dc,0,0,width,height,RGB(19,21,23));fill(dc,12,10,736,600,RGB(106,108,108),30);fill(dc,16,12,728,592,RGB(187,188,181),28);
  unsigned random=42;for(int i=0;i<12000;++i){random=1664525u*random+1013904223u;int x=20+random%720;random=1664525u*random+1013904223u;int y=16+random%580;int c=145+(random&63);SetPixelV(dc,x,y,RGB(c,c,c-4));}
  fill(dc,32,28,696,560,accent,22);fill(dc,36,32,688,552,RGB(18,20,23),18);
  for(int x:{50,710})for(int y:{46,570}){circle(dc,x,y,6,RGB(103,107,108));line(dc,x-3,y+2,x+3,y-2,RGB(26,28,29),2);}
  text(dc,70,42,180,"J E R Z Y   A U D I O",14,RGB(193,194,185));text(dc,495,42,180,"BBD MODULATION / 01",12,RGB(154,156,151),false);
  fill(dc,100,76,560,42,RGB(38,39,43),8);text(dc,115,83,520,circuits[model].name,21);text(dc,622,84,25,"v",18,accent);
  knob(dc,Rate,160,191,"RATE",accent);knob(dc,Depth,380,191,clone?"DEPTH":"RANGE",accent);knob(dc,Color,600,191,clone?"COLOR (NOT USED)":"COLOR / FEEDBACK",clone?RGB(100,103,105):accent);
  knob(dc,Mix,160,363,"CHORUS / VIBRATO",accent);knob(dc,Level,380,363,"OUTPUT",accent);knob(dc,Stereo,600,363,"STEREO WIDTH",accent);
  fill(dc,86,485,185,42,value(Filter)>=.5?accent:RGB(41,43,46),7);text(dc,86,493,185,value(Filter)>=.5?"FILTER MATRIX: ON":"FILTER MATRIX: OFF",13,value(Filter)>=.5?RGB(18,20,23):RGB(216,215,204));
  fill(dc,86,535,90,25,value(Sync)>=.5?accent:RGB(41,43,46),5);text(dc,86,539,90,value(Sync)>=.5?"BPM SYNC":"FREE",12,value(Sync)>=.5?RGB(18,20,23):RGB(216,215,204));
  fill(dc,181,535,90,25,RGB(41,43,46),5);text(dc,181,539,90,divisions[divisionIndex(value(Division))].label,12,accent);
  circle(dc,380,482,10,RGB(6,6,7));circle(dc,380,482,6,value(Bypass)<.5?accent:RGB(61,38,40));
  circle(dc,380,536,28,RGB(9,10,11));circle(dc,380,534,26,RGB(173,174,168));circle(dc,380,533,21,RGB(82,87,89));circle(dc,380,531,17,RGB(220,220,209));circle(dc,378,528,12,RGB(162,166,161));
  text(dc,480,483,205,"CLONE COLLECTION",19,accent);text(dc,480,509,205,"CHORUS  /  VIBRATO  /  FLANGER",10,RGB(197,198,188),false);
  text(dc,70,566,225,"DRAG KNOBS  /  SHIFT: FINE",10,RGB(142,146,144),false);text(dc,465,566,225,"RIGHT CLICK KNOB: RESET",10,RGB(142,146,144),false);
  BitBlt(target,0,0,width,height,dc,0,0,SRCCOPY);SelectObject(dc,previous);DeleteObject(bmp);DeleteDC(dc);EndPaint(window,&ps);
 }
 int hit(int x,int y){const int ids[]{Rate,Depth,Color,Mix,Level,Stereo};const int xs[]{160,380,600,160,380,600},ys[]{191,191,191,363,363,363};for(int i=0;i<6;++i)if(std::hypot(double(x-xs[i]),double(y-ys[i]))<50)return ids[i]==Color&&modelIndex(value(Model))<3?-1:ids[i];return -1;}
 void beatMenu(){auto menu=CreatePopupMenu();for(size_t i=0;i<divisions.size();++i)AppendMenuA(menu,MF_STRING|(divisionIndex(value(Division))==int(i)?MF_CHECKED:0),200+i,divisions[i].label);POINT pt{181,560};ClientToScreen(window,&pt);int result=TrackPopupMenu(menu,TPM_RETURNCMD,pt.x,pt.y,0,window,nullptr);DestroyMenu(menu);if(result>=200&&result<200+int(divisions.size()))change(Division,double(result-200)/(divisions.size()-1));}
 void models(){auto menu=CreatePopupMenu();for(size_t i=0;i<circuits.size();++i)AppendMenuA(menu,MF_STRING|(modelIndex(value(Model))==int(i)?MF_CHECKED:0),100+i,circuits[i].name);POINT pt{100,118};ClientToScreen(window,&pt);int result=TrackPopupMenu(menu,TPM_RETURNCMD,pt.x,pt.y,0,window,nullptr);DestroyMenu(menu);if(result>=100&&result<100+int(circuits.size()))change(Model,double(result-100)/(circuits.size()-1));}
 LRESULT message(UINT msg,WPARAM wp,LPARAM lp){int x=GET_X_LPARAM(lp),y=GET_Y_LPARAM(lp);switch(msg){
  case WM_PAINT:paint();return 0;case WM_ERASEBKGND:return 1;case WM_TIMER:InvalidateRect(window,nullptr,FALSE);return 0;
  case WM_LBUTTONDOWN:SetFocus(window);if(y>=535&&y<=560&&x>=86&&x<=176){change(Sync,value(Sync)<.5?1:0);return 0;}if(y>=535&&y<=560&&x>=181&&x<=271){beatMenu();return 0;}if(y>=76&&y<=118&&x>=100&&x<=660){models();return 0;}if(y>=485&&y<=527&&x>=86&&x<=271){change(Filter,value(Filter)<.5?1:0);return 0;}if(std::hypot(double(x-380),double(y-534))<32){change(Bypass,value(Bypass)<.5?1:0);return 0;}drag=hit(x,y);if(drag==Rate&&value(Sync)>=.5)drag=Division;if(drag>=0){startY=y;startValue=value(drag);controller->beginEdit(drag);SetCapture(window);}return 0;
  case WM_MOUSEMOVE:if(drag>=0)change(drag,startValue+(startY-y)*((wp&MK_SHIFT)?.0005:.004),false);return 0;
  case WM_LBUTTONUP:case WM_CAPTURECHANGED:if(drag>=0){controller->endEdit(drag);drag=-1;}if(msg==WM_LBUTTONUP)ReleaseCapture();return 0;
  case WM_RBUTTONUP:{int id=hit(x,y);if(id==Rate&&value(Sync)>=.5)id=Division;if(id>=0)change(id,defaults()[id]);return 0;}
  case WM_MOUSEWHEEL:{POINT pt{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};ScreenToClient(window,&pt);int id=hit(pt.x,pt.y);if(id==Rate&&value(Sync)>=.5)id=Division;if(id>=0){double step=id==Division?1./(divisions.size()-1):.02;change(id,value(id)+(GET_WHEEL_DELTA_WPARAM(wp)>0?step:-step));}return 0;}
 }return DefWindowProcA(window,msg,wp,lp);}
 static LRESULT CALLBACK procedure(HWND h,UINT m,WPARAM w,LPARAM l){auto* self=reinterpret_cast<PedalEditor*>(GetWindowLongPtrA(h,GWLP_USERDATA));if(m==WM_NCCREATE){self=static_cast<PedalEditor*>(reinterpret_cast<CREATESTRUCTA*>(l)->lpCreateParams);self->window=h;SetWindowLongPtrA(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}return self?self->message(m,w,l):DefWindowProcA(h,m,w,l);}
 public:
 explicit PedalEditor(Controller* c):controller(c){controller->addRef();ViewRect r(0,0,width,height);setRect(r);}
 ~PedalEditor()override{removed();controller->release();}
 tresult PLUGIN_API isPlatformTypeSupported(FIDString type)override{return type&&!strcmp(type,kPlatformTypeHWND)?kResultTrue:kResultFalse;}
 tresult PLUGIN_API attached(void* parent,FIDString type)override{if(!parent||window||isPlatformTypeSupported(type)!=kResultTrue)return kResultFalse;GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCSTR>(&procedure),&instance);WNDCLASSA wc{};wc.lpfnWndProc=procedure;wc.hInstance=instance;wc.lpszClassName="JerzyCloneCollectionEditor010";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);if(!RegisterClassA(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return kResultFalse;window=CreateWindowExA(0,wc.lpszClassName,"Jerzy Clone Collection",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,width,height,static_cast<HWND>(parent),nullptr,instance,this);if(!window)return kResultFalse;SetTimer(window,1,50,nullptr);systemWindow=parent;return kResultOk;}
 tresult PLUGIN_API removed()override{if(window){KillTimer(window,1);DestroyWindow(window);window=nullptr;UnregisterClassA("JerzyCloneCollectionEditor010",instance);}systemWindow=nullptr;return kResultOk;}
};
IPlugView* createEditor(Controller* c){return new PedalEditor(c);}
}
#else
namespace jerzy {Steinberg::IPlugView* createEditor(Controller*){return nullptr;}}
#endif
