// Interface.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <windows.h>

// TODO: Reference additional headers your program requires here.

__inline static
LPWORD WORDSALIGN(LPWORD lpWord)
{
    //ULONG ul = (ULONG)lpIn;ul += 3;ul >>= 2; ul <<= 2; return (LPWORD)ul;
    //return (LPWORD)(((((ULONG)lpWord) + 1) >> 1) << 1);
    return (LPWORD)(((((SIZE_T)lpWord) + 3) >> 2) << 2);
}

__inline static
LPWORD WORDSALIGN_ORIGIN(LPWORD lpWord)
{
    //ULONG ul = (ULONG)lpIn;ul += 3;ul >>= 2; ul <<= 2; return (LPWORD)ul;
    //return (LPWORD)(((((ULONG)lpWord) + 1) >> 1) << 1);
    return (LPWORD)(((((ULONG)lpWord) + 3) >> 2) << 2);
}

//--------------------------------------------
//  Initialize a dialog box.
//--------------------------------------------
#define DIALOGBOX_STARTUP() \
    HGLOBAL hGDT; \
    LPDLGTEMPLATE lpDT; \
    LPDLGITEMTEMPLATE lpDIT;\
    LPWORD lpW;\
    LPWSTR lpWS;\
    INT nN;\
    INITCOMMONCONTROLSEX iccex = { 0 }; \
    InitCommonControls(); \
    iccex.dwSize = sizeof(iccex); \
    iccex.dwICC = ICC_WIN95_CLASSES; \
    InitCommonControlsEx(&iccex); \
    hGDT = GlobalAlloc(GMEM_ZEROINIT, 4096);\
    lpDT = (LPDLGTEMPLATE)GlobalLock(hGDT);

//--------------------------------------------
//  UnInitialize a dialog box.
//--------------------------------------------
#define DIALOGBOX_CLEANUP() GlobalLock(hGDT);GlobalFree(hGDT);
//--------------------------------------------
//  Define a dialog box.
//--------------------------------------------
#define CREATE_DIALOGBOX_KERNEL(Style,ExStyle,Cnt,X,Y,CX,CY,Caption,nFontSize,FontName) \
{\
    lpDT->style = Style;\
    lpDT->dwExtendedStyle = ExStyle;\
    lpDT->cdit = Cnt;  \
    lpDT->x = X, lpDT->y = Y, lpDT->cx = CX, lpDT->cy = CY;\
    lpW = (LPWORD)(lpDT + 1);\
    *lpW++ = 0;   \
    *lpW++ = 0;   \
    lpWS = (LPWSTR)lpW;\
    nN = (lstrlenW(Caption)) + 1;\
    memcpy((LPVOID)lpWS, (LPCVOID)Caption, nN * sizeof(WCHAR));\
    lpW += nN;\
    if(Style & DS_SETFONT){\
    *lpW++ = nFontSize;   \
    lpWS = (LPWSTR)lpW;\
    nN = (lstrlenW(FontName)) + 1;\
    memcpy((LPVOID)lpWS, (LPCVOID)FontName, nN * sizeof(WCHAR));\
    lpW += nN;\
    }\
}
#define CREATE_DIALOGBOX(Style,ExStyle,Cnt,X,Y,CX,CY,Caption) CREATE_DIALOGBOX_KERNEL(Style | DS_SETFONT,ExStyle,Cnt,X,Y,CX,CY,Caption,9,L"宋体")

//--------------------------------------------
// Define a CONTROL.
//--------------------------------------------
#define TYPE_BUTTON      0x0080	//Button
#define TYPE_EDIT        0x0081	//Edit
#define TYPE_STATIC      0x0082	//Static
#define TYPE_LISTBOX     0x0083	//List box
#define TYPE_SCROLLBAR   0x0084	//Scroll bar
#define TYPE_COMBOBOX    0x0085	//Combo box
//--------------------------------------------
#define CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,Type) \
{\
    lpW = WORDSALIGN(lpW);\
    lpDIT = (LPDLGITEMTEMPLATE)lpW;\
    lpDIT->x = X, lpDIT->y = Y, lpDIT->cx = CX, lpDIT->cy = CY;\
    lpDIT->id = Id; \
    lpDIT->style = Style;\
    lpDIT->dwExtendedStyle = ExStyle;\
    lpW = (LPWORD)(lpDIT + 1);\
    *lpW++ = 0xFFFF;\
    *lpW++ = Type;\
    lpWS = (LPWSTR)lpW;\
    nN = (lstrlenW(Caption)) + 1;\
    memcpy((LPVOID)lpWS, (LPCVOID)Caption, nN * sizeof(WCHAR));\
    lpW += nN;\
    *lpW++ = 0;\
}

//-----------------------
// Define a BUTTON.
//-----------------------
#define CREATE_BUTTON(Id,Style,ExStyle,X,Y,CX,CY,Caption) CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,TYPE_BUTTON)
//-----------------------
// Define a EDIT.
//-----------------------
#define CREATE_EDIT(Id,Style,ExStyle,X,Y,CX,CY,Caption) CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,TYPE_EDIT)
//-----------------------
// Define a STATIC.
//-----------------------
#define CREATE_STATIC(Id,Style,ExStyle,X,Y,CX,CY,Caption) CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,TYPE_STATIC)
//-----------------------
// Define a LISTBOX.
//-----------------------
#define CREATE_LISTBOX(Id,Style,ExStyle,X,Y,CX,CY,Caption) CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,TYPE_LISTBOX)
//-----------------------
// Define a SCROLLBAR.
//-----------------------
#define CREATE_SCROLLBAR(Id,Style,ExStyle,X,Y,CX,CY,Caption) CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,TYPE_SCROLLBAR)
//-----------------------
// Define a COMBOBOX.
//-----------------------
#define CREATE_COMBOBOX(Id,Style,ExStyle,X,Y,CX,CY,Caption) CREATE_CONTROL(Id,Style,ExStyle,X,Y,CX,CY,Caption,TYPE_COMBOBOX)

#define DIALOG_RUN(hInstance,hParendWnd,DlgProc,lParam)  DialogBoxIndirectParam(hInstance, (LPCDLGTEMPLATE)hGDT, hParendWnd, (DLGPROC)DlgProc, (LPARAM)lParam);
#define DIALOG_CREATE(hInstance,hParendWnd,DlgProc,lParam) CreateDialogIndirectParam(hInstance, (LPCDLGTEMPLATE)hGDT, hParendWnd, (DLGPROC)DlgProc, (LPARAM)lParam);
#define DIALOG_DISPLAY(hDlgWnd,nShowFlag) ShowWindow(hDlgWnd,nShowFlag);
