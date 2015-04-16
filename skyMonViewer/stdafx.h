
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#ifndef SKYMONVIEWER_STDAFX_H_
#define SKYMONVIEWER_STDAFX_H_

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>


#include <afxdisp.h>



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif  // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>
#include <afxcview.h>

#include <vector>
#include <afxdlgs.h>

#import "C:\tools\Skype4COM-1.0.38.0\Skype4COM.dll"
using namespace SKYPE4COMLib;

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32'" \
" name='Microsoft.Windows.Common-Controls' version='6.0.0.0'" \
  " processorArchitecture='x86' publicKeyToken='6595b64144ccf1df'" \
" language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32'" \
  " name='Microsoft.Windows.Common-Controls' version='6.0.0.0'" \
  " processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df'" \
  " language='*'\"")
#else
#pragma comment(linker,
"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls'" \
  " version='6.0.0.0' processorArchitecture='*'" \
  " publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#endif  // SKYMONVIEWER_STDAFX_H_