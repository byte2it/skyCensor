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
#include "UserInfoPropertyPage.h"
#include "afxdialogex.h"


// CUserInfoPropertyPage dialog

IMPLEMENT_DYNAMIC(CUserInfoPropertyPage, CPropertyPage)

CUserInfoPropertyPage::CUserInfoPropertyPage()
    : CPropertyPage(CUserInfoPropertyPage::IDD)
    , _isEmail(false) {
}

CUserInfoPropertyPage::~CUserInfoPropertyPage() {
}

void CUserInfoPropertyPage::DoDataExchange(CDataExchange* pDX) {
    CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUserInfoPropertyPage, CPropertyPage)
    ON_BN_CLICKED(IDC_CHECK1, &CUserInfoPropertyPage::OnBnClickedCheck1)
END_MESSAGE_MAP()


// CUserInfoPropertyPage message handlers


LRESULT CUserInfoPropertyPage::OnWizardNext() {
    GetDlgItemText(IDC_EDIT_FIRST_NAME, _firstName);

    if (_firstName.GetLength() <= 0) {
        MessageBox(
          TEXT("Please enter valid first name!"),
          TEXT("Error"),
          MB_ICONERROR | MB_OK);
        return -1;
    }

    GetDlgItemText(IDC_EDIT_LAST_NAME, _lastName);

    if (_lastName.GetLength() <= 0) {
        MessageBox(
          TEXT("Please enter valid last name!"),
          TEXT("Error"),
          MB_ICONERROR | MB_OK);
        return -1;
    }

    if (!ValidateEmailAddress()) {
        MessageBox(
          TEXT("Please enter valid email address!"),
          TEXT("Error"),
          MB_ICONERROR | MB_OK);
        return -1;
    }

    return CPropertyPage::OnWizardNext();
}


BOOL CUserInfoPropertyPage::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    CWnd* pWnd = GetDlgItem(IDC_EDIT_EMAIL);
    pWnd->EnableWindow(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CUserInfoPropertyPage::OnBnClickedCheck1() {
    CButton *m_ctlCheck = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK1));
    int ChkBox = m_ctlCheck->GetCheck();

    CWnd* pWnd = GetDlgItem(IDC_EDIT_EMAIL);
    pWnd->EnableWindow(ChkBox == BST_CHECKED);
}

bool CUserInfoPropertyPage::ValidateEmailAddress() {
    CButton *m_ctlCheck = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK1));
    int ChkBox = m_ctlCheck->GetCheck();

    if (ChkBox == BST_CHECKED) {
        GetDlgItemText(IDC_EDIT_EMAIL, _email);

        if (_email.GetLength() <= 0)
            return false;

        if (_email.Find('@') <= 0)
            return false;
    } else {
      return true;
    }
    return false;
}
