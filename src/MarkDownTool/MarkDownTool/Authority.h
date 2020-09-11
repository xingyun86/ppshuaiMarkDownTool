// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include <windows.h>

/***************************************************/
/*  函数: 进程提权
/*  参数：HANDLE 目标进程句柄
/*  返回: 是否成功
/***************************************************/
__inline static
BOOL IncreasingPermission(HANDLE hProcessHandle, LPCTSTR lpSePrivilegeName = SE_DEBUG_NAME)
{
	BOOL bRet = FALSE;
	LUID luid = { 0 };
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES tp = { 0 };
	if (::OpenProcessToken(hProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if (::LookupPrivilegeValue(NULL, lpSePrivilegeName, &luid))
		{
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			bRet = ::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		}
		::CloseHandle(hToken);
	}

	return bRet;
}

__inline static
SECURITY_ATTRIBUTES* EveryoneSecurityAttributes(SECURITY_ATTRIBUTES& sa, SECURITY_DESCRIPTOR& sd)
{
	BOOL bRet = FALSE;
	bRet = InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	bRet = SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
	return &sa;
}