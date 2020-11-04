// MarkDownTool.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <operator.h>
#include <interface.h>
#include <securityprotects.h>
#include <converter.h>
#include <randnonce.h>
#include <windowapi.h>
#include <windowfwp.h>
#include <Authority.h>
#include <Iexplorer.h>
#include <nifnotify.h>
#include <dbgoutput.h>

#include <tlhelp32.h>

class WindowHelper {
	typedef enum ChildrenControls {
		IDCC_MINIMUM = WM_USER,
		IDCC_BUTTON_VIEW,
		IDCC_BUTTON_SAVE,
		IDCC_BUTTON_EXPORT,
		IDCC_BUTTON_PRINTPDF,
		IDCC_EDIT_STATUS_TEXT,
		IDCC_EDIT_MARKDOWN_TEXT,
		IDCC_WEBBROWSER2_CTRL,
		IDCC_MAXIMUM,
	};
public:
	boolean m_thread_status = false;
	std::string m_jj_time = std::to_string(time(nullptr));
	std::string m_jj_port = CRandomString::GetInstance()->make_random_long(9999, 65535);
	std::shared_ptr<std::thread> m_thread = nullptr;
	void start_thread()
	{
		stop_thread();
		m_thread = std::make_shared<std::thread>([]()
			{
				std::string jjexe("");
				std::string cmdLine("");
				CHAR czTempPath[MAX_PATH] = { 0 };

				GetTempPathA(sizeof(czTempPath) / sizeof(*czTempPath), czTempPath);
				strcat(czTempPath, "MDT\\");

				jjexe = std::string(czTempPath) + "jre\\bin\\jj" + WindowHelper::Inst()->m_jj_time + ".exe";

				CreateCascadeDirectory((std::string(czTempPath) + TEXT("css\\")).c_str(), NULL);
				CreateCascadeDirectory((std::string(czTempPath) + TEXT("config\\")).c_str(), NULL);
				CreateCascadeDirectory((std::string(czTempPath) + TEXT("fonts\\HTML-CSS\\TeX\\woff\\")).c_str(), NULL);
				CreateCascadeDirectory((std::string(czTempPath) + TEXT("jax\\output\\HTML-CSS\\fonts\\TeX\\")).c_str(), NULL);

				FILE_WRITER(TakeResourceData(111, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(111, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("u.exe")), std::ios::binary);
				FILE_WRITER(TakeResourceData(112, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(112, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("r.rar")), std::ios::binary);

				cmdLine = (std::string(czTempPath) + "u.exe x -inul -o+ -pppss "
					+ std::string(czTempPath) + "r.rar " + std::string(czTempPath));
				ExecuteCommand(cmdLine.c_str());

				FILE_WRITER(TakeResourceData(101, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(101, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("j.js")), std::ios::binary);

				FILE_WRITER(TakeResourceData(102, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(102, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("p.js")), std::ios::binary);
				{
					auto s = TakeResourceData(102, TEXT("MARKDOWNDATA"));
					StringConvertUtils::Instance()->string_replace_all(s, "http://127.0.0.1:" + WindowHelper::Inst()->m_jj_port + "/img/", "http://www.plantuml.com/plantuml/img/");
					FILE_WRITER(s.data(), s.size(), (std::string(czTempPath) + TEXT("p.js")), std::ios::binary);
				}
				FILE_WRITER(TakeResourceData(103, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(103, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("m.js")), std::ios::binary);
				FILE_WRITER(TakeResourceData(104, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(104, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("MathJax.js")), std::ios::binary);
				FILE_WRITER(TakeResourceData(105, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(105, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("config\\TeX-AMS-MML_HTMLorMML.js")), std::ios::binary);
				FILE_WRITER(TakeResourceData(106, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(106, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("fonts\\HTML-CSS\\TeX\\woff\\MathJax_Main-Regular.woff")), std::ios::binary);
				FILE_WRITER(TakeResourceData(107, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(107, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("fonts\\HTML-CSS\\TeX\\woff\\MathJax_Math-Italic.woff")), std::ios::binary);
				FILE_WRITER(TakeResourceData(108, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(108, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("fonts\\HTML-CSS\\TeX\\woff\\MathJax_Size1-Regular.woff")), std::ios::binary);
				FILE_WRITER(TakeResourceData(109, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(109, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("jax\\output\\HTML-CSS\\jax.js")), std::ios::binary);
				FILE_WRITER(TakeResourceData(110, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(110, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("jax\\output\\HTML-CSS\\fonts\\TeX\\fontdata.js")), std::ios::binary);
				FILE_WRITER(TakeResourceData(113, TEXT("MARKDOWNDATA")).data(),
					TakeResourceData(113, TEXT("MARKDOWNDATA")).size(),
					(std::string(czTempPath) + TEXT("css\\css.css")), std::ios::binary);

				cmdLine = ("CMD /C \"MOVE " + std::string(czTempPath) + "jre\\bin\\jj.exe " + jjexe + "\"");
				ExecuteCommand(cmdLine.c_str());
				cmdLine = ("CMD /C \"DEL /S /Q " + std::string(czTempPath) + "r.rar\"");
				ExecuteCommand(cmdLine.c_str());
				cmdLine = ("CMD /C \"DEL /S /Q " + std::string(czTempPath) + "u.exe\"");
				ExecuteCommand(cmdLine.c_str());

				// Add Windows Messenger to the authorized application collection.
				WindowFirewall::Inst()->WindowsFirewallAddApp(
					(LPOLESTR)StringConvertUtils::Instance()->AToW(jjexe).c_str(),
					OLESTR("JJEXE")
				);
				WindowHelper::Inst()->m_thread_status = true;
				cmdLine = (jjexe + " -Djetty.home=" + std::string(czTempPath) + " -jar " + std::string(czTempPath) + "jra --host 127.0.0.1 --port " + WindowHelper::Inst()->m_jj_port + " " + std::string(czTempPath) + "pus");
				while (WindowHelper::Inst()->m_thread_status)
				{
					ExecuteCommand(cmdLine.c_str());
					Sleep(1000);
				}
			});
	}
	void stop_thread() {
		std::string cmdLine("");
		std::vector<std::string> sv = {};
		CHAR czTempPath[MAX_PATH] = { 0 };
		GetTempPathA(sizeof(czTempPath) / sizeof(*czTempPath), czTempPath);
		DirectoryTraversalPath(&sv, czTempPath, "jetty-*");
		strcat(czTempPath, "MDT\\");

		cmdLine = "CMD /C \"TASKKILL /F /IM JJ" + WindowHelper::Inst()->m_jj_time + ".EXE\"";
		ExecuteCommand(cmdLine.c_str());
		cmdLine = "CMD /C \"TASKKILL /F /IM JJ*.EXE\"";
		ExecuteCommand(cmdLine.c_str());
		if (m_thread_status == true)
		{
			m_thread_status = false;
			if (m_thread != nullptr && m_thread->joinable())
			{
				m_thread->join();
			}
		}
		cmdLine = "CMD /C \"DEL /S /Q " + std::string(czTempPath) + "*\"";
		ExecuteCommand(cmdLine.c_str());
		cmdLine = "CMD /C \"RMDIR /S /Q " + std::string(czTempPath) + "\"";
		ExecuteCommand(cmdLine.c_str());
		for (auto& it : sv)
		{
			cmdLine = "CMD /C \"DEL /S /Q " + it + "\\*\"";
			ExecuteCommand(cmdLine.c_str());
			cmdLine = "CMD /C \"RMDIR /S /Q " + it + "\"";
			ExecuteCommand(cmdLine.c_str());
		}
	}
	static LRESULT CALLBACK HandleButtonViewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		std::vector<std::string> tagList;
		std::vector<std::string> valueList;
		std::string strText(GetWindowTextLengthA(GetDlgItem(hWnd, IDCC_EDIT_MARKDOWN_TEXT)) + 1, '\0');
		if (strText.size() > 0)
		{
			GetDlgItemText(hWnd, IDCC_EDIT_MARKDOWN_TEXT, (LPSTR)strText.data(), strText.size());
			strText = strText.c_str();
			StringConvertUtils::Instance()->string_replace_all(strText, "", "\r");
			IexplorerUtils::Inst()->LoadHtml(IexplorerUtils::Inst()->MarkDownHtml(strText.c_str(), false).c_str(), true);
		}
		return 0;
	}
	static LRESULT CALLBACK HandleButtonSaveWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		std::vector<std::string> tagList;
		std::vector<std::string> valueList; 
		std::string strText(GetWindowTextLengthA(GetDlgItem(hWnd, IDCC_EDIT_MARKDOWN_TEXT)) + 1, '\0');
		GetDlgItemText(hWnd, IDCC_EDIT_MARKDOWN_TEXT, (LPSTR)strText.data(), strText.size());
		strText = strText.c_str();
		if (strText.size() > 0)
		{
			if (WindowHelper::Inst()->m_savePath.empty())
			{
				WindowHelper::Inst()->m_savePath = std::string(APP_DIR) + "\\mdt-" + std::to_string(time(nullptr)) + ".md";
			}
			FILE_WRITER(strText.data(), strText.size(), WindowHelper::Inst()->m_savePath, std::ios::binary);
		}
		return 0;
	}
	static LRESULT CALLBACK HandleButtonExportWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		IexplorerUtils::Inst()->SwitchToPrinter();
		HandleButtonViewWndProc(hWnd, uMsg, wParam, lParam);
		IexplorerUtils::Inst()->CaptureToImage();
		IexplorerUtils::Inst()->SwitchToPreview();
		return 0;
	}
	static LRESULT CALLBACK HandleButtonPrintPdfWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		IexplorerUtils::Inst()->PrintToPdf();
		return 0;
	}
	std::string m_savePath = ("");
	static LRESULT CALLBACK MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bHandled = FALSE;
		WindowHelper* thiz = WindowHelper::Inst();

		NotifyTray::Inst()->HandleEvent(hWnd, uMsg, wParam, lParam, bHandled);

		switch (uMsg)
		{
		case WM_CREATE:
		{
			HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(128));
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL2, (LPARAM)hIcon);

			DisplayRequestRepaint(hWnd, 800, 600, TRUE);

			SetWindowText(hWnd, TEXT("MarkDown编辑预览工具"));
			NotifyTray::Inst()->init(hWnd, hIcon, TEXT("MarkDown编辑预览工具"));

			RegisterDropFilesEvent(hWnd);

			thiz->CreateControl(IDCC_BUTTON_VIEW, WC_BUTTON, TEXT("预览"), BS_PUSHBUTTON | BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, 0L, hWnd);
			thiz->CreateControl(IDCC_BUTTON_SAVE, WC_BUTTON, TEXT("保存"), BS_PUSHBUTTON | BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, 0L, hWnd);
			thiz->CreateControl(IDCC_BUTTON_EXPORT, WC_BUTTON, TEXT("导出"), BS_PUSHBUTTON | BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, 0L, hWnd);
			thiz->CreateControl(IDCC_BUTTON_PRINTPDF, WC_BUTTON, TEXT("打印PDF"), BS_PUSHBUTTON | BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, 0L, hWnd);
			thiz->CreateControl(IDCC_EDIT_STATUS_TEXT, WC_EDIT, TEXT("状态"), ES_LEFT | ES_READONLY | WS_CHILD | WS_VISIBLE, 0L, hWnd);
			thiz->CreateControl(IDCC_EDIT_MARKDOWN_TEXT, WC_EDIT, TEXT(""), ES_LEFT | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE, 0L, hWnd);

			IexplorerUtils::Inst()->Initialize(hWnd, IDCC_EDIT_STATUS_TEXT, RECT(), TEXT(""), WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER, 0, IDCC_WEBBROWSER2_CTRL);

			thiz->InitGridLayout(hWnd);

			thiz->AddControlToLayout(hWnd, 0, 0, IDCC_BUTTON_VIEW, HandleButtonViewWndProc, WindowHelper::cc_item::ST_FIXED, 60, 40, 0.0, 0.0);
			thiz->AddControlToLayout(hWnd, 0, 1, IDCC_BUTTON_SAVE, HandleButtonSaveWndProc, WindowHelper::cc_item::ST_FIXED, 60, 40, 0.0, 0.0);
			thiz->AddControlToLayout(hWnd, 0, 2, IDCC_BUTTON_EXPORT, HandleButtonExportWndProc, WindowHelper::cc_item::ST_FIXED, 60, 40, 0.0, 0.0);
			thiz->AddControlToLayout(hWnd, 0, 3, IDCC_BUTTON_PRINTPDF, HandleButtonPrintPdfWndProc, WindowHelper::cc_item::ST_FIXED, 60, 40, 0.0, 0.0);
			thiz->AddControlToLayout(hWnd, 0, 4, IDCC_EDIT_STATUS_TEXT, NULL, WindowHelper::cc_item::ST_FIXED, 160, 40, 0.0, 0.0);
			thiz->AddControlToLayout(hWnd, 1, 0, IDCC_EDIT_MARKDOWN_TEXT, NULL, WindowHelper::cc_item::ST_XY_SCALE, 0, 0, 0.5, 1.0);
			thiz->AddControlToLayout(hWnd, 1, 1, IDCC_WEBBROWSER2_CTRL, NULL, WindowHelper::cc_item::ST_XY_SCALE, 0, 0, 0.5, 1.0);

			thiz->LayoutOptimize(hWnd);

			WindowHelper::Inst()->start_thread();
			if (WindowHelper::Inst()->m_thread_status == false)
			{
				RemoveMenu(GetSystemMenu(hWnd, FALSE), SC_CLOSE, MF_BYCOMMAND);
				std::thread([](void* p)
					{
						DWORD dwTimeSecs = 0;
						while (dwTimeSecs <= 600 && WindowHelper::Inst()->m_thread_status == false)
						{
							Sleep(1000);
							dwTimeSecs ++;
						}
						AppendMenu(GetSystemMenu((HWND)(p), TRUE), SC_CLOSE, MF_BYCOMMAND, NULL);
					}, hWnd).detach();
			}
		}
		return 0;
		case WM_COMMAND:
		{
			thiz->CommandHandler(hWnd, uMsg, wParam, lParam);
		}
		break;
		case WM_DROPFILES:
		{
			std::vector<std::string> sv;
			GetDropFiles(&sv, (HDROP)(wParam));
			if (!sv.empty()) {
				//WindowHelper::Inst()->m_savePath = std::string(APP_DIR) + sv.begin()->substr(sv.begin()->rfind('\\'));
				WindowHelper::Inst()->m_savePath.assign(sv.begin()->data(), sv.begin()->size());
				SetDlgItemTextA(hWnd, IDCC_EDIT_MARKDOWN_TEXT, thiz->AnsiReadFile(sv.begin()->c_str()).c_str());
			}
		}
		break;
		case WM_DRAWITEM:
		{
			thiz->RepaintButton(hWnd, lParam);
			return 0;
		}
		break;
		case WM_SIZE:
		{
			thiz->RelayoutControls(hWnd);
		}
		break;
		case WM_PAINT:
		{
			HDC hDC = nullptr;
			PAINTSTRUCT ps = { 0 };
			hDC = BeginPaint(hWnd, &ps);
			if (hDC != nullptr)
			{
				RECT rc = { 0 };
				GetClientRect(hWnd, &rc);
				FillRect(hDC, &rc, (HBRUSH)HOLLOW_BRUSH);
				//TextOut(hDC, 0, 0, TEXT("Hello world!"), lstrlen(TEXT("Hello world!")));
				//TextOut(hDC, 32, 32, TEXT("Win32 Api!"), lstrlen(TEXT("Win32 Api!")));
				EndPaint(hWnd, &ps);
			}
		}
		break;
		case WM_CLOSE:
		{
			NotifyTray::Inst()->DelNotifyIcon();
			IexplorerUtils::Inst()->Unitialize();
			WindowHelper::Inst()->stop_thread();
		}
		break;
		}
		return 0;
	}
	INT Run()
	{
		return CreatePpsdlg(MainProc);
	}
public:
	INT CreatePpsdlg(LPVOID lpWndProc = NULL, HWND hParent = NULL, BOOL bModal = TRUE, BOOL bCenter = TRUE)
	{
		INT nRet = (-1);
		HWND hWnd = NULL;
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };

		bModal &= (hParent != NULL);
		if (bModal) { EnableWindow(hParent, FALSE); }

		GetClassInfoEx(GetModuleHandle(NULL), WC_DIALOG, &wcex);
		wcex.lpszClassName = TEXT(__func__);
		wcex.lpfnWndProc = ([](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
			{
				WNDPROC fnWndProc = NULL;
				switch (uMsg)
				{
				case WM_NCCREATE:
				{
					if ((lParam != NULL) && (fnWndProc = *(WNDPROC*)lParam) != NULL)
					{
						SetProp(hWnd, TEXT(__func__), (HANDLE)(fnWndProc));
					}
				}
				break;
				case WM_NCDESTROY:
				{
					PostMessage(hWnd, WM_QUIT, (WPARAM)(NULL), (LPARAM)(NULL));
				}
				break;
				default:
				{
					switch (uMsg)
					{
					case WM_CREATE:
						EnableWindow(GetParent(hWnd), FALSE);
						break;
					case WM_CLOSE:
						EnableWindow(GetParent(hWnd), TRUE);
						break;
					default:
						break;
					}
					if ((fnWndProc = (WNDPROC)GetProp(hWnd, TEXT(__func__))) != NULL)
					{
						fnWndProc(hWnd, uMsg, wParam, lParam);
					}
				}
				break;
				}
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			});
		RegisterClassEx(&wcex);

		/////////////////////////////////////////////////
		if ((hWnd = CreateWindowEx(
			WS_EX_DLGMODALFRAME,
			wcex.lpszClassName,
			wcex.lpszClassName,
			bModal ? WS_POPUPWINDOW | WS_CAPTION : WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hParent,
			NULL,
			wcex.hInstance,
			lpWndProc)
			) != NULL) {

			if (bCenter == TRUE)
			{
				if (hParent != NULL)
				{
					CenterWindowInParent(hWnd, hParent);
				}
				else
				{
					CenterWindowInScreen(hWnd);
				}
			}
			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

			while (hWnd != NULL)
			{
				MSG msg = { 0 };
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						break;
					}
					else
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else
				{
					// 完成某些工作的其他行程式 
					Sleep(1);
				}
			}
			nRet = (0);
		}

		return nRet;
	}

public:
	int ROW_NUM = 9;
	int COL_NUM = 9;
	int SPACE_X = 10;
	int SPACE_Y = 10;
public:
	class cc_item {
	public:
		typedef enum ScaleType {
			ST_MIN = 0,
			ST_FIXED,
			ST_X_SCALE,
			ST_Y_SCALE,
			ST_XY_SCALE,
			ST_MAX,
		};
	public:
		cc_item(ScaleType sType, int w, int h, double xScale, double yScale, WNDPROC fnWndProc) :
			sType(sType), w(w), h(h), xScale(xScale), yScale(yScale), fnWndProc(fnWndProc)
		{
		}
	public:
		ScaleType sType;
		int w;
		int h;
		double xScale;//宽度占用比例
		double yScale;//高度占用比例
		int xExclude;//当前行的所有固定项宽度
		int yExclude;//当前列的所有固定项高度
		int xN;//有效行数
		int yN;//有效列数
		WNDPROC fnWndProc;
	};

public:
	//Controls array
	 std::unordered_map<HWND, std::vector<std::vector<uint32_t>>> cc_array;
	//Controls layout info array
	std::unordered_map<uint32_t, cc_item> cc_umap;
public:
	void InitGridLayout(HWND hWnd)
	{
		if (this->cc_array.find(hWnd) != this->cc_array.end())
		{
			this->cc_array.at(hWnd).clear();
		}
		else
		{
			this->cc_array.emplace(hWnd, std::vector<std::vector<uint32_t>>{});
		}
		for (auto row = 0; row < this->ROW_NUM; row++)
		{
			this->cc_array.at(hWnd).push_back({});
			for (auto col = 0; col < this->COL_NUM; col++)
			{
				this->cc_array.at(hWnd)[row].push_back(0);
			}
		}
	}
	void InitGridLayout(HWND hWnd, int nRowNum, int nColNum)
	{
		if (this->cc_array.find(hWnd) != this->cc_array.end())
		{
			this->cc_array.at(hWnd).clear();
		}
		else
		{
			this->cc_array.emplace(hWnd, std::vector<std::vector<uint32_t>>{});
		}
		for (auto row = 0; row < nRowNum; row++)
		{
			this->cc_array.at(hWnd).push_back({});
			for (auto col = 0; col < nColNum; col++)
			{
				this->cc_array.at(hWnd)[row].push_back(0);
			}
		}
	}
	void AddControlToLayout(HWND hWnd, uint32_t row, uint32_t col, uint32_t uCtrlID,
		WNDPROC fnWndProc = NULL,
		cc_item::ScaleType scaleType = cc_item::ST_FIXED,
		int W = 0, int H = 0,
		double dScaleX = 0.0f, double dScaleY = 0.0f)
	{
		this->cc_umap.emplace(uCtrlID, cc_item(scaleType, W, H, dScaleX, dScaleY, fnWndProc));
		this->cc_array.at(hWnd)[row][col] = uCtrlID;
	}
	void CreateControl(uint32_t uCtrlID, LPCTSTR lpClassName, LPCTSTR lpWindowName,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE,
		DWORD dwExStyle = 0L,
		HWND hParent = NULL,
		int W = 0, int H = 0,
		int X = 0, int Y = 0,
		HINSTANCE hInstance = NULL,
		LPVOID lpParam = NULL)
	{
		SendMessage(CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, W, H, hParent, (HMENU)uCtrlID, hInstance, lpParam), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}
	void LayoutOptimize(HWND hWnd)
	{
		//行运算
		auto rows = this->cc_array.at(hWnd);
		auto cols = rows.at(0);
		int xN = 0;
		int yN = 0;
		for (auto row = 0; row < rows.size(); row++)
		{
			bool bFound = false;
			int xExclude = 0;
			for (auto col = 0; col < cols.size(); col++)
			{
				auto item = rows.at(row).at(col);
				if (this->cc_umap.find(item) != this->cc_umap.end())
				{
					bFound = true;
					if (this->cc_umap.at(item).sType == cc_item::ST_FIXED || this->cc_umap.at(item).sType == cc_item::ST_Y_SCALE)
					{
						xExclude += this->cc_umap.at(item).w;
					}
				}
			}
			if (bFound == true)
			{
				yN++;
			}
			//修正
			for (auto col = 0; col < cols.size(); col++)
			{
				auto item = rows.at(row).at(col);
				if (this->cc_umap.find(item) != this->cc_umap.end())
				{
					if (this->cc_umap.at(item).sType != cc_item::ST_FIXED)
					{
						this->cc_umap.at(item).xExclude = xExclude;
					}
				}
			}
		}
		//列运算
		for (auto col = 0; col < cols.size(); col++)
		{
			bool bFound = false;
			int yExclude = 0;
			for (auto row = 0; row < rows.size(); row++)
			{
				auto item = rows.at(row).at(col);
				if (this->cc_umap.find(item) != this->cc_umap.end())
				{
					bFound = true;
					if (this->cc_umap.at(item).sType == cc_item::ST_FIXED || this->cc_umap.at(item).sType == cc_item::ST_X_SCALE)
					{
						yExclude += this->cc_umap.at(item).h;
					}
				}
			}
			if (bFound == true)
			{
				xN++;
			}
			//修正
			for (auto row = 0; row < rows.size(); row++)
			{
				auto item = rows.at(row).at(col);
				if (this->cc_umap.find(item) != this->cc_umap.end())
				{
					bFound = true;
					if (this->cc_umap.at(item).sType != cc_item::ST_FIXED)
					{
						this->cc_umap.at(item).yExclude = yExclude;
					}
				}
			}
		}
		//优化数值
		for (auto col = 0; col < cols.size(); col++)
		{
			for (auto row = 0; row < rows.size(); row++)
			{
				auto item = rows.at(row).at(col);
				if (this->cc_umap.find(item) != this->cc_umap.end())
				{
					this->cc_umap.at(item).xN = xN;
					this->cc_umap.at(item).yN = yN;
				}
			}
		}
	}
	void CommandHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (this->cc_umap.find(LOWORD(wParam)) != this->cc_umap.end() && (this->cc_umap.at(LOWORD(wParam)).fnWndProc != NULL)) {
			this->cc_umap.at(LOWORD(wParam)).fnWndProc(hWnd, uMsg, wParam, lParam);
		}
	}
	void RepaintButton(HWND hWnd, LPARAM lParam)
	{
		RECT rect;
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		UINT itemState = lpDrawItemStruct->itemState;
		TCHAR tClassName[MAXCHAR] = { 0 };
		GetClassName(lpDrawItemStruct->hwndItem, tClassName, sizeof(tClassName) / sizeof(*tClassName));
		if ((lstrcmp(tClassName, WC_BUTTON) == 0) || lpDrawItemStruct->CtlID == IDOK || lpDrawItemStruct->CtlID == IDCANCEL)
		{
			TCHAR tText[MAX_PATH] = { 0 };
			GetWindowText(lpDrawItemStruct->hwndItem, tText, sizeof(tText) / sizeof(*tText));
			GetClientRect(lpDrawItemStruct->hwndItem, &rect);

			UINT edge_type = EDGE_SUNKEN;
			COLORREF bk_color = RGB(0, 128, 0);
			COLORREF text_color = RGB(255, 0, 255);
			COLORREF lt_color = RGB(255, 0, 0);
			COLORREF rb_color = RGB(255, 0, 255);

			if ((itemState & ODS_SELECTED) == ODS_SELECTED)
			{
				edge_type = EDGE_SUNKEN;
				bk_color = RGB(86, 90, 207);
				text_color = RGB(255, 255, 255);
				lt_color = RGB(10, 0, 0);
				rb_color = RGB(10, 0, 10);
			}
			else if ((itemState & ODS_FOCUS) == ODS_FOCUS)
			{
				edge_type = EDGE_RAISED;
				bk_color = RGB(86, 90, 207);
				text_color = RGB(255, 255, 255);
				lt_color = RGB(10, 0, 0);
				rb_color = RGB(10, 0, 10);
			}
			else
			{
				edge_type = EDGE_RAISED;
				bk_color = RGB(86, 90, 207);
				text_color = RGB(255, 255, 255);
				lt_color = RGB(10, 0, 0);
				rb_color = RGB(10, 0, 10);
			}
			SetBkColor(lpDrawItemStruct->hDC, bk_color);
			SetTextColor(lpDrawItemStruct->hDC, text_color);
			FillSolidRect(lpDrawItemStruct->hDC, &rect, bk_color);
			Draw3dRect(lpDrawItemStruct->hDC, &rect, lt_color, rb_color, 3);
			DrawEdge(lpDrawItemStruct->hDC, &rect, edge_type, BF_RECT);
			DrawText(lpDrawItemStruct->hDC, tText, lstrlen(tText), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//Redraw the Caption of Button Window 
		}
	}
	void RelayoutControls(HWND hWnd)
	{
		if (this->cc_array.find(hWnd) != this->cc_array.end())
		{
			RECT rcWnd = { 0 };
			GetClientRect(hWnd, &rcWnd);

			int left = 0;
			int top = 0;
			auto rows = this->cc_array.at(hWnd);
			for (auto row = 0; row < rows.size(); row++)
			{
				left = 0;
				uint32_t uItem = 0;
				auto cols = rows.at(row);
				for (auto col = 0; col < cols.size(); col++)
				{
					auto item = cols.at(col);
					//存在控件
					if (item > 0)
					{
						RECT rcItem = { 0 };
						cc_item* p_cc_item = &this->cc_umap.at(item);
						if (p_cc_item->sType > cc_item::ST_MIN && p_cc_item->sType < cc_item::ST_MAX)
						{
							switch (p_cc_item->sType)
							{
							case cc_item::ST_FIXED:
								break;
							case cc_item::ST_X_SCALE:
								p_cc_item->w = (rcWnd.right - rcWnd.left - p_cc_item->xExclude) * p_cc_item->xScale;
								break;
							case cc_item::ST_Y_SCALE:
								p_cc_item->h = (rcWnd.bottom - rcWnd.top - p_cc_item->yExclude) * p_cc_item->yScale;
								break;
							case cc_item::ST_XY_SCALE:
								p_cc_item->w = (rcWnd.right - rcWnd.left - p_cc_item->xExclude) * p_cc_item->xScale;
								p_cc_item->h = (rcWnd.bottom - rcWnd.top - p_cc_item->yExclude) * p_cc_item->yScale;
								break;
							default:
								break;
							}
						}
						rcItem.left = left + this->SPACE_X;
						rcItem.top = top + this->SPACE_Y;
						rcItem.right = p_cc_item->w - this->SPACE_X;
						if (((rcItem.left + rcItem.right - rcWnd.right) > this->SPACE_X)
							|| ((rcWnd.right - rcItem.left - rcItem.right) < this->SPACE_X))
						{
							rcItem.right = rcWnd.right - this->SPACE_X - rcItem.left;
						}
						rcItem.bottom = p_cc_item->h - this->SPACE_Y;
						if (((rcItem.top + rcItem.bottom - rcWnd.bottom) > this->SPACE_Y)
							|| ((rcWnd.bottom - rcItem.top - rcItem.bottom) < this->SPACE_Y))
						{
							rcItem.bottom = rcWnd.bottom - this->SPACE_Y - rcItem.top;
						}
						MoveWindow(GetDlgItem(hWnd, item), rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, TRUE);
						left += p_cc_item->w;
						uItem = p_cc_item->h;
					}
				}
				top += uItem;
			}
		}
	}
	std::string AnsiReadFile(const std::string& fileName)
	{
		auto s = FILE_READER(fileName, std::ios::binary);
		switch (DetectEncode((const uint8_t*)s.data(), s.size()))
		{
		case ANSI:
			break;
		case UTF16_LE:
			s.erase(s.begin());
			s.erase(s.begin());
			s = StringConvertUtils::Instance()->WToA(std::wstring((const wchar_t*)s.data(), s.length() / sizeof(wchar_t)));
			break;
		case UTF16_BE:
			s.erase(s.begin());
			s.erase(s.begin());
			s = StringConvertUtils::Instance()->WToA(std::wstring((const wchar_t*)s.data(), s.length() / sizeof(wchar_t)));
			break;
		case UTF8_BOM:
			s.erase(s.begin());
			s.erase(s.begin());
			s.erase(s.begin());
			s = StringConvertUtils::Instance()->WToA(StringConvertUtils::Instance()->UTF8ToW(s));
			break;
		case UTF8:
			s = StringConvertUtils::Instance()->WToA(StringConvertUtils::Instance()->UTF8ToW(s));
			break;
		default:
			break;
		}
		return (s);
	}
public:
	static WindowHelper* Inst() {
		static WindowHelper WindowHelperInstance;
		return &WindowHelperInstance;
	}
};
#include <winspool.h>
class PrinterManager
{
	std::wstring ENVIRONMENT = L"";
	std::wstring PRINTERNAME = L"clawPDF";
	std::wstring DRIVERNAME = L"clawPDF Virtual Printer";
	std::wstring HARDWAREID = L"clawPDF_Driver";
	std::wstring PORTMONITOR = L"CLAWMON";
	std::wstring MONITORDLL = L"clawmon.dll";
	std::wstring MONITORUIDLL = L"clawmonui.dll";
	std::wstring PORTNAME = L"CLAWMON:";
	std::wstring PRINTPROCESOR = L"winprint";
	std::wstring DRIVERMANUFACTURER = L"Andrew Hess // clawSoft";
	std::wstring DRIVERFILE = L"PSCRIPT5.DLL";
	std::wstring DRIVERUIFILE = L"PS5UI.DLL";
	std::wstring DRIVERHELPFILE = L"PSCRIPT.HLP";
	std::wstring DRIVERDATAFILE = L"SCPDFPRN.PPD";

	std::vector<std::wstring> printerDriverFiles = { DRIVERFILE, DRIVERUIFILE, DRIVERHELPFILE, DRIVERDATAFILE };
	std::vector<std::wstring> printerDriverDependentFiles = { L"PSCRIPT.NTF" };

	std::wstring PrinterDriverPathx86 = L"clawpdf\\clawmon\\x86";
	std::wstring PrinterDriverPathx64 = L"clawpdf\\clawmon\\x64";
	std::wstring PrinterDriverPath = IsSystem64() ? PrinterDriverPathx64 : PrinterDriverPathx86;
	std::wstring OutputHandlerCommand = L"clawPDF.exe";
public:
	std::wstring DriverSourceDirectory = L".";
	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	typedef void (WINAPI* LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
	HMODULE hModuleKernel32 = NULL;
	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = NULL;
	LPBYTE lpDriverInfo6List = NULL;
	std::vector<DRIVER_INFO_6W> driverInfo6List = {};

	virtual ~PrinterManager()
	{
		driverInfo6List.clear();
		if (lpDriverInfo6List != NULL)
		{
			delete[] lpDriverInfo6List;
			lpDriverInfo6List = NULL;
		}
	}
	std::wstring DriverName()
	{
		return this->DRIVERNAME;
	}
	BOOL IsSystem64()
	{
		BOOL bRetVal = FALSE;
		SYSTEM_INFO si = { 0 };
		if (fnGetNativeSystemInfo == NULL)
		{
			if (hModuleKernel32 == NULL)
			{
				hModuleKernel32 = GetModuleHandle(TEXT("KERNEL32"));
			}
			if (IsProcess23in64())
			{
				fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(hModuleKernel32, "GetNativeSystemInfo");
			}
			else
			{
				fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetSystemInfo;
			}
		}
		if (fnGetNativeSystemInfo != NULL)
		{
			fnGetNativeSystemInfo(&si);
			if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
				si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
			{
				bRetVal = TRUE;
			}
			else
			{
				//32 位操作系统
				_tprintf(_T("is 32 OS\r\n"));
			}
		}
		return bRetVal;
	}
	BOOL IsProcess23in64()
	{
		BOOL bIsWow64 = FALSE;
		//IsWow64Process is not available on all supported versions of Windows.
		//Use GetModuleHandle to get a handle to the DLL that contains the function
		//and GetProcAddress to get a pointer to the function if available.
		if (fnIsWow64Process == NULL)
		{
			if (hModuleKernel32 == NULL)
			{
				hModuleKernel32 = GetModuleHandle(TEXT("KERNEL32"));
			}
			fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(hModuleKernel32, "IsWow64Process");
		}
		if (NULL != fnIsWow64Process)
		{
			if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
			{
				//handle error
			}
		}
		return bIsWow64;
	}
	BOOL ConfigureclawPDFPort()
	{
		std::wstring cmdLine = L"";
		std::wstring regHKLMPrinterKey = L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\" + PORTMONITOR;
		std::wstring regHKLMPrinterSubKey = L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\" + PORTMONITOR + L"\\" + PORTMONITOR + L":";

		//配置打印机端口
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterKey + L" /v LogLevel /t REG_DWORD /d 0 /f\"";	ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v Domain /t REG_SZ /d \".\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v ExecPath /t REG_SZ /d \"C:\\Program Files (x86)\\clawpdf\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v FilePattern /t REG_SZ /d \"\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v HideProcess /t REG_DWORD /d 0 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v OutputPath /t REG_SZ /d \"\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v Overwrite /t REG_DWORD /d 1 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v Password /t REG_BINARY /d 000000000000 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v PipeData /t REG_DWORD /d 0 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v RunAsPUser /t REG_DWORD /d 1 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v User /t REG_SZ /d \"\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v WaitTermination /t REG_DWORD /d 0 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v WaitTimeout /t REG_DWORD /d 0 /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v Description /t REG_SZ /d \"" + PRINTERNAME + L"\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v UserCommand /t REG_SZ /d \"C:\\Program Files (x86)\\clawpdf\\clawPDF.Bridge.exe\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v Printer /t REG_SZ /d \"" + PRINTERNAME + L"\" /f\""; ExecuteCommand(cmdLine.c_str());

		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v ExecPath /t REG_SZ /d \"" + StringConvertUtils::Instance()->AToW(APP_DIR) + L"\\clawpdf\" /f\""; ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"REG ADD " + regHKLMPrinterSubKey + L" /v UserCommand /t REG_SZ /d \"" + StringConvertUtils::Instance()->AToW(APP_DIR) + L"\\clawpdf\\clawPDF.Bridge.exe\" /f\""; ExecuteCommand(cmdLine.c_str());
		return TRUE;
	}
	BOOL AddclawPDFPortMonitor()
	{
		BOOL bResult = FALSE;
		std::wstring systemFolder(MAX_PATH, L'\0');
		GetSystemDirectoryW((LPWSTR)systemFolder.data(), systemFolder.size());
		systemFolder = systemFolder.c_str();

		PVOID oldRedirectValue = NULL;
		if (IsSystem64() && IsProcess23in64())
		{
			Wow64DisableWow64FsRedirection(&oldRedirectValue);
		}

		// Copy the monitor DLL to the system directory
		std::wstring cmdLine = L"";
		std::wstring srcFile = L"";
		std::wstring dstFile = L"";

		cmdLine = L"CMD /C \"net stop Spooler\""; ExecuteCommand(cmdLine.c_str());
		srcFile = DriverSourceDirectory + L"\\" + PrinterDriverPath + L"\\" +  MONITORDLL; dstFile = systemFolder + L"\\" + MONITORDLL;
		cmdLine = L"CMD /C \"COPY " + srcFile + L" " + dstFile + L" /Y\"";	ExecuteCommand(cmdLine.c_str());

		srcFile = DriverSourceDirectory + L"\\" + PrinterDriverPath + L"\\" + MONITORUIDLL;	dstFile = systemFolder + L"\\" + MONITORUIDLL;
		cmdLine = L"CMD /C \"COPY " + srcFile + L" " + dstFile + L" /Y\"";	ExecuteCommand(cmdLine.c_str());
		cmdLine = L"CMD /C \"net start Spooler\""; ExecuteCommand(cmdLine.c_str());

		//配置打印监控

		MONITOR_INFO_2W monitorInfo2 = { 0 };
		monitorInfo2.pName = (LPWSTR)PORTMONITOR.c_str();
		monitorInfo2.pEnvironment = (LPWSTR)ENVIRONMENT.c_str();
		std::wstring DLLName = systemFolder + L"\\" + MONITORDLL;
		monitorInfo2.pDLLName = (LPWSTR)DLLName.c_str();
		bResult = AddMonitorW(NULL, 2, (LPBYTE)&monitorInfo2);
		DWORD dwErr = GetLastError();
		if (IsSystem64() && !IsProcess23in64())
		{
			if (oldRedirectValue != NULL)
			{
				Wow64RevertWow64FsRedirection(oldRedirectValue);
			}
		}

		return bResult;
	}
	BOOL CopyPrinterDriverFiles() 
	{
		BOOL bResult = FALSE;
		DWORD dwNeeded = 0L;
		std::wstring driverFolder(MAX_PATH, L'\0');
		bResult = GetPrinterDriverDirectoryW(NULL, NULL, 1, (LPBYTE)driverFolder.data(), driverFolder.size(), &dwNeeded);
		driverFolder = driverFolder.c_str();

		for (auto it : printerDriverDependentFiles)
		{
			printerDriverFiles.emplace_back(it);
		}
		std::wstring cmdLine = L"";
		std::wstring srcFile = L"";
		std::wstring dstFile = L"";
		for (auto it : printerDriverFiles)
		{
			srcFile = DriverSourceDirectory + L"\\" + PrinterDriverPath + L"\\" + it;	dstFile = driverFolder + L"\\" + it;
			cmdLine = L"CMD /C \"COPY " + srcFile + L" " + dstFile + L" /Y\"";	ExecuteCommand(cmdLine.c_str());
		}
		return bResult;
	}
	BOOL AddclawPDFPort()
	{
		BOOL bResult = FALSE;
		DWORD dwOutputNeeded = 0L;
		DWORD dwXcvResultStatus = 0L;
		std::wstring xcvDataOperation = L"AddPort";
		PRINTER_DEFAULTSW printerDefaults = { 0 };

		printerDefaults.pDatatype = L"";
		printerDefaults.pDevMode = NULL;
		printerDefaults.DesiredAccess = 1; //Server Access Administer

		HANDLE hPrinter = NULL;
		bResult = OpenPrinterW((LPWSTR)(L",XcvMonitor " + PORTMONITOR).c_str(), &hPrinter, &printerDefaults);
		if (hPrinter != NULL)
		{
			WCHAR* pPortName = new WCHAR[PORTNAME.size() * sizeof(WCHAR) * sizeof(WCHAR)]();
			if (pPortName != NULL)
			{
				wmemset(pPortName, L'\0', PORTNAME.size() * sizeof(WCHAR));
				wmemcpy(pPortName, PORTNAME.data(), PORTNAME.length());
				bResult = XcvDataW(hPrinter, xcvDataOperation.c_str(), (PBYTE)pPortName, PORTNAME.size() * sizeof(WCHAR) * sizeof(WCHAR), NULL, 0, &dwOutputNeeded, &dwXcvResultStatus);
				DWORD dwErr = GetLastError();
				delete[]pPortName;
			}
			bResult = ClosePrinter(hPrinter);
		}
		return bResult;
	}
	std::vector<DRIVER_INFO_6W> EnumeratePrinterDrivers()
	{
		BOOL bResult = FALSE;

		DWORD dwbNeeded = 0L;
		DWORD dwReturned = 0L;
		
		driverInfo6List.clear();
		if (lpDriverInfo6List != NULL)
		{
			delete[] lpDriverInfo6List;
			lpDriverInfo6List = NULL;
		}
		bResult = EnumPrinterDriversW(NULL, (LPWSTR)ENVIRONMENT.c_str(), 6, NULL, 0, &dwbNeeded, &dwReturned);
		if (bResult == FALSE)
		{
			lpDriverInfo6List = new BYTE[dwbNeeded]();
			if (lpDriverInfo6List != NULL)
			{
				bResult = EnumPrinterDriversW(NULL, (LPWSTR)ENVIRONMENT.c_str(), 6, (LPBYTE)lpDriverInfo6List, dwbNeeded, &dwbNeeded, &dwReturned);

				if (bResult == TRUE)
				{
					for (DWORD i = 0; i < dwReturned; i++)
					{
						driverInfo6List.emplace_back(((DRIVER_INFO_6W*)lpDriverInfo6List)[i]);
					}
				}
				//delete[] pDriverInfo6List;
			}
		}

		return driverInfo6List;
	}
	BOOL IsPrinterDriverInstalled(const std::wstring& driverName)
	{
		BOOL driverInstalled = FALSE;
		std::vector<DRIVER_INFO_6W> driverInfo6List = EnumeratePrinterDrivers();
		for (auto driverInfo6 : driverInfo6List)
		{
			if (driverName.compare(driverInfo6.pName) == 0)
			{
				driverInstalled = TRUE;
				break;
			}
		}
		return driverInstalled;
	}
	BOOL InstallclawPDFPrinterDriver()
	{
		BOOL bResult = FALSE;
		DWORD dwNeeded = 0L;
		std::wstring driverFolder(MAX_PATH, L'0');
		bResult = GetPrinterDriverDirectoryW(NULL, NULL, 1, (LPBYTE)driverFolder.data(), driverFolder.size(), &dwNeeded);
		driverFolder = driverFolder.c_str();

		UINT uLen = 0;
		WCHAR * driverFileList = new WCHAR[printerDriverDependentFiles.size() * MAX_PATH]();
		if (driverFileList != NULL)
		{
			if (!printerDriverDependentFiles.empty())
			{
				for (auto it : printerDriverDependentFiles)
				{
					auto line = driverFolder + L"\\" + it;
					wmemcpy(driverFileList + uLen, line.data(), line.size());
					uLen += line.size();
					wmemset(driverFileList + uLen, L'\0', 1);
					uLen += 1;
				}
				wmemset(driverFileList + uLen, L'\0', 1);
			}
			else
			{
				wmemset(driverFileList + uLen, L'\0', 1);
				uLen += 1;
				wmemset(driverFileList + uLen, L'\0', 1);
				uLen += 1;
			}

			DRIVER_INFO_6W driverInfo6 = { 0 };

			driverInfo6.cVersion = 3;
			driverInfo6.pName = (LPWSTR)DRIVERNAME.c_str();
			driverInfo6.pEnvironment = (LPWSTR)ENVIRONMENT.c_str();
			auto DriverPath = (driverFolder + L"\\" + DRIVERFILE);
			driverInfo6.pDriverPath = (LPWSTR)DriverPath.c_str();
			auto ConfigFile = (driverFolder + L"\\" + DRIVERUIFILE);
			driverInfo6.pConfigFile = (LPWSTR)ConfigFile.c_str();
			auto HelpFile = (driverFolder + L"\\" + DRIVERHELPFILE);
			driverInfo6.pHelpFile = (LPWSTR)HelpFile.c_str();
			auto DataFile = (driverFolder + L"\\" + DRIVERDATAFILE);
			driverInfo6.pDataFile = (LPWSTR)DataFile.c_str();
			auto DependentFiles = driverFileList;
			driverInfo6.pDependentFiles = (LPWSTR)driverFileList;

			driverInfo6.pMonitorName = (LPWSTR)PORTMONITOR.c_str();
			driverInfo6.pDefaultDataType = (LPWSTR)NULL;
			driverInfo6.dwlDriverVersion = 0x0001000000000000U;
			driverInfo6.pszMfgName = (LPWSTR)DRIVERMANUFACTURER.c_str();
			driverInfo6.pszHardwareID = (LPWSTR)HARDWAREID.c_str();
			driverInfo6.pszProvider = (LPWSTR)DRIVERMANUFACTURER.c_str();

			bResult = AddPrinterDriverW(NULL, 6, (LPBYTE)&driverInfo6);
			DWORD dwErr = GetLastError();
			delete[]driverFileList;
		}
		return bResult;
	}
	BOOL AddclawPDFPrinter()
	{
		BOOL bResult = FALSE;

		PRINTER_INFO_2W printerInfo2 = { 0 };

		printerInfo2.pServerName = NULL;
		printerInfo2.pPrinterName = (LPWSTR)PRINTERNAME.c_str();
		printerInfo2.pPortName = (LPWSTR)PORTNAME.c_str();
		printerInfo2.pDriverName = (LPWSTR)DRIVERNAME.c_str();
		printerInfo2.pPrintProcessor = (LPWSTR)PRINTPROCESOR.c_str();
		printerInfo2.pDatatype = (LPWSTR)L"RAW";
		printerInfo2.Attributes = 0x00000002;
		HANDLE hPrinter = AddPrinterW(NULL, 2, (LPBYTE)&printerInfo2);
		DWORD dwErr = GetLastError();
		if (hPrinter != NULL)
		{
			bResult = ClosePrinter(hPrinter);
		}
		return bResult;
	}
public:
	static PrinterManager* Inst() {
		static PrinterManager printerManagerInstance;
		return &printerManagerInstance;
	}
};

__inline static BOOL EnablePriv()
{
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		//用上面这个办法，在执行ExitWindowsEX注销的时候可以，但是重启，关机就不行了，改成以下可以解决
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			TOKEN_PRIVILEGES tkp;
			LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &tkp.Privileges[0].Luid);//修改进程权限
			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof tkp, NULL, NULL);//通知系统修改进程权限
			return((GetLastError() == ERROR_SUCCESS));
		}
	return TRUE;
}

__inline static
DWORD WINAPI ThreadStartRoutine(LPVOID lpThreadParameter)
{
	{
		if (!PrinterManager::Inst()->IsPrinterDriverInstalled(PrinterManager::Inst()->DriverName()))
		{
			EnablePriv();
			PrinterManager::Inst()->DriverSourceDirectory = StringConvertUtils::Instance()->AToW(APP_DIR);
			PrinterManager::Inst()->ConfigureclawPDFPort();
			PrinterManager::Inst()->AddclawPDFPortMonitor();
			PrinterManager::Inst()->CopyPrinterDriverFiles();
			PrinterManager::Inst()->AddclawPDFPort();
			PrinterManager::Inst()->InstallclawPDFPrinterDriver();
			PrinterManager::Inst()->AddclawPDFPrinter();
			PrinterManager::Inst()->ConfigureclawPDFPort();
		}
		if ((__argc > 1))
		{
			WindowHelper::Inst()->CreatePpsdlg((LPVOID)(WNDPROC)[]
			(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
				{
#define MESSAGE_TEXT_ID 1001
#define WM_USER_MESSAGE WM_USER + WM_NOTIFY
#define MESSAGE_DEFINES_TEXT "MESSAGE_DEFINES_TEXT"
					WindowHelper* thiz = WindowHelper::Inst();
					switch (uMsg)
					{
					case WM_CREATE:
					{
						RemoveMenu(GetSystemMenu(hWnd, FALSE), SC_CLOSE, MF_BYCOMMAND);

						HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(128));
						SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
						SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
						SendMessage(hWnd, WM_SETICON, ICON_SMALL2, (LPARAM)hIcon);

						DisplayRequestRepaint(hWnd, 300, 200, TRUE);

						SetWindowText(hWnd, TEXT("MarkDown编辑预览工具"));

						SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
						
						thiz->CreateControl(MESSAGE_TEXT_ID, WC_STATIC, TEXT(""), SS_CENTER | WS_CHILD | WS_VISIBLE, 0L, hWnd);
						thiz->InitGridLayout(hWnd);
						thiz->AddControlToLayout(hWnd, 0, 0, MESSAGE_TEXT_ID, NULL, WindowHelper::cc_item::ST_XY_SCALE, 160, 40, 1.0, 1.0);
						thiz->LayoutOptimize(hWnd);

						SetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT), (HANDLE)new TCHAR[MAX_PATH]());
						memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
						lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装准备中..."));
						PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
						std::thread([hWnd]() {
							EnablePriv();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装开始..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->DriverSourceDirectory = StringConvertUtils::Instance()->AToW(APP_DIR);
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR)); 
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第一步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->ConfigureclawPDFPort();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第二步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->AddclawPDFPortMonitor();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第三步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->CopyPrinterDriverFiles();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第四步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->AddclawPDFPort();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第五步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->InstallclawPDFPrinterDriver();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第六步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->AddclawPDFPrinter();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("(共七步)驱动安装第七步..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							PrinterManager::Inst()->ConfigureclawPDFPort();
							memset((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), 0, MAX_PATH * sizeof(TCHAR));
							lstrcpy((LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)), TEXT("驱动安装完成..."));
							PostMessage(hWnd, WM_USER_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
							AppendMenu(GetSystemMenu(hWnd, TRUE), SC_CLOSE, MF_BYCOMMAND, NULL);
							PostMessage(hWnd, WM_QUIT, (WPARAM)NULL, (LPARAM)NULL);
							}).detach();
					}
					return 0;
					case WM_USER_MESSAGE:
					{
						SetDlgItemText(hWnd, MESSAGE_TEXT_ID, (LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT)));
					}
					break;
					case WM_SIZE:
					{
						thiz->RelayoutControls(hWnd);
					}
					break;
					case WM_PAINT:
					{
						HDC hDC = nullptr;
						PAINTSTRUCT ps = { 0 };
						hDC = BeginPaint(hWnd, &ps);
						if (hDC != nullptr)
						{
							RECT rc = { 0 };
							GetClientRect(hWnd, &rc);
							FillRect(hDC, &rc, (HBRUSH)HOLLOW_BRUSH);
							EndPaint(hWnd, &ps);
						}
					}
					break;
					case WM_CLOSE:
					{
						delete (LPTSTR)GetProp(hWnd, TEXT(MESSAGE_DEFINES_TEXT));
					}
					break;
					}
					return 0;
				});
			return(0);
		}
	}
	/*{
		bool flag = false;
		std::string headline = "# 1234";
		std::regex reHeadLine("^(?:#){1,6} (.*)");
		std::string codeBlockLine = "```test```";
		std::regex reCodeBlock("^(?:`){3}(.*)$");
		flag = std::regex_match(codeBlockLine, reCodeBlock);
		flag = 0;
		std::regex reBreakLine("\\r\\n");
		std::string replacementBreakLine = "<br>";
		std::string breakLine = "a\\r\\nb\\r\\nm";
		flag = std::regex_match(breakLine, reBreakLine);
		breakLine = std::regex_replace(breakLine, reBreakLine, replacementBreakLine);
		flag = 0;
	}*/

	UNREFERENCED_PARAMETER(lpThreadParameter);
	WindowHelper::Inst()->Run();
	return 0;
}