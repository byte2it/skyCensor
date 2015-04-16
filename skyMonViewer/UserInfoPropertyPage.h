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
#ifndef SKYMONVIEWER_USERINFOPROPERTYPAGE_H_
#define SKYMONVIEWER_USERINFOPROPERTYPAGE_H_


// CUserInfoPropertyPage dialog

class CUserInfoPropertyPage : public CPropertyPage {
    DECLARE_DYNAMIC(CUserInfoPropertyPage)

 public:
    CUserInfoPropertyPage();
    virtual ~CUserInfoPropertyPage();

// Dialog Data
    enum { IDD = IDD_USERINFO_PAGE };

 protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

 public:
    CString get_lastName()const {return _lastName;}
    CString get_firstName()const {return _firstName;}
    CString get_email()const {return _email;}
    virtual LRESULT OnWizardNext();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCheck1();

 private:
    bool ValidateEmailAddress();

    CString _firstName;
    CString _lastName;
    bool _isEmail;
    CString _email;
};

#endif  // SKYMONVIEWER_USERINFOPROPERTYPAGE_H_
