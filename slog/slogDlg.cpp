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
#include "slog.h"
#include "slogDlg.h"

#include "../common/datetime.h"
#include <algorithm>
#include <cstdlib>
#include  <wchar.h>

#define GET_MSG_BY_USER             -1000
#define GET_MSG_BY_KEYWORD          -1001
#define ACTIVATE_APPLICATION        -1002
#define DELETE_CONVERSATION_BY_USER -1003

#define WM_MY_NOTIFY   WM_APP + 101
#define IDT_CHECK_LIC_TIMER  100001

#define TIMER_ONE_HOUR  1000 * 60 * 60

#define VER_MAJOR 1
#define VER_MINOR 1
#define VER_REV   186
#define VER_BUILD 13

#ifdef UNICODE
#define _print_t wsprintf
#define string_t std::wstring
#else
#define _print_s sprintf
#define string_t std::string
#define
#endif

string_t GetVersionString() {
    TCHAR szVer[1024];
    _print_t(
      szVer,
      _T("%d.%d.%d.%d",
      VER_MAJOR,
      VER_MINOR,
      VER_REV,
      VER_BUILD));
    return string_t(szVer);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

    typedef struct _STATUS {
    short        key;
    unsigned int stat;
    const char*  desc;
}STATUS;

STATUS statusesArray[] = {
    {0, TOnlineStatus::olsOffline, "Offline"},
    {1, TOnlineStatus::olsOnline, "Online"},
    {2, TOnlineStatus::olsAway, "Away"},
    {3, TOnlineStatus::olsNotAvailable, "Not available"},
    {4, TOnlineStatus::olsDoNotDisturb, "Do not disturb"},
    {5, TOnlineStatus::olsSkypeOut, "Skype out"},
    {6, TOnlineStatus::olsSkypeMe, "Skype ME"},
    {-1, TOnlineStatus::olsUnknown, "Uknown"}
};

CTestClientDlg::CTestClientDlg(CWnd* pParent /*=NULL*/, const char* dbpath)
    : CDialog(CTestClientDlg::IDD, pParent)
    , db(dbpath)
    , m_spSkype(0)
    , m_pViewMMFFile(0)
    , m_hFileMMF(0)
    , items(0) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTestClientDlg::~CTestClientDlg() {
    try {
        if (m_pViewMMFFile) {
        UnmapViewOfFile(m_pViewMMFFile);
        m_pViewMMFFile = NULL;
    }

    if (m_hFileMMF) CloseHandle(m_hFileMMF);
    m_spSkype.Release();
    }
    catch (...) {
        ExitProcess(-1);
    }
}

void CTestClientDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestClientDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BUTTON_CALL_TEST_FUNCTION, OnButtonCallTestFunction)
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


BOOL CTestClientDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    db.ExecQuery("create table statuses(name char(16), statusid integer);");
    db.ExecQuery("create table user_statuses(" \
      "time double, user char(32), statusid integer);");

    m_SystemTray.Create(
      this,
      WM_MY_NOTIFY,
      L"Click here",
      m_hIcon,
      0/* nSystemTrayID*/);

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    try {
        HRESULT hr = m_spSkype.CreateInstance(__uuidof(Skype));
        if (FAILED(hr)) {
            m_SystemTray.ShowBalloon(
                L"Please run Skype if do you want to use skyCensor application",
                L"skyCensor - Skype instance not running",
                0,
                2000);

            Sleep(60000);

            OutputDebugStringA("m_spSkype.CreateInstance failed");
            ExitProcess(-1);
        }

        m_spSkypeEventHandler = new ISkypeEventHandler(
                                       *this,
                                       m_spSkype,
                                       &CTestClientDlg::OnSkypeInvoke);

        if (!m_spSkypeEventHandler) {
            m_SystemTray.ShowBalloon(
                L"Please run Skype if do you want to use skyCensor application",
                L"skyCensor - Skype instance not running",
                0,
                2000);

            Sleep(60000);

            m_spSkype.Release();
            OutputDebugStringA("creating m_spSkypeEventHandler failed");
            ExitProcess(-1);
        }

        if (!m_spSkype || !m_spSkype->Client->IsRunning) {
            OutputDebugString(L"Skype is not running!");
            m_SystemTray.ShowBalloon(
                L"Please run Skype if do you want to use skyCensor application",
                L"skyCensor - Skype instance not running",
                0,
                2000);

            Sleep(60000);

            ExitProcess(-1);
        }

        hr = m_spSkype->Attach(8, true);
        if (FAILED(hr)) {
            OutputDebugString(L"m_spSkype->Attach failed");
            m_spSkype->Release();
            return FALSE;
        }
    }
    catch(...) { ExitProcess(-1); }

    PopulateUsers();

    SetWindowText(L"TestClient");

    return TRUE;
}

void CTestClientDlg::CleanMemoryMapped() {
        if (m_pViewMMFFile) {
        UnmapViewOfFile(m_pViewMMFFile);
        m_pViewMMFFile = NULL;
    }

    if (m_hFileMMF) {
        CloseHandle(m_hFileMMF);
        m_hFileMMF = NULL;
    }
}

void CTestClientDlg::CleanResults() {
    CleanMemoryMapped();
}



bool CTestClientDlg::PassSelectedResults(std::vector<std::wstring>& resCol) {
    CleanMemoryMapped();

    int sz = resCol.size();
    std::wstring ss;

    for (int i = 0; i < sz ; ++i) {
            ss.append(resCol[i]);
            ss.append(L"`!@");
    }


    int mem_size = (ss.size() * sizeof(wchar_t));

    m_hFileMMF = CreateFileMapping(
                        INVALID_HANDLE_VALUE,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        mem_size,
                        L"MyMMF");

    if (!m_hFileMMF) {
        OutputDebugString(_T("Creation of file mapping failed"));
    }

    // map all file
    m_pViewMMFFile = MapViewOfFile(m_hFileMMF, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (!m_pViewMMFFile) {
        OutputDebugString(_T("MapViewOfFile function failed"));
    }

    ghMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        _T("MMFMutex"));

    if (ghMutex == NULL) {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

    CopyMemory((PVOID)m_pViewMMFFile, (PVOID)ss.c_str(), mem_size);

    ReleaseMutex(ghMutex);
    CloseHandle(ghMutex);

    return true;
}

bool CTestClientDlg::FindMessagesForUser(
  const wchar_t* sUser,
  std::vector<std::wstring>& resCol) {
    bool res = false;
    IChatCollection* chats;
    m_spSkype->get_Chats(&chats);

    if (!chats) return false;

    long lCnt = chats->GetCount();

    for (long l = 1; l <= lCnt; ++l) {
        IChat* chat;
        chats->get_Item(l, &chat);
        IChatMessageCollection* msgCol;
        chat->get_Messages(&msgCol);

        std::wstring sChat(chat->GetName());

        long lCntMes = msgCol->GetCount();

        for (long ll = 1; ll <= lCntMes; ++ll) {
            IChatMessage* msg;
            msgCol->get_Item(ll, &msg);
            std::wstring sHandle(msg->GetFromHandle());
            if (sHandle.compare(sUser) == 0) {
                std::wstring data;

                SYSTEMTIME st;
                VariantTimeToSystemTime(msg->GetTimestamp(), &st);

                wchar_t szLocTime[512] = {0};
                wsprintf(
                  szLocTime,
                  L"%02d.%02d.%02d.%02d:%02d:%02d\0",
                  st.wDay,
                  st.wMonth,
                  st.wYear,
                  st.wHour,
                  st.wMinute,
                  st.wSecond);

                data.append(szLocTime);
                resCol.push_back(data);
                data.clear();

                data.append(msg->Body.operator LPCWSTR());
                resCol.push_back(data);
                res = true;
            }
            msg->Release();
        }
        msgCol->Release();
        chat->Release();
    }
    chats->Release();
    return res;
}

bool CTestClientDlg::FindMessagesByKeyword(
  const wchar_t* sKey,
  std::vector<std::wstring>& resCol) {
    bool res = false;
    IChatCollection* chats;
    m_spSkype->get_Chats(&chats);

    if (!chats) return false;

    long lCnt = chats->GetCount();

    for (long l = 1; l <= lCnt; ++l) {
        IChat* chat;
        chats->get_Item(l, &chat);
        IChatMessageCollection* msgCol;
        chat->get_Messages(&msgCol);

        long lCntMes = msgCol->GetCount();

        for (long ll = 1; ll <= lCntMes; ++ll) {
            IChatMessage* msg;
            msgCol->get_Item(ll, &msg);

            std::wstring message(msg->Body);

            if (message.find(sKey) !=std::wstring::npos) {
                std::wstring data;

                SYSTEMTIME st;
                VariantTimeToSystemTime(msg->GetTimestamp(), &st);
                std::wstring handle;

                IUserPtr usr = msg->GetSender();
                if (usr) {
                    handle.append(usr->GetHandle().operator LPCWSTR());
                }

                wchar_t szLocTime[512] = {0};
                wsprintf(
                  szLocTime,
                  L"%02d.%02d.%02d.%02d:%02d:%02d - %s\0",
                  st.wDay,
                  st.wMonth,
                  st.wYear,
                  st.wHour,
                  st.wMinute,
                  st.wSecond,
                  handle.c_str());

                data.append(szLocTime);
                resCol.push_back(data);
                data.clear();

                data.append(message);
                resCol.push_back(data);
                res = true;
            }
            msg->Release();
        }
        msgCol->Release();
        chat->Release();
    }
    chats->Release();
    return res;
}

bool CTestClientDlg::DeleteConversationByUser(const wchar_t* sUser) {
    bool res = false;
    IChatCollection* chats;
    m_spSkype->get_Chats(&chats);

    if (!chats) return false;

    long lCnt = chats->GetCount();

    for (long l = 1; l <= lCnt; ++l) {
        IChat* chat;
        chats->get_Item(l, &chat);

        IUserCollectionPtr users = chat->GetMembers();
        if (users->GetCount() == 2) {
            for (long il = 1; il <=2; ++il) {
                IUserPtr usr = users->GetItem(il);
                std::wstring sHandle = usr->GetHandle();
                if (sHandle.compare(sUser) == 0) {
                    IChatMessageCollectionPtr messages;
                    chat->get_Messages(&messages);
                    messages->raw_RemoveAll();
                    messages->Release();
                }
            }
        }
        chat->Release();
    }
    chats->raw_RemoveAll();
    chats->Release();
    return res;
}

bool CTestClientDlg::FindMessageString(
  const wchar_t* sKey,
  std::vector<IChatMessage*>& resCol) {
    bool res = false;
    IChatCollection* chats;
    m_spSkype->get_Chats(&chats);

    long lCnt = chats->GetCount();

    unsigned int uId = db.GetMaxMsgId();
    if (uId == 0) uId = 1;

    for (long l = 1; l <= lCnt; ++l) {
        IChat* chat;
        chats->get_Item(l, &chat);
        IChatMessageCollection* msgCol;
        chat->get_Messages(&msgCol);

        std::wstring sChat(chat->GetName());

        long lCntMes = msgCol->GetCount();

        for (long ll = 1; ll <= lCntMes; ++ll, ++uId) {
            IChatMessage* msg;
            msgCol->get_Item(ll, &msg);
            std::wstring sHandle(msg->GetFromHandle());
            std::wstring sMsg(msg->GetBody());

            if (sMsg.find(sKey) != -1) {
                resCol.push_back(msg);
                res = true;
            }
        }
    }
    return res;
}

void CTestClientDlg::PopulateUsers() {
    db.ExecQuery("drop table users;");
    db.ExecQuery("create table users(handle char(32), name char(50));");

    std::vector<std::wstring> client_users;

    IUserCollection* userCol;
    m_spSkype->get_Friends(&userCol);

    int count = userCol->GetCount();

    for (int i = 0 ; i < count; ++i) {
        IUserPtr pUser;
        userCol->get_Item(i+1, &pUser);
        if (pUser) {
            std::wstring sName(pUser->FullName.operator LPCWSTR());
            if (sName.empty()) {
                sName.append(pUser->DisplayName.operator LPCWSTR());
                if (sName.empty()) {
                    sName.append(pUser->Handle.operator LPCWSTR());
                }
            }

            wchar_t buf[128];
            wsprintf(
              buf,
              L"insert into users values ('%s', '%s');",
              pUser->Handle.operator LPCWSTR(),
              sName.c_str());
            OutputDebugString(buf);
            db.ExecQuery16((const void**)buf);
            pUser->Release();
        }
    }
}

void CTestClientDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    CDialog::OnSysCommand(nID, lParam);
}

void CTestClientDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialog::OnPaint();
    }
}

HCURSOR CTestClientDlg::OnQueryDragIcon() {
    return (HCURSOR) m_hIcon;
}


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
  UINT* puArgErr) {
  if (dispidMember == 0x07) {  // User online status event
      try {
        VARIANT varUser;
        VariantInit(&varUser);
        VariantClear(&varUser);
        varUser = (pdispparams -> rgvarg)[1];

        SYSTEMTIME lt = {0};
        GetLocalTime(&lt);

        double time;

        SystemTimeToVariantTimeWithMilliseconds(lt, &time);

        std::wstring sUser;

        IUser* pUser = reinterpret_cast<IUser*>(V_INT_PTR(&varUser));
        if (pUser) {
            sUser.append(pUser->Handle.operator LPCWSTR());
        }

        VARIANT varOnlineStatus;
        VariantInit(&varOnlineStatus);
        VariantClear(&varOnlineStatus);
        varOnlineStatus = (pdispparams -> rgvarg)[0];

        TOnlineStatus status = TOnlineStatus(V_I4(&varOnlineStatus));

        typedef struct _STATUS {
           short        key;
           unsigned int stat;
           const wchar_t*  desc;
        }STATUS;

        const int SIZEOF_DOUBLE = 32;
        char szDouble[SIZEOF_DOUBLE] = {0};
        wchar_t wcsDouble[SIZEOF_DOUBLE] = {0};
        _snprintf_s(szDouble, SIZEOF_DOUBLE, "%12.20f", time);
        size_t num_converted = 0;
        mbstowcs_s(&num_converted, wcsDouble, szDouble, 64);
        std::wstring sDouble(wcsDouble);

        wchar_t buf[256];
        wsprintf(
          buf,
          L"insert into user_statuses values (%s, '%s', %d);",
          sDouble.c_str(),
          sUser.c_str(),
          (unsigned int)status);
        OutputDebugString(buf);
        db.ExecQuery16((const void**)buf);
      }
      catch (...) {
          ExitProcess(-1);
      }
  } else if (dispidMember == 0x11) {  // MessageStatus event
      //...
  }

  return S_OK;
}

void CTestClientDlg::OnDestroy() {
    CDialog::OnDestroy();

    try {
        if (m_spSkypeEventHandler) {
          m_spSkypeEventHandler -> ShutdownConnectionPoint();
          m_spSkypeEventHandler -> Release();
          m_spSkypeEventHandler = NULL;
        }

        if (m_spSkype) {
            m_spSkype->Release();
            m_spSkype = NULL;
        }
    }
    catch(...) {
        ExitProcess(-1);
    }
}

void CTestClientDlg::OnButtonCallTestFunction() {
}


void CTestClientDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos) {
    CDialog::OnWindowPosChanging(lpwndpos);

    lpwndpos->flags &= ~SWP_SHOWWINDOW;
}


BOOL CTestClientDlg::OnCmdMsg(
  UINT nID,
  int nCode,
  void* pExtra,
  AFX_CMDHANDLERINFO* pHandlerInfo) {
    if (nID == GET_CHATS_MSG) {
        int g = 0;
        g++;
    }

    return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CTestClientDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
    return CDialog::OnCommand(wParam, lParam);
}


LRESULT CTestClientDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_COPYDATA) {
        try {
        if (GET_MSG_BY_USER == wParam) {
            PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT) lParam;
            std::wstring str((const wchar_t*)pMyCDS->lpData);
            str.resize(pMyCDS->cbData / sizeof(wchar_t));

            std::vector<std::wstring> resCol;
            if (FindMessagesForUser(str.c_str(), resCol)) {
                PassSelectedResults(resCol);
            } else {
                CleanResults();
            }
        } else if (GET_MSG_BY_KEYWORD == wParam) {
            PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT) lParam;
            std::wstring str((const wchar_t*)pMyCDS->lpData);
            str.resize(pMyCDS->cbData / sizeof(wchar_t));

            std::vector<std::wstring> resCol;
            if (FindMessagesByKeyword(str.c_str(), resCol)) {
                PassSelectedResults(resCol);
            } else {
                CleanResults();
            }
        } else if (DELETE_CONVERSATION_BY_USER == wParam) {
            PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT) lParam;
            std::wstring str((const wchar_t*)pMyCDS->lpData);
            str.resize(pMyCDS->cbData / sizeof(wchar_t));

            DeleteConversationByUser(str.c_str());
        }
    }
    catch(...) {
            ExitProcess(-1);
        }
    }

    return CDialog::WindowProc(message, wParam, lParam);
}

