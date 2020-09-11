// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>

// TODO: Reference additional headers your program requires here.
#ifdef _MSC_VER
#include <stdlib.h>
#define MAX_DIR_LEN        _MAX_PATH
#else
#include <limits.h>
#define MAX_DIR_LEN        PATH_MAX
#endif
#define __DEF_APP_DIR__        char APP_DIR[MAX_DIR_LEN]={0},APP_EXE[MAX_DIR_LEN]={0};
#ifdef _MSC_VER
#define __DEF_APP_DIR_INIT__(X) {char*P=NULL;strncpy(APP_DIR,X,sizeof(APP_DIR));P=APP_DIR;while(*P!='\0'){if(*P=='\\')*P='\\';P++;}P=strrchr(APP_DIR,'\\');if(P!=NULL){*P='\0';strncpy(APP_EXE,P+1,sizeof(APP_EXE));}}
#else
#define __DEF_APP_DIR_INIT__(X) {char*P=NULL;strncpy(APP_DIR,X,sizeof(APP_DIR));P=APP_DIR;while(*P!='\0'){if(*P=='/')*P='/';P++;}P=strrchr(APP_DIR,'/');if(P!=NULL){*P='\0';strncpy(APP_EXE,P+1,sizeof(APP_EXE));}}
#endif
#define __EXT_APP_DIR__        extern char APP_DIR[MAX_DIR_LEN],APP_EXE[MAX_DIR_LEN];
__EXT_APP_DIR__

#define SINGLETON_INSTANCE(C) static C * GetInstance(){static C instance##C;return &instance##C;}
#define FILE_READER(F, M) std::string((std::istreambuf_iterator<char>(std::ifstream(F, M | std::ios::in).rdbuf())), std::istreambuf_iterator<char>())
#define FILE_WRITER(D, S, F, M) std::ofstream(F, M | std::ios::out).write((D), (S))

int __user_task(const char* argp, int argc, char** argv);
int __user_task(const char* argp, unsigned long ul_reason_for_call);

#ifdef _MSC_VER
#define CHECK_MUTEX(X) {\
HANDLE hMutex = CreateMutex(NULL, FALSE, X);\
if (GetLastError() == ERROR_ALREADY_EXISTS){CloseHandle(hMutex);hMutex = NULL;return FALSE;}\
}
#else
#include <semaphore.h>
#define CHECK_MUTEX(X, Y) {\
if(strcmp(getenv(X),Y) == 0) return 0;\
char e[MAX_DIR_LEN]={0};snprintf(e,MAX_DIR_LEN,"%s=%s",X,Y);putenv(e);\
}
#define CLOSE_MUTEX(X){\
char e[MAX_DIR_LEN]={0};snprintf(e,MAX_DIR_LEN,"%s=0",X);putenv(e);\
}
#endif

#define UPDATE_WINDOW_REGION(WND,ID) {RECT r={0};::GetWindowRect(::GetDlgItem(WND,ID),&r);for(size_t i=0;i<sizeof(rect)/sizeof(POINT);i++){::ScreenToClient(WND,&((LPPOINT)&r)[i]);}::InvalidateRect(WND,&r,TRUE);}

#ifdef _MSC_VER
#include <windows.h>
#include <wtypes.h>
#include <unknwn.h>
#include <basetyps.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm")
#include <commctrl.h>
#pragma comment(lib, "comctl32")
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")

#define GDIPLUS_STARTUP() Gdiplus::GdiplusStartupInput gdiplusStartupInput;ULONG_PTR gdiplusToken;GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#define GDIPLUS_CLEANUP() Gdiplus::GdiplusShutdown(gdiplusToken);

#if !defined(DEBUG) && !defined(_DEBUG)
#pragma comment(linker, "/subsystem:windows") 
#define START_ENTRY __DEF_APP_DIR__ INT APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,INT nCmdShow){GDIPLUS_STARTUP()__DEF_APP_DIR_INIT__(*__argv);CHECK_MUTEX(APP_EXE);CSecurityProtects::get_interface()->CheckValid(); int ret=__user_task(APP_DIR,__argc,__argv);GDIPLUS_CLEANUP();return(ret);}
#else
#define START_ENTRY __DEF_APP_DIR__ int main(int argc,char** argv){__DEF_APP_DIR_INIT__(*__argv);CHECK_MUTEX(APP_EXE);CSecurityProtects::get_interface()->CheckValid();return(__user_task(APP_DIR,__argc,__argv));}
#endif

#define SHARE_ENTRY __DEF_APP_DIR__ int APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved){__DEF_APP_DIR_INIT__(*__argv);CHECK_MUTEX(APP_EXE);CSecurityProtects::get_interface()->CheckValid();return(__user_task(APP_DIR,ul_reason_for_call));}
#else
#define START_ENTRY __DEF_APP_DIR__ int main(int argc,char** argv){__DEF_APP_DIR_INIT__(*__argv);CHECK_MUTEX(APP_EXE);CSecurityProtects::get_interface()->CheckValid();__user_task(APP_DIR,argc,argv);CLOSE_MUTEX(APP_EXE);return 0;}

#define SHARE_ENTRY __DEF_APP_DIR__ int DllMain(const char * argp){__DEF_APP_DIR_INIT__(*__argv);CHECK_MUTEX(APP_EXE);CSecurityProtects::get_interface()->CheckValid();__user_task(APP_DIR, 0);CLOSE_MUTEX(APP_EXE);return 0;}
#endif

#define START_ENTRY_BINARY() START_ENTRY int __user_task(const char* argp, int argc, char** argv)
#define START_ENTRY_SHARED() SHARE_ENTRY int __user_task(const char* argp, unsigned long ul_reason_for_call)