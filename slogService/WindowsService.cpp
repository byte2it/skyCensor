/****************************** Module Header ******************************\ 
* Module Name:  CppWindowsService.cpp 
* Project:      CppWindowsService 
* Copyright (c) Microsoft Corporation. 
*  
* The file defines the entry point of the application. According to the  
* arguments in the command line, the function installs or uninstalls or  
* starts the service by calling into different routines. 
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
#include "ServiceBase.h"
#include "SampleService.h"

#define SERVICE_NAME             L"slog Service"
#define SERVICE_DISPLAY_NAME     L"slog Service"
#define SERVICE_START_TYPE       SERVICE_AUTO_START
#define SERVICE_DEPENDENCIES     L""
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"
#define SERVICE_PASSWORD         NULL

int wmain(int argc, wchar_t *argv[]) {
    if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/')))) {
        if (_wcsicmp(L"install", argv[1] + 1) == 0) {
            InstallService(
                SERVICE_NAME,
                SERVICE_DISPLAY_NAME,
                SERVICE_START_TYPE,
                SERVICE_DEPENDENCIES,
                SERVICE_ACCOUNT,
                SERVICE_PASSWORD);

                 StartSrvc(SERVICE_NAME);
           } else if (_wcsicmp(L"remove", argv[1] + 1) == 0) {
            UninstallService(SERVICE_NAME);
       }
    } else {
        wprintf(L"Parameters:\n");
        wprintf(L" -install  to install the service.\n");
        wprintf(L" -remove   to remove the service.\n");

        CSampleService service(SERVICE_NAME);
        if (!CServiceBase::Run(&service)) {
            wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
        }
    }

    return 0;
}
