// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include <windows.h>
#include <deque>
#include <mutex>
#include <string>

// 应用程序和调试器之间传递数据是通过一个 4KB 大小的共享内存块完成的
#define MAX_DEBUG_CACHE_LEN 4096
#define _TRY_ __try
#define _FINALLY_ __finally

#pragma pack(push)
#pragma pack(1)
typedef struct __DEBUGBUFFER
{
	DWORD dwProcessId;
	CHAR szDebugCache[4096 - sizeof(DWORD)];
} DEBUGBUFFER, * PDEBUGBUFFER;
#pragma pack(pop)

#define WM_USER_NOTIFY_TEXT WM_USER+WM_NOTIFY+WM_SETTEXT+0
#define WM_USER_NOTIFY_START WM_USER+WM_NOTIFY+WM_SETTEXT+1
#define WM_USER_NOTIFY_STOP WM_USER+WM_NOTIFY+WM_SETTEXT+2

class CDbgLogCat {
private:
	std::mutex m_locker;
	std::deque<std::string> m_text_list;
	int m_is_running;
	HWND m_hWndParent;
	DWORD m_dwTextWndID;
	DWORD m_dwStatusWndID;

	std::shared_ptr<std::thread> m_thread;
public:
	std::shared_ptr<std::thread> get_thread_ptr() { return m_thread; }
	void set_thread_ptr(const std::shared_ptr<std::thread>& _thread) { m_thread = _thread; }
	HWND get_notify_hwnd() { return m_hWndParent; }
	void set_notify_hwnd(HWND hWnd) { m_hWndParent = hWnd; }
	DWORD get_notify_textwndid() { return m_dwTextWndID; }
	void set_notify_textwndid(DWORD dwTextWndID) { m_dwTextWndID = dwTextWndID; }
	DWORD get_notify_statuswndid() { return m_dwStatusWndID; }
	void set_notify_statuswndid(DWORD dwStatusWndID) { m_dwStatusWndID = dwStatusWndID; }
	std::size_t get_list_size() { return m_text_list.size(); }
	std::string get_front() {
		std::string text = "";
		m_locker.lock();
		if (!m_text_list.empty())
		{
			text = m_text_list.front();
			m_text_list.pop_front();
		}
		m_locker.unlock();
		return text;
	}
	bool is_running() { return (m_is_running == 1); }
	void set_running(int is_running) { m_is_running = is_running; }
	void add_text_to_list(const std::string& text) {
		m_locker.lock();
		m_text_list.emplace_back(text + std::string("\r\n"));
		m_locker.unlock();
	}
	void notify_start()
	{
		PostMessage(m_hWndParent, WM_USER_NOTIFY_START, (WPARAM)(0), (LPARAM)(0));
	}
	void notify_stop()
	{
		PostMessage(m_hWndParent, WM_USER_NOTIFY_STOP, (WPARAM)(0), (LPARAM)(0));
	}
	void notify_text(const char* text) {
		add_text_to_list(text);
		PostMessage(m_hWndParent, WM_USER_NOTIFY_TEXT, (WPARAM)(0), (LPARAM)(0));
	}

	BOOL HandleEvent(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		switch (message)
		{
		case WM_USER_NOTIFY_START:
		{
			SetDlgItemText(hDlg, m_dwStatusWndID, TEXT("停止监视"));
			EnableWindow(GetDlgItem(hDlg, m_dwStatusWndID), TRUE);
		}
		break;
		case WM_USER_NOTIFY_STOP:
		{
			this->set_thread_ptr(nullptr);
			SetDlgItemText(hDlg, m_dwStatusWndID, TEXT("启动监视"));
			EnableWindow(GetDlgItem(hDlg, m_dwStatusWndID), TRUE);
		}
		break;
		case WM_USER_NOTIFY_TEXT:
		{
			std::string text = this->get_front();
			if (text.length() > 0)
			{
				AppendEditText(GetDlgItem(hDlg, m_dwTextWndID), text.c_str());
			}
		}
		break;
		}
		return FALSE;
	}
	int dbglogcat()
	{
		HANDLE hReadyEvent = NULL;
		DWORD dwResult = 0L;
		HANDLE hMapping = NULL;
		HANDLE hAckEvent = NULL;
		DWORD dwWaitResult = 0L;
		DWORD dwTimeoutMSecs = 100L;
		PDEBUGBUFFER pdbBuffer = NULL;
		CHAR szLogCatBuffer[MAX_DEBUG_CACHE_LEN + 1] = { 0 };

		_TRY_{
	#define GLOBAL_WIN_BUFFER	//"Global\\"
	#define LEAVEIF(expr) if(expr == TRUE) return FALSE

			SECURITY_ATTRIBUTES sa = { 0 };
			SECURITY_DESCRIPTOR sd = { 0 };
			SECURITY_ATTRIBUTES* psa = NULL;
			psa = EveryoneSecurityAttributes(sa, sd);

			// 设置初始结果
			dwResult = ERROR_INVALID_HANDLE;
			// 打开事件句柄
			hAckEvent = CreateEvent(psa, FALSE, TRUE, TEXT(GLOBAL_WIN_BUFFER"DBWIN_BUFFER_READY"));
			LEAVEIF(hAckEvent == NULL);
			hReadyEvent = CreateEvent(psa, FALSE, FALSE, TEXT(GLOBAL_WIN_BUFFER"DBWIN_DATA_READY"));
			LEAVEIF(hReadyEvent == NULL);
			// 创建文件映射
			hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, psa, PAGE_READWRITE, 0, MAX_DEBUG_CACHE_LEN, TEXT(GLOBAL_WIN_BUFFER"DBWIN_BUFFER"));
			LEAVEIF(hMapping == NULL);
			// 映射调试缓冲区
			pdbBuffer = (PDEBUGBUFFER)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
			LEAVEIF(pdbBuffer == NULL);

			set_running(1);

			notify_start();

			// 循环
			while (is_running())
			{
				// 等待缓冲区数据
				SetEvent(hAckEvent);
				dwWaitResult = WaitForSingleObject(hReadyEvent, dwTimeoutMSecs);
				//dwWaitResult = WaitForSingleObject(m_hReadyEvent, INFINITE);
				switch (dwWaitResult)
				{
				case WAIT_OBJECT_0:
				{
					// 等待成功
					wsprintfA(szLogCatBuffer, ("PID:(%10d) %s"), pdbBuffer->dwProcessId, pdbBuffer->szDebugCache);
					notify_text(szLogCatBuffer);
				}
				break;
				case WAIT_TIMEOUT:
				{
					// 等待超时
					continue;
				}
				default:
				{
					// 等待失败
					set_running(0);
				}
					break;
				}
			}
			notify_stop();
		} _FINALLY_{
	   #define SAFECLOSEHANDLE(handle) if(handle != NULL) CloseHandle(handle)
			// 释放
			if (pdbBuffer)
			{
				UnmapViewOfFile(pdbBuffer);
			}
			SAFECLOSEHANDLE(hMapping);
			SAFECLOSEHANDLE(hReadyEvent);
			SAFECLOSEHANDLE(hAckEvent);

			set_running(0);

			// 返回结果
			return dwResult;
		}
	}

public:
	void Init(HWND hWndParent, DWORD dwStatusWndID, DWORD dwTextWndID)
	{
		set_notify_hwnd(hWndParent);
		set_notify_statuswndid(dwStatusWndID);
		set_notify_textwndid(dwTextWndID);
		
	}
	void Start()
	{
		if (get_thread_ptr() == nullptr)
		{
			EnableWindow(GetDlgItem(m_hWndParent, m_dwStatusWndID), FALSE);
			set_thread_ptr(std::make_shared<std::thread>([](void* p)
				{
					CDbgLogCat* thiz = (CDbgLogCat*)(p);
					if (IncreasingPermission(GetCurrentProcess()))
					{
						thiz->dbglogcat();
					}
				}, this));
		}
	}
	void Stop()
	{
		if (get_thread_ptr() != nullptr)
		{
			EnableWindow(GetDlgItem(m_hWndParent, m_dwStatusWndID), FALSE);
			set_running(0);
			if (get_thread_ptr()->joinable())
			{
				get_thread_ptr()->join();
				set_thread_ptr(nullptr);
			}
		}
	}
public:
	static CDbgLogCat* Inst() {
		static CDbgLogCat cdbgLogCatInstance;
		return &cdbgLogCatInstance;
	}
};

