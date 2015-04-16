
/****************************** Module Header ******************************\ 
* Module Name:  SampleService.cpp 
* Project:      CppWindowsService 
* Copyright (c) Microsoft Corporation. 
*  
* Provides a sample service class that derives from the service base class -  
* CServiceBase. The sample service logs the service start and stop  
* information to the Application event log, and shows how to run the main  
* function of the service in a thread pool worker thread. 
*  
* This source is subject to the Microsoft Public License. 
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL. 
* All other rights reserved. 
*  
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,  
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED  
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE. 
\***************************************************************************/ 

#include "SampleService.h"
#include "ThreadPool.h"

#include <tlhelp32.h>
#include <string>

#include <Wtsapi32.h>
#include <Userenv.h>

CSampleService::CSampleService(PWSTR service_name,
                               BOOL can_stop,
                               BOOL can_shutdown,
                               BOOL can_pause_continue)
: CServiceBase(service_name, can_stop, can_shutdown, can_pause_continue) {
    m_fStopping = FALSE;

    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL) {
        OutputDebugStringA(
      "CSampleService::CSampleService - m_hStoppedEvent == NULL");
        throw GetLastError();
    }
    m_hProcHandle = NULL;
}

CSampleService::~CSampleService(void) {
    if (m_hStoppedEvent) {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
    if (m_hProcHandle) {
        TerminateProcess(m_hProcHandle, 0);
        CloseHandle(m_hProcHandle);
    }
}

void CSampleService::OnStart(DWORD argc, LPWSTR *argv) {
    WriteEventLogEntry(L"slogService in OnStart",
        EVENTLOG_INFORMATION_TYPE);

    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}

bool CSampleService::IsProcessRunning(const wchar_t *process_name) const {
    bool exists = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry))
        while (Process32Next(snapshot, &entry))
            if (!wcsicmp(entry.szExeFile, process_name))
                exists = true;

    CloseHandle(snapshot);
    return exists;
}

HANDLE CreateUserModeToken() {
    OutputDebugString(L"CreateUserModeToken\n");
    HANDLE current_token;
    HANDLE primary_token;

    int session_id = 0;
    PHANDLE user_token = 0;
    PHANDLE token_dup = 0;

    PWTS_SESSION_INFO session_info = 0;
    DWORD count = 0;

    WTSEnumerateSessions(
      WTS_CURRENT_SERVER_HANDLE,
      0,
      1,
      &session_info,
      &count);

    int dataSize = sizeof(WTS_SESSION_INFO);

    for (DWORD i = 0; i < count; ++i) {
        WTS_SESSION_INFO si = session_info[i];
        if (WTSActive == si.State) {
            session_id = si.SessionId;

            TCHAR msg[512] = {0};
            wsprintf(msg, _T("Session id = ! (%d)\n"), session_id);
            OutputDebugString(msg);
            break;
        }
    }

    BOOL ret = WTSQueryUserToken(session_id, &current_token);
    if (ret == false) {
        TCHAR szMsg[512] = {0};
        wsprintf(szMsg, _T("WTSQueryUserToken failed! (%d)\n"), GetLastError());
        OutputDebugString(szMsg);
        return 0;
    }

    ret = DuplicateTokenEx(current_token,
      TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS,
      0,
      SecurityImpersonation,
      TokenPrimary,
      &primary_token);

    if (ret == false) {
        TCHAR msg[512] = {0};
        wsprintf(msg, _T("DuplicateTokenEx failed! (%d)\n"), GetLastError());
        OutputDebugString(msg);
        return 0;
    }

    OutputDebugString(L"after DuplicateTokenEx\n");
    return primary_token;
}

void CSampleService::StartSlogApp() {
    OutputDebugString(L":StartSlogApp");
    HANDLE usr_token = CreateUserModeToken();
    if (usr_token == 0) {
        OutputDebugString(L"usrToken == 0\n");
        return;
    }

    OutputDebugString(L"1\n");

    STARTUPINFO startup_info = {0};
    PROCESS_INFORMATION processInfo = {0};
    startup_info.cb = sizeof(STARTUPINFO);

    SECURITY_ATTRIBUTES security1;
    SECURITY_ATTRIBUTES security2;

    TCHAR path[MAX_PATH];

    if (!GetModuleFileName(NULL, path, MAX_PATH )) {
        TCHAR msg[512] = {0};
        wsprintf(msg, _T("Cannot get module path! (%d)\n"), GetLastError());
        OutputDebugString(msg);
        return;
    }

    std::wstring slog_path(path);
    int iPos = std::string::npos;
    iPos = slog_path.find_last_of('\\');

    if (iPos != std::string::npos) {
        slog_path.erase(iPos+1);
        slog_path.append(L"slog.exe");
        OutputDebugString(slog_path.c_str());
        OutputDebugString(L"\n");
    }

    if (!CreateProcessAsUser(usr_token,
                             0,
                            (LPWSTR)slog_path.data(),
                             NULL,
                             NULL,
                             FALSE,
                             CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS,
                             NULL,
                             0,
                             &startup_info,
                             &processInfo)) {
        TCHAR message[512] = {0};
        wsprintf(message, L"CreateProcess failed (%d).\n", GetLastError());
        OutputDebugString(message);
        return;
    }
    CloseHandle(usr_token);

    OutputDebugString(L"slog process has been started successfully\n");
    m_hProcHandle = processInfo.hProcess;
    CloseHandle(processInfo.hThread);
}

void CSampleService::ServiceWorkerThread(void) {
    while (!m_fStopping) {
      if (!IsProcessRunning(L"slog.exe")) {
        OutputDebugStringA(" --- slog.exe is not running --- \n");
        StartSlogApp();
      }
        ::Sleep(5000);
    }
    SetEvent(m_hStoppedEvent);
}

void CSampleService::OnStop() {
    WriteEventLogEntry(L"slogService in OnStop",
        EVENTLOG_INFORMATION_TYPE);

    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0) {
        throw GetLastError();
    }
}
