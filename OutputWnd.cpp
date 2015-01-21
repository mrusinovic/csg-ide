
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "ScintillaCtrl.h"
#include "ScintillaDocView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_LBN_DBLCLK(2, OnLbnDblClick)
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
// 	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
// 	{
// 		TRACE0("Failed to create output tab window\n");
// 		return -1;      // fail to create
// 	}

	// Create output panes:
	const DWORD dwStyle = LBS_NOTIFY | LBS_USETABSTOPS | LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutput.Create(dwStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}

	UpdateFonts();

//	CString strTabName;
//	BOOL bNameValid;

	// Attach list windows to tab:
// 	bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
// 	ASSERT(bNameValid);
// 	m_wndTabs.AddTab(&m_wndOutput, strTabName, (UINT)0);

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndOutput.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

std::vector<CString> SplitString(TCHAR* str, const TCHAR* sep)
{
	std::vector<CString> ret;

	TCHAR* token = wcstok(str,sep);

	while( token != NULL )
	{
		ret.push_back(token);
		token = wcstok(NULL, sep);
	}

	return ret;
}


void COutputWnd::AddString( const char* str )
{
	auto vec = SplitString(const_cast<TCHAR*>((LPCWSTR)CScintillaCtrl::UTF82W(str,strlen(str))),L"\n");
	if (vec.empty()) return;

	auto i = vec.begin();
	m_wndOutput.AddString(*i);
	
	for(i++; i!=vec.end(); i++){
		m_wndOutput.AddString(*i);
	}

}

void COutputWnd::UpdateFonts()
{
	m_wndOutput.SetFont(&afxGlobalData.fontRegular);
}

void COutputWnd::OnLbnDblClick()
{
	CString str;
	m_wndOutput.GetText(m_wndOutput.GetCurSel(),str);

	if (str[0]==L'\t'){
		int index = str.Find(L"]:");
		int lnNum = _wtoi(((LPCWSTR)str)+index+2);
		
		if (lnNum<1) return;

	  	auto mf = (CMainFrame*)AfxGetMainWnd();
		auto mc = mf->MDIGetActive();
		if (mc){
			auto v = (CScintillaView*)mc->GetActiveView();
			v->GetCtrl().GotoLine(lnNum-1);
			v->SetFocus();
			v->GetCtrl().SetFocus();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void COutputList::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}

void COutputList::OnEditClear()
{
	MessageBox(_T("Clear output"));
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}
