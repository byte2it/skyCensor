/****************************** Module Header ******************************\ 
* Module Name:  ServiceInstaller.h 
* Project:      CppWindowsService 
* Copyright (c) Microsoft Corporation. 
*  
* The file declares functions that install and uninstall the service. 
*  
* This source is subject to the Microsoft Public License. 
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL. 
* All other rights reserved. 
*  
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,  
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED  
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE. 
\***************************************************************************/ 

#ifndef SLOGSERVICE_SERVICEINSTALLER_H_
#define SLOGSERVICE_SERVICEINSTALLER_H_

void InstallService(PWSTR service_name,
                    PWSTR display_name,
                    DWORD start_type,
                    PWSTR dependencies,
                    PWSTR account,
                    PWSTR password);

void StartSrvc(PWSTR pszServiceName);
void UninstallService(PWSTR pszServiceName);

#endif  // SLOGSERVICE_SERVICEINSTALLER_H_
