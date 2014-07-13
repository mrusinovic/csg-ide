
// CsgIdeView.h : interface of the CCsgIdeView class
//

#pragma once
#include "ScintillaDocView.h"


class CCsgIdeView : public CScintillaView
{
protected: // create from serialization only
	CCsgIdeView();
	DECLARE_DYNCREATE(CCsgIdeView)

// Attributes
public:
	CCsgIdeDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CCsgIdeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void SetAStyle(int style, COLORREF fore, COLORREF back=RGB(0xff, 0xff, 0xff), int size=-1, const char *face=0);
	void DefineMarker(int marker, int markerType, COLORREF fore, COLORREF back);

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

	virtual void OnCharAdded( SCNotification* pSCNotification );
	BOOL SaveFile (LPCTSTR szPath);
	BOOL LoadFile (LPCTSTR szPath);

	afx_msg void OnRunRun();
};

#ifndef _DEBUG  // debug version in CsgIdeView.cpp
inline CCsgIdeDoc* CCsgIdeView::GetDocument() const
   { return reinterpret_cast<CCsgIdeDoc*>(m_pDocument); }
#endif

