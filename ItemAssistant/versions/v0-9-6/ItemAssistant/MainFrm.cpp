#include "stdafx.h"
#include "aboutdlg.h"
#include "InventoryView.h"
#include "MainFrm.h"
#include "shared/aopackets.h"
#include "InjectionSupport.h"
#include "ntray.h"
#include "Version.h"


// Delay loaded function definition
typedef  BOOL (WINAPI *ChangeWindowMessageFilterFunc)(UINT message, DWORD dwFlag);
#define MSGFLT_ADD 1
#define MSGFLT_REMOVE 2


CMainFrame::CMainFrame()
{
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
    PluginViewInterface *plugin = m_tabbedChildWindow.GetActivePluginView();
    if (plugin != NULL)
    {
        if (plugin->PreTranslateMsg(pMsg))
        {
            return TRUE;
        }
    }

    if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
        return TRUE;

    //HWND hWnd = m_tabbedChildWindow.GetActiveView();
    //if (hWnd != NULL)
    //{
    //   return (BOOL)::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM)pMsg);
    //}

    return FALSE;
}


BOOL CMainFrame::OnIdle()
{
    UIUpdateToolBar();
    return FALSE;
}


LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_minimized = false;

    // create command bar window
    HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    // attach menu
    m_CmdBar.AttachMenu(GetMenu());
    // load command bar images
    m_CmdBar.LoadImages(IDR_MAINFRAME);
    // remove old menu
    SetMenu(NULL);

    //HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

    CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
    AddSimpleReBarBand(hWndCmdBar);
    //AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
    {
        CReBarCtrl rebar(m_hWndToolBar);
        rebar.LockBands(true);
    }

    CreateSimpleStatusBar();

    DWORD style = WS_CHILD | /*WS_VISIBLE |*/ WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    m_tabbedChildWindow.SetToolBarPanel(m_hWndToolBar);
    m_tabbedChildWindow.SetStatusBar(m_hWndStatusBar);
    m_tabbedChildWindow.SetTabStyles(CTCS_TOOLTIPS | CTCS_DRAGREARRANGE);
    m_hWndClient = m_tabbedChildWindow.Create(m_hWnd, rcDefault, 0, style | WS_VISIBLE);

    //UIAddToolBar(hWndToolBar);

    m_tabbedChildWindow.SetToolbarVisibility(true);
    UISetCheck(ID_VIEW_STATUS_BAR, true);
    UISetCheck(ID_VIEW_TOOLBAR, true);
    UpdateLayout();

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    // Load a tray icon
    HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
        IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    m_trayIcon = boost::shared_ptr<CTrayNotifyIcon>(new CTrayNotifyIcon());
    m_trayIcon->Create(this, IDR_TRAY_POPUP, _T("AO Item Assistant"), hIconSmall, WM_TRAYICON);

    // To allow incoming messages from lower level applications like the AO Client we need to add the
    // message we use to the exception list.
    // http://blogs.msdn.com/vishalsi/archive/2006/11/30/what-is-user-interface-privilege-isolation-uipi-on-vista.aspx
    // It is however not available in all versions of windows, so we need to check for it existance 
    // before we try to call it.
    {
        ChangeWindowMessageFilterFunc f = (ChangeWindowMessageFilterFunc)GetProcAddress(LoadLibrary(_T("user32.dll")), "ChangeWindowMessageFilter");
        if (f) {
            (f)(WM_COPYDATA, MSGFLT_ADD);
        }
    }

    // This is the injection timer. We delay 1 sec initially so the GUI has time to be set up completely.
    SetTimer(1, 1000);

    return 0;
}


LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_trayIcon) {
        m_trayIcon->RemoveIcon();
        m_trayIcon.reset();
    }
    PostMessage(WM_CLOSE);
    return 0;
}


LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    static bool bVisible = true;	// initially state
    bVisible = !bVisible;
    m_tabbedChildWindow.SetToolbarVisibility(bVisible);
    UISetCheck(ID_VIEW_TOOLBAR, bVisible);
    UpdateLayout();
    return 0;
}


LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
    ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
    UpdateLayout();
    return 0;
}


LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    static bool isInside = false;

    if (!isInside) {
        isInside = true;
        CAboutDlg dlg;
        dlg.DoModal();
        isInside = false;
    }

    return 0;
}


LRESULT CMainFrame::OnTrayShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_minimized) {
        SendMessage(WM_SYSCOMMAND, SC_RESTORE, NULL);
    }
    else {
        SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, NULL);
    }
    return 0;
}


LRESULT CMainFrame::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SharedServices::ShowHelp(_T(""));
    return 0;
}


LRESULT CMainFrame::OnCheckForUpdates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SharedServices::OpenURL(STREAM2STR(_T("http://ia.frellu.net/?topic=checkversion&version=") << g_versionNumber));
    return 0;
}


LRESULT CMainFrame::OnSupportForum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SharedServices::OpenURL(_T("http://apps.sourceforge.net/phpbb/aoia/"));
    return 0;
}


LRESULT CMainFrame::OnAOMessage(HWND wnd, PCOPYDATASTRUCT pData)
{
    char* datablock = (char*)(pData->lpData);
    unsigned int datasize = pData->cbData;

    if (pData->dwData == 1) {
        AOMessageBase msg(datablock, datasize);
        m_tabbedChildWindow.OnAOServerMessage(msg);
    }
    else if (pData->dwData == 2) {
        AOClientMessageBase msg(datablock, datasize);
        m_tabbedChildWindow.OnAOClientMessage(msg);
    }

    return 0;
}


LRESULT CMainFrame::OnTimer(UINT wParam, TIMERPROC lParam)
{
    static bool first_injection_try = true;

    if (wParam == 1)
    {
        bool injected = Inject();
        if (injected && first_injection_try) {
            // Show warning about AO being started before AOIA
            ServicesSingleton::Instance()->ShowTrayIconBalloon(_T("Anarchy Online was started before AO Item Assistant was started.\nItem database might be out of sync."));
        }
        first_injection_try = false;
        SetTimer(1, 10000);
    }
    return 0;
}


void CMainFrame::OnSysCommand(UINT wParam, CPoint mousePos)
{
    switch (wParam) {
    case SC_CLOSE:
    case SC_MINIMIZE:
    {
        DefWindowProc(WM_SYSCOMMAND, SC_MINIMIZE, NULL);
        ShowWindow(SW_HIDE); // Hides the task bar button.
        m_minimized = true;
    }
    break;
    case SC_RESTORE:
    {
        if (m_minimized) {
            ShowWindow(SW_SHOW);
            m_minimized = false;
        }
        DefWindowProc(WM_SYSCOMMAND, wParam, NULL);
    }
    break;
    default:
        SetMsgHandled(FALSE);
        break;
    }
}


bool CMainFrame::Inject()
{
    HWND AOWnd = FindWindow( _T("Anarchy client"), _T("Anarchy Online") );

    if(!AOWnd)
    {
        Logger::instance().log(_T("Could not locate Anarchy Online window."));
        return false;
    }

    // Get process id
    DWORD AOProcessId;
    GetWindowThreadProcessId( AOWnd, &AOProcessId );

    TCHAR CurrDir[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, CurrDir );

    // Inject the dll into client process
    std::tstringstream temp;
    temp << CurrDir << _T("\\ItemAssistantHook.dll");
    return InjectDLL(AOProcessId, temp.str());
}


LRESULT CMainFrame::OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    if (m_trayIcon) {
        return m_trayIcon->OnTrayNotification(wParam, lParam);
    }
    return 0;
}
