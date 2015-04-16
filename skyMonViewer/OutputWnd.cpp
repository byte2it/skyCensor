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
#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#include "KeywordDialog.h"

#include <vector>
#include <string>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define APP         theApp
#define MAINFRAME   ((CMainFrame *) APP.m_pMainWnd)

#define GET_MSG_BY_USER    -1000
#define GET_MSG_BY_KEYWORD -1001

COutputWnd::COutputWnd() {
}

COutputWnd::~COutputWnd() {
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create tabs window:
    if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1)) {
        TRACE0("Failed to create output tab window\n");
        return -1;      // fail to create
    }

    // Create output panes:
    const DWORD dwStyle =
      LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

    if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2)) {
        TRACE0("Failed to create output windows\n");
        return -1;      // fail to create
    }

    if (!m_editSearch.Create(
      WS_CHILD | WS_VISIBLE| WS_BORDER,
      CRect(40, 40, 40, 40),
      this,
      3)) {
        TRACE0("Failed to create output windows\n");
        return -1;      // fail to create
    }

    m_editSearch.ShowWindow(SW_SHOW);
    m_editSearch.UpdateWindow();

    UpdateFonts();

    CString strTabName;
    BOOL bNameValid;
    m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);

    return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy) {
    CDockablePane::OnSize(nType, cx, cy);

    m_wndTabs.SetWindowPos(
      NULL,
      -1,
      -1,
      cx,
      cy,
      SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox) {
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

    int cxExtentMax = 0;

    for (int i = 0; i < wndListBox.GetCount(); i ++) {
        CString strItem;
        wndListBox.GetText(i, strItem);
        cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
    }

    wndListBox.SetHorizontalExtent(cxExtentMax);
    dc.SelectObject(pOldFont);
}

void COutputWnd::FillBuildWindow() {
}

void COutputWnd::UpdateFonts() {
    m_wndOutputBuild.SetFont(&afxGlobalData.fontRegular);
}

void COutputWnd::UpdateWithQuery(const wchar_t* pData) {
    m_wndOutputBuild.ResetContent();

    CWnd* pWnd = FindWindow(0, L"TestClient");

    if (pWnd) {
        OutputDebugString(
          L"\n\n\n -- skyMonViewer -- sent GET_CHATS_MSG message ---\n\n\n");

        COPYDATASTRUCT MyCDS = {0};

        MyCDS.cbData = wcslen(pData) * sizeof(wchar_t);
        MyCDS.lpData = (LPVOID)pData;

        ::SendMessage(
          pWnd->m_hWnd,
          WM_COPYDATA,
          GET_MSG_BY_USER ,
          (LPARAM)&MyCDS);

        HANDLE ghMutex = CreateMutex(NULL, FALSE, _T("MMFMutex"));

        if (ghMutex == NULL) {
          if (ERROR_ALREADY_EXISTS == GetLastError())
            return;
        }

        HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MyMMF");

       if (hMapFile == NULL) {
          _tprintf(TEXT("Could not open file mapping object (%d).\n"),
          GetLastError());
          return;
       }

       LPVOID pBuf =  MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

       if (pBuf == NULL) {
          _tprintf(TEXT("Could not map view of file (%d).\n"),
                 GetLastError());

          CloseHandle(hMapFile);

          return;
       }

       int len = wcslen((const wchar_t*)pBuf);

       wchar_t* rBuf = new wchar_t[len+1];

       if (rBuf) {
           std::wstring s;

           wcscpy(rBuf, (const wchar_t*)pBuf);
           s.append(rBuf);

           if (0 != s.compare(L"TRIAL_EXPIRED")) {
               int iPos = std::wstring::npos;
               iPos = s.find_first_of(L"`!@");
               while (iPos != std::wstring::npos) {
                    std::wstring message = s.substr(0, iPos);
                    s.erase(0, iPos+3);
                    m_wndOutputBuild.AddString(message.c_str());
                    iPos = s.find_first_of(L"`!@");
               }
               m_wndOutputBuild.AddString(s.c_str());
           } else {
               theApp.setItems(16);
           }
       }

        UnmapViewOfFile(pBuf);

        CloseHandle(hMapFile);

        ReleaseMutex(ghMutex);
        CloseHandle(ghMutex);

        if (rBuf)
            delete[] rBuf;
    }
}

void COutputWnd::UpdateByKeyword(const wchar_t* key) {
    m_wndOutputBuild.ResetContent();

    CWnd* pWnd = FindWindow(0, L"TestClient");

    if (pWnd) {
        COPYDATASTRUCT MyCDS = {0};

        MyCDS.cbData = wcslen(key) * sizeof(wchar_t);
        MyCDS.lpData = (LPVOID)key;

        ::SendMessage(
          pWnd->m_hWnd,
          WM_COPYDATA,
          GET_MSG_BY_KEYWORD,
          (LPARAM)&MyCDS);

        HANDLE ghMutex = CreateMutex(NULL, FALSE, _T("MMFMutex"));

        if (ghMutex == NULL) {
          if (ERROR_ALREADY_EXISTS == GetLastError())
            return;
        }

        HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MyMMF");

       if (hMapFile == NULL) {
          _tprintf(TEXT("Could not open file mapping object (%d).\n"),
          GetLastError());
          return;
       }

       LPVOID pBuf =  MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

       if (pBuf == NULL) {
          _tprintf(TEXT("Could not map view of file (%d).\n"),
                 GetLastError());

          CloseHandle(hMapFile);

          return;
       }

       int len = wcslen((const wchar_t*)pBuf);

       wchar_t* rBuf = new wchar_t[len+1];

       if (rBuf) {
           std::wstring s;

           wcscpy(rBuf, (const wchar_t*)pBuf);
           s.append(rBuf);

           int iPos = std::wstring::npos;
           iPos = s.find(L"`!@");
           int i = 1;
           while (iPos != std::wstring::npos) {
               ++i;
                std::wstring message = s.substr(0, iPos);
                s.erase(0, iPos+3);
                m_wndOutputBuild.AddString(message.c_str());
                iPos = s.find(L"`!@");
                if (i %2)
                    m_wndOutputBuild.AddString(L"");
           }
           m_wndOutputBuild.AddString(s.c_str());
       }

        UnmapViewOfFile(pBuf);

        CloseHandle(hMapFile);

        ReleaseMutex(ghMutex);
        CloseHandle(ghMutex);

        if (rBuf)
            delete[] rBuf;
    }
}

COutputList::COutputList() {
}

COutputList::~COutputList() {
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
    ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
    ON_COMMAND(ID_SEARCH_BYKEYWORD, OnSearchByKeyword)
    ON_COMMAND(ID_SAVE_TO_FILE, OnSaveToFile)
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_CREATE()
END_MESSAGE_MAP()

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point) {
    CMenu menu;
    menu.LoadMenu(IDR_OUTPUT_POPUP);

    CMenu* pSumMenu = menu.GetSubMenu(0);

    CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

    if (!pPopupMenu->Create(
      this,
      point.x,
      point.y,
      (HMENU)pSumMenu->m_hMenu,
      FALSE, TRUE))
        return;

    MAINFRAME->OnShowPopupMenu(pPopupMenu);
    UpdateDialogControls(this, FALSE);

    SetFocus();
}

void COutputList::OnEditCopy() {
    int idx = GetCurSel();
    CString sText;
    GetText(idx, sText);

    size_t buf_size = sText.GetLength() * sizeof(wchar_t);

    HGLOBAL glob = GlobalAlloc(GMEM_FIXED, buf_size);
    memcpy(glob, sText.GetBuffer(), buf_size);

    OpenClipboard();
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, glob);
    CloseClipboard();
}

void COutputList::OnEditClear() {
    // MessageBox(_T("Clear output"));
}

void COutputList::OnViewOutput() {
    CDockablePane* pParentBar =
      DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
    CMDIFrameWndEx* pMainFrame =
      DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

    if (pMainFrame != NULL && pParentBar != NULL) {
        pMainFrame->SetFocus();
        pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
        pMainFrame->RecalcLayout();
    }
}

int COutputList::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CListBox::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void COutputList::OnSearchByKeyword() {
    CKeywordDialog dlg;
    dlg.DoModal();

    CString s = dlg.GetKeyword();

    if (!s.IsEmpty())
        MAINFRAME->SearchByKeyword(s);
}

void COutputList::OnSaveToFile() {
    char strFilter[] = { "Text Files (*.txt)|*.txt|" };

    CFileDialog FileDlg(FALSE, CString(".txt"), NULL, 0, CString(strFilter));

    if ( FileDlg.DoModal() == IDOK ) {
        CString fileName = FileDlg.GetFileName();
        CString path = FileDlg.GetFolderPath();

        path.Append(L"\\");
        path.Append(fileName);

        SaveWindowToFile(path);
    }
}

void COutputList::SaveWindowToFile(const CString& file) {
  FILE * pFile;
  char buffer[] = { 'x' , 'y' , 'z' };
  pFile = _wfopen(file, L"wt");

  if (pFile) {
    CString lineText;
    int cnt = GetCount();
    for (int i = 0; i < cnt; ++i) {
      GetText(i, lineText);
      std::wstring s(lineText.operator LPCWSTR());
      fwrite (s.c_str() , sizeof(wchar_t) , s.size() , pFile );
    }
    fclose(pFile);
  }
}
