// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef SLOG_STDAFX_H_
#define SLOG_STDAFX_H_

#if _MSC_VER > 1000
#pragma once
#endif  // _MSC_VER > 1000

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif  // _AFX_NO_AFXCMN_SUPPORT

#include <string>

#define GET_CHATS_MSG WM_USER +1

//  {{AFX_INSERT_LOCATION}}
#pragma warning(disable : 4786)

#import "C:\tools\Skype4COM-1.0.38.0\Skype4COM.dll"
using namespace SKYPE4COMLib;

#include "TEventHandler.h"
using namespace TEventHandlerNamespace;

#endif  // SLOG_STDAFX_H_
