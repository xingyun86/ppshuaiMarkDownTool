// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#include <atlbase.h>
#include <atlwin.h>
#include <wininet.h>
#include <string>
#include <thread>

#include <WebEventSink.h>
#include <IDocHostUIHandlerImpl.h>

#include <shlobj.h>
#include <urlhist.h>
#include <shellapi.h>

#include <mkdio.h>

#include <memory>
#include <sstream>
#include <maddy/parser.h>

class MyIeUtils {
public:
	static MyIeUtils* Inst() {
		static MyIeUtils myIeUtilsInstance;
		return &myIeUtilsInstance;
	}
	enum DEL_CACHE_TYPE //要删除的类型。
	{
		File,//表示internet临时文件
		Cookie //表示Cookie
	};
	BOOL  DeleteUrlCache(DEL_CACHE_TYPE type)
	{
		BOOL bRet = FALSE;
		HANDLE hEntry;
		LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryW = NULL;
		DWORD dwEntrySize;

		//delete the files
		dwEntrySize = 0;
		hEntry = FindFirstUrlCacheEntryW(NULL, NULL, &dwEntrySize);
		lpCacheEntryW = (LPINTERNET_CACHE_ENTRY_INFOW) new char[dwEntrySize];
		hEntry = FindFirstUrlCacheEntryW(NULL, lpCacheEntryW, &dwEntrySize);
		if (!hEntry)
		{
			goto cleanup;
		}

		do
		{
			if (type == File &&
				!(lpCacheEntryW->CacheEntryType & COOKIE_CACHE_ENTRY))
			{
				DeleteUrlCacheEntryW(lpCacheEntryW->lpszSourceUrlName);
			}
			else if (type == Cookie &&
				(lpCacheEntryW->CacheEntryType & COOKIE_CACHE_ENTRY))
			{
				DeleteUrlCacheEntryW(lpCacheEntryW->lpszSourceUrlName);
			}

			dwEntrySize = 0;
			FindNextUrlCacheEntryW(hEntry, NULL, &dwEntrySize);
			delete[] lpCacheEntryW;
			lpCacheEntryW = (LPINTERNET_CACHE_ENTRY_INFOW) new char[dwEntrySize];
		} while (FindNextUrlCacheEntryW(hEntry, lpCacheEntryW, &dwEntrySize));

		bRet = TRUE;
	cleanup:
		if (lpCacheEntryW)
		{
			delete[] lpCacheEntryW;
		}
		return bRet;
	}

	BOOL  EmptyDirectory(LPCWSTR szPath, BOOL bDeleteDesktopIni = FALSE,
		BOOL bWipeIndexDat = FALSE)
	{
		WIN32_FIND_DATAW wfd;
		HANDLE hFind;
		std::wstring sFullPath(1024,0);
		std::wstring sFindFilter(1024, 0);
		DWORD dwAttributes = 0;

		sFindFilter = szPath;
		sFindFilter += OLESTR("\\*.*");
		if ((hFind = FindFirstFileW(sFindFilter.c_str(), &wfd)) == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}

		do
		{
			if (StrCmpW(wfd.cFileName, OLESTR(".")) == 0 ||
				StrCmpW(wfd.cFileName, OLESTR("..")) == 0 ||
				(bDeleteDesktopIni == FALSE && StrCmpIW(wfd.cFileName, OLESTR("desktop.ini")) == 0))
			{
				continue;
			}

			sFullPath = szPath;
			sFullPath += _T('\\');
			sFullPath += wfd.cFileName;

			//去掉只读属性
			dwAttributes = GetFileAttributesW(sFullPath.c_str());
			if (dwAttributes & FILE_ATTRIBUTE_READONLY)
			{
				dwAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributesW(sFullPath.c_str(), dwAttributes);
			}

			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				EmptyDirectory(sFullPath.c_str(), bDeleteDesktopIni, bWipeIndexDat);
				RemoveDirectoryW(sFullPath.c_str());
			}
			else
			{
				if (bWipeIndexDat && StrCmpIW(wfd.cFileName, OLESTR("index.dat")) == 0)
				{
					WipeFile(szPath, wfd.cFileName);
				}
				DeleteFileW(sFullPath.c_str());
			}
		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);

		return TRUE;
	}

	//判断系统类型
	BOOL  IsWindowsNT()
	{
		BOOL bRet = FALSE;
		BOOL bOsVersionInfoEx;
		OSVERSIONINFOEX osvi;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
		// If that fails, try using the OSVERSIONINFO structure.
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi)))
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if (!GetVersionEx((OSVERSIONINFO*)&osvi))
				return bRet;
		}

		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion <= 4)
		{
			bRet = TRUE;
		}

		return bRet;
	}

	BOOL  IsWindows2k()
	{
		BOOL bRet = FALSE;
		BOOL bOsVersionInfoEx;
		OSVERSIONINFOEX osvi;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
		// If that fails, try using the OSVERSIONINFO structure.
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi)))
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if (!GetVersionEx((OSVERSIONINFO*)&osvi))
				return bRet;
		}

		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
		{
			bRet = TRUE;
		}

		return bRet;
	}
	BOOL  GetUserSid(PSID* ppSid)
	{
		HANDLE hToken;
		BOOL bRes;
		DWORD cbBuffer, cbRequired;
		PTOKEN_USER pUserInfo;

		// The User's SID can be obtained from the process token
		bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
		if (FALSE == bRes)
		{
			return FALSE;
		}

		// Set buffer size to 0 for first call to determine
		// the size of buffer we need.
		cbBuffer = 0;
		bRes = GetTokenInformation(hToken, TokenUser, NULL, cbBuffer, &cbRequired);
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			return FALSE;
		}

		// Allocate a buffer for our token user data
		cbBuffer = cbRequired;
		pUserInfo = (PTOKEN_USER)HeapAlloc(GetProcessHeap(), 0, cbBuffer);
		if (NULL == pUserInfo)
		{
			return FALSE;
		}

		// Make the "real" call
		bRes = GetTokenInformation(hToken, TokenUser, pUserInfo, cbBuffer, &cbRequired);
		if (FALSE == bRes)
		{
			return FALSE;
		}

		// Make another copy of the SID for the return value
		cbBuffer = GetLengthSid(pUserInfo->User.Sid);

		*ppSid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbBuffer);
		if (NULL == *ppSid)
		{
			return FALSE;
		}

		bRes = CopySid(cbBuffer, *ppSid, pUserInfo->User.Sid);
		if (FALSE == bRes)
		{
			HeapFree(GetProcessHeap(), 0, *ppSid);
			return FALSE;
		}

		bRes = HeapFree(GetProcessHeap(), 0, pUserInfo);

		return TRUE;
	}

	void  GetSidString(PSID pSid, LPWSTR szBuffer)
	{
		//convert SID to string
		SID_IDENTIFIER_AUTHORITY* psia = ::GetSidIdentifierAuthority(pSid);
		DWORD dwTopAuthority = psia->Value[5];
		wsprintfW(szBuffer, OLESTR("S-1-%lu"), dwTopAuthority);

		WCHAR szTemp[32];
		int iSubAuthorityCount = *(GetSidSubAuthorityCount(pSid));
		for (int i = 0; i < iSubAuthorityCount; i++)
		{
			DWORD dwSubAuthority = *(GetSidSubAuthority(pSid, i));
			wsprintfW(szTemp, OLESTR("%lu"), dwSubAuthority);
			StrCatW(szBuffer, OLESTR("-"));
			StrCatW(szBuffer, szTemp);
		}
	}

	BOOL  GetOldSD(HKEY hKey, LPCWSTR pszSubKey, BYTE** pSD)
	{
		BOOL bRet = FALSE;
		HKEY hNewKey = NULL;
		DWORD dwSize = 0;
		LONG lRetCode;
		*pSD = NULL;

		lRetCode = RegOpenKeyExW(hKey, pszSubKey, 0, READ_CONTROL, &hNewKey);
		if (lRetCode != ERROR_SUCCESS)
			goto cleanup;

		lRetCode = RegGetKeySecurity(hNewKey,
			(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, *pSD, &dwSize);
		if (lRetCode == ERROR_INSUFFICIENT_BUFFER)
		{
			*pSD = new BYTE[dwSize];
			lRetCode = RegGetKeySecurity(hNewKey,
				(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, *pSD, &dwSize);
			if (lRetCode != ERROR_SUCCESS)
			{
				delete* pSD;
				*pSD = NULL;
				goto cleanup;
			}
		}
		else if (lRetCode != ERROR_SUCCESS)
			goto cleanup;

		bRet = TRUE; // indicate success

	cleanup:
		if (hNewKey)
		{
			RegCloseKey(hNewKey);
		}
		return bRet;
	}

	BOOL  CreateNewSD(PSID pSid, SECURITY_DESCRIPTOR* pSD, PACL* ppDacl)
	{
		BOOL bRet = FALSE;
		PSID pSystemSid = NULL;
		SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
		ACCESS_ALLOWED_ACE* pACE = NULL;
		DWORD dwAclSize;
		DWORD dwAceSize;

		// prepare a Sid representing local system account
		if (!AllocateAndInitializeSid(&sia, 1, SECURITY_LOCAL_SYSTEM_RID,
			0, 0, 0, 0, 0, 0, 0, &pSystemSid))
		{
			goto cleanup;
		}

		// compute size of new acl
		dwAclSize = sizeof(ACL) + 2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
			GetLengthSid(pSid) + GetLengthSid(pSystemSid);

		// allocate storage for Acl
		*ppDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
		if (*ppDacl == NULL)
			goto cleanup;

		if (!InitializeAcl(*ppDacl, dwAclSize, ACL_REVISION))
			goto cleanup;

		//    if(!AddAccessAllowedAce(pDacl, ACL_REVISION, KEY_WRITE, pSid))
		//		goto cleanup;

			// add current user
		dwAceSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSid);
		pACE = (ACCESS_ALLOWED_ACE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwAceSize);

		pACE->Mask = KEY_READ | KEY_WRITE | KEY_ALL_ACCESS;
		pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
		pACE->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
		pACE->Header.AceSize = dwAceSize;

		memcpy(&pACE->SidStart, pSid, GetLengthSid(pSid));
		if (!AddAce(*ppDacl, ACL_REVISION, MAXDWORD, pACE, dwAceSize))
			goto cleanup;

		// add local system account
		HeapFree(GetProcessHeap(), 0, pACE);
		pACE = NULL;
		dwAceSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSystemSid);
		pACE = (ACCESS_ALLOWED_ACE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwAceSize);

		pACE->Mask = KEY_READ | KEY_WRITE | KEY_ALL_ACCESS;
		pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
		pACE->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
		pACE->Header.AceSize = dwAceSize;

		memcpy(&pACE->SidStart, pSystemSid, GetLengthSid(pSystemSid));
		if (!AddAce(*ppDacl, ACL_REVISION, MAXDWORD, pACE, dwAceSize))
			goto cleanup;

		if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
			goto cleanup;

		if (!SetSecurityDescriptorDacl(pSD, TRUE, *ppDacl, FALSE))
			goto cleanup;

		bRet = TRUE; // indicate success

	cleanup:
		if (pACE != NULL)
			HeapFree(GetProcessHeap(), 0, pACE);
		if (pSystemSid != NULL)
			FreeSid(pSystemSid);

		return bRet;
	}

	BOOL  RegSetPrivilege(HKEY hKey, LPCWSTR pszSubKey,
		SECURITY_DESCRIPTOR* pSD, BOOL bRecursive)
	{
		BOOL bRet = FALSE;
		HKEY hSubKey = NULL;
		LONG lRetCode;
		LPWSTR pszKeyName = NULL;;
		DWORD dwSubKeyCnt;
		DWORD dwMaxSubKey;
		DWORD dwValueCnt;
		DWORD dwMaxValueName;
		DWORD dwMaxValueData;
		DWORD i;

		if (!pszSubKey)
			goto cleanup;

		// open the key for WRITE_DAC access
		lRetCode = RegOpenKeyExW(hKey, pszSubKey, 0, WRITE_DAC, &hSubKey);
		if (lRetCode != ERROR_SUCCESS)
			goto cleanup;

		// apply the security descriptor to the registry key
		lRetCode = RegSetKeySecurity(hSubKey,
			(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, pSD);
		if (lRetCode != ERROR_SUCCESS)
			goto cleanup;

		if (bRecursive)
		{
			// reopen the key for KEY_READ access
			RegCloseKey(hSubKey);
			hSubKey = NULL;
			lRetCode = RegOpenKeyExW(hKey, pszSubKey, 0, KEY_READ, &hSubKey);
			if (lRetCode != ERROR_SUCCESS)
				goto cleanup;

			// first get an info about this subkey ...
			lRetCode = RegQueryInfoKey(hSubKey, 0, 0, 0, &dwSubKeyCnt, &dwMaxSubKey,
				0, &dwValueCnt, &dwMaxValueName, &dwMaxValueData, 0, 0);
			if (lRetCode != ERROR_SUCCESS)
				goto cleanup;

			// enumerate the subkeys and call RegTreeWalk() recursivly
			pszKeyName = new WCHAR[MAX_PATH + 1];
			for (i = 0; i < dwSubKeyCnt; i++)
			{
				lRetCode = RegEnumKeyW(hSubKey, i, pszKeyName, MAX_PATH + 1);
				if (lRetCode == ERROR_SUCCESS)
				{
					RegSetPrivilege(hSubKey, pszKeyName, pSD, TRUE);
				}
				else if (lRetCode == ERROR_NO_MORE_ITEMS)
				{
					break;
				}
			}
			delete[] pszKeyName;
		}

		bRet = TRUE; // indicate success

	cleanup:
		if (hSubKey)
		{
			RegCloseKey(hSubKey);
		}
		return bRet;
	}
#define SWEEP_BUFFER_SIZE			10000
	BOOL  WipeFile(LPCWSTR szDir, LPCWSTR szFile)
	{
		std::wstring sPath(1024,0);
		HANDLE	hFile;
		DWORD	dwSize;
		DWORD	dwWrite;
		CHAR	sZero[SWEEP_BUFFER_SIZE];
		memset(sZero, 0, SWEEP_BUFFER_SIZE);

		sPath = szDir;
		sPath += _T('\\');
		sPath += szFile;

		hFile = CreateFileW(sPath.c_str(), GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}

		dwSize = GetFileSize(hFile, NULL);

		//skip file header (actually, I don't know the file format of index.dat)
		dwSize -= 64;
		SetFilePointer(hFile, 64, NULL, FILE_BEGIN);

		while (dwSize > 0)
		{
			if (dwSize > SWEEP_BUFFER_SIZE)
			{
				WriteFile(hFile, sZero, SWEEP_BUFFER_SIZE, &dwWrite, NULL);
				dwSize -= SWEEP_BUFFER_SIZE;
			}
			else
			{
				WriteFile(hFile, sZero, dwSize, &dwWrite, NULL);
				break;
			}
		}

		CloseHandle(hFile);
		return TRUE;
	}
	
	void  OnButton1()
	{
		// //清internet临时文件
		WCHAR szPath[MAX_PATH];
		DeleteUrlCache(File);
		if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_INTERNET_CACHE, FALSE))
		{  //得到临时目录，并清空它.
			EmptyDirectory(szPath);
		}
	}

	void  ClearCookie()
	{
		//Cookie的清除
		WCHAR szPath[MAX_PATH];
		DeleteUrlCache(Cookie);
		if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_COOKIES, FALSE))
		{
			//得到目录，并清空
			EmptyDirectory(szPath);
		}
	}

	void  OnButton3()
	{
		//浏览器地址栏历史地址的清除
		SHDeleteKeyW(HKEY_CURRENT_USER, OLESTR("Software\\Microsoft\\Internet Explorer\\TypedURLs"));
	}

	void ClearFormAutoCompleteHistory()
	{
		// 清除表单自动完成历史记录
		WCHAR sKey[4096];
		DWORD dwRet;

		if (IsWindows2k() || IsWindowsNT())//先判断系统
		{
			WCHAR sBaseKey[4096];
			SECURITY_DESCRIPTOR NewSD;
			BYTE* pOldSD;
			PACL pDacl = NULL;
			PSID pSid = NULL;
			WCHAR szSid[256];
			if (GetUserSid(&pSid))
			{
				//get the hiden key name
				GetSidString(pSid, szSid);

				StrCpyW(sKey, OLESTR("Software\\Microsoft\\Protected Storage System Provider\\"));
				StrCatW(sKey, szSid);

				//get old SD
				StrCpyW(sBaseKey, sKey);
				GetOldSD(HKEY_CURRENT_USER, sBaseKey, &pOldSD);

				//set new SD and then clear
				if (CreateNewSD(pSid, &NewSD, &pDacl))
				{
					RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, FALSE);

					StrCatW(sKey, OLESTR("\\Data"));
					RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, FALSE);

					StrCatW(sKey, OLESTR("\\e161255a-37c3-11d2-bcaa-00c04fd929db"));
					RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, TRUE);

					dwRet = SHDeleteKeyW(HKEY_CURRENT_USER, sKey);
				}

				if (pDacl != NULL)
					HeapFree(GetProcessHeap(), 0, pDacl);

				//restore old SD
				if (pOldSD)
				{
					RegSetPrivilege(HKEY_CURRENT_USER, sBaseKey,
						(SECURITY_DESCRIPTOR*)pOldSD, FALSE);
					delete pOldSD;
				}
			}
			if (pSid)
				HeapFree(GetProcessHeap(), 0, pSid);
		}

		//win9x
		DWORD dwSize = MAX_PATH;
		WCHAR szUserName[MAX_PATH];
		GetUserNameW(szUserName, &dwSize);

		StrCatW(sKey, OLESTR("Software\\Microsoft\\Protected Storage System Provider\\"));
		StrCatW(sKey, szUserName);
		StrCatW(sKey, OLESTR("\\Data\\e161255a-37c3-11d2-bcaa-00c04fd929db"));
		dwRet = SHDeleteKeyW(HKEY_LOCAL_MACHINE, sKey);
	}

	void  ClearPassword()
	{
		// 清除自动密码历史记录
		SHDeleteKeyW(HKEY_CURRENT_USER,	OLESTR("Software\\Microsoft\\Internet Explorer\\IntelliForms"));
	}

	void  OnButton6()
	{
		//清收藏夹中的内容
		WCHAR szPath[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_FAVORITES, FALSE))
		{ //得到目录，并清空 
			EmptyDirectory(szPath);
		}
	}


	void  OnButton7()
	{
		// 清RAS自动拨号历史记录
		SHDeleteKeyW(HKEY_CURRENT_USER, OLESTR("Software\\Microsoft\\RAS Autodial\\Addresses"));
	}

	void  OnButton8()
	{
		// //清系统临时文件夹
		WCHAR szPath[MAX_PATH];
		if (GetTempPathW(MAX_PATH, szPath))//得到系统临时目录
		{
			EmptyDirectory(szPath, TRUE);
		}

	}

	void  OnButton9()
	{
		// 清空回收站
		SHEmptyRecycleBinW(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
	}

	void  OnButton10()
	{
		// 清除"运行"中的自动匹配历史记录
		SHDeleteKeyW(HKEY_CURRENT_USER,	OLESTR("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"));

	}

	void  OnButton11()
	{
		// //清"文档"中的历史记录
		WCHAR szPath[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_RECENT, FALSE))
		{
			EmptyDirectory(szPath);
		}

		SHDeleteKeyW(HKEY_CURRENT_USER,	OLESTR("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs"));
	}

	void  OnButton12()
	{
		// 清除上次登陆用户记录
		SHDeleteValueW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
			OLESTR("DefaultUserName"));
		SHDeleteValueW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
			OLESTR("AltDefaultUserName"));
		SHDeleteValueW(HKEY_LOCAL_MACHINE,
			OLESTR("Software\\Microsoft\\Windows\\CurrentVersion\\Winlogon"),
			OLESTR("DefaultUserName"));
	}

	void  OnButton13()
	{
		//清除"查找文件"自动匹配历史记录
		SHDeleteKeyW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Doc Find Spec MRU"));
		SHDeleteKeyW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Internet Explorer\\Explorer Bars\\{C4EE31F3-4768-11D2-BE5C-00A0C9A83DA1}\\ContainingTextMRU"));
		SHDeleteKeyW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Internet Explorer\\Explorer Bars\\{C4EE31F3-4768-11D2-BE5C-00A0C9A83DA1}\\FilesNamedMRU"));
	}

	void  OnButton14()
	{
		// 清除"查找计算机"自动匹配历史记录
		SHDeleteKeyW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FindComputerMRU"));
		SHDeleteKeyW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Internet Explorer\\Explorer Bars\\{C4EE31F3-4768-11D2-BE5C-00A0C9A83DA1}\\ComputerNameMRU"));
	}

	void  OnButton15()
	{
		//清除网络联接历史记录
		WCHAR szPath[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_NETHOOD, FALSE))
		{ //得到目录，并清空
			EmptyDirectory(szPath);
		}
	}

	void OnButton16()
	{
		// 清远程登录历史记录
		WCHAR sKey[4096];
		for (int i = 1; i <= 8; i++)
		{
			wsprintfW(sKey, OLESTR("Machine%d"), i);
			SHDeleteValueW(HKEY_CURRENT_USER,
				OLESTR("Software\\Microsoft\\Telnet"), sKey);

			wsprintfW(sKey, OLESTR("Service%d"), i);
			SHDeleteValueW(HKEY_CURRENT_USER,
				OLESTR("Software\\Microsoft\\Telnet"), sKey);

			wsprintfW(sKey, OLESTR("TermType%d"), i);
			SHDeleteValueW(HKEY_CURRENT_USER,
				OLESTR("Software\\Microsoft\\Telnet"), sKey);
		}

		SHDeleteValueW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Telnet"), OLESTR("LastMachine"));
		SHDeleteValueW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Telnet"), OLESTR("LastService"));
		SHDeleteValueW(HKEY_CURRENT_USER,
			OLESTR("Software\\Microsoft\\Telnet"), OLESTR("LastTermType"));
	}
	
	void OnButton17()
	{
		// 清浏览网址历史记录
		HRESULT hr;
		WCHAR szPath[MAX_PATH];
		IUrlHistoryStg2* pUrlHistoryStg2 = NULL;
		hr = CoCreateInstance(CLSID_CUrlHistory, NULL,
			CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg2,
			(void**)&pUrlHistoryStg2);
		if (SUCCEEDED(hr))
		{
			hr = pUrlHistoryStg2->ClearHistory();
			pUrlHistoryStg2->Release();
		}

		// 如果上面代码不能清
		// 则有下面的，不完美，但能工作.
		GetWindowsDirectoryW(szPath, MAX_PATH);
		StrCatW(szPath, OLESTR("\\History"));
		EmptyDirectory(szPath, FALSE, TRUE);

		if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_HISTORY, FALSE))
		{
			EmptyDirectory(szPath, FALSE, TRUE);
		}
	}

};
class IexplorerUtils :public WebeventsInterface
{
public:
	STDMETHODIMP OnBeforeNavigate2(
		IDispatch* pDisp, VARIANT* pvUrl, VARIANT* pvFlags, VARIANT* pvTargetFrameName,
		VARIANT* pvPostData, VARIANT* pvHeaders, VARIANT_BOOL* pvCancel) {
		SetDlgItemTextW(hWndParent, dwEditStatusID, OLESTR("已加载。"));
		//MessageBox(hWndParent, _T("捕获到:Html按9999!"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		return S_OK;
	}
	STDMETHODIMP OnDocumentComplete(IDispatch* pDisp, BSTR bstrUrl)
	{
		SetDlgItemTextW(hWndParent, dwEditStatusID, OLESTR("加载完成。"));
		//MessageBox(hWndParent, _T("捕获到:Html按钮45678!"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		/*_bstr_t bstr_t(bstrUrl);

		std::string strUrl(bstr_t);

		EnableWindow(GetDlgItem(m_hDlg, IDC_BTN_GO), TRUE);
		EnableWindow(GetDlgItem(m_hDlg, IDC_BTN_CALLJS), TRUE);
		EnableWindow(GetDlgItem(m_hDlg, IDC_EDIT_URL), TRUE);
		SetDlgItemText(m_hDlg, IDC_STATIC_LOADTEXT, L"加载完成");
		//页面加载完成，如果是目标页面就开始绑定 监听的HTMLElement
		if (0 == strcmp("https://passport.csdn.net/account/login", strUrl.c_str()))
		{
			BindHTMLElement();
		}
		else
		{
			char szAppPath[MAX_PATH] = { 0 };
			GetModuleFileNameA(NULL, szAppPath, MAX_PATH);
			(strrchr(szAppPath, '\\'))[1] = 0;
			strcat(szAppPath, "test.html");
			//if ((0 == strcmp(szAppPath,strUrl.c_str())))
			//不区分大小写
			if ((0 == stricmp(szAppPath, strUrl.c_str())))
			{
				RegisterUIHandlerToJs();
			}
		}
		// free the BSTR
		::SysFreeString(bstrUrl);*/
		return S_OK;
	}

	STDMETHODIMP OnStatusTextChange(IDispatch* pDisp, BSTR bstrText) {
		/*_bstr_t bstr_t(bstrText);
		std::wstring strText(bstr_t);
		SetDlgItemText(m_hDlg, IDC_STATIC_STATUS, strText.c_str());*/
		SetDlgItemTextW(hWndParent, dwEditStatusID, OLESTR("")/*bstrText*/);
		return S_OK;
	}
	STDMETHODIMP OnClick() {
		SetDlgItemTextW(hWndParent, dwEditStatusID, OLESTR("已点击。"));
		//MessageBox(hWndParent, _T("捕获到:Html按钮点击!"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		return S_OK;
	}

	//这个是测试 的目前是 csdn 测试
	void BindHTMLElement()
	{
		CComPtr<IHTMLElement> pIHTMLElement = NULL;// GetHTMLElementByTag(L"input", L"value", L"登 录");
		if (pIHTMLElement != NULL)
		{
			ConnectHTMLElementEvent(pIHTMLElement);
			pIHTMLElement.Release();
		}
	}

	//监听页面事件 
	void ConnectHTMLElementEvent(const CComPtr<IHTMLElement>& pIHTMLElement)
	{
		if (pIHTMLElement != NULL)
		{
			return;
		}
		DWORD dwCookie = 0L;
		CComPtr<IConnectionPoint> pCP = NULL;
		CComPtr<IConnectionPointContainer> pCPC = NULL;

		// Check that this is a connectable object.
		if (SUCCEEDED(pIHTMLElement->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC)))
		{
			// 枚举 查看一下 支持的 ConnectionPoint
			//IEnumConnectionPoints *pECPS = NULL;
			//hr = pCPC->EnumConnectionPoints(&pECPS);
			//if (SUCCEEDED(hr))
			//{
			//	IConnectionPoint* tmppCP = NULL;
			//	ULONG cFetched = 1;
			//	IID guid;
			//	while(SUCCEEDED(hr = pECPS->Next(1,&tmppCP,&cFetched))
			//		&&(cFetched>0))
			//	{	
			//		tmppCP->GetConnectionInterface(&guid);
			//	}
			//}

			// Find the connection point.
			//hr = pCPC->FindConnectionPoint(DIID_HTMLElementEvents2, &pCP);
			if (SUCCEEDED(pCPC->FindConnectionPoint(DIID_HTMLButtonElementEvents, &pCP)))
			{
				// Advise the connection point.
				// pUnk is the IUnknown interface pointer for your event sink
				if (SUCCEEDED(pCP->Advise(pHtmlElementSink, &dwCookie)))
				{
					// Successfully advised
				}
				pCP.Release();
			}
			pCPC.Release();
		}
	}

private:
#define IE_WM_NOTIFY_CHANGE_EVENT_TIMERID WM_USER + WM_NOTIFY + WM_CHANGEUISTATE + 0
#define IE_WM_NOTIFY_CHANGE_EVENT WM_USER + WM_NOTIFY + WM_CHANGEUISTATE + 1
	const LPOLESTR OLE_IEXPLORER_2_NAME = OLESTR("SHELL.EXPLORER.2");
public:
	CComModule _Module;
	~IexplorerUtils()
	{
		Unitialize();
	}
public:
	HWND hWnd = NULL;
	DWORD dwCookie = 0;
	HWND hWndParent = NULL;
	DWORD dwEditStatusID = 0L;
	CAxWindow axWindowContainer = NULL;
	CComPtr<IWebBrowser2> pIWebBrowser2 = NULL;
	CComPtr<IDocHostUIHandlerImpl> pDocHostUIHandler = NULL;
	CComPtr<IAxWinAmbientDispatch> pIAxWinAmbientDispatch = NULL;
	CComPtr<IConnectionPoint> pConnectionPointBrowserEvents = NULL;
	CComPtr<WebbrowserSink> pWebBrowserSink = new WebbrowserSink(this);
	CComPtr<IConnectionPointContainer> pConnectionPointContainer = NULL;
	CComPtr<HtmlElementSink> pHtmlElementSink = new HtmlElementSink(this);
	std::string j_string = "";
	std::string p_string = "";
	std::string m_string = "";
	std::string mj_string = "";
private:
	int SaveToFile(const BSTR bstrFileName, const CComPtr<IDispatch>& spIDispatchDocument)
	{
		CComPtr<IStream> spStream = NULL;
		HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMemory != NULL)
		{
			CComQIPtr<IPersistStreamInit> spPersistStream(spIDispatchDocument);
			if (SUCCEEDED(CreateStreamOnHGlobal(hMemory, TRUE, &spStream)) && SUCCEEDED(spPersistStream->Save(spStream, FALSE)))
			{
				LPCSTR lpData = (LPCSTR)GlobalLock(hMemory);
				if (lpData != NULL)
				{
					// Stream is always ANSI, but CString 
					// assignment operator will convert implicitly.
					HANDLE hFile = CreateFileW(bstrFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						WriteFile(hFile, lpData, lstrlenA(lpData), NULL, NULL);
						CloseHandle(hFile);
					}
				}
				GlobalUnlock(hMemory);
			}
			GlobalFree(hMemory);
		}
		return (0);
	}
	int SelectMail(const CComQIPtr<IHTMLDocument2>& spIHTMLDocument2)
	{
		int nResult = (-1);
		CComPtr<IHTMLElement> pIHTMLElement = NULL;
		CComQIPtr<IHTMLDocument3> spIHTMLDocument3(spIHTMLDocument2);
		CComPtr<IHTMLElementCollection> pIHTMLElementCollection = NULL;
		if (SUCCEEDED(spIHTMLDocument3->getElementsByTagName(OLESTR("span"), &pIHTMLElementCollection)))
		{
			LONG nLength = 0;
			if (SUCCEEDED(pIHTMLElementCollection->get_length(&nLength)) && (nLength > 0))
			{
				VARIANT varName = { 0 };
				VariantInit(&varName);
				varName.vt = VT_I4;
				VARIANT varIndex = { 0 };
				VariantInit(&varIndex);
				varIndex.vt = VT_I4;
				CComPtr<IHTMLElement> pIHTMLElement = NULL;
				for (LONG i = 0; i < nLength; i++)
				{
					varName.llVal = i;
					varIndex.llVal = 0;
					CComPtr<IDispatch> pIDispatch = NULL;
					// this is like doing document.all["messages"]
					if (SUCCEEDED(pIHTMLElementCollection->item(varName, varIndex, &pIDispatch)) && (pIDispatch != NULL))
					{
						// it's the caller's responsibility to release 
						if (SUCCEEDED(pIDispatch->QueryInterface(IID_IHTMLElement, (void**)&pIHTMLElement)) && (pIHTMLElement != NULL))
						{
							BSTR bstrId = NULL;
							if (SUCCEEDED(pIHTMLElement->get_id(&bstrId)) && bstrId != NULL)
							{
								BSTR bStr = StrStrW(bstrId, OLESTR("_mail_userlabel_"));
								if (bStr != NULL)
								{
									pIHTMLElement->click();
									nResult = (0);
									break;
								}
							}
						}
						
						pIHTMLElement.Release();						
					}
				}
				VariantClear(&varIndex);
				VariantClear(&varName);
			}
		}
		return(nResult);
	}
	int CloseAd(const CComQIPtr<IHTMLDocument2>& spIHTMLDocument2)
	{
		BSTR bstrText = NULL;
		CComPtr<IHTMLElement> pIHTMLElement = NULL;
		CComQIPtr<IHTMLDocument3> spIHTMLDocument3(spIHTMLDocument2);
		if (SUCCEEDED(spIHTMLDocument3->getElementById(OLESTR("_mail_link_14_253"), &pIHTMLElement)) && pIHTMLElement != NULL)
		{
			if (SUCCEEDED(pIHTMLElement->click()))
			{
				MessageBoxW(hWndParent, OLESTR("关闭广告成功!"), OLESTR("Message"), MB_OK);
				return(0);
			}
		}
		return (-1);
	}
	int ReadMail(std::wstring& wstrText, const CComQIPtr<IHTMLDocument2>& spIHTMLDocument2)
	{
		BSTR bstrText = NULL;
		CComPtr<IHTMLElement> pIHTMLElement = NULL;
		CComQIPtr<IHTMLDocument3> spIHTMLDocument3(spIHTMLDocument2);
		if (SUCCEEDED(spIHTMLDocument3->getElementById(OLESTR("content"), &pIHTMLElement)) && pIHTMLElement != NULL)
		{
			if (SUCCEEDED(pIHTMLElement->get_innerText(&bstrText)) && bstrText != NULL)
			{
				wstrText.assign(bstrText);
				MessageBoxW(hWndParent, wstrText.c_str(), OLESTR("Message"), MB_OK);
				return(0);
			}
		}
		return (-1);
	}

	int EnterInbox(const CComQIPtr<IHTMLDocument2>& spIHTMLDocument2)
	{
		CComPtr<IHTMLElement> pIHTMLElement = NULL;
		CComQIPtr<IHTMLDocument3> spIHTMLDocument3(spIHTMLDocument2);
		if (SUCCEEDED(spIHTMLDocument3->getElementById(OLESTR("_mail_component_145_145"), &pIHTMLElement)) && pIHTMLElement != NULL)
		{
			if (SUCCEEDED(pIHTMLElement->click()))
			{
				return(0);
			}
		}
		return(-1);
	}

	int FindSendKeys(const CComQIPtr<IHTMLDocument3>& spIHTMLDocument3, const BSTR bstrName, const BSTR bstrNewValue)
	{
		int nResult = (-1);
		CComPtr<IHTMLElementCollection> pIHTMLElementCollection = NULL;
		if (SUCCEEDED(spIHTMLDocument3->getElementsByName(bstrName, &pIHTMLElementCollection)) && pIHTMLElementCollection != NULL)
		{
			LONG nLength = 0;
			if (SUCCEEDED(pIHTMLElementCollection->get_length(&nLength)) && (nLength > 0))
			{
				VARIANT varName = { 0 };
				VariantInit(&varName);
				varName.vt = VT_I4;
				VARIANT varIndex = { 0 };
				VariantInit(&varIndex);
				varIndex.vt = VT_I4;
				VARIANT varType = { 0 };
				VariantInit(&varType);
				varType.vt = VT_BSTR;
				varType.bstrVal = SysAllocString(bstrNewValue);
				CComPtr<IHTMLElement> pIHTMLElement = NULL;
				for (LONG i = 0; i < nLength; i++)
				{
					varName.llVal = i;
					varIndex.llVal = 0;
					CComPtr<IDispatch> pIDispatch = NULL;
					// this is like doing document.all["messages"]
					if (SUCCEEDED(pIHTMLElementCollection->item(varName, varIndex, &pIDispatch)) && (pIDispatch != NULL))
					{
						// it's the caller's responsibility to release 
						if (SUCCEEDED(pIDispatch->QueryInterface(IID_IHTMLElement, (void**)&pIHTMLElement)) && (pIHTMLElement != NULL))
						{
							if (SUCCEEDED(pIHTMLElement->put_innerText(OLESTR(""))) &&
								SUCCEEDED(pIHTMLElement->put_innerText(bstrNewValue)) &&
								SUCCEEDED(pIHTMLElement->setAttribute(OLESTR("value"), varType)))
							{
								nResult = (0);
								break;
							}
						}
						pIHTMLElement.Release();
					}
				}
				SysFreeString(varType.bstrVal);
				VariantClear(&varType);
				VariantClear(&varIndex);
				VariantClear(&varName);
			}
		}
		return (nResult);
	}
	
	int AutoLogon(const CComQIPtr<IHTMLDocument2>& spIHTMLDocument2, const BSTR bstrUserName, const BSTR bstrPassword)
	{
		CComPtr<IHTMLElement> pElemPass = NULL;
		CComPtr<IHTMLElement> pElemLogin = NULL;
		CComQIPtr<IHTMLDocument3> spIHTMLDocument3(spIHTMLDocument2);
		//srand(time(0));
		if (FindSendKeys(spIHTMLDocument3, OLESTR("email"), bstrUserName) == 0)
		{
			//Sleep(rand() % 500 + rand() % 1000 + rand() % 1000);
			if (SUCCEEDED(spIHTMLDocument3->getElementById(OLESTR("pwdtext"), &pElemPass)) && pElemPass != NULL)
			{
				if (SUCCEEDED(pElemPass->click()) && (FindSendKeys(spIHTMLDocument3, OLESTR("password"), bstrPassword) == 0))
				{
					//Sleep(rand() % 500 + rand() % 1000 + rand() % 1000);
					if (SUCCEEDED(spIHTMLDocument3->getElementById(OLESTR("dologin"), &pElemLogin)) && pElemLogin != NULL && SUCCEEDED(pElemLogin->click()))
					{
						return(0);
					}
				}
			}
		}
		return(-1);
	}
public:
	DWORD dwPreviewFlags = DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DISABLE_HELP_MENU;
	DWORD dwPrinterFlags = 
		DOCHOSTUIFLAG_DIALOG |
		DOCHOSTUIFLAG_DISABLE_HELP_MENU |
		DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY |
		DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8 |
		DOCHOSTUIFLAG_NO3DOUTERBORDER |
		DOCHOSTUIFLAG_NO3DBORDER |
		DOCHOSTUIFLAG_SCROLL_NO |
		DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL;
	BOOL bStartPrint = FALSE;
	void SwitchToPrinter()
	{
		pIAxWinAmbientDispatch->put_DocHostFlags(dwPrinterFlags);
	}
	void SwitchToPreview()
	{
		pIAxWinAmbientDispatch->put_DocHostFlags(dwPreviewFlags);
	}
	void PrintToPdf()
	{
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
			if (spIHTMLDocument2)
			{
				pIWebBrowser2->ExecWB(OLECMDID_PRINTPREVIEW, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
			}
		}
	}
	void CaptureToImage()
	{
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
			if (spIHTMLDocument2)
			{
				CComPtr<IHTMLElement> pBodyElem;
				CComPtr<IHTMLWindow2> pWnd2, pTopWnd2;
				if (S_OK == spIHTMLDocument2->get_body(&pBodyElem) &&
					S_OK == spIHTMLDocument2->get_parentWindow(&pWnd2) &&
					S_OK == pWnd2->get_top(&pTopWnd2))
				{
					long nScrollHeight = 0L, nClientWidth = 0L, nClientHeight = 0L;

					CComPtr<IHTMLElement> pDocElem;
					spIHTMLDocument3->get_documentElement(&pDocElem);
					CComQIPtr<IHTMLElement2> pDocElem2(pDocElem);

					CComQIPtr<IHTMLElement2> pBodyElem2(pBodyElem);
					pBodyElem2->get_scrollHeight(&nScrollHeight);

					RECT rect;
					GetClientRect(hWnd, &rect);
					nClientWidth = rect.right - rect.left;
					nClientHeight = rect.bottom - rect.top;
					if (nScrollHeight > 0 && nClientWidth > 0 && nClientHeight > 0)
					{
						Gdiplus::Bitmap bitmap(nClientWidth, nScrollHeight);
						Gdiplus::Graphics g(&bitmap);
						HDC hDC = g.GetHDC();
						if (hDC != NULL)
						{
							long nYPos = nScrollHeight - nClientHeight;

							do
							{
								pTopWnd2->scrollTo(0, nYPos);

								{
									long y1 = 0, y2 = 0;
									pDocElem2->get_scrollTop(&y1);
									pBodyElem2->get_scrollTop(&y2);
									nYPos = max(y1, y2);
								}

								HDC hMemDC = ::CreateCompatibleDC(hDC);
								HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, nClientWidth, nClientHeight);
								::SelectObject(hMemDC, hBitmap);
								::PrintWindow(hWnd, hMemDC, PW_CLIENTONLY);
								::SelectObject(hMemDC, NULL);

								::BitBlt(hDC, 0, nYPos, nClientWidth, nClientHeight, hMemDC, 0, 0, SRCCOPY);

								::DeleteDC(hMemDC);
								::DeleteObject(hBitmap);

								if (nYPos <= 0)
								{
									break;
								}
								nYPos -= nClientHeight;
								if (nYPos < 0)
								{
									nYPos = 0;
								}

							} while (true);

							g.ReleaseHDC(hDC);

							CLSID pngClsid;
							GetEncoderClsid(OLESTR("image/png"), &pngClsid);
							bitmap.Save((StringConvertUtils::Instance()->AToW(APP_DIR) + OLESTR("\\mdt.png")).c_str(), &pngClsid);
						}
					}
				}
			}
		}
	}

	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		UINT  num = 0;          // number of image encoders  
		UINT  size = 0;         // size of the image encoder array in bytes  

		Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;  // Failure  

		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1;  // Failure  

		GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success  
			}
		}

		free(pImageCodecInfo);
		return -1;  // Failure  
	}
	void Nav2Url(const BSTR url)
	{
		{
			MyIeUtils::Inst()->ClearFormAutoCompleteHistory();
			MyIeUtils::Inst()->ClearPassword();
			MyIeUtils::Inst()->ClearCookie();
		}
		//SHDeleteKeyW(HKEY_CURRENT_USER, OLESTR("Software\\Microsoft\\Internet Explorer\\IntelliForms"));
		//CleanUrlCache(url);
		//RemoveCache();

		VARIANT varUrl = { 0 };
		VariantInit(&varUrl);
		varUrl.vt = VT_BSTR;
		varUrl.bstrVal = SysAllocString(url);
		VARIANT varFlags = { 0 };
		VariantInit(&varFlags);
		varFlags.vt = VT_I4;
		varFlags.lVal = navNoHistory | navNoReadFromCache | navNoWriteToCache;
		pIWebBrowser2->Navigate2(&varUrl, &varFlags, 0, 0, 0);
		VariantClear(&varFlags);
		SysFreeString(varUrl.bstrVal);
		VariantClear(&varUrl);
	}
	
	int EnumFrameAutoLogon(const BSTR bstrUserName, const  BSTR bstrPassword)
	{
		int nResult = (-1);
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			if (AutoLogon(spIHTMLDocument2, bstrUserName, bstrPassword) != 0)
			{
				// 注：参数OLECONTF_EMBEDDINGS // 表示列举容器内的所有嵌入对象
				CComPtr<IEnumUnknown> spEnumerator = NULL;
				CComQIPtr<IOleContainer> spContainer(spIHTMLDocument2);
				if (SUCCEEDED(spContainer->EnumObjects(OLECONTF_EMBEDDINGS, &spEnumerator)) && (spEnumerator != NULL))
				{
					ULONG celt = 1;
					CComPtr<IUnknown> spIUnknown = NULL;
					while ((nResult != 0) && !(spIUnknown = NULL) && SUCCEEDED(spEnumerator->Next(celt, &spIUnknown, &celt)) && (spIUnknown != NULL))
					{
						CComPtr<IDispatch> pIDispatchTemporary = NULL;
						CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> spIWebBrowser2(spIUnknown);
						if (SUCCEEDED(spIWebBrowser2->get_Document(&pIDispatchTemporary)))
						{
							CComQIPtr<IHTMLDocument2> spIHTMLDocument2Temporary(pIDispatchTemporary);
							nResult = AutoLogon(spIHTMLDocument2Temporary, bstrUserName, bstrPassword);
						}
					}
				}
			}
		}
		return nResult;
	}
	int EnumFrameEnterInbox()
	{
		int nResult = (-1);
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			if (EnterInbox(spIHTMLDocument2) != 0)
			{
				// 注：参数OLECONTF_EMBEDDINGS // 表示列举容器内的所有嵌入对象
				CComPtr<IEnumUnknown> spEnumerator = NULL;
				CComQIPtr<IOleContainer> spContainer(spIHTMLDocument2);
				if (SUCCEEDED(spContainer->EnumObjects(OLECONTF_EMBEDDINGS, &spEnumerator)) && (spEnumerator != NULL))
				{
					ULONG celt = 1;
					CComPtr<IUnknown> spIUnknown = NULL;
					while ((nResult != 0) && !(spIUnknown = NULL) && SUCCEEDED(spEnumerator->Next(celt, &spIUnknown, &celt)) && (spIUnknown != NULL))
					{
						CComPtr<IDispatch> pIDispatchTemporary = NULL;
						CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> spIWebBrowser2(spIUnknown);
						if (SUCCEEDED(spIWebBrowser2->get_Document(&pIDispatchTemporary)))
						{
							CComQIPtr<IHTMLDocument2> spIHTMLDocument2Temporary(pIDispatchTemporary);
							nResult = EnterInbox(spIHTMLDocument2Temporary);
						}
					}
				}
			}
		}
		return nResult;
	}
	int EnumFrameSelectMail()
	{
		int nResult = (-1);
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			if (SelectMail(spIHTMLDocument2) != 0)
			{
				// 注：参数OLECONTF_EMBEDDINGS // 表示列举容器内的所有嵌入对象
				CComPtr<IEnumUnknown> spEnumerator = NULL;
				CComQIPtr<IOleContainer> spContainer(spIHTMLDocument2);
				if (SUCCEEDED(spContainer->EnumObjects(OLECONTF_EMBEDDINGS, &spEnumerator)) && (spEnumerator != NULL))
				{
					ULONG celt = 1;
					CComPtr<IUnknown> spIUnknown = NULL;
					while ((nResult != 0) && !(spIUnknown = NULL) && SUCCEEDED(spEnumerator->Next(celt, &spIUnknown, &celt)) && (spIUnknown != NULL))
					{
						CComPtr<IDispatch> pIDispatchTemporary = NULL;
						CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> spIWebBrowser2(spIUnknown);
						if (SUCCEEDED(spIWebBrowser2->get_Document(&pIDispatchTemporary)))
						{
							CComQIPtr<IHTMLDocument2> spIHTMLDocument2Temporary(pIDispatchTemporary);
							nResult = SelectMail(spIHTMLDocument2Temporary);
						}
					}
				}
			}
		}
		return nResult;
	}
	int EnumFrameReadMail(std::wstring& wstrText)
	{
		int nResult = (-1);
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			if (ReadMail(wstrText, spIHTMLDocument2) != 0)
			{
				// 注：参数OLECONTF_EMBEDDINGS // 表示列举容器内的所有嵌入对象
				CComPtr<IEnumUnknown> spEnumerator = NULL;
				CComQIPtr<IOleContainer> spContainer(spIHTMLDocument2);
				if (SUCCEEDED(spContainer->EnumObjects(OLECONTF_EMBEDDINGS, &spEnumerator)) && (spEnumerator != NULL))
				{
					ULONG celt = 1;
					CComPtr<IUnknown> spIUnknown = NULL;
					while ((nResult != 0) && !(spIUnknown = NULL) && SUCCEEDED(spEnumerator->Next(celt, &spIUnknown, &celt)) && (spIUnknown != NULL))
					{
						CComPtr<IDispatch> pIDispatchTemporary = NULL;
						CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> spIWebBrowser2(spIUnknown);
						if (SUCCEEDED(spIWebBrowser2->get_Document(&pIDispatchTemporary)))
						{
							CComQIPtr<IHTMLDocument2> spIHTMLDocument2Temporary(pIDispatchTemporary);
							nResult = ReadMail(wstrText, spIHTMLDocument2Temporary);
						}
					}
				}
			}
		}
		return nResult;
	}
	
	int RemoveCache()
	{
		// Pointer to a GROUPID variable
		GROUPID groupId = 0;

		// Local variables
		DWORD cacheEntryInfoBufferSizeInitial = 0;
		DWORD cacheEntryInfoBufferSize = 0;
		int* cacheEntryInfoBuffer = 0;
		INTERNET_CACHE_ENTRY_INFOA* internetCacheEntry;
		HANDLE enumHandle = NULL;
		BOOL returnValue = false;

		// Delete the groups first.
		// Groups may not always exist on the system.
		// For more information, visit the following Microsoft Web site:
		// [url]http://msdn2.microsoft.com/en-us/library/ms909365.aspx[/url]                        
		// By default, a URL does not belong to any group. Therefore, that cache may become
		// empty even when the CacheGroup APIs are not used because the existing URL does not belong to any group.                        
		enumHandle = FindFirstUrlCacheGroup(0, CACHEGROUP_SEARCH_ALL, 0, 0, &groupId, 0);

		// If there are no items in the Cache, you are finished.
		if (enumHandle != NULL && ERROR_NO_MORE_ITEMS == GetLastError())
			return 0;

		// Loop through Cache Group, and then delete entries.
		while (1)
		{
			// Delete a particular Cache Group.
			returnValue = DeleteUrlCacheGroup(groupId, CACHEGROUP_FLAG_FLUSHURL_ONDELETE, 0);
			if (groupId <= 0)
				break;

			if (!returnValue && ERROR_FILE_NOT_FOUND == GetLastError())
			{
				returnValue = FindNextUrlCacheGroup(enumHandle, &groupId, 0);
			}

			if (!returnValue && (ERROR_NO_MORE_ITEMS == GetLastError() || ERROR_FILE_NOT_FOUND == GetLastError()))
			{
				break;
			}
		}

		// Start to delete URLs that do not belong to any group.
		enumHandle = FindFirstUrlCacheEntry(NULL, 0, &cacheEntryInfoBufferSizeInitial);
		if (enumHandle != NULL && ERROR_NO_MORE_ITEMS == GetLastError())
			return 0;

		cacheEntryInfoBufferSize = cacheEntryInfoBufferSizeInitial;
		internetCacheEntry = (INTERNET_CACHE_ENTRY_INFOA*)malloc(cacheEntryInfoBufferSize);
		enumHandle = FindFirstUrlCacheEntry(NULL, internetCacheEntry, &cacheEntryInfoBufferSizeInitial);
		while (1)
		{
			cacheEntryInfoBufferSizeInitial = cacheEntryInfoBufferSize;
			returnValue = DeleteUrlCacheEntry(internetCacheEntry->lpszSourceUrlName);

			if (!returnValue)
			{
				returnValue = FindNextUrlCacheEntry(enumHandle, internetCacheEntry, &cacheEntryInfoBufferSizeInitial);
			}

			DWORD dwError = GetLastError();
			if (!returnValue && ERROR_NO_MORE_ITEMS == dwError)
			{
				break;
			}

			if (!returnValue && cacheEntryInfoBufferSizeInitial > cacheEntryInfoBufferSize)
			{
				cacheEntryInfoBufferSize = cacheEntryInfoBufferSizeInitial;
				internetCacheEntry = (INTERNET_CACHE_ENTRY_INFOA*)realloc(internetCacheEntry, cacheEntryInfoBufferSize);
				returnValue = FindNextUrlCacheEntry(enumHandle, internetCacheEntry, &cacheEntryInfoBufferSizeInitial);
			}
		}

		free(internetCacheEntry);
		printf("deleted the cache entries/n");
		return 0;

	}

	CComQIPtr<IHTMLDOMNode> CreateHtmlNode(const CComPtr<IDispatch>& pIDispatch,
		BSTR bstrTagName, std::unordered_map<std::wstring, std::wstring> mapAttribute,
		BSTR bstrValue, bool IsDisableDragEvent = false)
	{
		CComPtr<IHTMLElement> htmlElem = NULL;
		CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
		CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
		CComQIPtr<IHTMLDocument4> spIHTMLDocument4(pIDispatch);

		spIHTMLDocument2->createElement(bstrTagName, (IHTMLElement**)&htmlElem);

		//set attribute
		for (auto it = mapAttribute.begin(); it != mapAttribute.end(); it++)
		{
			if (it->first.length() > 0 && it->second.length() > 0)
			{
				VARIANT attrVal;
				attrVal.vt = VT_BSTR;
				attrVal.bstrVal = (BSTR)(it->second.c_str());
				//htmlElem->setAttribute(L"style",style,0);//style属性是设不上去的。
				htmlElem->setAttribute((BSTR)(it->first.c_str()), attrVal);
			}//end if
		}//end for

		//if we want disable drag event
		if (IsDisableDragEvent)
		{
			VARIANT v;
			v.vt = VT_DISPATCH;
			//v.pdispVal = &m_dp;
			htmlElem->put_ondragstart(v);
			//从下面网址下载所依赖的源代码，为源代码添加头文件引用如果有必要的话，修改不必要的字串A2W格式转换代码。
			//http://www.codeproject.com/Articles/6546/Using-IHTMLEditDesigner
		}

		//set text node
		if (*bstrValue != 0x00)
		{
			htmlElem->put_innerText(bstrValue);
		}

		return CComQIPtr<IHTMLDOMNode>(htmlElem);
	}
	void InjectScript(const CComQIPtr<IHTMLDocument2> & pDocument)
	{
		CComPtr<IHTMLElement> body;
		pDocument->get_body(&body);

		CComQIPtr<IHTMLScriptElement> spScriptObject;
		pDocument->createElement(OLESTR("script"), (IHTMLElement**)&spScriptObject);

		spScriptObject->put_type(OLESTR("text/javascript"));
		spScriptObject->put_text(OLESTR("\nfunction hidediv(){document.getElementById('myOwnUniqueId12345').style.visibility = 'hidden';}\n\n"));
		
		CComQIPtr<IHTMLDOMNode> domnodebody(body);

		CComQIPtr<IHTMLDOMNode> domnodescript(spScriptObject);

		CComPtr<IHTMLDOMNode> pRefNode = NULL;
		domnodebody->appendChild(domnodescript, &pRefNode);
	}
	BOOL TakeHtml(std::string & html)
	{
		BOOL bRetVal = FALSE;
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
			CComQIPtr<IHTMLDocument4> spIHTMLDocument4(pIDispatch);

			HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, 0);
			if (hMemory != NULL)
			{
				CComQIPtr<IPersistStreamInit> spPersistStream(pIDispatch);
				if (spPersistStream != NULL)
				{
					CComPtr<IStream> spIStream = NULL;
					if (SUCCEEDED(CreateStreamOnHGlobal(hMemory, TRUE, &spIStream)) && SUCCEEDED(spPersistStream->Save(spIStream, FALSE)))
					{
						LPCSTR lpHtml = (LPCSTR)GlobalLock(hMemory);
						if (lpHtml != NULL)
						{
							bRetVal = TRUE;
							html = lpHtml;
							GlobalUnlock(hMemory);
						}
					}
				}
				GlobalFree(hMemory);
			}
		}

		return bRetVal;
	}

	std::string MarkDownHtml(const std::string& strText, bool utf8 = false)
	{
		{
			std::stringstream markdownInput(strText);
			if (utf8)
			{
				markdownInput.str(StringConvertUtils::Instance()->WToUTF8(StringConvertUtils::Instance()->AToW(strText)));
			}
			// config is optional
			std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
			config->isEmphasizedParserEnabled = true; // default
			config->isHTMLWrappedInParagraph = true; // default

			std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);
			std::string htmlOutput = parser->Parse(markdownInput);

			//返回UTF-8字符串
			return StringConvertUtils::Instance()->WToUTF8(StringConvertUtils::Instance()->AToW(htmlOutput));
		}
		std::string s((""));
		char* docs = nullptr;
		MMIOT* mmiot = mkd_string(strText.data(), strText.length(), 0);
		if (mmiot != nullptr)
		{
			if (mkd_compile(mmiot, 0) != 0)
			{
				mkd_document(mmiot, &docs);
				if (docs != nullptr)
				{
					if (utf8 == true)
					{
						s = (StringConvertUtils::Instance()->WToA(StringConvertUtils::Instance()->UTF8ToW(docs)));
					}
					else 
					{
						s = (docs);
					}
				}
			}
			mkd_cleanup(mmiot);
		}
		return s;
	}
	int LoadHtml(const std::string & strHtml, bool bAutoCompleted = false)
	{
		int nResult = (-1);
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
			CComQIPtr<IHTMLDocument4> spIHTMLDocument4(pIDispatch);
			std::string dataHtml = "";
			
			CComQIPtr<IPersistStreamInit> spIPersistStreamInit(spIHTMLDocument2);
			if (spIPersistStreamInit == NULL)
			{
				return nResult;
			}
			if (!bAutoCompleted)
			{
				dataHtml.append(strHtml.c_str());
			}
			else
			{
				dataHtml
					.append("<!DOCTYPE html>").append("\n")
					.append("<!-- saved from url=(0014)about:internet -->").append("\n")
					.append("<html xmins='http://www.w3.org/1999/xhtml'>").append("\n")
					.append("<head>").append("\n")
					.append("<meta charset='UTF-8'>").append("\n")
					.append("<meta name='viewport' content='width=device-width, initial-scale=1.0'>").append("\n")
					.append("<meta http-equiv='X-UA-Compatible' content='ie=edge'>").append("\n")
					.append("<title></title>").append("\n")
					//.append("<script type='text/javascript'>" + j_string + "</script>").append("\n")
					//.append("<script type='text/javascript'>" + p_string + "</script>").append("\n")
					//.append("<script type='text/javascript'>" + m_string + "</script>").append("\n")
					.append("<link rel='stylesheet' type='text/css' href='css/css.css'>").append("\n")
					.append("<script type='text/javascript' src='j.js'></script>").append("\n")
					.append("<script type='text/javascript' src='p.js'></script>").append("\n")
					.append("<script type='text/javascript' src='m.js'></script>").append("\n")
					.append("<script type='text/x-mathjax-config'>"
						"MathJax.Hub.Config({"
						"showMathMenu:false,"
						"showProcessingMessages:false,"
						"messageStyle:'none',"
						"jax:['input/TeX','output/HTML-CSS'],"						
						"extensions: [],"
						"tex2jax:{"
						"inlineMath:[['$','$'],['\\(','\\)']],"
						"displayMath:[['$$','$$'],['\\[','\\]']],"
						"skipTags:['script','noscript','style','textarea','pre','code','a']"
						"},"
						"'HTML-CSS':{"
						"availableFonts:['STIX','TeX'],"
						"showMathMenu:false"
						"}"
						"});"
						"MathJax.Hub.Queue(['Typeset',MathJax.Hub]);"
						"</script>").append("\n")
					.append("<script type='text/javascript' src='MathJax.js?config=TeX-AMS-MML_HTMLorMML'></script>").append("\n")
					//.append("<link rel='dns-prefetch' href='https://cdn.bootcss.com' />").append("\n")
					//.append("<script type='text/javascript' src='https://cdn.bootcss.com/mathjax/2.7.2/latest.js?config=TeX-AMS-MML_HTMLorMML'></script>").append("\n")
					.append("</head>").append("\n")
					.append("<body>").append("\n")
					.append("<script>var config={startOnLoad:true,flowchart:{htmlLabels:false}};mermaid.initialize(config);</script>").append("\n")
					.append(strHtml.c_str()).append("\n")
					.append("</body>").append("\n")
					.append("</html>");

				CHAR czTempPath[MAX_PATH] = { 0 };
				GetTempPathA(sizeof(czTempPath) / sizeof(*czTempPath), czTempPath);
				strcat(czTempPath, "MDT\\");
				std::string my_html = czTempPath + std::string("MDT.HTML");
				FILE_WRITER(dataHtml.data(), dataHtml.size(), my_html.c_str(), std::ios::binary);
				Nav2Url((BSTR)StringConvertUtils::Instance()->AToW(my_html).c_str());
				return 0;
			}
			LPSTR lpMemory = (LPSTR)::GlobalAlloc(GPTR, (dataHtml.length() + 1) * sizeof(CHAR));
			if (lpMemory != NULL)
			{
				lstrcpyA(lpMemory, dataHtml.c_str());

				CComPtr<IStream> spIStream = NULL;
				// 初始化后，装载显示
				if (SUCCEEDED(CreateStreamOnHGlobal(lpMemory, TRUE, &spIStream)) &&
					SUCCEEDED(spIPersistStreamInit->InitNew()) &&
					SUCCEEDED(spIPersistStreamInit->Load(spIStream)))
				{
					nResult = 0;
				}
				//GlobalFree(lpMemory);
			}
		}
		return nResult;
	}
	int FrameConvert()
	{
		CComPtr<IDispatch> pIDispatch = NULL;
		if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
		{
			CComPtr<IHTMLDOMNode> pRefNode = NULL;
			CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
			CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
			CComQIPtr<IHTMLDocument4> spIHTMLDocument4(pIDispatch);

			VARIANT varBgColor;
			//varBgColor.
			spIHTMLDocument2->get_bgColor(&varBgColor);
			varBgColor.vt = VT_COLOR;
			varBgColor.bstrVal = OLESTR("#FF0000");
			spIHTMLDocument2->put_bgColor(varBgColor);
			//Root node
			std::unordered_map<std::wstring, std::wstring> mapAttr;
			//mapAttr[OLESTR("class")] = OLESTR("slot");
			CComQIPtr<IHTMLDOMNode> nodeRoot = CreateHtmlNode(pIDispatch, OLESTR("div"), mapAttr, OLESTR(""));

			//
			mapAttr.clear();
			//mapAttr[OLESTR("class")] = OLESTR("pic-panel");
			CComQIPtr<IHTMLDOMNode> nodeDiv = CreateHtmlNode(pIDispatch, OLESTR("div"), mapAttr, OLESTR(""));

			pRefNode = NULL;
			nodeRoot->appendChild(nodeDiv, &pRefNode);

			mapAttr.clear();
			//mapAttr[OLESTR("class")] = OLESTR("pic-panel");
			CComQIPtr<IHTMLDOMNode> nodeH1 = CreateHtmlNode(pIDispatch, OLESTR("h1"), mapAttr, OLESTR("一级标题"));
			CComQIPtr<IHTMLDOMNode> nodeH2 = CreateHtmlNode(pIDispatch, OLESTR("h2"), mapAttr, OLESTR("二级标题"));
			CComQIPtr<IHTMLDOMNode> nodeH3 = CreateHtmlNode(pIDispatch, OLESTR("h3"), mapAttr, OLESTR("三级标题"));

			{
				CComPtr<IHTMLElement> elemBody;
				//spIHTMLDocument3->getElementById(OLESTR("mydiv"), &elemBody);
				spIHTMLDocument2->get_body(&elemBody);
				CComQIPtr<IHTMLDOMNode> domBody(elemBody);
				pRefNode = NULL; domBody->appendChild(nodeRoot, &pRefNode);
				pRefNode = NULL; domBody->appendChild(nodeH1, &pRefNode);
				pRefNode = NULL; domBody->appendChild(nodeH2, &pRefNode);
				pRefNode = NULL; domBody->appendChild(nodeH3, &pRefNode);
				
				//调试使用
				CComPtr<IHTMLElement> body;
				spIHTMLDocument2->get_body((IHTMLElement**)&body);
				BSTR bHtml = NULL;
				body->get_outerHTML(&bHtml);
				LoadHtml(StringConvertUtils::Instance()->WToA(bHtml));
				std::string strHtml;
				TakeHtml(strHtml);
				bHtml = NULL;
			}
		}
		return 0;
	}

	int MarkdownToHtmlview(std::vector<std::string>& strTag, std::vector<std::string>& strValue, const std::string& strText)
	{
		std::vector<std::string> sv;
		StringConvertUtils::Instance()->string_split_to_vector(sv, strText, "\r\n");
		for (auto it : sv)
		{
			std::string t = it.c_str();
			OutputDebugStringA(t.c_str());
			OutputDebugStringA("\n");

			if (t.length() > 4)
			{
				if (t.compare(0, 2, "**") == 0 && t.compare(t.length() - 2, 2, "**") == 0)
				{
					std::string text = t.substr(2, t.length() - 4);
					strTag.emplace_back("strong");
					strValue.emplace_back(text);
					continue;
				}
				if (t.compare(0, 2, "__") == 0 && t.compare(t.length() - 2, 2, "__") == 0)
				{
					std::string text = t.substr(2, t.length() - 4);
					strTag.emplace_back("b");
					strValue.emplace_back(text);
					continue;
				}
			}
			if (t.length() > 3)
			{
				if (t.compare(0, 3, "###") == 0)
				{
					std::string text = t.substr(3);
					strTag.emplace_back("h3");
					strValue.emplace_back(text);
					continue;
				}
			}
			if (t.length() > 2)
			{
				if (t.compare(0, 2, "##") == 0)
				{
					std::string text = t.substr(2);
					strTag.emplace_back("h2");
					strValue.emplace_back(text);
					continue;
				}

				if (*t.begin() == '*' && *t.rbegin() == '*')
				{
					std::string text = t.substr(1, t.length() - 2);
					strTag.emplace_back("em");
					strValue.emplace_back(text);
					continue;
				}
				if (*t.begin() == '_' && *t.rbegin() == '_')
				{
					std::string text = t.substr(1, t.length() - 2);
					strTag.emplace_back("i");
					strValue.emplace_back(text);
					continue;
				}
			}
			if (t.length() > 1)
			{
				if (t.compare(0, 1, "#") == 0)
				{
					std::string text = t.substr(1);
					strTag.emplace_back("h1");
					strValue.emplace_back(text);
					continue;
				}
			}
		}
		return 0;
	}
	int AddNewElem(const std::vector<std::string>& strTag, const std::vector<std::string>& strValue)
	{
		if (strTag.size() > 0 && strTag.size() == strValue.size())
		{
			CComPtr<IDispatch> pIDispatch = NULL;
			if (SUCCEEDED(pIWebBrowser2->get_Document(&pIDispatch)) && (pIDispatch != NULL))
			{
				CComPtr<IHTMLDOMNode> pRefNode = NULL;
				CComQIPtr<IHTMLDocument2> spIHTMLDocument2(pIDispatch);
				CComQIPtr<IHTMLDocument3> spIHTMLDocument3(pIDispatch);
				CComQIPtr<IHTMLDocument4> spIHTMLDocument4(pIDispatch);

				VARIANT varBgColor;
				spIHTMLDocument2->get_bgColor(&varBgColor);
				varBgColor.vt = VT_COLOR;
				varBgColor.bstrVal = OLESTR("#FF0000");
				spIHTMLDocument2->put_bgColor(varBgColor);
				//Root node
				std::unordered_map<std::wstring, std::wstring> mapAttr;
				//mapAttr[OLESTR("class")] = OLESTR("slot");
				std::vector< CComQIPtr<IHTMLDOMNode>> domNodeList;
				for (auto i = 0; i < strTag.size(); i++)
				{
					domNodeList.emplace_back(CreateHtmlNode(pIDispatch, (BSTR)StringConvertUtils::Instance()->AToW(strTag.at(i)).c_str(), mapAttr, (BSTR)StringConvertUtils::Instance()->AToW(strValue.at(i)).c_str()));
				}
				//CComQIPtr<IHTMLDOMNode> tagDomNode = CreateHtmlNode(pIDispatch, bstrTag, mapAttr, bstrValue);

				{
					CComPtr<IHTMLElement> elemBody;
					spIHTMLDocument2->get_body(&elemBody);
					CComQIPtr<IHTMLDOMNode> domBody(elemBody);
					for (auto& it : domNodeList)
					{
						pRefNode = NULL; domBody->appendChild(it, &pRefNode);
					}

					//调试使用
					CComPtr<IHTMLElement> body;
					spIHTMLDocument2->get_body((IHTMLElement**)&body);
					BSTR bHtml = NULL;
					body->get_outerHTML(&bHtml);
					LoadHtml(StringConvertUtils::Instance()->WToA(bHtml));
					std::string strHtml;
					TakeHtml(strHtml);
					bHtml = NULL;
				}
			}
		}
		return 0;
	}

public:

	HRESULT RegisterIEEventDealer()
	{
		// 声明一个IConnectionPointContainer和IConnectionPoint实例。    
		// 利用 IWebBrowser2 接口的 QueryInterface 方法获得 IConnectionPointContainer 接口  
		if (SUCCEEDED(pIWebBrowser2->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer)))
		{
			// 利用 IConnectionPointContainer 接口的 FindConnectionPoint 获取 IID为DIID_DWebBrowserEvents2 的连接点  
			if (SUCCEEDED(pConnectionPointContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &pConnectionPointBrowserEvents)))
			{
				// 利用IID为DIID_DWebBrowserEvents2的连接点的Advise建立一个实现了DWebBrowserEvents2接口的接收器的实例和此连接点的连接。
				// 第一个参数就是接收器的实例，必须是一个实现了DWebBrowserEvents2接口的类的实例 
				// 在这里我们设置成this,也就是自己实现了DWebBrowserEvents2接口，这个是通过继承CWebEventSink实现的   
				if (SUCCEEDED(pConnectionPointBrowserEvents->Advise((IUnknown*)pWebBrowserSink, &dwCookie)))
				{
					// Successfully advised
					return S_OK;
				}
			}
		}
		return S_FALSE;
	}

	void UnRegisterIEEventDealer()
	{
		if (dwCookie > 0 && pConnectionPointBrowserEvents != NULL)
		{
			pConnectionPointBrowserEvents->Unadvise(dwCookie);
			dwCookie = 0;
		}
	}
	bool Initialize(HWND hWndParent, 
		DWORD dwEditStatusID,
		RECT& rect,
		LPCTSTR lpWindowName = TEXT(""),
		DWORD dwStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER,
		DWORD dwExStyle = 0,
		_U_MENUorID MenuOrID = 0U,
		LPVOID lpCreateParam = NULL
	)
	{
		this->hWndParent = hWndParent;
		this->dwEditStatusID = dwEditStatusID;
		if (((this->hWnd = axWindowContainer.Create(this->hWndParent, &rect, lpWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam)) != NULL)
			&& SUCCEEDED(axWindowContainer.CreateControl(OLE_IEXPLORER_2_NAME))
			&& SUCCEEDED(axWindowContainer.QueryControl(__uuidof(IWebBrowser2), (void**)&pIWebBrowser2))
			&& SUCCEEDED(PutSilent(TRUE))
			&& SUCCEEDED(RegisterIEEventDealer())
			&& SUCCEEDED(axWindowContainer.QueryHost(IID_IAxWinAmbientDispatch, (LPVOID*)&pIAxWinAmbientDispatch))
			&& SUCCEEDED(pIAxWinAmbientDispatch->put_AllowContextMenu(VARIANT_FALSE))
			&& SUCCEEDED(pIAxWinAmbientDispatch->put_AllowShowUI(VARIANT_FALSE))
			&& SUCCEEDED(pIAxWinAmbientDispatch->put_DocHostFlags(dwPreviewFlags)))
		{
			Nav2Url(OLESTR("about:blank"));
			SetBrowserFeatureControl();
			return true;
		}
		return false;
	}
	void Unitialize()
	{
		UnRegisterIEEventDealer();
		if (pIWebBrowser2 != NULL)
		{
			pIWebBrowser2.Release();
		}
		if (pWebBrowserSink != NULL)
		{
			pWebBrowserSink.Release();
		}
		if (pHtmlElementSink != NULL)
		{
			pHtmlElementSink.Release();
		}
		if (pDocHostUIHandler != NULL)
		{
			pDocHostUIHandler.Release();
		}
		if (pIAxWinAmbientDispatch != NULL)
		{
			pIAxWinAmbientDispatch.Release();
		}
		if (pConnectionPointContainer != NULL)
		{
			pConnectionPointContainer.Release();
		}
		if (pConnectionPointBrowserEvents != NULL)
		{
			pConnectionPointBrowserEvents.Release();
		}
	}
	HRESULT PutSilent(VARIANT_BOOL bSilent = TRUE)
	{
		return pIWebBrowser2->put_Silent(bSilent);
	}

private:
	BOOL CleanUrlCache(const BSTR bstrUrl)
	{
		BOOL bResult = FALSE;
		LPINTERNET_CACHE_ENTRY_INFOW lpICEIW = NULL;
		DWORD dwSize = 0;
		DWORD dwFindSize = 0;
		HANDLE hFind = FindFirstUrlCacheEntryW(NULL, NULL, &dwSize);
		if (hFind == NULL && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			lpICEIW = (LPINTERNET_CACHE_ENTRY_INFOW)malloc(dwSize);
			if (lpICEIW != NULL && (hFind = FindFirstUrlCacheEntryW(NULL, lpICEIW, &dwSize)) != NULL)
			{
				do
				{
					if (StrStrIW(lpICEIW->lpszSourceUrlName, bstrUrl))
					{
						int bRet = DeleteUrlCacheEntryW(lpICEIW->lpszSourceUrlName);
					}

					dwFindSize = 0;
					if (!FindNextUrlCacheEntryW(hFind, NULL, &dwFindSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					{
						break;
					}
					if (dwFindSize > dwSize)
					{
						dwSize = dwFindSize;
						lpICEIW = (LPINTERNET_CACHE_ENTRY_INFOW)realloc(lpICEIW, dwSize);
					}
				} while (FindNextUrlCacheEntryW(hFind, lpICEIW, &dwSize));

				free(lpICEIW);
				bResult = TRUE;
			}
		}
		return bResult;
	}
	bool SetBrowserFeatureControlKey(std::wstring feature, wchar_t* appName, DWORD value) {
		HKEY key;
		bool success = true;
		std::wstring featuresPath(L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\");
		std::wstring path(featuresPath + feature);
		LONG nError = RegCreateKeyExW(HKEY_CURRENT_USER, path.c_str(), 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL);
		if (nError != ERROR_SUCCESS) {
			success = false;
		}
		else {
			nError = RegSetValueExW(key, appName, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
			if (nError != ERROR_SUCCESS) {
				success = false;
			}
			nError = RegCloseKey(key);
			if (nError != ERROR_SUCCESS) {
				success = false;
			}
		}
		return success;
	}
	void SetBrowserFeatureControl() {
		// http://msdn.microsoft.com/en-us/library/ee330720(v=vs.85).aspx
		DWORD emulationMode = GetBrowserEmulationMode();
		if (emulationMode > 0) {
			// FeatureControl settings are per-process
			wchar_t fileName[MAX_PATH + 1];
			ZeroMemory(fileName, (MAX_PATH + 1) * sizeof(wchar_t));
			GetModuleFileNameW(NULL, fileName, MAX_PATH);
			std::vector<std::string> splittedFileName;
			StringConvertUtils::Instance()->string_split_to_vector(splittedFileName, StringConvertUtils::Instance()->WToA(fileName), ("\\"));
			ZeroMemory(fileName, (MAX_PATH + 1) * sizeof(wchar_t));
			std::wstring exeName = StringConvertUtils::Instance()->AToW(splittedFileName.at(splittedFileName.size() - 1));
			memcpy(fileName, exeName.c_str(), sizeof(wchar_t) * exeName.length());
			// make the control is not running inside Visual Studio Designer
			//if (String.Compare(fileName, "devenv.exe", true) == 0 || String.Compare(fileName, "XDesProc.exe", true) == 0) {
			//  return;
			//}
			// Windows Internet Explorer 8 and later. The FEATURE_BROWSER_EMULATION feature defines the default emulation mode for Internet
			// Explorer and supports the following values.
			// Webpages containing standards-based !DOCTYPE directives are displayed in IE10 Standards mode.
			SetBrowserFeatureControlKey(L"FEATURE_BROWSER_EMULATION", fileName, emulationMode);
			// Internet Explorer 8 or later. The FEATURE_AJAX_CONNECTIONEVENTS feature enables events that occur when the value of the online
			// property of the navigator object changes, such as when the user chooses to work offline. For more information, see the ononline
			// and onoffline events.
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_AJAX_CONNECTIONEVENTS", fileName, 1);
			// Internet Explorer 9. Internet Explorer 9 optimized the performance of window-drawing routines that involve clipping regions associated
			// with child windows. This helped improve the performance of certain window drawing operations. However, certain applications hosting the
			// WebBrowser Control rely on the previous behavior and do not function correctly when these optimizations are enabled. The
			// FEATURE_ENABLE_CLIPCHILDREN_OPTIMIZATION feature can disable these optimizations.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(L"FEATURE_ENABLE_CLIPCHILDREN_OPTIMIZATION", fileName, 1);
			// Internet Explorer 8 and later. By default, Internet Explorer reduces memory leaks caused by circular references between Internet Explorer
			// and the Microsoft JScript engine, especially in scenarios where a webpage defines an expando and the page is refreshed. If a legacy
			// application no longer functions with these changes, the FEATURE_MANAGE_SCRIPT_CIRCULAR_REFS feature can disable these improvements.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(L"FEATURE_MANAGE_SCRIPT_CIRCULAR_REFS", fileName, 1);
			// Windows Internet Explorer 8. When enabled, the FEATURE_DOMSTORAGE feature allows Internet Explorer and applications hosting the WebBrowser
			// Control to use the Web Storage API. For more information, see Introduction to Web Storage.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(L"FEATURE_DOMSTORAGE ", fileName, 1);
			// Internet Explorer 9. The FEATURE_GPU_RENDERING feature enables Internet Explorer to use a graphics processing unit (GPU) to render content.
			// This dramatically improves performance for webpages that are rich in graphics.
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_GPU_RENDERING ", fileName, 1);
			// Internet Explorer 9. By default, the WebBrowser Control uses Microsoft DirectX to render webpages, which might cause problems for
			// applications that use the Draw method to create bitmaps from certain webpages. In Internet Explorer 9, this method returns a bitmap
			// (in a Windows Graphics Device Interface (GDI) wrapper) instead of a GDI metafile representation of the webpage. When the
			// FEATURE_IVIEWOBJECTDRAW_DMLT9_WITH_GDI feature is enabled, the following conditions cause the Draw method to use GDI instead of DirectX
			// to create the resulting representation. The GDI representation will contain text records and vector data, but is not guaranteed to be
			// similar to the same represenation returned in earlier versions of the browser:
			//    The device context passed to the Draw method points to an enhanced metafile.
			//    The webpage is not displayed in IE9 Standards mode.
			// By default, this feature is ENABLED for applications hosting the WebBrowser Control. This feature is ignored by Internet Explorer and
			// Windows Explorer. To enable this feature by using the registry, add the name of your executable file to the following setting.
			SetBrowserFeatureControlKey(L"FEATURE_IVIEWOBJECTDRAW_DMLT9_WITH_GDI  ", fileName, 0);
			// Windows 8 introduces a new input model that is different from the Windows 7 input model. In order to provide the broadest compatibility
			// for legacy applications, the WebBrowser Control for Windows 8 emulates the Windows 7 mouse, touch, and pen input model (also known as the
			// legacy input model). When the legacy input model is in effect, the following conditions are true:
			//    Windows pointer messages are not processed by the Trident rendering engine (mshtml.dll).
			//    Document Object Model (DOM) pointer and gesture events do not fire.
			//    Mouse and touch messages are dispatched according to the Windows 7 input model.
			//    Touch selection follows the Windows 7 model ("drag to select") instead of the Windows 8 model ("tap to select").
			//    Hardware accelerated panning and zooming is disabled.
			//    The Zoom and Pan Cascading Style Sheets (CSS) properties are ignored.
			// The FEATURE_NINPUT_LEGACYMODE feature control determines whether the legacy input model is enabled
			// Default: ENABLED
			SetBrowserFeatureControlKey(L"FEATURE_NINPUT_LEGACYMODE", fileName, 0);
			// Internet Explorer 7 consolidated HTTP compression and data manipulation into a centralized component in order to improve performance and
			// to provide greater consistency between transfer encodings (such as HTTP no-cache headers). For compatibility reasons, the original
			// implementation was left in place. When the FEATURE_DISABLE_LEGACY_COMPRESSION feature is disabled, the original compression implementation
			// is used.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(L"FEATURE_DISABLE_LEGACY_COMPRESSION", fileName, 1);
			// When the FEATURE_LOCALMACHINE_LOCKDOWN feature is enabled, Internet Explorer applies security restrictions on content loaded from the
			// user's local machine, which helps prevent malicious behavior involving local files:
			//    Scripts, Microsoft ActiveX controls, and binary behaviors are not allowed to run.
			//    Object safety settings cannot be overridden.
			//    Cross-domain data actions require confirmation from the user.
			// Default: DISABLED
			// SetBrowserFeatureControlKey(L"FEATURE_LOCALMACHINE_LOCKDOWN", fileName, 0);
			// Internet Explorer 7 and later. When enabled, the FEATURE_BLOCK_LMZ_??? feature allows ??? stored in the Local Machine zone to be
			// loaded only by webpages loaded from the Local Machine zone or by webpages hosted by sites in the Trusted Sites list. For more information,
			// see Security and Compatibility in Internet Explorer 7.
			// Default: DISABLED
			//    FEATURE_BLOCK_LMZ_IMG can block images that try to load from the user's local file system. To opt in, add your process name and set 
			//                          the value to 0x00000001.
			//    FEATURE_BLOCK_LMZ_OBJECT can block objects that try to load from the user's local file system. To opt in, add your process name and
			//                          set the value to 0x00000001.
			//    FEATURE_BLOCK_LMZ_SCRIPT can block script access from the user's local file system. To opt in, add your process name and set the value
			//                          to 0x00000001.
			// SetBrowserFeatureControlKey(L"FEATURE_BLOCK_LMZ_OBJECT", fileName, 0);
			// SetBrowserFeatureControlKey(L"FEATURE_BLOCK_LMZ_OBJECT", fileName, 0);
			// SetBrowserFeatureControlKey(L"FEATURE_BLOCK_LMZ_SCRIPT", fileName, 0);
			// Internet Explorer 8 and later. When enabled, the FEATURE_DISABLE_NAVIGATION_SOUNDS feature disables the sounds played when you open a
			// link in a webpage.
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_DISABLE_NAVIGATION_SOUNDS", fileName, 1);
			// Windows Internet Explorer 7 and later. Prior to Internet Explorer 7, href attributes of a objects supported the javascript prototcol;
			// this allowed webpages to execute script when the user clicked a link. For security reasons, this support was disabled in Internet
			// Explorer 7. For more information, see Event 1034 - Cross-Domain Barrier and Script URL Mitigation.
			// When enabled, the FEATURE_SCRIPTURL_MITIGATION feature allows the href attribute of a objects to support the javascript prototcol. 
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_SCRIPTURL_MITIGATION", fileName, 1);
			// For Windows 8 and later, the FEATURE_SPELLCHECKING feature controls this behavior for Internet Explorer and for applications hosting
			// the web browser control (WebOC). When fully enabled, this feature automatically corrects grammar issues and identifies misspelled words
			// for the conditions described earlier.
			//    (DWORD) 00000000 - Features are disabled.
			//    (DWORD) 00000001 - Features are enabled for the conditions described earlier. (This is the default value.)
			//    (DWORD) 00000002 - Features are enabled, but only for elements that specifically set the spellcheck attribute to true.
			SetBrowserFeatureControlKey(L"FEATURE_SPELLCHECKING", fileName, 0);
			// When enabled, the FEATURE_STATUS_BAR_THROTTLING feature limits the frequency of status bar updates to one update every 200 milliseconds.
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_STATUS_BAR_THROTTLING", fileName, 1);
			// Internet Explorer 7 or later. When enabled, the FEATURE_TABBED_BROWSING feature enables tabbed browsing navigation shortcuts and
			// notifications. For more information, see Tabbed Browsing for Developers.
			// Default: DISABLED
			// SetBrowserFeatureControlKey(L"FEATURE_TABBED_BROWSING", fileName, 1);
			// When enabled, the FEATURE_VALIDATE_NAVIGATE_URL feature control prevents Windows Internet Explorer from navigating to a badly formed URL.
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_VALIDATE_NAVIGATE_URL", fileName, 1);
			// When enabled,the FEATURE_WEBOC_DOCUMENT_ZOOM feature allows HTML dialog boxes to inherit the zoom state of the parent window.
			// Default: DISABLED
			SetBrowserFeatureControlKey(L"FEATURE_WEBOC_DOCUMENT_ZOOM", fileName, 1);
			// The FEATURE_WEBOC_POPUPMANAGEMENT feature allows applications hosting the WebBrowser Control to receive the default Internet Explorer
			// pop-up window management behavior.
			// Default: ENABLED
			SetBrowserFeatureControlKey(L"FEATURE_WEBOC_POPUPMANAGEMENT", fileName, 0);
			// Applications hosting the WebBrowser Control should ensure that window resizing and movement events are handled appropriately for the
			// needs of the application. By default, these events are ignored if the WebBrowser Control is not hosted in a proper container. When enabled,
			// the FEATURE_WEBOC_MOVESIZECHILD feature allows these events to affect the parent window of the application hosting the WebBrowser Control.
			// Because this can lead to unpredictable results, it is not considered desirable behavior.
			// Default: DISABLED
			// SetBrowserFeatureControlKey(L"FEATURE_WEBOC_MOVESIZECHILD", fileName, 0);
			// The FEATURE_ADDON_MANAGEMENT feature enables applications hosting the WebBrowser Control
			// to respect add-on management selections made using the Add-on Manager feature of Internet Explorer.
			// Add-ons disabled by the user or by administrative group policy will also be disabled in applications that enable this feature.
			SetBrowserFeatureControlKey(L"FEATURE_ADDON_MANAGEMENT", fileName, 0);
			// Internet Explorer 10. When enabled, the FEATURE_WEBSOCKET feature allows script to create and use WebSocket objects.
			// The WebSocketobject allows websites to request data across domains from your browser by using the WebSocket protocol.
			// Default: ENABLED
			SetBrowserFeatureControlKey(L"FEATURE_WEBSOCKET", fileName, 1);
			// When enabled, the FEATURE_WINDOW_RESTRICTIONS feature adds several restrictions to the size and behavior of popup windows:
			//    Popup windows must appear in the visible display area.
			//    Popup windows are forced to have status and address bars.
			//    Popup windows must have minimum sizes.
			//    Popup windows cannot cover important areas of the parent window.
			// When enabled, this feature can be configured differently for each security zone by using the URLACTION_FEATURE_WINDOW_RESTRICTIONS URL
			// action flag. 
			// Default: ENABLED
			SetBrowserFeatureControlKey(L"FEATURE_WINDOW_RESTRICTIONS", fileName, 0);
			// Internet Explorer 7 and later. The FEATURE_XMLHTTP feature enables or disables the native XMLHttpRequest object.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(L"FEATURE_XMLHTTP", fileName, 1);
		}
	}
	LONG GetDWORDRegKey(HKEY hKey, const std::wstring& strValueName, DWORD& nValue, DWORD nDefaultValue) {
		nValue = nDefaultValue;
		DWORD dwBufferSize(sizeof(DWORD));
		DWORD nResult(0);
		LONG nError = ::RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, reinterpret_cast<LPBYTE>(&nResult), &dwBufferSize);
		if (ERROR_SUCCESS == nError) {
			nValue = nResult;
		}
		return nError;
	}
	LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue) {
		strValue = strDefaultValue;
		WCHAR szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError;
		nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		if (ERROR_SUCCESS == nError) {
			strValue = szBuffer;
		}
		return nError;
	}
	DWORD GetBrowserEmulationMode() {
			int browserVersion = 11;
			std::wstring sBrowserVersion;
			HKEY key;
			bool success = true;
			BYTE data[256];
			std::wstring path(L"SOFTWARE\\Microsoft\\Internet Explorer");
			LONG nError = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &key);
			DWORD mode = 11000; // Internet Explorer 11. Webpages containing standards-based !DOCTYPE directives are displayed in IE11 Standards mode. Default value for Internet Explorer 11.
			if (nError != ERROR_SUCCESS) {
				success = false;
			}
			else {
				nError = GetStringRegKey(key, L"svcVersion", sBrowserVersion, L"7");
				if (nError != ERROR_SUCCESS) {
					nError = GetStringRegKey(key, L"version", sBrowserVersion, L"7");
					if (nError != ERROR_SUCCESS) {
						success = false;
					}
				}
				if (RegCloseKey(key) != ERROR_SUCCESS) {
					success = false;
				}
			}
			if (success) {
				std::vector<std::string> splittedBrowserVersion;
				StringConvertUtils::Instance()->string_split_to_vector(splittedBrowserVersion, StringConvertUtils::Instance()->WToA(sBrowserVersion), ("."));
				browserVersion = std::atoi(splittedBrowserVersion.at(0).c_str()); // convert base 16 number in s to int
				/////////////////////////////////////////////////////////
				//  11001(0×2af9)    IE11  忽略html5
				//  11000(0×2af8)    IE11
				//  10001(0×2711)    IE10  忽略html5
				//  10000(0×2710)    IE10
				//  9999 (0x270F)    IE9   忽略html5
				//  9000 (0×2328)    IE9
				//  8888 (0x22B8)    IE8   忽略html5
				//  8000 (0x1F40)    IE8
				//  7000 (0x1B58)    IE7
				switch (browserVersion) {
				case 7:
					mode = 7000; // Webpages containing standards-based !DOCTYPE directives are displayed in IE7 Standards mode. Default value for applications hosting the WebBrowser Control.
					break;
				case 8:
					mode = 8000; // Webpages containing standards-based !DOCTYPE directives are displayed in IE8 mode. Default value for Internet Explorer 8
					break;
				case 9:
					mode = 9000; // Internet Explorer 9. Webpages containing standards-based !DOCTYPE directives are displayed in IE9 mode. Default value for Internet Explorer 9.
					break;
				case 10:
					mode = 10000; // Internet Explorer 10. Webpages containing standards-based !DOCTYPE directives are displayed in IE10 mode. Default value for Internet Explorer 10.
					break;
				case 11:
					mode = 11000; // Internet Explorer 11. Webpages containing standards-based !DOCTYPE directives are displayed in IE11 mode. Default value for Internet Explorer 11.
					break;
				default:
					// use IE11 mode by default
					mode = 11000; // Internet Explorer 11. Webpages containing standards-based !DOCTYPE directives are displayed in IE11 mode. Default value for Internet Explorer 11.
					break;
				}
			}
			else {
				mode = -1;
			}
			return mode;
		}
public:
	void EnumIE(void)
	{
		CComPtr<IShellWindows> spShellWin;
		HRESULT hr = spShellWin.CoCreateInstance(CLSID_ShellWindows);
		if (FAILED(hr))
		{
			return;
		}

		long nCount = 0;    //取得浏览器实例个数(Explorer和IExplorer)  
		spShellWin->get_Count(&nCount);
		if (0 == nCount)
		{
			return;
		}

		for (int i = 0; i < nCount; i++)
		{
			CComPtr<IDispatch> spDispIE;
			hr = spShellWin->Item(CComVariant((long)i), &spDispIE);
			if (FAILED(hr)) continue;

			CComQIPtr<IWebBrowser2>spBrowser = spDispIE;
			if (!spBrowser) continue;

			CComPtr<IDispatch> spDispDoc;
			hr = spBrowser->get_Document(&spDispDoc);
			if (FAILED(hr)) continue;

			CComQIPtr<IHTMLDocument2>spDocument2 = spDispDoc;
			if (!spDocument2) continue;

			//Modify by jncao 2007-09-17
			//*******************************************************************************
			std::wstring cIEUrl_Filter;  //设置的URL(必须是此URL的网站才有效);
			cIEUrl_Filter = L"http://127.0.0.1:8082/csp/"; //设置过滤的网址
			//*******************************************************************************

			CComBSTR IEUrl;
			spBrowser->get_LocationURL(&IEUrl);
			std::wstring cIEUrl_Get;     //从机器上取得的HTTP的完整的URL;
			cIEUrl_Get = IEUrl;
			cIEUrl_Get = cIEUrl_Get.substr(0, cIEUrl_Filter.length()); //截取前面N位

			if (lstrcmpW(cIEUrl_Get.c_str(), cIEUrl_Filter.c_str()) == 0)
			{
				// 程序运行到此，已经找到了IHTMLDocument2的接口指针      
				EnumAllElement(spDocument2);//枚举所有元素
			}
		}
	}
	void EnumFrame(IHTMLDocument2* pIHTMLDocument2)
	{
		if (!pIHTMLDocument2) return;
		HRESULT   hr;

		CComPtr<IHTMLFramesCollection2> spFramesCollection2;
		pIHTMLDocument2->get_frames(&spFramesCollection2); //取得框架frame的集合  

		long nFrameCount = 0;        //取得子框架个数  
		hr = spFramesCollection2->get_length(&nFrameCount);
		if (FAILED(hr) || 0 == nFrameCount) return;

		for (long i = 0; i < nFrameCount; i++)
		{
			CComVariant vDispWin2; //取得子框架的自动化接口  
			hr = spFramesCollection2->item(&CComVariant(i), &vDispWin2);
			if (FAILED(hr)) continue;
			CComQIPtr<IHTMLWindow2>spWin2 = vDispWin2.pdispVal;
			if (!spWin2) continue; //取得子框架的   IHTMLWindow2   接口      
			CComPtr <IHTMLDocument2> spDoc2;
			spWin2->get_document(&spDoc2); //取得子框架的   IHTMLDocument2   接口

			EnumAllElement(spDoc2);      //递归枚举当前子框架   IHTMLDocument2   上的所有控件
		}
	}
	void EnumAllElement(IHTMLDocument2* pIHTMLDocument2, BOOL bContinue = TRUE) //枚举所有字段
	{
		if (!pIHTMLDocument2) return;
		if (bContinue)
		{
			EnumFrame(pIHTMLDocument2);   //递归枚举当前IHTMLDocument2上的子框架frame  
		}
		HRESULT   hr;

		CComQIPtr<IHTMLElementCollection> spAllElement;
		hr = pIHTMLDocument2->get_all(&spAllElement);//获取所有网页内所有元素
		if (FAILED(hr))  return;

		long nLength = 0;
		spAllElement->get_length(&nLength);
		for (int i = 0; i < nLength; i++)
		{
			CComPtr<IDispatch> pDisp;
			VARIANT varName;
			varName.vt = VT_I4;
			varName.llVal = i;
			VARIANT varIndex;
			varIndex.vt = VT_I4;
			varIndex.llVal = 0;
			hr = spAllElement->item(varName, varIndex, &pDisp); //获取单个元素
			if (SUCCEEDED(hr))
			{
				//CComQIPtr <IHTMLElement, &IID_IHTMLElement> pElement(pDisp);
				CComQIPtr<IHTMLElement, &IID_IHTMLElement> pElement;
				pDisp->QueryInterface(&pElement);
				BSTR bTemp;
				//pElement->get_id(&bTemp);//可以获取其他特征，根据具体元素而定
				//pElement->get_className(&bTemp);
				pElement->get_tagName(&bTemp);
				//pElement->get_title(&bTemp);
				//pElement->get_innerHTML(&bTemp);
				//pElement->get_innerText(&bTemp);
				if (bTemp == NULL)
				{
					continue;
				}
				std::wstring strTemp = bTemp;
				if (!strTemp.empty() && strTemp.compare(L"callNo") == 0)//根据callNo（效能提升text控件id）是主叫号码获取值或作其他处理
				{
					IHTMLInputTextElement* input;
					pDisp->QueryInterface(IID_IHTMLInputTextElement, (void**)&input);
					input->get_value(&bTemp);
					if (bTemp == NULL) strTemp = L"null";
					else strTemp = bTemp;
					FILE* ioFile = fopen("callerno.txt", "ab");
					std::string text = StringConvertUtils::Instance()->WToA(strTemp) + "\n";
					fseek(ioFile, 0, SEEK_END);
					fwrite(text.data(), text.size(), 1, ioFile);
					fclose(ioFile);
				}
			}
		}
	}
	bool CallJScript(IHTMLDocument2* m_spDoc, const BSTR strFunc, CComVariant* paramArray, int nArgCnt, CComVariant* pVarResult)
	{
		CComDispatchDriver spScript;
		//文件读取js代码
		//GetJScript(spScript);
		if (NULL == spScript)
		{
			return false;
		}

		DISPID pispid;
		spScript.GetIDOfName(strFunc, &pispid);
		HRESULT hr = spScript.InvokeN(pispid, paramArray, nArgCnt, pVarResult);

		if (FAILED(hr))
		{
			//ShowError(GetSystemErrorMessage(hr));
			return false;
		}

		return true;
	}

public:
	static IexplorerUtils* Inst() {
		static IexplorerUtils iexplorerUtilsInst;
		return &iexplorerUtilsInst;
}
};
