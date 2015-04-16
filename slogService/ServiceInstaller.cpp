/****************************** Module Header ******************************\ 
* Module Name:  ServiceInstaller.cpp 
* Project:      CppWindowsService 
* Copyright (c) Microsoft Corporation. 
*  
* The file implements functions that install and uninstall the service. 
*  
* This source is subject to the Microsoft Public License. 
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL. 
* All other rights reserved. 
*  
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,  
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED  
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE. 
\***************************************************************************/ 

#include <stdio.h>
#include <windows.h>
#include "ServiceInstaller.h"

void InstallService(PWSTR service_name,
                    PWSTR display_name,
                    DWORD start_type,
                    PWSTR dependencies,
                    PWSTR account,
                    PWSTR password) {
    wchar_t path[MAX_PATH];
    SC_HANDLE service_manager = NULL;
    SC_HANDLE service = NULL;

    if (GetModuleFileName(NULL, path, ARRAYSIZE(path)) == 0) {
        wprintf(L"GetModuleFileName failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    service_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
        SC_MANAGER_CREATE_SERVICE);
    if (service_manager == NULL) {
        wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    service = CreateService(
        service_manager,
        service_name,
        display_name,
        SERVICE_QUERY_STATUS,
        SERVICE_WIN32_OWN_PROCESS,
        start_type,
        SERVICE_ERROR_NORMAL,
        path,
        NULL,
        NULL,
        dependencies,
        /*pszAccount*/NULL,
        password);

    if (service == NULL) {
        wprintf(L"CreateService failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    wprintf(L"%s is installed.\n", service_name);

Cleanup:
    if (service_manager) {
        CloseServiceHandle(service_manager);
        service_manager = NULL;
    }
    if (service) {
        CloseServiceHandle(service);
        service = NULL;
    }
}


void StartSrvc(PWSTR service_name) {
    SERVICE_STATUS_PROCESS serv_stat_proc;
    DWORD old_check_point;
    DWORD start_tick_count;
    DWORD wait_time;
    DWORD bytes_needed;

    SC_HANDLE service_manager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == service_manager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    SC_HANDLE service = OpenService(
        service_manager,
        service_name,
        SERVICE_ALL_ACCESS);

    if (service == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(service_manager);
        return;
    }

    if (!QueryServiceStatusEx(
            service,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE) &serv_stat_proc,
            sizeof(SERVICE_STATUS_PROCESS),
            &bytes_needed)) {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(service);
        CloseServiceHandle(service_manager);
        return;
    }

    if (serv_stat_proc.dwCurrentState != SERVICE_STOPPED &&
      serv_stat_proc.dwCurrentState != SERVICE_STOP_PENDING) {
        printf("Cannot start the service because it is already running\n");
        CloseServiceHandle(service);
        CloseServiceHandle(service_manager);
        return;
    }

    start_tick_count = GetTickCount();
    old_check_point = serv_stat_proc.dwCheckPoint;

    while (serv_stat_proc.dwCurrentState == SERVICE_STOP_PENDING) {
        wait_time = serv_stat_proc.dwWaitHint / 10;

        if ( wait_time < 1000 )
            wait_time = 1000;
        else if ( wait_time > 10000 )
            wait_time = 10000;

        Sleep(wait_time);

        if (!QueryServiceStatusEx(
                service,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE) &serv_stat_proc,
                sizeof(SERVICE_STATUS_PROCESS),
                &bytes_needed)) {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            CloseServiceHandle(service);
            CloseServiceHandle(service_manager);
            return;
        }

        if ( serv_stat_proc.dwCheckPoint > old_check_point ) {
            start_tick_count = GetTickCount();
            old_check_point = serv_stat_proc.dwCheckPoint;
        } else {
            if (GetTickCount()-start_tick_count > serv_stat_proc.dwWaitHint) {
                printf("Timeout waiting for service to stop\n");
                CloseServiceHandle(service);
                CloseServiceHandle(service_manager);
                return;
            }
        }
    }

    if (!StartService(service, 0, NULL)) {
        printf("StartService failed (%d)\n", GetLastError());
        CloseServiceHandle(service);
        CloseServiceHandle(service_manager);
        return;
    } else {
      printf("Service start pending...\n");
    }

    if (!QueryServiceStatusEx(
            service,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE) &serv_stat_proc,
            sizeof(SERVICE_STATUS_PROCESS),
            &bytes_needed)) {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(service);
        CloseServiceHandle(service_manager);
        return;
    }

    start_tick_count = GetTickCount();
    old_check_point = serv_stat_proc.dwCheckPoint;

    while (serv_stat_proc.dwCurrentState == SERVICE_START_PENDING)  {
        wait_time = serv_stat_proc.dwWaitHint / 10;

        if ( wait_time < 1000 )
            wait_time = 1000;
        else if ( wait_time > 10000 )
            wait_time = 10000;

        Sleep(wait_time);

        if (!QueryServiceStatusEx(
            service,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE) &serv_stat_proc,
            sizeof(SERVICE_STATUS_PROCESS),
            &bytes_needed)) {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            break;
        }

        if ( serv_stat_proc.dwCheckPoint > old_check_point ) {
            start_tick_count = GetTickCount();
            old_check_point = serv_stat_proc.dwCheckPoint;
        } else {
            if (GetTickCount()-start_tick_count > serv_stat_proc.dwWaitHint) {
                break;
            }
        }
    }

    if (serv_stat_proc.dwCurrentState == SERVICE_RUNNING) {
        printf("Service started successfully.\n");
    } else {
        printf("Service not started. \n");
        printf("  Current State: %d\n", serv_stat_proc.dwCurrentState);
        printf("  Exit Code: %d\n", serv_stat_proc.dwWin32ExitCode);
        printf("  Check Point: %d\n", serv_stat_proc.dwCheckPoint);
        printf("  Wait Hint: %d\n", serv_stat_proc.dwWaitHint);
    }

    CloseServiceHandle(service);
    CloseServiceHandle(service_manager);
}


void UninstallService(PWSTR service_name) {
    SC_HANDLE service_manager = NULL;
    SC_HANDLE service = NULL;
    SERVICE_STATUS service_status = {};

    service_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (service_manager == NULL) {
        wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    service = OpenService(service_manager, service_name, SERVICE_STOP |
        SERVICE_QUERY_STATUS | DELETE);
    if (service == NULL) {
        wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    if (ControlService(service, SERVICE_CONTROL_STOP, &service_status)) {
        wprintf(L"Stopping %s.", service_name);
        Sleep(1000);

        while (QueryServiceStatus(service, &service_status)) {
            if (service_status.dwCurrentState == SERVICE_STOP_PENDING) {
                wprintf(L".");
                Sleep(1000);
            } else {
              break;
            }
        }

        if (service_status.dwCurrentState == SERVICE_STOPPED) {
            wprintf(L"\n%s is stopped.\n", service_name);
        } else {
            wprintf(L"\n%s failed to stop.\n", service_name);
        }
    }

    if (!DeleteService(service)) {
        wprintf(L"DeleteService failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }

    wprintf(L"%s is removed.\n", service_name);

Cleanup:

    if (service_manager) {
        CloseServiceHandle(service_manager);
        service_manager = NULL;
    }
    if (service) {
        CloseServiceHandle(service);
        service = NULL;
    }
}
