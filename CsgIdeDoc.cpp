
// CsgIdeDoc.cpp : implementation of the CCsgIdeDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
// #ifndef SHARED_HANDLERS
// #include "CsgIde.h"
// #endif

#include "CsgIdeDoc.h"

#include <propkey.h>
#include "CsgIdeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCsgIdeDoc

IMPLEMENT_DYNCREATE(CCsgIdeDoc, CDocument)

BEGIN_MESSAGE_MAP(CCsgIdeDoc, CDocument)
END_MESSAGE_MAP()


// CCsgIdeDoc construction/destruction

CCsgIdeDoc::CCsgIdeDoc()
{
	// TODO: add one-time construction code here

}

CCsgIdeDoc::~CCsgIdeDoc()
{
}

BOOL CCsgIdeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CCsgIdeDoc serialization

void CCsgIdeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CCsgIdeDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CCsgIdeDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CCsgIdeDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCsgIdeDoc diagnostics

#ifdef _DEBUG
void CCsgIdeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCsgIdeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCsgIdeDoc commands


BOOL CCsgIdeDoc::OnSaveDocument(LPCTSTR lpszPathName)
{

	POSITION pos = GetFirstViewPosition();
	CCsgIdeView *pFirstView = (CCsgIdeView*)GetNextView( pos );
	if (pFirstView != NULL){
		return pFirstView->SaveFile(lpszPathName);
	}

	return CDocument::OnSaveDocument(lpszPathName);
}
