#include <windows.h>
#include <shellapi.h>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>
#include "simulation.hpp"

using namespace antsim;
static Simulation sim(uint32_t(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
static std::vector<uint32_t> pixels(WORLD_W*WORLD_H);
static bool paused=false, showTrails=true, showHelp=true;
static int speed=1;
static BITMAPINFO bmi{};

static void put(int x,int y,uint32_t c){if(unsigned(x)<WORLD_W&&unsigned(y)<WORLD_H)pixels[y*WORLD_W+x]=c;}
static void blend(int x,int y,int r,int g,int b){
 if(unsigned(x)>=WORLD_W||unsigned(y)>=WORLD_H)return; uint32_t &p=pixels[y*WORLD_W+x];
 int pb=p&255,pg=(p>>8)&255,pr=(p>>16)&255; p=uint32_t(std::min(255,pb+b)|std::min(255,pg+g)<<8|std::min(255,pr+r)<<16);
}
static void disk(int cx,int cy,int rad,uint32_t c){for(int y=-rad;y<=rad;y++)for(int x=-rad;x<=rad;x++)if(x*x+y*y<=rad*rad)put(cx+x,cy+y,c);}

static void renderWorld(){
 std::fill(pixels.begin(),pixels.end(),0x00100c0a);
 if(showTrails){
  for(int gy=0;gy<GRID_H;gy++)for(int gx=0;gx<GRID_W;gx++){
   int i=gy*GRID_W+gx; int f=std::min(35,int(sim.foodTrail[i]*1.4f)),h=std::min(24,int(sim.homeTrail[i]));
   if(!f&&!h)continue; int px=gx*4,py=gy*4;for(int yy=0;yy<4;yy++)for(int xx=0;xx<4;xx++)blend(px+xx,py+yy,h,f,h/3);
  }
 }
 for(const auto&p:sim.food){int rad=std::max(3,int(p.radius*std::sqrt(std::max(0.f,p.amount)/2800.f)));disk(int(p.x),int(p.y),rad,0x0029b85a);}
 disk(int(sim.nestX),int(sim.nestY),31,0x00352a20);disk(int(sim.nestX),int(sim.nestY),23,0x0051422c);
 for(const auto&a:sim.ants){int x=int(a.x),y=int(a.y);uint32_t c=a.carrying?0x0035e9ff:0x00e1d3ba;put(x,y,c);put(x+1,y,c);}
}

static void text(HDC dc,int x,int y,const wchar_t*s,COLORREF col=RGB(235,238,225)){SetTextColor(dc,col);SetBkMode(dc,TRANSPARENT);TextOutW(dc,x,y,s,lstrlenW(s));}
static void overlay(HDC dc,int w,int h,double fps){
 RECT panel{14,14,382,showHelp?294:184};HBRUSH bg=CreateSolidBrush(RGB(15,18,16));FillRect(dc,&panel,bg);DeleteObject(bg);
 HPEN pen=CreatePen(PS_SOLID,1,RGB(67,86,69));SelectObject(dc,pen);SelectObject(dc,GetStockObject(NULL_BRUSH));Rectangle(dc,panel.left,panel.top,panel.right,panel.bottom);DeleteObject(pen);
 HFONT title=CreateFontW(25,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
 HFONT body=CreateFontW(17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Consolas");
 auto old=SelectObject(dc,title);text(dc,30,27,L"ANT EVOLUTION LAB",RGB(146,231,141));SelectObject(dc,body);
 wchar_t line[256];
 swprintf(line,256,L"Population  %d     Speed  %dx",int(sim.ants.size()),speed);text(dc,30,67,line);
 swprintf(line,256,L"Food        %.0f     FPS    %.0f",sim.colonyFood,fps);text(dc,30,91,line);
 swprintf(line,256,L"Deliveries  %llu     Gen.   %.2f",sim.deliveries,double(sim.births)/double(ANT_COUNT));text(dc,30,115,line);
 auto g=sim.averages();swprintf(line,256,L"Genes: speed %.2f  sense %.1f",g.speed,g.sensorDistance);text(dc,30,139,line,RGB(178,204,166));
 swprintf(line,256,L"       efficiency %.2f  turn %.2f",2.f-g.metabolism,g.turnRate);text(dc,30,163,line,RGB(178,204,166));
 if(showHelp){
  text(dc,30,199,L"SPACE  pause     +/-  simulation speed",RGB(177,179,170));
  text(dc,30,221,L"T      trails    R    new world",RGB(177,179,170));
  text(dc,30,243,L"F5     save      F9   load",RGB(177,179,170));
  text(dc,30,265,L"H      help      ESC  quit",RGB(177,179,170));
 }
 if(paused)text(dc,w/2-46,25,L"PAUSED",RGB(255,210,70));
 SelectObject(dc,old);DeleteObject(title);DeleteObject(body);
}

static LRESULT CALLBACK wndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
 switch(msg){
 case WM_KEYDOWN:
  if(wp==VK_ESCAPE)DestroyWindow(hwnd);
  else if(wp==VK_SPACE)paused=!paused;
  else if(wp==L'H')showHelp=!showHelp;
  else if(wp==L'T')showTrails=!showTrails;
  else if(wp==L'R')sim.reset(uint32_t(GetTickCount64()));
  else if(wp==VK_OEM_PLUS||wp==VK_ADD)speed=std::min(128,speed*2);
  else if(wp==VK_OEM_MINUS||wp==VK_SUBTRACT)speed=std::max(1,speed/2);
  else if(wp==VK_F5){sim.save("ant-world.ants");SetWindowTextW(hwnd,L"Ant Evolution Lab - saved ant-world.ants");}
  else if(wp==VK_F9){if(sim.load("ant-world.ants"))SetWindowTextW(hwnd,L"Ant Evolution Lab - world loaded");}
  return 0;
 case WM_DESTROY:PostQuitMessage(0);return 0;
 } return DefWindowProcW(hwnd,msg,wp,lp);
}

int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,LPWSTR,int show){
 SetProcessDPIAware(); WNDCLASSW wc{};wc.lpfnWndProc=wndProc;wc.hInstance=hi;wc.lpszClassName=L"AntEvolutionWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);RegisterClassW(&wc);
 HWND hwnd=CreateWindowExW(0,wc.lpszClassName,L"Ant Evolution Lab - 50,000 ants",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1300,780,nullptr,nullptr,hi,nullptr);ShowWindow(hwnd,show);
 bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bmi.bmiHeader.biWidth=WORLD_W;bmi.bmiHeader.biHeight=-WORLD_H;bmi.bmiHeader.biPlanes=1;bmi.bmiHeader.biBitCount=32;bmi.bmiHeader.biCompression=BI_RGB;
 MSG msg{};auto prev=std::chrono::steady_clock::now(),lastTitle=prev;double fps=0;int frames=0;
 while(msg.message!=WM_QUIT){
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);} if(msg.message==WM_QUIT)break;
  if(!paused)for(int i=0;i<speed;i++)sim.step();renderWorld();
  RECT rc;GetClientRect(hwnd,&rc);HDC dc=GetDC(hwnd);StretchDIBits(dc,0,0,rc.right,rc.bottom,0,0,WORLD_W,WORLD_H,pixels.data(),&bmi,DIB_RGB_COLORS,SRCCOPY);overlay(dc,rc.right,rc.bottom,fps);ReleaseDC(hwnd,dc);
  ++frames;auto now=std::chrono::steady_clock::now();double span=std::chrono::duration<double>(now-lastTitle).count();if(span>=.5){fps=frames/span;frames=0;lastTitle=now;}
  double elapsed=std::chrono::duration<double,std::milli>(now-prev).count();prev=now;if(elapsed<16.0)Sleep(DWORD(16.0-elapsed));
 }
 return 0;
}
