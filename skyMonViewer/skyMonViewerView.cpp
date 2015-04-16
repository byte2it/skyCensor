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

#ifndef SHARED_HANDLERS
#include "skyMonViewer.h"
#endif

#include "skyMonViewerDoc.h"
#include "skyMonViewerView.h"
#include "MainFrm.h"

#include "../common/datetime.h"

#pragma warning(disable : 4786)
#include <functional>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct _STATUS {
    short        icon;
    unsigned int stat;
    const wchar_t*  desc;
}STATUS;

STATUS statusesArray[] = {
    {2, TOnlineStatus::olsOffline, L"Offline"},
    {0, TOnlineStatus::olsOnline, L"Online"},
    {1, TOnlineStatus::olsAway, L"Away"},

    {1, TOnlineStatus::olsNotAvailable, L"Not available"},
    {3, TOnlineStatus::olsDoNotDisturb, L"Do not disturb"},

    {2, TOnlineStatus::olsSkypeOut, L"Skype out"},
    {6, TOnlineStatus::olsSkypeMe, L"Skype ME"},
    {-1, TOnlineStatus::olsUnknown, L"Uknown"}
};


namespace LOGVIEWER {
  enum eColumns {
    COLUMN_IMAGE,
    COLUMN_DATE,
    COLUMN_SEQUENCE,
    COLUMN_CODE,
    COLUMN_THREAD,
    COLUMN_MODULE,
    COLUMN_MACHINE,
    COLUMN_MESSAGE
  };

  enum eLevels {
    ERRORS = 0x01,
    WARNINGS = 0x02,
    SUCCESS = 0x04,
    INFORMATION = 0x08
  };

  enum eImage {
    IMG_ERROR,
    IMG_WARNING,
    IMG_SUCCESS,
    IMG_INFORMATION,
    IMG_CONTINUATION,
  };
}  // namespace LOGVIEWER



// CskyMonViewerView

IMPLEMENT_DYNCREATE(CskyMonViewerView, CListView)

BEGIN_MESSAGE_MAP(CskyMonViewerView, CListView)
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, &CListView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CListView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CskyMonViewerView::OnFilePrintPreview)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CskyMonViewerView construction/destruction

CskyMonViewerView::CskyMonViewerView() {
}

CskyMonViewerView::~CskyMonViewerView() {
}

BOOL CskyMonViewerView::PreCreateWindow(CREATESTRUCT& cs) {
    cs.style |= LVS_REPORT | LVS_SHOWSELALWAYS | WS_BORDER | WS_TABSTOP;

    return CListView::PreCreateWindow(cs);
}

// CskyMonViewerView drawing

void CskyMonViewerView::OnDraw(CDC* /*pDC*/) {
    CskyMonViewerDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;
}

// CskyMonViewerView printing

void CskyMonViewerView::OnFilePrintPreview() {
#ifndef SHARED_HANDLERS
    AFXPrintPreview(this);
#endif
}

BOOL CskyMonViewerView::OnPreparePrinting(CPrintInfo* pInfo) {
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CskyMonViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {
}

void CskyMonViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {
}

void CskyMonViewerView::OnRButtonUp(UINT /* nFlags */, CPoint point) {
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CskyMonViewerView::OnContextMenu(CWnd* /* pWnd */, CPoint point) {
#ifndef SHARED_HANDLERS
    theApp.GetContextMenuManager()->ShowPopupMenu(
      IDR_POPUP_EDIT,
      point.x,
      point.y,
      this,
      TRUE);
#endif
}

// CskyMonViewerView diagnostics

#ifdef _DEBUG
void CskyMonViewerView::AssertValid() const {
    CListView::AssertValid();
}

void CskyMonViewerView::Dump(CDumpContext& dc) const {
    CListView::Dump(dc);
}

  // non-debug version is inline
CskyMonViewerDoc* CskyMonViewerView::GetDocument() const {
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CskyMonViewerDoc)));
    return reinterpret_cast<CskyMonViewerDoc*>(m_pDocument);
}
#endif  // _DEBUG


// CskyMonViewerView message handlers

void CskyMonViewerView::OnInitialUpdate() {
    CListView::OnInitialUpdate();

    CRect      rect;
    CListCtrl& ctl = GetListCtrl();

    ctl.GetClientRect(&rect);

    ctl.InsertColumn(
      LOGVIEWER::COLUMN_IMAGE,
      _T(""), LVCFMT_LEFT,
      20,
      LOGVIEWER::COLUMN_IMAGE);

    ctl.InsertColumn(
      LOGVIEWER::COLUMN_DATE,
      _T("Timestamp"),
      LVCFMT_LEFT,
      100,
      LOGVIEWER::COLUMN_DATE);

    ctl.InsertColumn(
      LOGVIEWER::COLUMN_SEQUENCE,
      _T("State"),
      LVCFMT_RIGHT,
      100,
      LOGVIEWER::COLUMN_SEQUENCE);

    ctl.SetExtendedStyle(
      ctl.GetExtendedStyle() |
      LVS_EX_FULLROWSELECT |
      LVS_EX_HEADERDRAGDROP |
      LVS_EX_GRIDLINES |
      LVS_EX_INFOTIP);

    ctl.SetImageList(MAINFRAME->GetImageList(), LVSIL_SMALL);
}

struct LVITEM_less : public std::binary_function<LVITEM*, LVITEM*, bool> {
    bool operator()(const LVITEM* pItem1, const LVITEM* pItem2) const {
        double* date1 = reinterpret_cast<double*>(pItem1->lParam);
        double* date2 = reinterpret_cast<double*>(pItem2->lParam);

        return (*date1 < *date2);
    }
};

void CskyMonViewerView::SortItemsAscending(CListCtrl& ctrl) {
    // Sort all items in descending aphabetical order using an STL set
    typedef std::set<LVITEM*, LVITEM_less> ItemSet;
    ItemSet setItems;
    int iCount = ctrl.GetItemCount();
    for (int i = 0; i < iCount; i++) {
        LVITEM* pLVI = new LVITEM();
        ::memset(pLVI, 0, sizeof(LVITEM));
        pLVI->iItem = i;
        pLVI->mask = LVIF_IMAGE |
                     LVIF_INDENT |
                     LVIF_PARAM |
                     LVIF_STATE |
                     LVIF_TEXT;

        pLVI->pszText = new TCHAR[1024];
        pLVI->cchTextMax = 1024;
        ctrl.GetItem(pLVI);

        setItems.insert(pLVI);
    }

    // Remove all items from the list control
    ctrl.DeleteAllItems();

    // Put the items back in the list control in reverse order
    int iIndex = 0;
    for (
      ItemSet::reverse_iterator it = setItems.rbegin();
      it != setItems.rend();
      ++it) {
        LVITEM lvi;

        lvi.mask =  LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
        lvi.iItem = iIndex++;
        lvi.iSubItem = 0;
        lvi.pszText = 0;
        lvi.lParam = 0;

        lvi.iImage = (*it)->iImage;
        ctrl.InsertItem(&lvi);

        lvi.iSubItem = 1;
        lvi.pszText = (*it)->pszText;
        ctrl.SetItem(&lvi);

        lvi.iSubItem = 2;
        lvi.pszText = (*it)->pszText;
        ctrl.SetItem(&lvi);



         // ctrl.InsertItem(*it);

        delete [] (*it)->pszText;
        delete *it;
    }
}

static int CALLBACK
CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
  CListCtrl* pListCtrl = reinterpret_cast<CListCtrl*>(lParamSort);
  double* d1 = reinterpret_cast<double*>(lParam1);
  double* d2 = reinterpret_cast<double*>(lParam2);
  return *d1 < *d2;
}

void CskyMonViewerView::UpdateWithQuery(const wchar_t* pData) {
    CRect      rect;
    CListCtrl& ctl = GetListCtrl();

    ctl.GetClientRect(&rect);

    std::vector<dbcon::UserStat> vecStat;
    theApp.GetDatabase().GetUserStatistic(pData, vecStat);

    ctl.DeleteAllItems();

    int iS = vecStat.size();

    LVITEM lvi;

    for (int i = 0; i < iS; ++i) {
        SYSTEMTIME st = {0};
        VariantTimeToSystemTimeWithMilliseconds(vecStat[i].getDate(), &st);

        wchar_t szTime[32] = {0};
        wsprintf(
          szTime,
          L"%.2d.%.2d.%.4d:%.2d:%.2d:%.2d",
          st.wDay,
          st.wMonth,
          st.wYear,
          st.wHour,
          st.wMinute,
          st.wSecond);

        CString strItemDate(szTime);

        lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = 0;
        lvi.lParam = 0;

        CString strItemStatus;
        int icon;

        for (
          int j = 0;
          j < sizeof(statusesArray) / sizeof(statusesArray[0]);
          ++j) {
            if (statusesArray[j].stat == vecStat[i].getStatus()) {
                strItemStatus.Append(statusesArray[j].desc);
                icon = statusesArray[j].icon;
            }
        }

        lvi.iImage = icon;
        ctl.InsertItem(&lvi);

        lvi.iSubItem = 1;
        lvi.pszText = (LPTSTR)(LPCTSTR)(strItemDate);
        ctl.SetItem(&lvi);

        lvi.iSubItem = 2;
        lvi.pszText = (LPTSTR)(LPCTSTR)(strItemStatus);
        ctl.SetItem(&lvi);
    }
}
