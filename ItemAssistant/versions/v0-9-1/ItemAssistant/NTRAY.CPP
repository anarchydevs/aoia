/*
Module : NTray.CPP
Purpose: implementation for a MFC class to encapsulate Shell_NotifyIcon
Created: PJN / 14-05-1997
History: PJN / 25-11-1997 Addition of the following
                          1. HideIcon(), ShowIcon() & MoveToExtremeRight() 
                          2. Support for animated tray icons
         PJN / 23-06-1998 Class now supports the new Taskbar Creation Notification 
                          message which comes with IE 4. This allows the tray icon
                          to be recreated whenever the explorer restarts (Crashes!!)
         PJN / 22-07-1998 1. Code now compiles cleanly at warning level 4
                          2. Code is now UNICODE enabled + build configurations are provided
                          3. The documentation for the class has been updated
         PJN / 27-01-1999 1. Code first tries to load a 16*16 icon before loading the 32*32
                          version. This removes the blurryness which was previously occuring
         PJN / 28-01-1999 1. Fixed a number of level 4 warnings which were occurring.
         PJN / 09-05-1999 1. Fixed a problem as documented in KB article "PRB: Menus for 
                          Notification Icons Do Not Work Correctly", Article ID: Q135788 
         PJN / 15-05-1999 1. Now uses the author's hookwnd class. This prevents the need to 
                          create the two hidden windows namely CTrayRessurectionWnd and
                          CTrayTimerWnd
                          2. Code now compiles cleanly at warning level 4
                          3. General code tidy up and rearrangement
                          4. Added numerous ATLASSERT's to improve the code's robustness
                          5. Added functions to allow context menu to be customized
         PJN / 01-01-2001 1. Now includes copyright message in the source code and documentation. 
                          2. Fixed problem where the window does not get focus after double clicking 
                          on the tray icon
                          3. Now fully supports the Windows 2000 balloon style tooltips
                          4. Fixed a off by one problem in some of the ATLASSERT's
                          5. Fixed problems with Unicode build configurations
                          6. Provided Win2k specific build configurations
         PJN / 10-02-2001 1. Now fully supports creation of 2 tray icons at the same time
         PJN / 13-06-2001 1. Now removes windows hook upon call to RemoveIcon
         PJN / 26-08-2001 1. Fixed memory leak in RemoveIcon.
                          2. Fixed GPF in RemoveIcon by removing call to Unhook
         PJN / 28-08-2001 1. Added support for direct access to the System Tray's HDC. This allows
                          you to generate an icon for the tray on the fly using GDI calls. The idea
                          came from the article by Jeff Heaton in the April issue of WDJ. Also added
                          are overriden Create methods to allow you to easily costruct a dynamic
                          tray icon given a BITMAP instead of an ICON.
         PJN / 21-03-2003 1. Fixed icon resource leaks in SetIcon(LPCTSTR lpIconName) and 
                          SetIcon(UINT nIDResource). Thanks to Egor Pervouninski for reporting this.
                          2. Fixed unhooking of the tray icon when the notification window is being
                          closed.
         PJN / 31-03-2003 1. Now uses V1.05 of my Hookwnd class
         PJN / 02-04-2003 1. Now uses v1.06 of my Hookwnd class
                          2. Fixed a bug in the sample app for this class where the hooks should
                          have been created as global instances rather than as member variables of
                          the mainframe window. This ensures that the hooks remain valid even after
                          calling DefWindowProc on the mainframe.
         PJN / 23-07-2004 1. Minor update to remove unnecessary include of "resource.h"
         PJN / 03-03-2006 1. Updated copyright details.
                          2. Updated the documentation to use the same style as the web site.
                          3. Did a spell check of the documentation.        
                          4. Fixed some issues when the code is compiled using /Wp64. Please note that
                          to support this the code now requires a recentish Platform SDK to be installed
                          if the code is compiled with Visual C++ 6.
                          5. Replaced all calls to ZeroMemory with memset.
                          6. Fixed an issue where SetBalloonDetails was not setting the cbSize parameter.
                          Thanks to Enrique Granda for reporting this issue.
                          7. Added support for NIIF_USER and NIIF_NONE flags.
                          8. Now includes support for NIM_NIMSETVERSION via SetVersion. In addition this
                          is now automatically set in the Create() calls if the Win2k boolean parameter
                          is set.
                          9. Removed derivation from CObject as it was not really needed.
                          10. Now includes support for NIM_SETFOCUS
                          11. Added support for NIS_HIDDEN via the ShowIcon and HideIcon methods.
                          12. Added support for NIIF_NOSOUND
         PJN / 27-06-2006 1. Code now uses new C++ style casts rather than old style C casts where necessary.
                          2. The class framework now requires the Platform SDK if compiled using VC 6.  
                          3. Updated the logic of the ASSERTs which validate the various string lengths.
                          4. Fixed a bug in CTrayNotifyIcon::SetFocus() where the cbSize value was not being
                          set correctly.
                          5. CTrayIconHooker class now uses ATL's CWindowImpl class in preference to the author's
                          CHookWnd class. This does mean that for MFC only client projects, you will need to add
                          ATL support to your project.
                          6. Optimized CTrayIconHooker constructor code
                          7. Updated code to compile cleanly using VC 2005. Thanks to "Itamar" for prompting this
                          update.
                          8. Addition of a CTRAYNOTIFYICON_EXT_CLASS and CTRAYNOTIFYICON_EXT_API macros which makes 
                          the class easier to use in an extension dll.
                          9. Made CTrayNotifyIcon destructor virtual
         PJN / 03-07-2005 1. Fixed a bug where the HideIcon functionality did not work on Windows 2000. This was 
                          related to how the cbSize member of the NOTIFYICONDATA structure was initialized. The code
                          now dynamically determines the correct size to set at runtime according to the instructions
                          provided by the MSDN documentation for this structure. As a result of this, all "bWin2k" 
                          parameters which were previously exposed via CTrayNotifyIcon have now been removed as there
                          is no need for them. Thanks to Edwin Geng for reporting this important bug. Client code will
                          still need to intelligently make decisions on what is supported by the OS. For example balloon
                          tray icons are only supported on Shell v5 (nominally Windows 2000 or later). CTrayNotifyIcon
                          will ATLASSERT if for example calls are made to it to create a balloon tray icon on operating 
                          systems < Windows 2000.
         PJN / 04-07-2006 1. Fixed a bug where the menu may pop up a second time after a menu item is chosen on 
                          Windows 2000. The problem was tracked down to the code in CTrayNotifyIcon::OnTrayNotification. 
                          During testing of this bug, I was unable to get a workable solution using the new shell 
                          messages of WM_CONTEXTMENU, NIN_KEYSELECT & NIN_SELECT on Windows 2000 and Windows XP. 
                          This means that the code in CTrayNotifyIcon::OnTrayNotification uses the old way of handling 
                          notifications (WM_RBUTTDOWN*). This does mean that by default, client apps which use the 
                          CTrayNotifyIcon class will not support the new keyboard and mouse semantics for tray icons
                          (IMHO this is no big loss!). Client code is of course free to handle their own notifications. 
                          If you go down this route then I would advise you to thoroughly test your application on 
                          Windows 2000 and Windows XP as my testing has shown that there is significant differences in 
                          how tray icons handle their messaging on these 2 operating systems. Thanks to Edwin Geng for 
                          reporting this issue.
                          2. Class now displays the menu based on the current message's screen coordinates, rather than
                          the current cursor screen coordinates.
                          3. Fixed bug in sample app where if the about dialog is already up and it is reactivated 
                          from the tray menu, it did not bring itself into the foreground
         PJN / 06-07-2006 1. Reverted the change made for v1.53 where the screen coordinates used to show the context 
                          menu use the current message's screen coordinates. Instead the pre v1.53 mechanism which 
                          uses the current cursor's screen coordinates is now used. Thanks to Itamar Syn-Hershko for 
                          reporting this issue.
         PJN / 19-07-2006 1. The default menu item can now be customized via SetDefaultMenuItem and 
                          GetDefaultMenuItem. Thanks to Mikhail Bykanov for suggesting this nice update.
                          2. Optimized CTrayNotifyIcon constructor code

Copyright (c) 1997 - 2006 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/

/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "NTray.h"


/////////////////////////////////  Macros /////////////////////////////////////


//Note this define is necessary because even though the code requires the Platform SDK. This is because the last 
//version of the Platform SDK which was compatible with VC 6 was February 2003 and it does not define this value. 
//This is because NIIF_USER is only supported on XP SP2 or later which was in the XP SP 2 Platform SDK which 
//was released much later.
#ifndef NIIF_USER
#define NIIF_USER 0x00000004
#endif

DWORD CTrayNotifyIcon::sm_dwShellVersion = 0;


///////////////////////////////// Implementation //////////////////////////////

const UINT wm_TaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

CTrayIconHooker::CTrayIconHooker() : m_pTrayIcon(NULL),
                                     m_phIcons(NULL),
                                     m_nNumIcons(0),
                                     m_nTimerID(0),
                                     m_nCurrentIconIndex(0)
{
}

CTrayIconHooker::~CTrayIconHooker()
{
  StopUsingAnimation();
}

BOOL CTrayIconHooker::Init(CTrayNotifyIcon* pTrayIcon, CWindow* pNotifyWnd)
{
  //Validate our parameters
  ATLASSERT(pTrayIcon); //must have a valid tray notify instance
  ATLASSERT(pNotifyWnd && pNotifyWnd->IsWindow());

  //Hive away the input parameter
  m_pTrayIcon = pTrayIcon;

  //Hook the top level frame of the notify window in preference 
  //to the notify window itself. This will ensure that we get
  //the taskbar created message
  CWindow TopLevelWnd = pNotifyWnd->GetTopLevelWindow();
  if (TopLevelWnd.IsWindow())
    return SubclassWindow(TopLevelWnd.m_hWnd); 
  else
    return SubclassWindow(pNotifyWnd->m_hWnd);
}

void CTrayIconHooker::StartUsingAnimation(HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
  //Validate our parameters
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(phIcons);        //array of icon handles must be valid
  ATLASSERT(dwDelay);        //must be non zero timer interval
  ATLASSERT(m_pTrayIcon);

  //Stop the animation if already started  
  StopUsingAnimation();

  //Hive away all the values locally
  ATLASSERT(m_phIcons == NULL);
  m_phIcons = new HICON[nNumIcons];
  for (int i=0; i<nNumIcons; i++)
    m_phIcons[i] = phIcons[i];
  m_nNumIcons = nNumIcons;

  //Start up the timer 
  m_nTimerID = SetTimer(m_pTrayIcon->m_NotifyIconData.uID, dwDelay);
}

void CTrayIconHooker::StopUsingAnimation()
{
  //Kill the timer
  if (m_nTimerID)
  {
    if (::IsWindow(m_hWnd))
      KillTimer(m_nTimerID);
    m_nTimerID = 0;
  }
 
  //Free up the memory
  if (m_phIcons)
  {
    delete [] m_phIcons;
    m_phIcons = NULL;
  }

  //Reset the other animation related variables
  m_nCurrentIconIndex = 0;
  m_nNumIcons = 0;
}

BOOL CTrayIconHooker::UsingAnimatedIcon() const
{
  return (m_nNumIcons != 0);
}

HICON CTrayIconHooker::GetCurrentIcon() const 
{ 
  ATLASSERT(UsingAnimatedIcon());
  ATLASSERT(m_phIcons);
  return m_phIcons[m_nCurrentIconIndex]; 
}

BOOL CTrayIconHooker::ProcessWindowMessage(HWND /*hWnd*/, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD /*dwMsgMapID*/)
{
  //Validate our parameters
  ATLASSERT(m_pTrayIcon);

  lResult = 0;
  BOOL bHandled = FALSE;

  if (nMsg == wm_TaskbarCreated)
  {
    lResult = OnTaskbarCreated(wParam, lParam);
    bHandled = TRUE;
  }
  else if ((nMsg == WM_TIMER) && (wParam == m_pTrayIcon->m_NotifyIconData.uID))
  {
    OnTimer(m_pTrayIcon->m_NotifyIconData.uID); 
    bHandled = TRUE;
  }
  else if (nMsg == WM_DESTROY)
  {
    m_pTrayIcon->RemoveIcon();
    bHandled = TRUE;
  }

  return bHandled;
}

#ifdef _DEBUG
void CTrayIconHooker::OnTimer(UINT_PTR nIDEvent)
#else
void CTrayIconHooker::OnTimer(UINT_PTR /*nIDEvent*/)  //Just to avoid a compiler warning 
#endif                                                //when being built for release
{
  ATLASSERT(nIDEvent == m_nTimerID);

  //increment the icon index
  ++m_nCurrentIconIndex;
  m_nCurrentIconIndex = m_nCurrentIconIndex % m_nNumIcons;

  //update the tray icon
  m_pTrayIcon->m_NotifyIconData.uFlags = NIF_ICON;
  m_pTrayIcon->m_NotifyIconData.hIcon = m_phIcons[m_nCurrentIconIndex];
  Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_pTrayIcon->m_NotifyIconData));
}

LRESULT CTrayIconHooker::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  ATLASSERT(m_pTrayIcon);

  //Refresh the tray icon if necessary
  if (m_pTrayIcon->IsShowing())
  {
    m_pTrayIcon->HideIcon();
    m_pTrayIcon->ShowIcon();
  }

  return 0L;
}


CTrayNotifyIcon::CTrayNotifyIcon() : m_bCreated(FALSE),
                                     m_bHidden(FALSE),
                                     m_pNotificationWnd(NULL),
                                     m_bDefaultMenuItemByPos(TRUE),
                                     m_nDefaultMenuItem(0)
{
  memset(&m_NotifyIconData, 0, sizeof(m_NotifyIconData));
  m_NotifyIconData.cbSize = GetNOTIFYICONDATASizeForOS();
}

CTrayNotifyIcon::~CTrayNotifyIcon()
{
  RemoveIcon();
}

void CTrayNotifyIcon::HideIcon()
{
  ATLASSERT(m_bCreated);
	if (!m_bHidden) 
  {
    if (GetShellVersion() >= 5) //use the Shell v5 way of hiding the icon
    {
#if (_WIN32_IE >= 0x0500)
	  m_NotifyIconData.uFlags = NIF_STATE;
      m_NotifyIconData.dwState = NIS_HIDDEN;
	  m_NotifyIconData.dwStateMask = NIS_HIDDEN; 
#endif
	  Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
    }
    else
    {
		  m_NotifyIconData.uFlags = 0;
		  Shell_NotifyIcon(NIM_DELETE, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
    }
		m_bHidden = TRUE;
	}
}

void CTrayNotifyIcon::ShowIcon()
{
  ATLASSERT(m_bCreated);
	if (m_bHidden) 
  {
    if (GetShellVersion() >= 5) //use the Shell v5 way of showing the icon
    {
#if (_WIN32_IE >= 0x0500)
	  m_NotifyIconData.uFlags = NIF_STATE;
      m_NotifyIconData.dwState = 0;
      m_NotifyIconData.dwStateMask = NIS_HIDDEN;
	  Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
#endif
    }
    else
    {
		  m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		  Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
    }
		m_bHidden = FALSE;
	}
}

void CTrayNotifyIcon::RemoveIcon()
{
  if (m_bCreated)
  {
    m_NotifyIconData.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));

    m_HookWnd.StopUsingAnimation();
    if (m_HookWnd.m_hWnd)
      m_HookWnd.UnsubclassWindow();
    m_bCreated = FALSE;
  }
}

void CTrayNotifyIcon::SetMenu(HMENU hMenu)
{
  //Validate our parameters
  ATLASSERT(hMenu);

  m_Menu.DestroyMenu();
  m_Menu.Attach(hMenu);

  CMenuHandle SubMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(SubMenu.IsMenu());

  //Make the specified menu item the default (bold font)
  SubMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos); // SAW
}

CMenu& CTrayNotifyIcon::GetMenu()
{
  return m_Menu;
}

void CTrayNotifyIcon::SetDefaultMenuItem(UINT uItem, BOOL fByPos) 
{ 
  m_nDefaultMenuItem = uItem; 
  m_bDefaultMenuItemByPos = fByPos; 

  //Also update in the live menu if it is present
  if (m_Menu.IsMenu())
  {
    CMenuHandle  pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu.IsMenu()) 
      ATLASSERT(FALSE); //Your menu resource has been designed incorrectly

    pSubMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
  }
}

BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage, UINT uMenuID)
{
  //Validate our parameters
  ATLASSERT(pNotifyWnd && pNotifyWnd->IsWindow());
#ifdef _DEBUG  
  if (GetShellVersion() >= 5) //If on Shell v5 or higher, then use the larger size tooltip
  {
    NOTIFYICONDATA_2 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
  else
  {
    NOTIFYICONDATA_1 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
#endif
  ATLASSERT(hIcon); 
  ATLASSERT(nNotifyMessage >= WM_USER); //Make sure we avoid conflict with other messages

  //Load up the menu resource which is to be used as the context menu
  if (!m_Menu.LoadMenu(uMenuID == 0 ? uID : uMenuID))
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
  CMenuHandle pSubMenu = m_Menu.GetSubMenu(0);
  if (!pSubMenu.IsMenu()) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  pSubMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);

  //Install the hook
  if (!m_HookWnd.Init(this, pNotifyWnd))
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  m_NotifyIconData.hWnd = pNotifyWnd->m_hWnd;
  m_NotifyIconData.uID = uID;
  m_NotifyIconData.uCallbackMessage = nNotifyMessage;
  m_NotifyIconData.hIcon = hIcon;
#if (_MSC_VER >= 1400)
  _tcscpy_s(m_NotifyIconData.szTip, sizeof(m_NotifyIconData.szTip)/sizeof(TCHAR), pszTooltipText);
#else  
  _tcscpy(m_NotifyIconData.szTip, pszTooltipText);
#endif  
  m_bCreated = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));

  //Turn on Shell v5 style behaviour if supported
#if (_WIN32_IE >= 0x0500)
  if (GetShellVersion() >= 5)
	  SetVersion(NOTIFYICON_VERSION);
#endif

  return m_bCreated;
}

BOOL CTrayNotifyIcon::SetVersion(UINT uVersion)
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uVersion = uVersion;
#if (_WIN32_IE >= 0x0500)
  return Shell_NotifyIcon(NIM_SETVERSION, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
#else
  return FALSE;
#endif
}

HICON CTrayNotifyIcon::BitmapToIcon(CBitmap* pBitmap)
{
  //Validate our parameters
  ATLASSERT(pBitmap);

  //Get the width and height of a small icon
  int w = GetSystemMetrics(SM_CXSMICON);
  int h = GetSystemMetrics(SM_CYSMICON);

  //Create a 0 mask
  int nMaskSize = h*(w/8);
  unsigned char* pMask = new unsigned char[nMaskSize];
  ZeroMemory(pMask, nMaskSize);

  //Create a mask bitmap
  CBitmap maskBitmap;
  maskBitmap.CreateBitmap(w, h, 1, 1, pMask);
  BOOL bSuccess = !maskBitmap.IsNull();

  //Free up the heap memory now that we have created the mask bitmap
  delete [] pMask;

  //Handle the error
  if (!bSuccess)
    return NULL;

  //Create an ICON base on the bitmap just created
  ICONINFO iconInfo;
  iconInfo.fIcon = TRUE;
  iconInfo.xHotspot = 0;
  iconInfo.yHotspot = 0;
  iconInfo.hbmMask = maskBitmap;
  iconInfo.hbmColor = *pBitmap; 
  return CreateIconIndirect(&iconInfo); 
}

BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID)
{
  //Convert the bitmap to an ICON
  HICON hIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, hIcon, nNotifyMessage, uMenuID);
      
  //Free up all the resources now that we are finished with them
  DestroyIcon(hIcon);

  return bSuccess;
}

BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID)
{
  //Validate our parameters
  ATLASSERT(phIcons);
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(dwDelay);

  //let the normal Create function do its stuff
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, phIcons[0], nNotifyMessage, uMenuID);

  //tell the hook class to do the animation
  m_HookWnd.StartUsingAnimation(phIcons, nNumIcons, dwDelay);

  return bSuccess;
}

BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON hIcon, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
{
  //Validate our parameters
  ATLASSERT(pNotifyWnd && pNotifyWnd->IsWindow());
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later
#ifdef _DEBUG
  NOTIFYICONDATA_2 dummy;
  DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
  ATLASSERT(_tcslen(pszBalloonText) < sizeof(dummy.szInfo)/sizeof(TCHAR));
  ATLASSERT(_tcslen(pszBalloonCaption) < sizeof(dummy.szInfoTitle)/sizeof(TCHAR));
  ATLASSERT(hIcon); 
  ATLASSERT(nNotifyMessage >= WM_USER); //Make sure we avoid conflict with other messages
#endif

  //Load up the menu resource which is to be used as the context menu
  if (!m_Menu.LoadMenu(uMenuID == 0 ? uID : uMenuID))
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
  CMenuHandle pSubMenu = m_Menu.GetSubMenu(0);
  if (!pSubMenu.IsMenu()) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  pSubMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);

  //Install the hook
  if (!m_HookWnd.Init(this, pNotifyWnd))
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.hWnd = pNotifyWnd->m_hWnd;
  m_NotifyIconData.uID = uID;
#if (_WIN32_IE >= 0x0500)
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
#else
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
#endif
  m_NotifyIconData.uCallbackMessage = nNotifyMessage;
  m_NotifyIconData.hIcon = hIcon;
#if (_MSC_VER >= 1400)
  _tcscpy_s(m_NotifyIconData.szTip, sizeof(m_NotifyIconData.szTip)/sizeof(TCHAR), pszTooltipText);
  _tcscpy_s(m_NotifyIconData.szInfo, sizeof(m_NotifyIconData.szInfo)/sizeof(TCHAR), pszBalloonText);
  _tcscpy_s(m_NotifyIconData.szInfoTitle, sizeof(m_NotifyIconData.szInfoTitle)/sizeof(TCHAR), pszBalloonCaption);
#else
  _tcscpy(m_NotifyIconData.szTip, pszTooltipText);
  _tcscpy(m_NotifyIconData.szInfo, pszBalloonText);
  _tcscpy(m_NotifyIconData.szInfoTitle, pszBalloonCaption);
#endif  
  m_NotifyIconData.uTimeout = nTimeout;
#if (_WIN32_IE >= 0x0500)
  switch (style)
  {
    case Warning:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_WARNING;
      break;
    }
    case Error:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_ERROR;
      break;
    }
    case Info:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_INFO;
      break;
    }
    case None:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_NONE;
      break;
    }
    case User:
    {
      ATLASSERT(hIcon != NULL); //You forget to provide a user icon
      m_NotifyIconData.dwInfoFlags = NIIF_USER;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
#endif
#if (_WIN32_IE >= 0x0501)
  if (bNoSound)
    m_NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;
#endif
  m_bCreated = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));

  //Turn on Shell v5 tray icon behaviour
#if (_WIN32_IE >= 0x0500)
  SetVersion(NOTIFYICON_VERSION);
#endif
  return m_bCreated;
}

BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
{
  //Convert the bitmap to an ICON
  HICON hIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, pszBalloonText, pszBalloonCaption, nTimeout, style, hIcon, nNotifyMessage, uMenuID, bNoSound);
      
  //Free up all the resources now that we are finished with them
  DestroyIcon(hIcon);

  return bSuccess;
}

BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
{
  //Validate our parameters
  ATLASSERT(phIcons);
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(dwDelay);

  //let the normal Create function do its stuff
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, pszBalloonText, pszBalloonCaption, nTimeout, style, phIcons[0], nNotifyMessage, uMenuID, bNoSound);

  //tell the hook class to do the animation
  m_HookWnd.StartUsingAnimation(phIcons, nNumIcons, dwDelay);

  return bSuccess;
}

BOOL CTrayNotifyIcon::SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style, UINT nTimeout, HICON hUserIcon, BOOL bNoSound)
{
  if (!m_bCreated)
    return FALSE;

  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later
#ifdef _DEBUG
  NOTIFYICONDATA_2 dummy;
  DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  ATLASSERT(_tcslen(pszBalloonText) < sizeof(dummy.szInfo)/sizeof(TCHAR));
  ATLASSERT(_tcslen(pszBalloonCaption) < sizeof(dummy.szInfoTitle)/sizeof(TCHAR));
#endif

  //Call the Shell_NotifyIcon function
#if (_WIN32_IE >= 0x0500)
  m_NotifyIconData.uFlags = NIF_INFO;
#endif
#if (_MSC_VER >= 1400)
  _tcscpy_s(m_NotifyIconData.szInfo, sizeof(m_NotifyIconData.szInfo)/sizeof(TCHAR), pszBalloonText);
  _tcscpy_s(m_NotifyIconData.szInfoTitle, sizeof(m_NotifyIconData.szInfoTitle)/sizeof(TCHAR), pszBalloonCaption);
#else  
  _tcscpy(m_NotifyIconData.szInfo, pszBalloonText);
  _tcscpy(m_NotifyIconData.szInfoTitle, pszBalloonCaption);
#endif  
  m_NotifyIconData.uTimeout = nTimeout;
#if (_WIN32_IE >= 0x0500)
  switch (style)
  {
    case Warning:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_WARNING;
      break;
    }
    case Error:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_ERROR;
      break;
    }
    case Info:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_INFO;
      break;
    }
    case None:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_NONE;
      break;
    }
    case User:
    {
      ATLASSERT(hUserIcon != NULL); //You forget to provide a user icon
      m_NotifyIconData.dwInfoFlags = NIIF_USER;
      m_NotifyIconData.hIcon = hUserIcon;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
#endif
#if (_WIN32_IE >= 0x0501)
  if (bNoSound)
    m_NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;
#endif
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

CString CTrayNotifyIcon::GetBalloonText() const
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  CString sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szInfo;

  return sText;
}

CString CTrayNotifyIcon::GetBalloonCaption() const
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  CString sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szInfoTitle;

  return sText;
}

UINT CTrayNotifyIcon::GetBalloonTimeout() const
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  UINT nTimeout = 0;
  if (m_bCreated)
    nTimeout = m_NotifyIconData.uTimeout;

  return nTimeout;
}

BOOL CTrayNotifyIcon::SetTooltipText(LPCTSTR pszTooltipText)
{
  if (!m_bCreated)
    return FALSE;

  if (GetShellVersion() >= 5) //Allow the larger size tooltip text if on Shell v5 or later
  {
  #ifdef _DEBUG
    NOTIFYICONDATA_2 dummy;
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
  #endif
  }
  else 
  {
  #ifdef _DEBUG
    NOTIFYICONDATA_1 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  #endif
  }

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_TIP;
#if (_MSC_VER >= 1400)  
  _tcscpy_s(m_NotifyIconData.szTip, sizeof(m_NotifyIconData.szTip)/sizeof(TCHAR), pszTooltipText);
#else
  _tcscpy(m_NotifyIconData.szTip, pszTooltipText);
#endif  
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::SetTooltipText(UINT nID)
{
  CString sToolTipText;
  ATLVERIFY(sToolTipText.LoadString(nID));

  //Let the other version of the function handle the rest
  return SetTooltipText(sToolTipText);
}

BOOL CTrayNotifyIcon::SetIcon(CBitmap* pBitmap)
{
  //Convert the bitmap to an ICON
  HICON hIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  BOOL bSuccess = SetIcon(hIcon);

  //Destroy the icon now that we are finished with it      
  DestroyIcon(hIcon);

  return bSuccess;
}

BOOL CTrayNotifyIcon::SetIcon(HICON hIcon)
{
  //Validate our parameters
  ATLASSERT(hIcon);

  if (!m_bCreated)
    return FALSE;

  //Since we are going to use one icon, stop any animation
  m_HookWnd.StopUsingAnimation();

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_ICON;
  m_NotifyIconData.hIcon = hIcon;
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::SetIcon(LPCTSTR lpIconName)
{
  HICON hIcon = LoadIconResource(lpIconName);
  BOOL bSuccess = SetIcon(hIcon);
  DestroyIcon(hIcon);
  return bSuccess;
}

BOOL CTrayNotifyIcon::SetIcon(UINT nIDResource)
{
  HICON hIcon = LoadIconResource(nIDResource);
  BOOL bSuccess = SetIcon(hIcon);
  DestroyIcon(hIcon);
  return bSuccess;
}

BOOL CTrayNotifyIcon::SetStandardIcon(LPCTSTR lpIconName)
{
  HICON hIcon = ::LoadIcon(NULL, lpIconName);
  return SetIcon(hIcon);
}

BOOL CTrayNotifyIcon::SetStandardIcon(UINT nIDResource)
{
  HICON hIcon = ::LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));
  return SetIcon(hIcon);
}

BOOL CTrayNotifyIcon::SetIcon(HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
  //Validate our parameters
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(phIcons);
  ATLASSERT(dwDelay);

  if (!SetIcon(phIcons[0]))
    return FALSE;

  //Install the hook
  m_HookWnd.StartUsingAnimation(phIcons, nNumIcons, dwDelay);

  return TRUE;
}

BOOL CTrayNotifyIcon::SetNotificationWnd(CWindow* pNotifyWnd)
{
  //Validate our parameters
  ATLASSERT(pNotifyWnd && pNotifyWnd->IsWindow());

  if (!m_bCreated)
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.hWnd = pNotifyWnd->m_hWnd;
  m_NotifyIconData.uFlags = 0;
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

CString CTrayNotifyIcon::GetTooltipText() const
{
  CString sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szTip;

  return sText;
}

HICON CTrayNotifyIcon::GetIcon() const
{
  HICON hIcon = NULL;
  if (m_bCreated)
  {
    if (UsingAnimatedIcon())
      hIcon = m_HookWnd.GetCurrentIcon();
    else
      hIcon = m_NotifyIconData.hIcon;
  }

  return hIcon;
}

BOOL CTrayNotifyIcon::UsingAnimatedIcon() const
{
  return m_HookWnd.UsingAnimatedIcon();
}

CWindow* CTrayNotifyIcon::GetNotificationWnd() const
{
  return m_pNotificationWnd;
}

BOOL CTrayNotifyIcon::SetFocus()
{
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or greater

  //Call the Shell_NotifyIcon function
#if (_WIN32_IE >= 0x0500)
  return Shell_NotifyIcon(NIM_SETFOCUS, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
#else
  return FALSE;
#endif
}

LRESULT CTrayNotifyIcon::OnTrayNotification(WPARAM wID, LPARAM lEvent)
{
  //Return quickly if its not for this tray icon
  if (wID != m_NotifyIconData.uID)
    return 0L;

  CMenuHandle pSubMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(pSubMenu.IsMenu()); //Your menu resource has been designed incorrectly

  if (lEvent == WM_RBUTTONUP)
  {
    CPoint ptCursor;
    GetCursorPos(&ptCursor);
    ::SetForegroundWindow(m_NotifyIconData.hWnd);  
    ::TrackPopupMenu(pSubMenu.m_hMenu, TPM_LEFTBUTTON, ptCursor.x, ptCursor.y, 0, m_NotifyIconData.hWnd, NULL);
    ::PostMessage(m_NotifyIconData.hWnd, WM_NULL, 0, 0);
  }
  else if (lEvent == WM_LBUTTONDBLCLK) //double click received, the default action is to execute first menu item 
  {
    ::SetForegroundWindow(m_NotifyIconData.hWnd);
    UINT nDefaultItem = pSubMenu.GetMenuDefaultItem(FALSE, GMDI_GOINTOPOPUPS);
    if (nDefaultItem != -1)
      ::SendMessage(m_NotifyIconData.hWnd, WM_COMMAND, nDefaultItem, 0);
  }

  return 1; // handled
}

HICON CTrayNotifyIcon::LoadIconResource(LPCTSTR lpIconName)
{
  //First try to load the 16*16 icon, if this fails, they fall back on the 32*32 
  HICON hIcon = (HICON) ::LoadImage(ATL::_AtlBaseModule.GetResourceInstance(), lpIconName, IMAGE_ICON, 16, 16, 0);
  if (hIcon == NULL)
    hIcon = ::LoadIcon(ATL::_AtlBaseModule.GetResourceInstance(), lpIconName);

  //Return the icon handle
  return hIcon;
}

HICON CTrayNotifyIcon::LoadIconResource(UINT nIDResource)
{
  return LoadIconResource(MAKEINTRESOURCE(nIDResource));
}

BOOL CTrayNotifyIcon::GetDynamicDCAndBitmap(CDC* pDC, CBitmap* pBitmap)
{
  //Validate our parameters
  ATLASSERT(pDC != NULL);
  ATLASSERT(pBitmap != NULL);

  //Get the HWND for the desktop
  CWindow WndScreen(::GetDesktopWindow());
  if (!WndScreen.IsWindow())
    return FALSE;

  //Get the desktop HDC to create a compatible bitmap from
  CDC DCScreen(WndScreen.GetDC());
  if ( DCScreen.IsNull() )
    return FALSE;

  //Get the width and height of a small icon
  int w = GetSystemMetrics(SM_CXSMICON);
  int h = GetSystemMetrics(SM_CYSMICON);

  //Create an off-screen bitmap that the dynamic tray icon 
  //can be drawn into. (Compatible with the desktop DC).
  BOOL bSuccess = pBitmap->CreateCompatibleBitmap(DCScreen.m_hDC, w, h)!=NULL;
  if (!bSuccess)
  {
    WndScreen.ReleaseDC(DCScreen);
    return FALSE;
  }

  //Get a HDC to the newly created off-screen bitmap
  bSuccess = pDC->CreateCompatibleDC(DCScreen.m_hDC)!=NULL;
  if (!bSuccess)
  {
    //Release the Screen DC now that we are finished with it
    WndScreen.ReleaseDC(DCScreen);

    //Free up the bitmap now that we are finished with it
    pBitmap->DeleteObject();

    return FALSE;
  }

  //Select the bitmap into the offscreen DC
  pDC->SelectBitmap(pBitmap->m_hBitmap);

  //Release the Screen DC now that we are finished with it
  WndScreen.ReleaseDC(DCScreen);

  return TRUE;
}

DWORD CTrayNotifyIcon::GetShellVersion()
{
  if (sm_dwShellVersion)
    return sm_dwShellVersion;
  else
  {
    typedef HRESULT (CALLBACK DLLGETVERSION)(DLLVERSIONINFO*);
    typedef DLLGETVERSION* LPDLLGETVERSION;

    //What will be the return value
    sm_dwShellVersion = 4; //Assume version 4 of the shell

    //Try to get the details with DllGetVersion
    HMODULE hShell32 = GetModuleHandle(_T("shell32.dll"));
    LPDLLGETVERSION lpfnDllGetVersion = reinterpret_cast<LPDLLGETVERSION>(GetProcAddress(hShell32, "DllGetVersion"));
    if (lpfnDllGetVersion)
    {
      DLLVERSIONINFO vinfo;
      vinfo.cbSize = sizeof(DLLVERSIONINFO);
      if (SUCCEEDED(lpfnDllGetVersion(&vinfo)))
        sm_dwShellVersion = vinfo.dwMajorVersion;
    }
  }
  
  return sm_dwShellVersion;
}

DWORD CTrayNotifyIcon::GetNOTIFYICONDATASizeForOS()
{
  DWORD dwVersion = GetShellVersion();
  if (dwVersion >= 6)
    return sizeof(NOTIFYICONDATA_3);
  else if (dwVersion >= 5)
    return sizeof(NOTIFYICONDATA_2);
  else
    return sizeof(NOTIFYICONDATA_1);
}
