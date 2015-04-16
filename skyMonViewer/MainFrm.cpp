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
#include "skyMonViewer.h"
#include "MainFrm.h"
#include "SkyMonViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)

    ON_REGISTERED_MESSAGE(
    AFX_WM_CREATETOOLBAR,
    &CMainFrame::OnToolbarCreateNew)

    ON_COMMAND_RANGE(
    ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7,
    &CMainFrame::OnApplicationLook)

    ON_UPDATE_COMMAND_UI_RANGE(
    ID_VIEW_APPLOOK_WIN_2000,
    ID_VIEW_APPLOOK_WINDOWS_7,
    &CMainFrame::OnUpdateApplicationLook)

    ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

static UINT indicators[] = {
    ID_SEPARATOR,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame() {
    theApp.m_nAppLook =
      theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
}

CMainFrame::~CMainFrame() {
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    BOOL bNameValid;
    // set the visual manager and style based on persisted value
    OnApplicationLook(theApp.m_nAppLook);

    if (!m_wndMenuBar.Create(this)) {
        TRACE0("Failed to create menubar\n");
        return -1;
    }

    m_wndMenuBar.SetPaneStyle(
      m_wndMenuBar.GetPaneStyle() |
      CBRS_SIZE_DYNAMIC |
      CBRS_TOOLTIPS |
      CBRS_FLYBY);

    CMFCPopupMenu::SetForceMenuFocus(FALSE);

    if (!m_wndStatusBar.Create(this)) {
        TRACE0("Failed to create status bar\n");
        return -1;
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndMenuBar);

    CDockingManager::SetDockingMode(DT_SMART);
    EnableAutoHidePanes(CBRS_ALIGN_ANY);

    CMFCToolBar::AddToolBarForImageCollection(
      IDR_MENU_IMAGES,
      theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

    if (!CreateDockingWindows()) {
        TRACE0("Failed to create docking windows\n");
        return -1;
    }

    m_wndFileView.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndFileView);
    m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndOutput);

    CList<UINT, UINT> lstBasicCommands;

    lstBasicCommands.AddTail(ID_SEARCH_BYKEYWORD);
    lstBasicCommands.AddTail(ID_APP_ABOUT);
    lstBasicCommands.AddTail(ID_TOOLS_LICENSE_INFO);
    lstBasicCommands.AddTail(ID_TOOLS_LICENSE_PURCHASE);
    lstBasicCommands.AddTail(ID_TOOLS_LICENSE_ACTIVATE);
    lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
    lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
    lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);

    CMFCToolBar::SetBasicCommands(lstBasicCommands);

    m_imageList.Create(16, 16, ILC_COLOR24, 4, 4);

    m_imageList.SetBkColor(RGB(255, 255, 255));

    HICON hOnlineIcon =
      LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDI_ONLINE));
    m_imageList.Add(hOnlineIcon);

    HICON hAwayIcon =
      LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDI_AWAY));
    m_imageList.Add(hAwayIcon);

    HICON hOfflineIcon =
      LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDI_OFFLINE));
    m_imageList.Add(hOfflineIcon);

    HICON hBusyIcon =
      LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDI_BUSY));
    m_imageList.Add(hBusyIcon);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
    if (!CFrameWndEx::PreCreateWindow(cs) )
        return FALSE;

    return TRUE;
}

BOOL CMainFrame::CreateDockingWindows() {
    BOOL bNameValid;

    CString strFileView;
    bNameValid = strFileView.LoadString(IDS_FILE_VIEW);
    ASSERT(bNameValid);
    if (!m_wndFileView.Create(
      strFileView,
      this,
      CRect(0, 0, 200, 200),
      TRUE,
      ID_VIEW_FILEVIEW,
        WS_CHILD |
        WS_VISIBLE |
        WS_CLIPSIBLINGS |
        WS_CLIPCHILDREN |
        CBRS_LEFT|
        CBRS_FLOAT_MULTI)) {
        TRACE0("Failed to create File View window\n");
        return FALSE;
    }

    CString strOutputWnd;
    bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
    ASSERT(bNameValid);
    if (!m_wndOutput.Create(
      strOutputWnd,
      this,
      CRect(0, 0, 100, 100),
      TRUE,
      ID_VIEW_OUTPUTWND,
      WS_CHILD |
        WS_VISIBLE |
        WS_CLIPSIBLINGS |
        WS_CLIPCHILDREN |
        CBRS_BOTTOM |
        CBRS_FLOAT_MULTI)) {
        TRACE0("Failed to create Output window\n");
        return FALSE;
    }

    SetDockingWindowIcons(theApp.m_bHiColorIcons);
    return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons) {
    HICON hFileViewIcon = (HICON) ::LoadImage(
      ::AfxGetResourceHandle(),
      MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW),
      IMAGE_ICON,
      ::GetSystemMetrics(SM_CXSMICON),
      ::GetSystemMetrics(SM_CYSMICON),
      0);

    m_wndFileView.SetIcon(hFileViewIcon, FALSE);

    HICON hOutputBarIcon = (HICON) ::LoadImage(
      ::AfxGetResourceHandle(),
      MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND),
      IMAGE_ICON,
      ::GetSystemMetrics(SM_CXSMICON),
      ::GetSystemMetrics(SM_CYSMICON),
      0);

    m_wndOutput.SetIcon(hOutputBarIcon, FALSE);
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const {
    CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const {
    CFrameWndEx::Dump(dc);
}
#endif  // _DEBUG

void CMainFrame::OnViewCustomize() {
    CMFCToolBarsCustomizeDialog* pDlgCust =
      new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
    pDlgCust->EnableUserDefinedToolbars();
    pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM lp) {
    LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp, lp);
    if (lres == 0) {
        return 0;
    }

    CMFCToolBar* pUserToolbar = reinterpret_cast<CMFCToolBar*>(lres);
    ASSERT_VALID(pUserToolbar);

    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
    return lres;
}

void CMainFrame::OnApplicationLook(UINT id) {
    CWaitCursor wait;

    theApp.m_nAppLook = id;

    switch (theApp.m_nAppLook) {
    case ID_VIEW_APPLOOK_WIN_2000:
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManager));
        break;

    case ID_VIEW_APPLOOK_OFF_XP:
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
        break;

    case ID_VIEW_APPLOOK_WIN_XP:
        CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerWindows));
        break;

    case ID_VIEW_APPLOOK_OFF_2003:
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerOffice2003));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2005:
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerVS2005));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2008:
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerVS2008));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_WINDOWS_7:
        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerWindows7));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    default:
        switch (theApp.m_nAppLook) {
        case ID_VIEW_APPLOOK_OFF_2007_BLUE:
            CMFCVisualManagerOffice2007::SetStyle(
              CMFCVisualManagerOffice2007::Office2007_LunaBlue);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_BLACK:
            CMFCVisualManagerOffice2007::SetStyle(
              CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_SILVER:
            CMFCVisualManagerOffice2007::SetStyle(
              CMFCVisualManagerOffice2007::Office2007_Silver);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_AQUA:
            CMFCVisualManagerOffice2007::SetStyle(
              CMFCVisualManagerOffice2007::Office2007_Aqua);
            break;
        }

        CMFCVisualManager::SetDefaultManager(
          RUNTIME_CLASS(CMFCVisualManagerOffice2007));
        CDockingManager::SetDockingMode(DT_SMART);
    }

    RedrawWindow(
      NULL,
      NULL,
      RDW_ALLCHILDREN |
        RDW_INVALIDATE |
        RDW_UPDATENOW |
        RDW_FRAME |
        RDW_ERASE);

    theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI) {
    pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(
  UINT nIDResource,
  DWORD dwDefaultStyle,
  CWnd* pParentWnd,
  CCreateContext* pContext) {
    if (!CFrameWndEx::LoadFrame(
      nIDResource,
      dwDefaultStyle,
      pParentWnd,
      pContext)) {
        return FALSE;
    }

    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    for (int i = 0; i < iMaxUserToolbars; i ++) {
        CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
        if (pUserToolbar != NULL) {
            pUserToolbar->EnableCustomizeButton(
              TRUE,
              ID_VIEW_CUSTOMIZE,
              strCustomize);
        }
    }

    return TRUE;
}


void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) {
    CFrameWndEx::OnSettingChange(uFlags, lpszSection);
    m_wndOutput.UpdateFonts();
}

void CMainFrame::FillViews(const wchar_t* pData, const CString& sName) {
    m_wndOutput.UpdateWithQuery(pData);
    m_wndOutput.SetWindowTextW(sName);

    CskyMonViewerView* view = static_cast<CskyMonViewerView*>(GetActiveView());
    if (view)
        view->UpdateWithQuery(pData);
}

void CMainFrame::SearchByKeyword(const CString& key) {
    m_wndOutput.UpdateByKeyword(key);
    m_wndOutput.SetWindowTextW(key);
}
