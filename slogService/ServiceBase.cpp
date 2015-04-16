/****************************** Module Header ******************************\ 
* Module Name:  ServiceBase.cpp 
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

#include "ServiceBase.h"
#include <assert.h>
#include <strsafe.h>

CServiceBase *CServiceBase::s_service = NULL;

BOOL CServiceBase::Run(CServiceBase* service) {
  OutputDebugStringA("CSlogServiceMain::Run");
  s_service = service;

  SERVICE_TABLE_ENTRY serviceTable[] = {
        { service->m_name, ServiceMain },
        { NULL, NULL }
  };

  return StartServiceCtrlDispatcher(serviceTable);
}

void WINAPI CServiceBase::ServiceMain(DWORD argc, PWSTR *argv) {
  OutputDebugStringA("CSlogServiceMain::ServiceMain");

  assert(s_service != NULL);

  s_service->m_statusHandle = RegisterServiceCtrlHandler(
        s_service->m_name, ServiceCtrlHandler);
  if (s_service->m_statusHandle == NULL) {
    throw GetLastError();
  }

  s_service->Start(argc, argv);
}

void WINAPI CServiceBase::ServiceCtrlHandler(DWORD ctrl) {
    OutputDebugStringA("\n -- ServiceCtrlHandler -- \n\n");

    switch (ctrl) {
    case SERVICE_CONTROL_STOP: {
        s_service->Stop();
        OutputDebugStringA("\n -- Stop -- \n\n");
        break;
    }
    case SERVICE_CONTROL_PAUSE: {
        OutputDebugStringA("\n -- Pause -- \n\n");
        s_service->Pause();
        break;
    }
    case SERVICE_CONTROL_CONTINUE: {
        OutputDebugStringA("\n -- Continue -- \n\n");
        s_service->Continue();
        break;
    }
    case SERVICE_CONTROL_SHUTDOWN: {
        OutputDebugStringA("\n -- Shutdown -- \n\n");
        s_service->Shutdown();
        break;
    }
    case SERVICE_CONTROL_INTERROGATE: {
        OutputDebugStringA("\n -- Interrogate -- \n\n");
        break;
    }
    default: {
        OutputDebugStringA("\n -- Default -- \n\n");
        break;
    }
    }
}

CServiceBase::CServiceBase(PWSTR service_name,
                           BOOL can_stop,
                           BOOL can_shutdown,
                           BOOL can_pause_continue) {
    OutputDebugStringA("CServiceBase::CServiceBase()");

    m_name = (service_name == NULL) ? L"" : service_name;
    m_statusHandle = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_START_PENDING;

    DWORD controls_accepted = 0;
    if (can_stop)
        controls_accepted |= SERVICE_ACCEPT_STOP;
    if (can_shutdown)
        controls_accepted |= SERVICE_ACCEPT_SHUTDOWN;
    if (can_pause_continue)
        controls_accepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    m_status.dwControlsAccepted = controls_accepted;

    m_status.dwWin32ExitCode = NO_ERROR;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

CServiceBase::~CServiceBase(void) {
    OutputDebugStringA("CServiceBase::~CServiceBase()");
}

void CServiceBase::Start(DWORD argc, PWSTR *argv) {
    OutputDebugStringA("CServiceBase::Start()");

    try {
        SetServiceStatus(SERVICE_START_PENDING);
        OnStart(argc, argv);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD error) {
        WriteErrorLogEntry(L"Service Start", error);
        SetServiceStatus(SERVICE_STOPPED, error);
    }
    catch (...) {
        WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_STOPPED);
    }
}

void CServiceBase::OnStart(DWORD argc, PWSTR *argv) {
    OutputDebugStringA("CServiceBase::OnStart()");
}

void CServiceBase::Stop() {
    OutputDebugStringA("CServiceBase::Stop()");
    DWORD original_state = m_status.dwCurrentState;
    try {
        SetServiceStatus(SERVICE_STOP_PENDING);
        OnStop();
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD error) {
        WriteErrorLogEntry(L"Service Stop", error);
        SetServiceStatus(original_state);
    }
    catch (...) {
        WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(original_state);
    }
}

void CServiceBase::OnStop() {
    OutputDebugStringA("CServiceBase::OnStop()");
}

void CServiceBase::Pause() {
    OutputDebugStringA("CServiceBase::Pause()");
    try {
        SetServiceStatus(SERVICE_PAUSE_PENDING);
        OnPause();
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (DWORD error) {
        WriteErrorLogEntry(L"Service Pause", error);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (...) {
        WriteEventLogEntry(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_RUNNING);
    }
}

void CServiceBase::OnPause() {
    OutputDebugStringA("CServiceBase::OnPause()");
}

void CServiceBase::Continue() {
    OutputDebugStringA("CServiceBase::COntinue()");
    try {
        SetServiceStatus(SERVICE_CONTINUE_PENDING);
        OnContinue();
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD error) {
        WriteErrorLogEntry(L"Service Continue", error);
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (...) {
        WriteEventLogEntry(L"Service failed to resume.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_PAUSED);
    }
}

void CServiceBase::OnContinue() {
    OutputDebugStringA("CServiceBase::OnContinue()");
}

void CServiceBase::Shutdown() {
    OutputDebugStringA("CServiceBase::Shutdown()");
    try {
        OnShutdown();
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD error) {
        WriteErrorLogEntry(L"Service Shutdown", error);
    }
    catch (...) {
        WriteEventLogEntry(
          L"Service failed to shut down.",
          EVENTLOG_ERROR_TYPE);
    }
}

void CServiceBase::OnShutdown() {
    OutputDebugStringA("CServiceBase::OnShutdown()");
}

void CServiceBase::SetServiceStatus(DWORD current_state,
                                    DWORD exit_code,
                                    DWORD wait_hint) {
    OutputDebugStringA("CServiceBase::SetServiceStatus()");
    static DWORD dwCheckPoint = 1;

    m_status.dwCurrentState = current_state;
    m_status.dwWin32ExitCode = exit_code;
    m_status.dwWaitHint = wait_hint;

    m_status.dwCheckPoint =
        ((current_state == SERVICE_RUNNING) ||
        (current_state == SERVICE_STOPPED)) ?
        0 : dwCheckPoint++;

    ::SetServiceStatus(m_statusHandle, &m_status);
}

void CServiceBase::WriteEventLogEntry(PWSTR message, WORD type) {
    HANDLE event_source = NULL;
    LPCWSTR event_strings[2] = { NULL, NULL };

    event_source = RegisterEventSource(NULL, m_name);
    if (event_source) {
        event_strings[0] = m_name;
        event_strings[1] = message;

        ReportEvent(event_source,
            type,
            0,
            0,
            NULL,
            2,
            0,
            event_strings,
            NULL);

        DeregisterEventSource(event_source);
    }
}

void CServiceBase::WriteErrorLogEntry(PWSTR function, DWORD error) {
    wchar_t message[260];
    StringCchPrintf(message, ARRAYSIZE(message),
        L"%s failed w/err 0x%08lx", function, error);
    WriteEventLogEntry(message, EVENTLOG_ERROR_TYPE);
}

