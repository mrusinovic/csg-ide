
// CsgIde.h : main header file for the CsgIde application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CCsgIdeApp:
// See CsgIde.cpp for the implementation of this class
//

class CCsgIdeApp : public CWinAppEx
{
public:
	CCsgIdeApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCsgIdeApp theApp;
