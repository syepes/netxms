// nxav.h : main header file for the NXAV application
//

#if !defined(AFX_NXAV_H__4741A39F_BBB2_467C_984A_CF9B24661DC3__INCLUDED_)
#define AFX_NXAV_H__4741A39F_BBB2_467C_984A_CF9B24661DC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include <nms_common.h>
#include <nxclapi.h>

#include "globals.h"

/////////////////////////////////////////////////////////////////////////////
// CAlarmViewApp:
// See nxav.cpp for the implementation of this class
//

class CAlarmViewApp : public CWinApp
{
public:
	CAlarmViewApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlarmViewApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	void ErrorBox(DWORD dwError, TCHAR *pszMessage = NULL, TCHAR *pszTitle = NULL);
	//{{AFX_MSG(CAlarmViewApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	BOOL InitWorkDir(void);
};


extern CAlarmViewApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXAV_H__4741A39F_BBB2_467C_984A_CF9B24661DC3__INCLUDED_)
