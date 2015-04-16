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

#ifndef SKYMONVIEWER_SKYMONVIEWER_H_
#define SKYMONVIEWER_SKYMONVIEWER_H_

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "..\dbcon\dbcon.h"


// CskyMonViewerApp:
// See skyMonViewer.cpp for the implementation of this class
//

class CskyMonViewerApp : public CWinAppEx {
 public:
    CskyMonViewerApp();

    dbcon::CDatabase& GetDatabase() {return db;}

    std::string GetUserAppFolderPath();

    virtual BOOL InitInstance();
    virtual int ExitInstance();

    UINT  m_nAppLook;
    BOOL  m_bHiColorIcons;

    virtual void PreLoadState();
    virtual void LoadCustomState();
    virtual void SaveCustomState();

    void setItems(unsigned long it) {items = it;}
    unsigned long getItems()const { return items; }

    afx_msg void OnAppAbout();
    void OnSearchByKeyword();
    DECLARE_MESSAGE_MAP()

 private:
    dbcon::CDatabase db;
    unsigned long items;
};

extern CskyMonViewerApp theApp;

#endif  // SKYMONVIEWER_SKYMONVIEWER_H_
