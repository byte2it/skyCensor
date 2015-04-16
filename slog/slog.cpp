/*
    Copyright (c) 2015 Denys Ponomarenko (denys.ponomarenko@gmail.com).

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "slog.h"
#include "slogDlg.h"

#include <tchar.h>
#include <strsafe.h>
#include <tlhelp32.h>

#include <Wtsapi32.h>
#include <Userenv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CTestClientApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CTestClientApp::CTestClientApp() {
}

CTestClientApp theApp;

static LPTOP_LEVEL_EXCEPTION_FILTER pDefFilter;
LONG WINAPI myfilter(struct _EXCEPTION_POINTERS *ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode ==
      EXCEPTION_ACCESS_VIOLATION)  {
        ::MessageBox(
            0,
            L"Please go to Skype and allow access for skyCensor application",
            L"skyCensor - allow access to Skype",
            MB_OK);


        ExitProcess(0);
        return EXCEPTION_EXECUTE_HANDLER;
    } else {
        if (pDefFilter)
            return pDefFilter(ExceptionInfo);
        else
            ExitProcess(0);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL CTestClientApp::InitInstance() {
    pDefFilter = SetUnhandledExceptionFilter(myfilter);

    AfxEnableControlContainer();

    // Standard initialization
#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif
    ::CoInitialize(NULL);

    std::string sPath = GetUserAppFolderPath();

    sPath.append("\\skype.db");

    CTestClientDlg dlg(NULL, sPath.c_str());
    m_pMainWnd = &dlg;
    int nResponse = dlg.DoModal();

    return FALSE;
}

HANDLE CreateUserModeToken() {
    OutputDebugString(L"CreateUserModeToken\n");
    HANDLE currentToken;
    HANDLE primaryToken;

    int dwSessionId = 0;
    PHANDLE hUserToken = 0;
    PHANDLE hTokenDup = 0;

    PWTS_SESSION_INFO pSessionInfo = 0;
    DWORD dwCount = 0;

    // Get the list of all terminal sessions
    WTSEnumerateSessions(
      WTS_CURRENT_SERVER_HANDLE,
      0,
      1,
      &pSessionInfo,
      &dwCount);

    int dataSize = sizeof(WTS_SESSION_INFO);

    // look over obtained list in search of the active session
    for (DWORD i = 0; i < dwCount; ++i) {
        WTS_SESSION_INFO si = pSessionInfo[i];
        if (WTSActive == si.State) {
        // If the current session is active - store its ID
            dwSessionId = si.SessionId;

            TCHAR szMsg[512] = {0};
            wsprintf(szMsg, _T("Session id = ! (%d)\n"), dwSessionId);
            OutputDebugString(szMsg);

            break;
        }
    }

    // Get token of the logged in user by the active session ID
    BOOL bRet = WTSQueryUserToken(dwSessionId, &currentToken);
    if (bRet == false) {
        TCHAR szMsg[512] = {0};
        wsprintf(szMsg, _T("WTSQueryUserToken failed! (%d)\n"), GetLastError());
        OutputDebugString(szMsg);
        return 0;
    }

    bRet = DuplicateTokenEx(
      currentToken,
      TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS,
      0,
      SecurityImpersonation,
      TokenPrimary,
      &primaryToken);

    if (bRet == false) {
        TCHAR szMsg[512] = {0};
        wsprintf(szMsg, _T("DuplicateTokenEx failed! (%d)\n"), GetLastError());
        OutputDebugString(szMsg);
        return 0;
    }

    OutputDebugString(L"after DuplicateTokenEx\n");
    return primaryToken;
}

std::string CTestClientApp::GetUserAppFolderPath() {
    std::string sPath;

    TCHAR* l_strExampleKey = TEXT("SOFTWARE\\dolphinden\\skyCensor");
    TCHAR* value = TEXT("dbpath");

    HKEY key;
    LSTATUS lRegResult = RegOpenKeyEx(
      HKEY_CURRENT_USER,
      l_strExampleKey,
      0,
      KEY_QUERY_VALUE,
      &key);


    if (ERROR_SUCCESS == lRegResult) {
            DWORD dataSize = MAX_PATH * sizeof(wchar_t);
            std::vector<wchar_t> dbPath(dataSize);

            lRegResult = RegQueryValueEx(
              key,
              value,
              NULL,
              NULL,
              (LPBYTE)&dbPath[0],
              &dataSize);

            if (ERROR_SUCCESS == lRegResult) {
                for (int  i = 0; i < dataSize/sizeof(wchar_t); ++i) {
                    char sz[2] = {0};
                    wctomb(sz, dbPath[i]);
                    sPath.append(sz);
                    }
                OutputDebugStringA(sPath.c_str());
            } else {
                TCHAR szM[128] = {0};
                wsprintf(
                  szM,
                  L"-FAILED RegQueryValueEx with error : %d -",
                  lRegResult);
                OutputDebugString(szM);
            }
    } else {
        TCHAR szM[128] = {0};
        wsprintf(szM, L"-FAILED RegOpenKeyEx with error : %d -", lRegResult);
        OutputDebugString(szM);
    }

    return sPath;
}


int CTestClientApp::ExitInstance() {
    CoUninitialize();

    return CWinApp::ExitInstance();
}
