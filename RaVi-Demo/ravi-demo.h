// RaVi.h : Haupt-Header-Datei für die Anwendung RAVI
//

#if !defined(AFX_RAVI_H__4FBF6F12_E584_11D6_A03B_00104BDC6A91__INCLUDED_)
#define AFX_RAVI_H__4FBF6F12_E584_11D6_A03B_00104BDC6A91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// cRaViApp:
// Siehe RaVi.cpp für die Implementierung dieser Klasse
//

class cRaViApp : public CWinApp
{
public:
	cRaViApp();

// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(cRaViApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementierung

public:
	//{{AFX_MSG(cRaViApp)
	afx_msg void OnAppAbout();
	afx_msg void OnUpdateAnimation(CCmdUI* pCmdUI);
	afx_msg void OnAnimationToggle();
	afx_msg void OnQuality_1by1();
	afx_msg void OnUpdateQuality_1by1(CCmdUI* pCmdUI);
	afx_msg void OnQualitySubsampling();
	afx_msg void OnDemoCube();
	afx_msg void OnMirrorSphere();
	afx_msg void OnUpdateDemoMirrSph(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDemoCube(CCmdUI* pCmdUI);
	afx_msg void OnDemo3Spheres();
	afx_msg void OnUpdate3Spheres(CCmdUI* pCmdUI);
	afx_msg void OnDemoShadows();
	afx_msg void OnUpdateShadows(CCmdUI* pCmdUI);
	afx_msg void OnShaderNormals();
	afx_msg void OnShaderPhong();
	afx_msg void OnShaderReflection();
	afx_msg void OnUpdateShaderNormals(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShaderPhong(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShaderReflection(CCmdUI* pCmdUI);
	afx_msg void OnShaderDepth();
	afx_msg void OnUpdateShaderDepth(CCmdUI* pCmdUI);
	afx_msg void OnFilter();
	afx_msg void OnUpdateFilter(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQualitySub(CCmdUI* pCmdUI);
	afx_msg void OnDemoTorus();
	afx_msg void OnUpdateDemoTorus(CCmdUI* pCmdUI);
	afx_msg void OnShaderUV();
	afx_msg void OnUpdateShaderUV(CCmdUI* pCmdUI);
	afx_msg void OnShaderSimple();
	afx_msg void OnUpdateShaderSimple(CCmdUI* pCmdUI);
	afx_msg void OnDemoSphereHeap();
	afx_msg void OnUpdateDemoSphereHeap(CCmdUI* pCmdUI);
	afx_msg void OnDemoAreaLight();
	afx_msg void OnUpdateDemoAreaLight(CCmdUI* pCmdUI);
	afx_msg void OnRes480();
	afx_msg void OnRes320();
	afx_msg void OnRes512();
	afx_msg void OnRes640();
	afx_msg void OnRes800();
	afx_msg void OnSingleThread();
	afx_msg void OnDualThread();
	afx_msg void OnQuadThread();
	afx_msg void OnUpdateSingleThread(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDualThread(CCmdUI* pCmdUI);
	afx_msg void OnSampInt16();
	afx_msg void OnSampInt4();
	afx_msg void OnSampRaw16();
	afx_msg void OnSampStd();
	afx_msg void OnSampFull();
	afx_msg void OnQualityAd20();
	afx_msg void OnQualityAd10();
	afx_msg void OnQualityAd30();
	afx_msg void OnQualityAd25();
	afx_msg void OnQualityAd15();
	afx_msg void OnQualityAd05();
	afx_msg void OnUpdateQualityAd20(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQualityAd10(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQualityAd15(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQualityAd25(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQualityAd30(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQualityAd05(CCmdUI* pCmdUI);
	afx_msg void OnAA_None();
	afx_msg void OnAA_Good();
	afx_msg void OnAA_Best();
	afx_msg void OnUpdateAA_None(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAA_Good(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAA_Best(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadThread(CCmdUI* pCmdUI);
	afx_msg void OnDemoEgg();
	afx_msg void OnUpdateDemoEgg(CCmdUI* pCmdUI);
	afx_msg void OnCaustics();
	afx_msg void OnUpdateCaustics(CCmdUI* pCmdUI);
	afx_msg void OnCaInNone();
	afx_msg void OnCaInKernel();
	afx_msg void OnCaInOther();
	afx_msg void OnUpdateCaInNone(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCaInKernel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCaInOther(CCmdUI* pCmdUI);
	afx_msg void OnCalnOther2();
	afx_msg void OnUpdateCalnOther2(CCmdUI* pCmdUI);
	afx_msg void OnCaSaCoarse();
	afx_msg void OnCaSaHig();
	afx_msg void OnCaSaMed();
	afx_msg void OnUpdateCaSaCoarse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCaSaHig(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCaSaMed(CCmdUI* pCmdUI);
	afx_msg void OnCaQu();
	afx_msg void OnUpdateCaQu(CCmdUI* pCmdUI);
	afx_msg void OnCaQuNoisy();
	afx_msg void OnUpdateCaQuNoisy(CCmdUI* pCmdUI);
	afx_msg void OnCaQuSmooth();
	afx_msg void OnUpdateCaQuSmooth(CCmdUI* pCmdUI);
	afx_msg void OnCaDisp();
	afx_msg void OnUpdateCaDisp(CCmdUI* pCmdUI);
	afx_msg void OnTestAsm();
	afx_msg void OnUpdateTestAsm(CCmdUI* pCmdUI);
	afx_msg void OnDemoCornell();
	afx_msg void OnUpdateDemoCornell(CCmdUI* pCmdUI);
	afx_msg void OnGISaMed();
	afx_msg void OnUpdateGISaMed(CCmdUI* pCmdUI);
	afx_msg void OnGISaFine();
	afx_msg void OnUpdateGISaFine(CCmdUI* pCmdUI);
	afx_msg void OnGISaCoarse();
	afx_msg void OnUpdateGISaCoarse(CCmdUI* pCmdUI);
	afx_msg void OnGIQuMed();
	afx_msg void OnUpdateGIQuMed(CCmdUI* pCmdUI);
	afx_msg void OnGIQuNoisy();
	afx_msg void OnUpdateGIQuNoisy(CCmdUI* pCmdUI);
	afx_msg void OnGIQuSmooth();
	afx_msg void OnUpdateGIQuSmooth(CCmdUI* pCmdUI);
	afx_msg void OnGICalc();
	afx_msg void OnUpdateGICalc(CCmdUI* pCmdUI);
	afx_msg void OnGIDisp();
	afx_msg void OnUpdateGIDisp(CCmdUI* pCmdUI);
	afx_msg void OnCalnDiff();
	afx_msg void OnUpdateCalnDiff(CCmdUI* pCmdUI);
	afx_msg void OnCalnStack();
	afx_msg void OnUpdateCalnStack(CCmdUI* pCmdUI);
	afx_msg void OnCalnPreBiPatch();
	afx_msg void OnUpdateCalnPreBiPatch(CCmdUI* pCmdUI);
	afx_msg void OnGIOnlyIndirect();
	afx_msg void OnUpdateGIOnlyIndirect(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_RAVI_H__4FBF6F12_E584_11D6_A03B_00104BDC6A91__INCLUDED_)
