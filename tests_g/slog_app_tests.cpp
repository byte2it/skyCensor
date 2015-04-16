
#include "stdafx.h"
#include "db_tests.h"
#include <Windows.h>

#import "C:\tools\Skype4COM-1.0.38.0\Skype4COM.dll"
using namespace SKYPE4COMLib;
//-----------------------------------------------------------------------------------

// ***** include our TEventHandler.h header class and use the TEventHandlerNamespace namespace. *****
#include "../slog/TEventHandler.h"
using namespace TEventHandlerNamespace;

extern dbcon::CDatabase db;

class SkypeInvoker;

typedef TEventHandler<SkypeInvoker, ISkype, _ISkypeEvents> ISkypeEventHandler;

ISkypePtr m_spSkype;
ISkypeEventHandler* m_spSkypeEventHandler;

class SkypeInvoker{

public:
    HRESULT OnSkypeInvoke
    (
      ISkypeEventHandler* pEventHandler,
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS* pdispparams,
      VARIANT* pvarResult,
      EXCEPINFO* pexcepinfo,
      UINT* puArgErr
      ) {return ERROR_SUCCESS;}
};

bool skype_api_test_run() {

    CoInitialize(0);
    HRESULT hr = m_spSkype.CreateInstance(__uuidof(Skype));
    if(FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    SkypeInvoker invoker;
    m_spSkypeEventHandler = new ISkypeEventHandler(invoker, m_spSkype, &SkypeInvoker::OnSkypeInvoke);
    if(!m_spSkypeEventHandler) {
        m_spSkype.Release();
        CoUninitialize();
        return false;
    }

    if(!m_spSkype || !m_spSkype->Client->IsRunning) {
        m_spSkype.Release();
        m_spSkypeEventHandler->Release();
        //delete m_spSkypeEventHandler;
        CoUninitialize();
        return false;
    }

    hr = m_spSkype->Attach(8, true);
    if(FAILED(hr)) {
        m_spSkype.Release();
        //m_spSkypeEventHandler->Release();
        //delete m_spSkypeEventHandler;
        CoUninitialize();
        return false;
    }

    return true;
}


bool skype_api_test_stop() {

    m_spSkypeEventHandler -> ShutdownConnectionPoint();
    m_spSkypeEventHandler -> Release();
    m_spSkypeEventHandler = NULL;

    m_spSkype.Release();

    //m_spSkypeEventHandler->Release();
    //delete m_spSkypeEventHandler;
    CoUninitialize();
    return true;
}

bool populate_users() {

    db.ExecQuery("drop table users;");
    db.ExecQuery("create table users(handle char(32), name char(50));");

    std::vector<std::wstring> client_users;

    IUserCollection* userCol;
    m_spSkype->get_Friends(&userCol);

    int count= userCol->GetCount();

    for (int i = 0 ; i < count; ++i) {
        IUserPtr pUser;
        userCol->get_Item(i+1, &pUser);
        if(pUser) {

            std::wstring sName(pUser->FullName.operator LPCWSTR());
            if(sName.empty()) {
                sName.append(pUser->DisplayName.operator LPCWSTR());
                if(sName.empty()) {
                    sName.append(pUser->Handle.operator LPCWSTR());
                }
            }

            wchar_t buf[128];
            wsprintf(buf, L"insert into users values ('%s', '%s');", pUser->Handle.operator LPCWSTR(), sName.c_str());
            OutputDebugString(buf);
            db.ExecQuery16((const void**)buf);
            pUser->Release();
        }
    }

    return true;
}




