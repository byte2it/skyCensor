/****************************** Module Header ******************************\ 
* Module Name:  ServiceBase.h 
* Project:      CppWindowsService 
* Copyright (c) Microsoft Corporation. 
*  
* Provides a base class for a service that will exist as part of a service  
* application. CServiceBase must be derived from when creating a new service  
* class. 
*  
* This source is subject to the Microsoft Public License. 
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL. 
* All other rights reserved. 
*  
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,  
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED  
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE. 
\***************************************************************************/ 

#ifndef SLOGSERVICE_SERVICEBASE_H_
#define SLOGSERVICE_SERVICEBASE_H_

#include <windows.h>

class CServiceBase {
 public:
    static BOOL Run(CServiceBase* service);

    CServiceBase(PWSTR pszServiceName,
        BOOL fCanStop = TRUE,
        BOOL fCanShutdown = TRUE,
        BOOL fCanPauseContinue = FALSE);

    virtual ~CServiceBase(void);

    void Stop();

 protected:
    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();
    virtual void OnPause();
    virtual void OnContinue();
    virtual void OnShutdown();
    void SetServiceStatus(DWORD dwCurrentState,
        DWORD dwWin32ExitCode = NO_ERROR,
        DWORD dwWaitHint = 0);

    void WriteEventLogEntry(PWSTR pszMessage, WORD wType);
    void WriteErrorLogEntry(PWSTR pszFunction,
        DWORD dwError = GetLastError());

 private:
    static void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
    static void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
    void Start(DWORD dwArgc, PWSTR *pszArgv);
    void Pause();
    void Continue();
    void Shutdown();
    static CServiceBase* s_service;
    PWSTR m_name;
    SERVICE_STATUS m_status;

    SERVICE_STATUS_HANDLE m_statusHandle;
};

#endif  // SLOGSERVICE_SERVICEBASE_H_
