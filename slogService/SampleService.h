/****************************** Module Header ******************************\ 
* Module Name:  SampleService.h 
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

#ifndef SLOGSERVICE_SAMPLESERVICE_H_
#define SLOGSERVICE_SAMPLESERVICE_H_

#include "ServiceBase.h"
#include <atlbase.h>

class CSampleService : public CServiceBase {
 public:
    CSampleService(PWSTR pszServiceName,
        BOOL fCanStop = TRUE,
        BOOL fCanShutdown = TRUE,
        BOOL fCanPauseContinue = FALSE);
    virtual ~CSampleService(void);

 protected:
    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();

    void ServiceWorkerThread(void);

    bool IsProcessRunning(const wchar_t *processName)const;
    void StartSlogApp();

 private:
    BOOL m_fStopping;
    HANDLE m_hStoppedEvent;
    HANDLE m_hProcHandle;
};

#endif  // SLOGSERVICE_SAMPLESERVICE_H_
