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
#include "ViewTree.h"
#include "skyMonViewer.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define APP         theApp
#define MAINFRAME   ((CMainFrame *) APP.m_pMainWnd)

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewTree::CViewTree() {
}

CViewTree::~CViewTree() {
}

BEGIN_MESSAGE_MAP(CViewTree, CTreeCtrl)
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CViewTree::OnTvnSelchanged)
END_MESSAGE_MAP()

// CViewTree message handlers

BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) {
    NMHDR* pNMHDR = reinterpret_cast<NMHDR*>(lParam);
    ASSERT(pNMHDR != NULL);

     if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL) {
        GetToolTips()->SetWindowPos(
          &wndTop,
          -1,
          -1,
          -1,
          -1,
          SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
    }

    BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

    return bRes;
}


void CViewTree::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult) {
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    CString sItem = GetItemText(pNMTreeView->itemNew.hItem);
    const wchar_t* pData =
      (const wchar_t*)GetItemData(pNMTreeView->itemNew.hItem);

    if (pData)
      MAINFRAME->FillViews(pData, sItem);

    *pResult = 0;
}





