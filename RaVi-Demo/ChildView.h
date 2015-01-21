// ChildView.h : Schnittstelle der Klasse CChildView
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__4FBF6F18_E584_11D6_A03B_00104BDC6A91__INCLUDED_)
#define AFX_CHILDVIEW_H__4FBF6F18_E584_11D6_A03B_00104BDC6A91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef unsigned char byte;

/////////////////////////////////////////////////////////////////////////////
// CChildView-Fenster

class CChildView : public CWnd
{
// Konstruktion
public:
  CChildView();

// Attribute
public:
  BITMAPINFO  BMPinfo;
  byte       *BMPbits;
//byte       *DEBbits;
  int         XRes;
  int         YRes;

// Operationen
public:
  void SetupBMP();

// Überladungen
	// Vom Klassen-Assistenten erstellte virtuelle Funktionsüberladungen
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CChildView();

	// Generierte Funktionen für die Nachrichtentabellen
protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern CChildView *ViewPort;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorherigen Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_CHILDVIEW_H__4FBF6F18_E584_11D6_A03B_00104BDC6A91__INCLUDED_)

