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

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CskyMonViewerDoc

IMPLEMENT_DYNCREATE(CskyMonViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CskyMonViewerDoc, CDocument)
END_MESSAGE_MAP()

CskyMonViewerDoc::CskyMonViewerDoc() {
}

CskyMonViewerDoc::~CskyMonViewerDoc() {
}

BOOL CskyMonViewerDoc::OnNewDocument() {
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}

void CskyMonViewerDoc::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        // TODO: add storing code here
    } else {
        // TODO: add loading code here
    }
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CskyMonViewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds) {
    // Modify this code to draw the document's data
    dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

    CString strText = _T("TODO: implement thumbnail drawing here");
    LOGFONT lf;

    CFont* pDefaultGUIFont =
      CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
    pDefaultGUIFont->GetLogFont(&lf);
    lf.lfHeight = 36;

    CFont fontDraw;
    fontDraw.CreateFontIndirect(&lf);

    CFont* pOldFont = dc.SelectObject(&fontDraw);
    dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
    dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CskyMonViewerDoc::InitializeSearchContent() {
    CString strSearchContent;
    // Set search contents from document's data.
    // The content parts should be separated by ";"

    // For example: strSearchContent = _T("point;rectangle;circle;ole object;");
    SetSearchContent(strSearchContent);
}

void CskyMonViewerDoc::SetSearchContent(const CString& value) {
    if (value.IsEmpty()) {
        RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
    } else {
        CMFCFilterChunkValueImpl *pChunk = NULL;
        ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
        if (pChunk != NULL) {
            pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
            SetChunkValue(pChunk);
        }
    }
}

#endif  // SHARED_HANDLERS

// CskyMonViewerDoc diagnostics

#ifdef _DEBUG
void CskyMonViewerDoc::AssertValid() const {
    CDocument::AssertValid();
}

void CskyMonViewerDoc::Dump(CDumpContext& dc) const {
    CDocument::Dump(dc);
}
#endif  // _DEBUG


// CskyMonViewerDoc commands
