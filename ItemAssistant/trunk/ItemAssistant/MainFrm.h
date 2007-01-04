#pragma once

#include <PluginSDK/PluginViewInterface.h>
#include "TabFrame.h"
#include <vector>
#include <shellapi.h>
#include "trayiconimpl.h"


class CMainFrame
 : public CFrameWindowImpl<CMainFrame>
 , public CUpdateUI<CMainFrame>
 , public CMessageFilter
 , public CIdleHandler
 , public CTrayIconImpl<CMainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(_T("ItemAssistantWindowClass"), IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_TRAY_SHOW, OnTrayShow)
		MSG_WM_COPYDATA(OnAOMessage)
      MSG_WM_TIMER(OnTimer)
      MSG_WM_ERASEBKGND(OnEraseBkgnd)
      MSG_WM_SYSCOMMAND(OnSysCommand)
      CHAIN_MSG_MAP(CTrayIconImpl<CMainFrame>)
      CHAIN_COMMANDS_MEMBER(m_tabbedChildWindow)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
      REFLECT_NOTIFICATIONS()
	END_MSG_MAP()


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTrayShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAOMessage(HWND wnd, PCOPYDATASTRUCT pData);
   LRESULT OnTimer(UINT wParam, TIMERPROC lParam);
   LRESULT OnEraseBkgnd(HDC dc) { return 1; }
   void OnSysCommand(UINT wParam, CPoint mousePos);

private:
   void Inject();

   //CTabbedChildWindow< CDotNetTabCtrl<CTabViewTabItem> > m_tabbedChildWindow;
   TabFrame m_tabbedChildWindow;

	CCommandBarCtrl            m_CmdBar;

   bool m_minimized;
};
