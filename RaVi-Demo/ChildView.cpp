// ChildView.cpp : Implementatierung der Klasse CChildView
//

#include "stdafx.h"
#include "RaVi-Demo.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CChildView *ViewPort;

/////////////////////////////////////////////////////////////////////////////
// CChildView

void CChildView::SetupBMP()
{
  delete[] BMPbits; BMPbits = NULL;
  //delete[] DEBbits; DEBbits = NULL;

  BMPinfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  BMPinfo.bmiHeader.biWidth         = XRes+1;
  BMPinfo.bmiHeader.biHeight        = YRes+1;
  BMPinfo.bmiHeader.biPlanes        = 1;
  BMPinfo.bmiHeader.biBitCount      = 32;
  BMPinfo.bmiHeader.biCompression   = BI_RGB;
  BMPinfo.bmiHeader.biSizeImage     = (YRes+1)*(XRes+1)*4;
  BMPinfo.bmiHeader.biXPelsPerMeter = 0;
  BMPinfo.bmiHeader.biYPelsPerMeter = 0;
  BMPinfo.bmiHeader.biClrUsed       = 0;
  BMPinfo.bmiHeader.biClrImportant  = 0;

  BMPbits = new byte[BMPinfo.bmiHeader.biSizeImage];
  //DEBbits = new byte[BMPinfo.bmiHeader.biSizeImage];
}

CChildView::CChildView(){
  ViewPort=this;
  XRes=320; YRes=240;
  memset(&BMPinfo, 1, sizeof(BITMAPINFO));
  BMPbits = NULL;
  SetupBMP();
}

CChildView::~CChildView(){
  if(BMPbits) delete BMPbits;
  //if(DEBbits) delete DEBbits;
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Nachrichtenbehandlungsroutinen von CChildView

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
  if (!CWnd::PreCreateWindow(cs))	return FALSE;
  cs.dwExStyle |= WS_EX_CLIENTEDGE;
  cs.style &= ~WS_BORDER;
  cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);
  return TRUE;
}

void CChildView::OnPaint() 
{
  CPaintDC dc(this); // Gerätekontext zum Zeichnen
  SetDIBitsToDevice(dc.m_hDC, 0, 0, XRes, YRes, 1, 1, 1, YRes+1, BMPbits, &BMPinfo, DIB_RGB_COLORS);
}

extern float C1 ,C2 ;
static float C1a,C2a;
static bool mousedown=false;
static CPoint SP;

void CChildView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  mousedown=true; SP=point; C1a=C1; C2a=C2;
  CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point) 
{
  if(mousedown){
    C1=C1a+(float)(SP.x-point.x)/(float)XRes*6.283185f;
    C2=C2a-(float)(SP.y-point.y)/(float)YRes*1.570796f; if(C2<0.0f) C2=0.0f;
  }
  CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  mousedown=false;
  CWnd::OnLButtonUp(nFlags, point);
}

// Thanks to Alek Kenton for implementing dynamic resizing
void CChildView::OnSize(UINT nType, int cx, int cy) 
{
  CWnd ::OnSize(nType, cx, cy);
  XRes = cx;
  YRes = cy;
  SetupBMP();
}
