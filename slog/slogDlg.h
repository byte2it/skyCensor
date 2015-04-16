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

#ifndef SLOG_SLOGDLG_H_
#define SLOG_SLOGDLG_H_

#if _MSC_VER > 1000
#pragma once
#endif  // _MSC_VER > 1000

#include "..\dbcon\dbcon.h"
#include "SystemTray.h"

class CTestClientDlg;

typedef struct _MESSAGE_DATA {
    double date;
    char status;
    std::wstring message;
} MESSAGE_DATA;

typedef TEventHandler<CTestClientDlg, ISkype, _ISkypeEvents>
  ISkypeEventHandler;


class CTestClientDlg : public CDialog {
    unsigned long items;

 public:
    explicit CTestClientDlg(CWnd* pParent = NULL, const char* dbpath = 0);
    ~CTestClientDlg();

    void setItems(unsigned long l) {items = l;}
    unsigned long getItems()const {return items;}


    enum { IDD = IDD_TESTCLIENT_DIALOG };

 protected:
    virtual void DoDataExchange(CDataExchange* pDX);

 protected:
    HICON                           m_hIcon;

    ISkypePtr m_spSkype;
    ISkypeEventHandler* m_spSkypeEventHandler;

    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDestroy();
    afx_msg void OnButtonCallTestFunction();

    DECLARE_MESSAGE_MAP()

    void PopulateUsers();
    bool FindMessageString(
      const wchar_t* sKey,
      std::vector<IChatMessage*>& resCol);

    bool FindMessagesForUser(
      const wchar_t* sUser,
      std::vector<std::wstring>& resCol);
    bool FindMessagesByKeyword(
      const wchar_t* sKey,
      std::vector<std::wstring>& resCol);
    bool DeleteConversationByUser(const wchar_t* sUser);
    bool PassSelectedResults(std::vector<std::wstring>& resCol);

    HRESULT CTestClientDlg::OnSkypeInvoke
    (
      ISkypeEventHandler* pEventHandler,
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS* pdispparams,
      VARIANT* pvarResult,
      EXCEPINFO* pexcepinfo,
      UINT* puArgErr);

 private:
    void CleanMemoryMapped();
    void CleanResults();

 private:
    dbcon::CDatabase db;

    CSystemTray m_SystemTray;

    HANDLE m_hFileMMF;      // memory mapped file
    LPVOID m_pViewMMFFile;  // view of file, contains text in edit box

    HANDLE ghMutex;

 public:
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
    virtual BOOL OnCmdMsg(
      UINT nID,
      int nCode,
      void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

#endif  // SLOG_SLOGDLG_H_
