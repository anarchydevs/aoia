#pragma once

#include <PluginSDK/ItemAssistView.h>
#include <boost/filesystem/path.hpp>
#include "shared/Thread.h"
#include "shared/Mutex.h"
#include <atlsplit.h>
#include "MFTreeView.h"
#include "PsmTreeItems.h"

class WatchDirectoryThread : public Thread
{
public:
	WatchDirectoryThread(): m_term(false)  { }
   virtual ~WatchDirectoryThread() { }


	void RefreshTree(LPTSTR lpDrive);
	void RefreshDirectory(LPTSTR lpDir);
	void WatchDirectory(LPTSTR lpDir);

   void SetOwner(PlayershopView* owner) { m_pOwner = owner; }
   void StopPlease() { if (IsRunning()) { m_term = true; } }

   virtual DWORD ThreadProc();

private:
   PlayershopView* m_pOwner;
   bool m_term;

};
class PlayershopView :
    public ItemAssistView<PlayershopView>
{
    typedef ItemAssistView<PlayershopView> inherited;
public:
   //enum { IDD = IDD_PLAYERSHOP };
   DECLARE_WND_CLASS(NULL)

   PlayershopView(void);
   virtual ~PlayershopView(void);

   enum
   {
      WM_POSTCREATE = WM_APP + 1,
   };

   enum {
      IDW_PSMLISTVIEW = 1,
      IDW_PSMTREEVIEW
   };

   BEGIN_MSG_MAP_EX(PlayershopView)
      MSG_WM_CREATE(OnCreate)
      MSG_WM_SIZE(OnSize)
      MESSAGE_HANDLER(WM_POSTCREATE, OnPostCreate)
      /*      COMMAND_ID_HANDLER(ID_INV_FIND, OnFind)
      COMMAND_ID_HANDLER(ID_INV_FIND_HIDE, OnFindHide)
      COMMAND_ID_HANDLER(ID_INFO, OnInfo)
      NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnColumnClick)
      NOTIFY_CODE_HANDLER_EX(LVN_ITEMACTIVATE, OnItemActivate)
      CHAIN_MSG_MAP(inherited)*/
      REFLECT_NOTIFICATIONS()
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT createStruct);
   LRESULT OnSize(UINT wParam, CSize newSize);
   LRESULT OnPostCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnColumnClick(LPNMHDR lParam);
   LRESULT OnItemActivate(LPNMHDR lParam);

   void UpdateListView(std::tstring const& where);
   void UpdateListView(std::vector<std::tstring> v);

   // ItemAssistView overrides
   virtual void OnActive(bool doActivation);

protected:
   void UpdateLayout(CSize newSize);
   static int CALLBACK CompareStr(LPARAM param1, LPARAM param2, LPARAM sort);

private:


   CSplitterWindow   m_splitter;

   //MFTreeView        m_treeview;
   PsmTreeView		   m_treeview;
   CListViewCtrl     m_listview;
   //CTreeItem         m_treeRoot;

   PlayershopTreeRoot m_treeRoot;
   WatchDirectoryThread m_directoryWatch;

   bool  m_sortDesc;
   int   m_sortColumn;

};

