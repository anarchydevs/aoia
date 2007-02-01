#pragma once

#include <PluginSDK/ItemAssistView.h>
#include <atlsplit.h>
#include "MFTreeView.h"
#include "PsmTreeItems.h"

class PlayershopView :
   public ItemAssistView<PlayershopView>//,
   //CDialogImpl<PlayershopView>,
   //public CDialogResize<PlayershopView>
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
   BEGIN_MSG_MAP_EX(InventoryView)
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

   void UpdateListView(std::tstring const& where);

protected:
   void UpdateLayout(CSize newSize);
   static int CALLBACK CompareStr(LPARAM param1, LPARAM param2, LPARAM sort);

private:
   CSplitterWindow   m_splitter;

   //MFTreeView        m_treeview;
   PsmTreeView		 m_treeview;
   CListViewCtrl     m_listview;

   //CTreeItem         m_treeRoot;

   PlayershopTreeRoot m_treeRoot;

   bool  m_sortDesc;
   int   m_sortColumn;


};
