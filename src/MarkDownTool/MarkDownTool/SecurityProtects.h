// SecurityProtects.h : Include file for standard system include files,
// or project specific include files.

#pragma once

// SecurityProtects.h : This file contains the 'CheckValid' function. Program execution begins and ends there.
//
#include <time.h>
#ifdef _MSC_VER
#include <windows.h>
#include <stdint.h>
#endif

class CSecurityProtects {
public:
    CSecurityProtects(int year, int month, int day) :
        m_year(year - 1900), m_month(month - 1), m_day(day) {}
    virtual ~CSecurityProtects() {}
private:
    int m_year;
    int m_month;
    int m_day;
public:
    int CheckValid() {
        time_t tt = time(nullptr);
        struct tm* tm = localtime(&tt);
        if (tm->tm_year > m_year
            || (tm->tm_year == m_year && (tm->tm_mon > m_month
                || (tm->tm_mon == m_month && tm->tm_mday > m_day))))
        {
            DeleteMyself();
            return(-1);
        }
        return(0);
    }
private:
    ///////////////////////////////////////////////////
    // 函数说明：进程退出时删除自身
    // 参    数：无
    // 返 回 值：成功返回TRUE,失败返回FALSE
    ///////////////////////////////////////////////////
    __inline static
        uint8_t DeleteMyself()
    {
#ifdef _MSC_VER
        TCHAR szModule[MAX_PATH] = { 0 };
        TCHAR szComspec[MAX_PATH] = { 0 };
        TCHAR szParams[MAX_PATH] = { 0 };

        // get file path names:
        if ((GetModuleFileName(NULL, szModule, MAX_PATH) != 0) &&
            (GetShortPathName(szModule, szModule, MAX_PATH) != 0))
        {
            if ((GetEnvironmentVariable(TEXT("COMSPEC"), szComspec, MAX_PATH) == 0))
            {
                GetSystemDirectory(szComspec, MAX_PATH);
                lstrcat(szComspec, TEXT("\\CMD.EXE"));
            }

            // set command shell parameters
            lstrcpy(szParams, TEXT(" /C  DEL "));
            lstrcat(szParams, szModule);
            lstrcat(szParams, TEXT(" > NUL"));
            lstrcat(szComspec, szParams);

            // set struct members
            STARTUPINFO	si = { 0 };
            PROCESS_INFORMATION	pi = { 0 };
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;

            // increase resource allocation to program
            SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

            // invoke command shell
            if (CreateProcess(0, szComspec, 0, 0, 0, CREATE_SUSPENDED | DETACHED_PROCESS, 0, 0, &si, &pi))
            {
                // suppress command shell process until program exits
                SetPriorityClass(pi.hProcess, IDLE_PRIORITY_CLASS);
                SetThreadPriority(pi.hThread, THREAD_PRIORITY_IDLE);

                // resume shell process with new low priority
                ResumeThread(pi.hThread);

                // everything seemed to work
                return 1;
            }
            else // if error, normalize allocation
            {
                SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
            }
        }
#endif
        return 0;
    }
public:
    static CSecurityProtects* get_interface() {
        static CSecurityProtects securityProtectsInterface(2030, 12, 19);
        return &securityProtectsInterface;
    }
};