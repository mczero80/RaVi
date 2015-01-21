// MainFrm.cpp : Implementierung der Klasse CMainFrame
//

#include "stdafx.h"
#include "RaVi-Demo.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMainFrame *FrmWin;

CStatusBar *StatusBar;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // Statusleistenanzeige
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Konstruktion/Zerstörung

CMainFrame::CMainFrame(){
  FrmWin=this;
}

CMainFrame::~CMainFrame() { }

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if(CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;
  // create a view to occupy the client area of the frame
  if(!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,	CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))	{
    TRACE0("Failed to create view window\n");
    return -1;
  }

  if(!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT))) {
    TRACE0("Statusbar could not be created\n");
    return -1;
  }

  StatusBar = &(m_wndStatusBar);
  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if(!CFrameWnd::PreCreateWindow(cs))	return FALSE;

  cs.cx         = 320+2*GetSystemMetrics(SM_CXSIZEFRAME)+4;
  cs.cy         = 240+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYMENU)+22;
  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass  = AfxRegisterWndClass(0);

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Diagnose

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Nachrichten-Handler
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// Fokus an das Ansichtfenster weitergeben
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Ansichtfenster erhält ersten Eindruck vom Befehl
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// andernfalls die Standardbehandlung durchführen
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

//extern FILE  *GraphFile;
//extern short  Graph;

void CMainFrame::OnClose() 
{
  //if(Graph){ fclose(GraphFile); Graph=0; }
  CFrameWnd::OnClose();
}
