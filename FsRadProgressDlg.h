#pragma once
#include "..\LibFSRad\IFSRadProgress.h"
#include "Resource.h"
// ---------------------------------------------------------------------------------------------------------------------------------
//  _____                                     _____  _           _     
// |  __ \                                   |  __ \| |         | |    
// | |__) |_ __  ___   __ _ _ __  ___ ___ ___| |  | | | __ _    | |__  
// |  ___/| '__|/ _ \ / _` | '__|/ _ \ __/ __| |  | | |/ _` |   | '_ \ 
// | |    | |  | (_) | (_| | |  |  __/__ \__ \ |__| | | (_| | _ | | | |
// |_|    |_|   \___/ \__, |_|   \___|___/___/_____/|_|\__, |(_)|_| |_|
//                     __/ |                            __/ |          
//                    |___/                            |___/           
//
// Description:
//
//   Progress dialog
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   08/03/2001 by Paul Nettle: Original creation
//
// Restrictions & freedoms pertaining to usage and redistribution of this software:
//
//   This software is 100% free. If you use this software (in part or in whole) you must credit the author. This software may not be
//   re-distributed (in part or in whole) in a modified form without clear documentation on how to obtain a copy of the original
//   work. You may not use this software to directly or indirectly cause harm to others. This software is provided as-is and without
//   warrantee -- Use at your own risk. For more information, visit HTTP://www.FluidStudios.com/
//
// Copyright 2002, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

class	FsRadProgressDlg : public CDialog, public IFSRadProgress
{
public:
				FsRadProgressDlg(CWnd* pParent = NULL);
virtual				~FsRadProgressDlg();

	//{{AFX_DATA(ProgressDlg)
	enum { IDD = IDD_FSRADPROGRESS };
	CButton	pauseButton;
	CStatic	text10;
	CStatic	text9;
	CStatic	text8;
	CStatic	text7;
	CStatic	text6;
	CStatic	text5;
	CStatic	text4;
	CStatic	text3;
	CStatic	text2;
	CStatic	text1;
	CStatic	label10;
	CStatic	label9;
	CStatic	label8;
	CStatic	label7;
	CStatic	label6;
	CStatic	label5;
	CStatic	label4;
	CStatic	label3;
	CStatic	label2;
	CStatic	label1;
	CStatic	runtimeText;
	CButton	stopButton;
	CStatic	currentTaskText;
	CProgressCtrl	currentTaskProgress;
	CStatic	currentTaskPercent;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(ProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

public:
	// Implementation

	virtual		void		setCurrentStatus(const TCHAR* text);
	virtual		void		setCurrentPercent(const float percent);
	virtual		void		setText1(const TCHAR* text);
	virtual		void		setText2(const TCHAR* text);
	virtual		void		setText3(const TCHAR* text);
	virtual		void		setText4(const TCHAR* text);
	virtual		void		setText5(const TCHAR* text);
	virtual		void		setText6(const TCHAR* text);
	virtual		void		setText7(const TCHAR* text);
	virtual		void		setText8(const TCHAR* text);
	virtual		void		setText9(const TCHAR* text);
	virtual		void		setText10(const TCHAR* text);
	virtual		void		setLabel1(const TCHAR* text);
	virtual		void		setLabel2(const TCHAR* text);
	virtual		void		setLabel3(const TCHAR* text);
	virtual		void		setLabel4(const TCHAR* text);
	virtual		void		setLabel5(const TCHAR* text);
	virtual		void		setLabel6(const TCHAR* text);
	virtual		void		setLabel7(const TCHAR* text);
	virtual		void		setLabel8(const TCHAR* text);
	virtual		void		setLabel9(const TCHAR* text);
	virtual		void		setLabel10(const TCHAR* text);

	virtual		void		allowBackgroundProcessing();
	virtual		void		resetStopped();

protected:

	//{{AFX_MSG(ProgressDlg)
	virtual	void	OnCancel();
	virtual	void	OnOK();
	virtual BOOL	OnInitDialog();
	afx_msg void OnStop();
	afx_msg void OnPause();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// ---------------------------------------------------------------------------------------------------------------------------------
// ProgressDlg.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
