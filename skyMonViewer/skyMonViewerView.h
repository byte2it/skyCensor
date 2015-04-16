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

#ifndef SKYMONVIEWER_SKYMONVIEWERVIEW_H_
#define SKYMONVIEWER_SKYMONVIEWERVIEW_H_

#define APP         theApp
#define MAINFRAME   ((CMainFrame *) APP.m_pMainWnd)

class CskyMonViewerDoc;

class CskyMonViewerView : public CListView {
 protected:
    CskyMonViewerView();
    DECLARE_DYNCREATE(CskyMonViewerView)

 public:
    CskyMonViewerDoc* GetDocument() const;

 public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    void SortItemsAscending(CListCtrl& ctrl);

 protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

 public:
    virtual ~CskyMonViewerView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

 protected:
    afx_msg void OnFilePrintPreview();
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    DECLARE_MESSAGE_MAP()

 public:
    virtual void OnInitialUpdate();
    void UpdateWithQuery(const wchar_t* pData);
};

#ifndef _DEBUG  // debug version in skyMonViewerView.cpp
inline CskyMonViewerDoc* CskyMonViewerView::GetDocument() const {
  return reinterpret_cast<CskyMonViewerDoc*>(m_pDocument);
}
#endif

#endif  // SKYMONVIEWER_SKYMONVIEWERVIEW_H_

