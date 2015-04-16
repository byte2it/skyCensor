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

#include "Shlobj.h"
#include <string>
#include <sstream>
#include <WinIoCtl.h>

TCHAR* l_strExampleKey = TEXT("SOFTWARE\\dolphinden\\skyCensor");

bool CreateDbPathKey(const TCHAR* path) {
     HKEY hKey;
     DWORD dwDisp = 0;
     LPDWORD lpdwDisp = &dwDisp;

     bool res = false;

     TCHAR* value_name = TEXT("dbpath");

     LONG iSuccess = RegCreateKeyEx(
       HKEY_CURRENT_USER,
       l_strExampleKey,
       0L,
       NULL,
       REG_OPTION_NON_VOLATILE,
       KEY_ALL_ACCESS,
       NULL,
       &hKey,
       lpdwDisp);

     if (iSuccess == ERROR_SUCCESS) {
         if (ERROR_SUCCESS == RegSetValueEx(
           hKey,
           value_name,
           0L,
           REG_SZ,
           (const BYTE*)path,
           _tcslen(path) * sizeof(TCHAR))) {
             res = true;
         }

          RegCloseKey(hKey);
     }
     return res;
}

bool RemoveDbPathKey() {
    HKEY key;
    if (ERROR_SUCCESS == RegOpenKeyEx(
      HKEY_CURRENT_USER,
      l_strExampleKey,
      0,
      KEY_ALL_ACCESS,
      &key)) {
        RegCloseKey(key);
        RegDeleteKeyEx(
          HKEY_CURRENT_USER,
          l_strExampleKey,
          KEY_WOW64_32KEY | KEY_WOW64_64KEY,
          0);
    }
    return true;
}

UINT sendIdviaHttp(std::string id) {
    return 0;
}

UINT __stdcall InstallSendHardwareId(MSIHANDLE hInstall) {
    return sendIdviaHttp(""/*getHardwareId()*/);
}

UINT __stdcall WriteUserDbPath(MSIHANDLE hInstall) {
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    TCHAR szPath[MAX_PATH] = {0};

    if (SUCCEEDED(SHGetFolderPath(NULL,
                                 CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                                 NULL,
                                 0,
                                 szPath))) {
        // write to registry
        if (!CreateDbPathKey(szPath))
            return -1;
    }

    hr = WcaInitialize(hInstall, "CustomAction1");
    ExitOnFailure(hr, "Failed to initialize");

    WcaLog(LOGMSG_STANDARD, "Initialized.");

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}

UINT __stdcall RemoveUserDbPath(MSIHANDLE hInstall) {
    if (RemoveDbPathKey())
        return 0;
    return -1;
}

// DllMain - Initialize and cleanup WiX custom action utils.
extern "C" BOOL WINAPI DllMain(
    __in HINSTANCE hInst,
    __in ULONG ulReason,
    __in LPVOID
    ) {
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        WcaGlobalInitialize(hInst);
        break;

    case DLL_PROCESS_DETACH:
        WcaGlobalFinalize();
        break;
    }

    return TRUE;
}
