
// CsgIdeView.cpp : implementation of the CCsgIdeView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CsgIde.h"
#endif

#include "CsgIdeDoc.h"
#include "CsgIdeView.h"
#include "MainFrm.h"
#include "SGScript.h"
#include "FsRadProgressDlg.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const char LuaKeywords[]= "and break do else elseif end false for function if in local nil not or repeat return then true until while";

const char LuaFunctions[]=
	"_G _VERSION assert collectgarbage dofile error getfenv getmetatable ipairs load loadfile loadstring module next pairs pcall "
	"print rawequal rawget rawset require select setfenv setmetatable tonumber tostring type unpack xpcall gcinfo newproxy "

	"coroutine.create coroutine.resume coroutine.running coroutine.status coroutine.wrap coroutine.yield "

	"debug.debug debug.getfenv debug.gethook debug.getinfo debug.getlocal debug.getmetatable debug.getregistry "
	"debug.getupvalue debug.setfenv debug.sethook debug.setlocal debug.setmetatable debug.setupvalue debug.traceback "

	"io.close io.flush io.input io.lines io.open io.output io.popen io.read io.stderr io.stdin io.stdout io.tmpfile io.type io.write "

	"math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp math.floor math.fmod "
	"math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.modf math.pi math.pow math.rad math.random "
	"math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh math.mod "

	"os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale os.time os.tmpname "

	"package.cpath package.loaded package.loaders package.loadlib package.path package.preload package.seeall package.config "

	"string.byte string.char string.dump string.find string.format string.gmatch string.gsub string.len string.lower string.match "
	"string.rep string.reverse string.sub string.upper string.gfind "

	"table.concat table.insert table.maxn table.remove table.sort table.setn table.getn table.foreachi table.foreach "

	"_LUA_VERSION";

const char UserFunctions[]=
	"render cube sphere cylinder cone log "
	"clear scale scaleby move split rotate transform invert contains clone init sides side size push pop ocluded divide ";

const char UserConstants[]=
	"world.width world.depth world.height PI X Y Z INVISIBLE ";

const char UserVariables[]=
	"world world.ocl world.ocl_pass V P material";

// CCsgIdeView

IMPLEMENT_DYNCREATE(CCsgIdeView, CScintillaView)

BEGIN_MESSAGE_MAP(CCsgIdeView, CScintillaView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_RUN_RUN, &CCsgIdeView::OnRunRun)
	ON_UPDATE_COMMAND_UI(ID_RUN_RUN, &CCsgIdeView::OnUpdateRunRun)
	ON_COMMAND(ID_RUN_RUNFINAL, &CCsgIdeView::OnRunRunfinal)
	ON_UPDATE_COMMAND_UI(ID_RUN_RUNFINAL, &CCsgIdeView::OnUpdateRunRunfinal)
END_MESSAGE_MAP()

// CCsgIdeView construction/destruction

CCsgIdeView::CCsgIdeView()
	:m_running(false)
{
	// TODO: add construction code here

}

CCsgIdeView::~CCsgIdeView()
{
}

BOOL CCsgIdeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return __super::PreCreateWindow(cs);
}

// CCsgIdeView drawing

void CCsgIdeView::OnDraw(CDC* /*pDC*/)
{
	CCsgIdeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CCsgIdeView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCsgIdeView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCsgIdeView diagnostics

#ifdef _DEBUG
void CCsgIdeView::AssertValid() const
{
	__super::AssertValid();
}

void CCsgIdeView::Dump(CDumpContext& dc) const
{
	__super::Dump(dc);
}

CCsgIdeDoc* CCsgIdeView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCsgIdeDoc)));
	return (CCsgIdeDoc*)m_pDocument;
}
#endif //_DEBUG


// CCsgIdeView message handlers

void CCsgIdeView::SetAStyle(int style, COLORREF fore, COLORREF back, int size, const char* face) 
{
	CScintillaCtrl& rCtrl = GetCtrl();

	rCtrl.StyleSetFore(style, fore);
	rCtrl.StyleSetBack(style, back);
	if (size >= 1)
		rCtrl.StyleSetSize(style, size);
	if (face) 
		rCtrl.StyleSetFont(style, face);
}

void CCsgIdeView::DefineMarker(int marker, int markerType, COLORREF fore, COLORREF back) 
{
	CScintillaCtrl& rCtrl = GetCtrl();

	rCtrl.MarkerDefine(marker, markerType);
	rCtrl.MarkerSetFore(marker, fore);
	rCtrl.MarkerSetBack(marker, back);
}

void CCsgIdeView::OnInitialUpdate()
{
	CScintillaView::OnInitialUpdate();

	CScintillaCtrl& rCtrl = GetCtrl();

	//Setup the Lexer
	rCtrl.SetLexer(SCLEX_LUA);
	rCtrl.StyleSetFont(STYLE_DEFAULT, "Consolas");
	rCtrl.StyleSetSize(STYLE_DEFAULT, 10);
	rCtrl.SetKeyWords(0, LuaKeywords);

	//Setup styles
	SetAStyle(STYLE_DEFAULT, RGB(0, 0, 0), RGB(0xff, 0xff, 0xff), 11, "Consolas");
	rCtrl.StyleClearAll();
	SetAStyle(SCE_LUA_DEFAULT, RGB(0, 0, 0));
	SetAStyle(SCE_LUA_COMMENT, RGB(0, 0x80, 0));
	SetAStyle(SCE_LUA_COMMENTLINE, RGB(0, 0x80, 0));
	SetAStyle(SCE_LUA_COMMENTDOC, RGB(0, 0x80, 0));
	SetAStyle(SCE_LUA_NUMBER, RGB(0, 0x80, 0x80));
	SetAStyle(SCE_LUA_WORD, RGB(0, 0, 0x80));
	//rCtrl.StyleSetBold(SCE_LUA_WORD, 1);
	SetAStyle(SCE_LUA_STRING, RGB(0x80, 0, 0x80));
	SetAStyle(SCE_LUA_IDENTIFIER, RGB(0, 0, 0));
	SetAStyle(SCE_LUA_PREPROCESSOR, RGB(0x80, 0, 0));
	SetAStyle(SCE_LUA_OPERATOR, RGB(0x80, 0x80, 0));

	rCtrl.SetKeyWords(1, LuaFunctions);
	SetAStyle(SCE_LUA_WORD2, RGB(0x20, 0x20, 0x80));
	//rCtrl.StyleSetBold(SCE_LUA_WORD2, 1);

	rCtrl.SetKeyWords(2, UserFunctions);
	SetAStyle(SCE_LUA_WORD3, RGB(150, 0, 50));
	//rCtrl.StyleSetBold(SCE_LUA_WORD3, 1);

	rCtrl.SetKeyWords(3, UserConstants);
	SetAStyle(SCE_LUA_WORD4, RGB(255, 0, 0));
	//rCtrl.StyleSetBold(SCE_LUA_WORD4, 1);

	rCtrl.SetKeyWords(4, UserVariables);
	SetAStyle(SCE_LUA_WORD5, RGB(200, 50, 0));
	//rCtrl.StyleSetBold(SCE_LUA_WORD5, 1);

	//Setup folding
	rCtrl.SetMarginWidthN(2, 16);
	rCtrl.SetMarginSensitiveN(2, TRUE);
	rCtrl.SetMarginTypeN(2, SC_MARGIN_SYMBOL);
	rCtrl.SetMarginMaskN(2, SC_MASK_FOLDERS);
	rCtrl.SetProperty(_T("fold"), _T("1"));

	rCtrl.SetTabWidth(4);

	//Setup markers
	DefineMarker(SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0xFF));
	DefineMarker(SC_MARKNUM_FOLDER, SC_MARK_PLUS, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));
	DefineMarker(SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));
	DefineMarker(SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));
	DefineMarker(SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));
	DefineMarker(SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));
	DefineMarker(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY, RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));

	//Setup auto completion
	rCtrl.AutoCSetSeparator(10); //Use a separator of line feed

	//Setup call tips
	rCtrl.SetMouseDwellTime(1000);

	//Enable Multiple selection
	rCtrl.SetMultipleSelection(TRUE);

	CDocument *pDoc = GetDocument();
	LoadFile(pDoc->GetPathName());
}

BOOL CCsgIdeView::LoadFile (LPCTSTR szPath)
{
	// if pathname is empty do nothing
	if (szPath == NULL || *szPath == L'\0')
		return TRUE;

	HANDLE h = CreateFile(szPath,GENERIC_READ, FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	DWORD fs=0, fsr=0;
	fs = GetFileSize(h,NULL);

	CStringA str;
	ReadFile(h,str.GetBufferSetLength(fs),fs,&fsr,NULL);

	ASSERT(fs == fsr);
	GetCtrl().SetText((LPCSTR)str);

	CloseHandle(h);

	GetCtrl().FoldAll(SC_FOLDACTION_CONTRACT);

	return TRUE;
}

BOOL CCsgIdeView::SaveFile (LPCTSTR szPath)
{
	// if pathname is empty do nothing
	if (szPath == NULL || *szPath == L'\0')
		return TRUE;

	HANDLE h = CreateFile(szPath,GENERIC_WRITE, FILE_SHARE_READ,NULL,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (h==INVALID_HANDLE_VALUE) 
		h = CreateFile(szPath,GENERIC_WRITE, FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	DWORD fs=0, fsr=0;

	fs = GetCtrl().GetTextLength();

	const auto& tmp = GetCtrl().GetText(fs+1);
	CStringA str = CScintillaCtrl::W2UTF8(tmp,tmp.GetLength());

	WriteFile(h,(LPCSTR)str,fs,&fsr,NULL);

	CloseHandle(h);
	GetDocument()->SetModifiedFlag(FALSE);
	return TRUE;
}


void CCsgIdeView::OnCharAdded( SCNotification* pSCNotification )
{
	CScintillaCtrl& ctrl = GetCtrl();

	if  (/*pSCNotification->ch  ==  '\r'  ||*/  pSCNotification->ch  ==  '\n')  {
		char  linebuf[1000];
		
		int  curLine  =  ctrl.LineFromPosition(ctrl.GetCurrentPos());
		int  lineLength  =  ctrl.LineLength(curLine);

		if  (curLine  >  0  &&  lineLength  <=  2)  {
			int  prevLineLength  =  ctrl.LineLength(curLine  -  1);
			if  (prevLineLength  <  sizeof(linebuf))  {
				WORD  buflen  =  sizeof(linebuf);
				memcpy(linebuf,  &buflen,  sizeof(buflen));
				
				ctrl.GetLine(curLine  -  1, static_cast<char  *>(linebuf));
				
				linebuf[prevLineLength]  =  '\0';
				for  (int  pos  =  0;  linebuf[pos];  pos++)  {
					if  (linebuf[pos]  !=  ' '  &&  linebuf[pos]  !=  '\t')
						linebuf[pos]  =  '\0';
				}
				ctrl.ReplaceSel(static_cast<char  *>(linebuf));
			}
		}
	}

	GetDocument()->SetModifiedFlag(TRUE);
}


void CCsgIdeView::OnRunRun()
{
	CSGProcessor::CSGScript sc;
	sc.m_calcLightMap=false;
	RunScript(&sc);
}


void CCsgIdeView::OnUpdateRunRun(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_running);
}
void CCsgIdeView::OnUpdateRunRunfinal(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_running);
}


void CCsgIdeView::OnRunRunfinal()
{
	CSGProcessor::CSGScript sc;
	sc.m_calcLightMap=true;
	RunScript(&sc);
}

void CCsgIdeView::RunScript( CSGProcessor::CSGScript* sc )
{
	struct tmp{
		bool* running;
		tmp(bool* run):running(run){*running = true;}
		~tmp(){*running = false;}
	};

	tmp t(&m_running);

	DWORD fs=0;

	fs = GetCtrl().GetTextLength();

	const auto& tmp = GetCtrl().GetText(fs+1);
	CStringA str = CScintillaCtrl::W2UTF8(tmp,tmp.GetLength());

	FsRadProgressDlg prog(this);
	sc->Run(str, &prog);

}


