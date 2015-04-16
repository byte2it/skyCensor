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
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "skyMonViewer.h"
#include "MainFrm.h"

#include "skyMonViewerDoc.h"
#include "skyMonViewerView.h"

#include "KeywordDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CskyMonViewerApp, CWinAppEx)
    ON_COMMAND(ID_APP_ABOUT, &CskyMonViewerApp::OnAppAbout)
    ON_COMMAND(ID_SEARCH_BYKEYWORD, OnSearchByKeyword)
    ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
    ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

CskyMonViewerApp::CskyMonViewerApp() {
    m_bHiColorIcons = TRUE;

    std::string sPath = GetUserAppFolderPath();
    sPath.append("\\skype.db");

    db.init(sPath.c_str());

    // support Restart Manager
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
    System::Windows::Forms::Application::SetUnhandledExceptionMode(
      System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

    SetAppID(_T("skyMonViewer.AppID.NoVersion"));
}

// The one and only CskyMonViewerApp object

CskyMonViewerApp theApp;

std::string CskyMonViewerApp::GetUserAppFolderPath() {
    CHAR szPath[MAX_PATH];

    std::string sPath;

    if (SUCCEEDED(SHGetFolderPathA(NULL,
                                 CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                                 NULL,
                                 0,
                                 szPath))) {
        sPath.append(szPath);
    }
    return sPath;
}

BOOL CskyMonViewerApp::InitInstance() {
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinAppEx::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit()) {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }

    AfxEnableControlContainer();

    EnableTaskbarInteraction(FALSE);

    SetRegistryKey(_T("Local AppWizard-Generated Applications"));
    LoadStdProfileSettings(4);

    InitContextMenuManager();

    InitKeyboardManager();

    InitTooltipManager();
    CMFCToolTipInfo ttParams;
    ttParams.m_bVislManagerTheme = TRUE;
    theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
        RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CskyMonViewerDoc),
        RUNTIME_CLASS(CMainFrame),
        RUNTIME_CLASS(CskyMonViewerView));
    if (!pDocTemplate)
        return FALSE;
    AddDocTemplate(pDocTemplate);

    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();
    m_pMainWnd->SetWindowText(L"skyCensor");

    return TRUE;
}

int CskyMonViewerApp::ExitInstance() {
    AfxOleTerm(FALSE);
    return CWinAppEx::ExitInstance();
}

class CAboutDlg : public CDialogEx {
 public:
    CAboutDlg();

    enum { IDD = IDD_ABOUTBOX };

 protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

void CskyMonViewerApp::OnAppAbout() {
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CskyMonViewerApp::PreLoadState() {
    BOOL bNameValid;
    CString strName;
    bNameValid = strName.LoadString(IDS_EDIT_MENU);
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
    bNameValid = strName.LoadString(IDS_EXPLORER);
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CskyMonViewerApp::LoadCustomState() {
}

void CskyMonViewerApp::SaveCustomState() {
}

void CskyMonViewerApp::OnSearchByKeyword() {
    CKeywordDialog dlg;
    dlg.DoModal();

    CString s = dlg.GetKeyword();

    if (!s.IsEmpty())
        MAINFRAME->SearchByKeyword(s);
}
