// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ScintillaCtrl.h"

class CInfView : 
	public CWindowImpl<CInfView, CScintillaCtrl>,
	public CScintillaCommands<CInfView> {
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/) override;

	BEGIN_MSG_MAP(CInfView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP_ALT(CScintillaCommands<CInfView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
