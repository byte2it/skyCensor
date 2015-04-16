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
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "skyMonViewer.h"
#include "afxcmn.h"

#define GET_CHATS_MSG WM_USER +1

#define DELETE_CONVERSATION_BY_USER -1003

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

HWND windowHandle = NULL;

BOOL CALLBACK FindSlogWndProc(HWND hWnd, LPARAM lParam) {
    TCHAR title[512];
    ZeroMemory(title, sizeof(title));

     GetWindowText(hWnd, title, sizeof(title)/sizeof(title[0]));

    if (_tcsstr(title, _T("TestClient"))) {
        windowHandle = hWnd;
        return FALSE;
    }
    return TRUE;
}

CFileView::CFileView() {
}

CFileView::~CFileView() {
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_PROPERTIES, OnProperties)
    ON_COMMAND(ID_OPEN, OnFileOpen)
    ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
    ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
    ON_COMMAND(ID_SEARCH_BYKEYWORD, OnSearchByKeyword)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    const DWORD dwViewStyle =
      WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

    if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4)) {
        TRACE0("Failed to create file view\n");
        return -1;
    }

    m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
    m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

    m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);

    OnChangeVisualStyle();

    FillFileView();
    AdjustLayout();

    return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy) {
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

void CFileView::PopulateUsers() {
}

void CFileView::FillFileView() {
    HTREEITEM hRoot = m_wndFileView.InsertItem(_T("Contacts"), 0, 0);
    m_wndFileView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

    theApp.GetDatabase().GetUsers(users);

    int size = users.size();
    for (int i = 0; i < size; ++i) {
        HTREEITEM hClass =
          m_wndFileView.InsertItem(users[i].second.data(), 1, 1, hRoot);

        m_wndFileView.SetItemData(hClass, (DWORD_PTR)users[i].first.data());
    }

    m_wndFileView.Expand(hRoot, TVE_EXPAND);
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point) {
    CTreeCtrl* pWndTree = reinterpret_cast<CTreeCtrl*>(&m_wndFileView);
    ASSERT_VALID(pWndTree);

    if (pWnd != pWndTree) {
        CDockablePane::OnContextMenu(pWnd, point);
        return;
    }

    if (point != CPoint(-1, -1)) {
        CPoint ptTree = point;
        pWndTree->ScreenToClient(&ptTree);

        UINT flags = 0;
        HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
        if (hTreeItem != NULL) {
            pWndTree->SelectItem(hTreeItem);
        }
    }

    pWndTree->SetFocus();
    theApp.GetContextMenuManager()->
      ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout() {
    if (GetSafeHwnd() == NULL) {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

    m_wndToolBar.SetWindowPos(
      NULL,
      rectClient.left,
      rectClient.top,
      rectClient.Width(),
      cyTlb,
      SWP_NOACTIVATE | SWP_NOZORDER);

    m_wndFileView.SetWindowPos(
      NULL,
      rectClient.left + 1,
      rectClient.top + cyTlb + 1,
      rectClient.Width() - 2,
      rectClient.Height() - cyTlb - 2,
      SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnProperties() {
    AfxMessageBox(_T("Properties...."));
}

void CFileView::OnFileOpen() {
}

void CFileView::OnFileOpenWith() {
}

void CFileView::OnDummyCompile() {
}

void CFileView::OnSearchByKeyword() {
}

void CFileView::OnEditCopy() {
}

void CFileView::OnEditClear() {
    CWnd* pWnd = FindWindow(0, L"TestClient");

    if (pWnd) {
        HTREEITEM item = m_wndFileView.GetSelectedItem();
        if (item != 0) {
            COPYDATASTRUCT MyCDS = {0};
            const wchar_t* pData =
              (const wchar_t*)m_wndFileView.GetItemData(item);
            MyCDS.cbData = _tcslen(pData) * sizeof(wchar_t);
            MyCDS.lpData = (LPVOID)pData;

            MessageBox(pData, pData, 0);
            ::SendMessage(
              pWnd->m_hWnd,
              WM_COPYDATA,
              DELETE_CONVERSATION_BY_USER ,
              (LPARAM)&MyCDS);
        }
    }
}

void CFileView::OnPaint() {
    CPaintDC dc(this);

    CRect rectTree;
    m_wndFileView.GetWindowRect(rectTree);
    ScreenToClient(rectTree);

    rectTree.InflateRect(1, 1);
    dc.Draw3dRect(
      rectTree,
      ::GetSysColor(COLOR_3DSHADOW),
      ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd) {
    CDockablePane::OnSetFocus(pOldWnd);
    m_wndFileView.SetFocus();
}

void CFileView::OnChangeVisualStyle() {
    m_wndToolBar.CleanUpLockedImages();
    m_wndToolBar.LoadBitmap(
      theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER,
      0,
      0,
      TRUE /* Locked */);

    m_FileViewImages.DeleteImageList();

    UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

    CBitmap bmp;
    if (!bmp.LoadBitmap(uiBmpId)) {
        TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
        ASSERT(FALSE);
        return;
    }

    BITMAP bmpObj;
    bmp.GetBitmap(&bmpObj);

    UINT nFlags = ILC_MASK;

    nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

    m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
    m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

    m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}

void CFileView::UpdateWithQuery(const wchar_t* pData) {
}
