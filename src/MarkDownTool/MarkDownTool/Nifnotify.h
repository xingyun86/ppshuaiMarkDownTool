// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include <shellapi.h>

class NotifyTray {
#define NIF_WM_NOTIFY_EVENT WM_USER + WM_COMMAND + WM_NOTIFY
public:
	NotifyTray() {}
	~NotifyTray() {}
private:
	NOTIFYICONDATA nid;
public:
	void init(HWND hWnd,
		HICON hIcon,
		LPCTSTR lpTip = TEXT("NotifyTray"),
		UINT uID = 0)
	{
		nid.cbSize = sizeof(nid);
		nid.hWnd = hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = NIF_WM_NOTIFY_EVENT;
		nid.hIcon = hIcon;
		lstrcpy(nid.szTip, lpTip);
		AddNotifyIcon();
	}
	void AddNotifyIcon()
	{
		Shell_NotifyIcon(NIM_ADD, &nid);
	}
	void DelNotifyIcon()
	{
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	BOOL HandleEvent(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		switch (message)
		{
		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{
				ShowWindow(hDlg, SW_HIDE);
				bHandled = TRUE;
				return FALSE;
			}
		}
		break;
		case NIF_WM_NOTIFY_EVENT:
		{
			if (lParam == WM_LBUTTONDBLCLK)
			{
				SetForegroundWindow(hDlg);
				ShowWindow(hDlg, SW_SHOWNORMAL);
				bHandled = TRUE;
				return TRUE;
			}
		}
		break;
		}
		return FALSE;
	}
public:
	static NotifyTray* Inst() {
		static NotifyTray instanceNotifyTray;
		return &instanceNotifyTray;
	}
};