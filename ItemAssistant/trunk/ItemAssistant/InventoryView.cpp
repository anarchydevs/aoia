#include "stdafx.h"
#include "DBManager.h"
#include "InventoryView.h"
#include "AOMessageParsers.h"
#include "resource.h"
#include <map>
#include <set>
#include <shared/AODB.h>
#include <shared/FileUtils.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <ShellAPI.h>
#include <ItemAssistantCore/AOManager.h>

#define TIXML_USE_STL
#include <TinyXml/tinyxml.h>

using namespace WTL;


enum ColumnID
{
    COL_NAME,
    COL_QL,
    //COL_KEYLOW,
    //COL_KEYHIGH,
    COL_STACK,
    COL_CHARACTER,
    //COL_SLOT,
    //COL_VALUE,
    COL_CONTAINER,

    // this should be last always
    COL_COUNT
};


InventoryView::InventoryView()
:  m_sortDesc(false)
,  m_sortColumn(0)
{
}


InventoryView::~InventoryView()
{
}


LRESULT InventoryView::OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (!::IsWindowVisible(m_findview))
    {
        m_findview.ShowWindow(SW_SHOW);
    }
    m_findview.SetFocus();

    RECT rect;
    GetClientRect(&rect);
    CSize newsize(rect.right, rect.bottom);
    UpdateLayout(newsize);

    m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, TRUE);

    return 0;
}


LRESULT InventoryView::OnFindToggle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (!::IsWindowVisible(m_findview)) {
        m_findview.ShowWindow(SW_SHOW);
        m_findview.SetFocus();
        RECT rect;
        GetClientRect(&rect);
        CSize newsize(rect.right, rect.bottom);
        UpdateLayout(newsize);
    }
    else {
        HideFindWindow();
    } 

    m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, m_findview.IsWindowVisible());

    return 0;
}


LRESULT InventoryView::OnFindHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    HideFindWindow();
    return 0;
}


LRESULT InventoryView::OnInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (!::IsWindowVisible(m_infoview))
    {
        m_infoview.ShowWindow(SW_SHOW);
    }
    else
    {
        m_infoview.ShowWindow(SW_HIDE);
    }

    m_toolbar.CheckButton(ID_INFO, m_infoview.IsWindowVisible());

    RECT rect;
    GetClientRect(&rect);
    CSize newsize(rect.right, rect.bottom);
    UpdateLayout(newsize);

    return 0;
}


LRESULT InventoryView::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SharedServices::ShowHelp(_T("inventory"));
    return 0;
}


LRESULT InventoryView::OnColumnClick(LPNMHDR lParam)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

    if (m_sortColumn != pnmv->iSubItem)
    {
        m_sortColumn = pnmv->iSubItem;
        m_sortDesc = false;
    }
    else
    {
        m_sortDesc = !m_sortDesc;
    }

    m_listview.SortItemsEx(CompareStr, (LPARAM)this);

    return 0;
}


LRESULT InventoryView::OnItemChanged(LPNMHDR lParam)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam; 

    if (pnmv->uChanged & LVIF_STATE) {
        OnSelectionChanged();
    }

    return 0;
}


LRESULT InventoryView::OnItemContextMenu(LPNMHDR lParam)
{
    LPNMITEMACTIVATE param = (LPNMITEMACTIVATE)lParam;

    int sel = param->iItem;
    if (sel >= 0) {
        WTL::CPoint pos;
        GetCursorPos(&pos);

        WTL::CMenu menu;
        if (m_listview.GetSelectedCount() > 1) {
            menu.LoadMenu(IDR_ITEM_POPUP_MULTISEL);
        }
        else {
            menu.LoadMenu(IDR_ITEM_POPUP);
        }
        WTL::CMenuHandle popup = menu.GetSubMenu(0);
        int cmd = popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, pos.x, pos.y, m_hWnd);
    }
    return 0;
}


LRESULT InventoryView::OnSellItemAoMarket(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    if (m_listview.GetSelectedCount() != 1) {
        return 0;
    }
    int activeItemIdx = m_listview.GetSelectionMark();
    DWORD_PTR data = m_listview.GetItemData(activeItemIdx);

    std::tstring url = _T("http://www.aomarket.com/bots/additem?id=%lowid%&ql=%ql%");

    g_DBManager.Lock();
    OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo((int)data);
    g_DBManager.UnLock();

    boost::replace_all(url, _T("%lowid%"), pItemInfo->itemloid);
    boost::replace_all(url, _T("%ql%"), pItemInfo->itemql);

    ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_SHOWDEFAULT);

    return 0;
}


LRESULT InventoryView::OnCopyItemRef(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    // Build the set of selected item indexes.
    std::set<int> selectedIndexes;
    if (m_listview.GetSelectedCount() < 1) {
        return 0;
    }
    else if (m_listview.GetSelectedCount() == 1) {
        selectedIndexes.insert(m_listview.GetNextItem(-1, LVNI_SELECTED));
    }
    else {
        int itemId = m_listview.GetNextItem(-1, LVNI_SELECTED);
        while (itemId >= 0) {
            selectedIndexes.insert(itemId);
            itemId = m_listview.GetNextItem(itemId, LVNI_SELECTED);
        }
    }

    ItemServer server;
    ExportFormat format;
    bool scriptOutput = false;

    switch (CommandId) {
        case ID_COPY_AUNO_ITEMREF:
            server = SERVER_AUNO;
            format = FORMAT_HTML;
            break;
        case ID_COPY_AUNO_ITEMREF_VBB:
            server = SERVER_AUNO;
            format = FORMAT_VBB;
            break;
        case ID_COPY_JAYDEE_ITEMREF:
            server = SERVER_JAYDEE;
            format = FORMAT_HTML;
            break;
        case ID_COPY_JAYDEE_ITEMREF_VBB:
            server = SERVER_JAYDEE;
            format = FORMAT_VBB;
            break;
        case ID_COPY_AO_ITEMREF:
            server = SERVER_AO;
            format = FORMAT_AO;
            scriptOutput = true;
            break;
        default:
            assert(false);  // Unknown command ID?
            break;
    }

    std::tstring urlTemplate = GetServerItemURLTemplate(server);
    std::tstring itemTemplate;
    std::tstring prefix;
    std::tstring postfix;
    std::tstring separator = _T("\r\n");

    switch (format) {
        case FORMAT_HTML:
            itemTemplate = _T("<a href=\"%url%\">%itemname%</a> [QL %ql%] [%itemlocation%]");
            separator = _T("<br />\r\n");
            break;
        case FORMAT_VBB:
            itemTemplate = _T("[url=%url%]%itemname%[/url] {QL %ql%} {%itemlocation%}");
            break;
        case FORMAT_AO:
            itemTemplate = _T("<a href=%url%>%itemname%</a> [QL %ql%] [%itemlocation%]");
            prefix = _T("<a href=\"text://");
            postfix = _T("<p><p><center>Generated by <a href='chatcmd:///start http://ia.frellu.net'>AOIA</a></center>\">Item List</a>");
            separator = _T("<br />");
            break;
        default:
            assert(false);  // Unknown type?
            break;
    }

    boost::replace_all(itemTemplate, _T("%url%"), urlTemplate);

    g_DBManager.Lock();
    std::tstring output = prefix;
    for (std::set<int>::iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); ++it)
    {
        if (it != selectedIndexes.begin()) {
            output += separator;
        }

        DWORD_PTR data = m_listview.GetItemData(*it);
        OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo((unsigned int)data);

        std::tstring itemlocation = pItemInfo->ownername;
        itemlocation += _T(" -> ");
        itemlocation += pItemInfo->containername;

        std::tstring itemStr = itemTemplate;
        boost::replace_all(itemStr, _T("%lowid%"), pItemInfo->itemloid);
        boost::replace_all(itemStr, _T("%hiid%"), pItemInfo->itemhiid);
        boost::replace_all(itemStr, _T("%ql%"), pItemInfo->itemql);
        boost::replace_all(itemStr, _T("%itemname%"), pItemInfo->itemname);
        boost::replace_all(itemStr, _T("%itemlocation%"), itemlocation);

        output += itemStr;
    }
    output += postfix;
    g_DBManager.UnLock();

    if (!output.empty() && OpenClipboard() && EmptyClipboard()) {
        HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (output.length() * 2 + 2)); 
        if (hglbCopy == NULL) {
            CloseClipboard();
            return true;
        }

        // Lock the handle and copy the text to the buffer.
        TCHAR* lptstrCopy = (TCHAR*)GlobalLock(hglbCopy);
        ZeroMemory(lptstrCopy, output.length() * 2);
        memcpy(lptstrCopy, output.c_str(), output.length() * 2);
        GlobalUnlock(hglbCopy);

        // Place the handle on the clipboard.
#ifdef UNICODE
        SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
        SetClipboardData(CF_TEXT, hglbCopy);
#endif
        CloseClipboard();
    }

    // Output to the aoia script in the AO script folder.
    if (scriptOutput) {
        if (output.length() <= 4096) {
            std::tstring filename = STREAM2STR(AOManager::instance().getAOFolder() << _T("\\scripts\\aoia"));
#ifdef UNICODE
            std::wofstream ofs(filename.c_str());
#else
            std::ofstream ofs(filename.c_str());
#endif  // UNICODE
            ofs << output;

            static bool showMessage = true;

            if (showMessage)
            {
                MessageBox(
                    _T("All selected items were copied to the clipboard AND written to the AO script called 'aoia' \r\nYou can access it ingame by typing /aoia"), _T("INFORMATION"),
                    MB_OK | MB_ICONINFORMATION
                    );
                showMessage = false;
            }
        }
        else {
            MessageBox(
                _T("All selected items were copied to the clipboard, however Anarchy Online does not support scripts of more than 4096 bytes.\r\n\r\nTry selecting fewer items if you want the script file updated."), _T("ERROR"),
                MB_OK | MB_ICONERROR
                );
        }
    }

    return 0;
}


LRESULT InventoryView::OnShowItemRef(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    if (m_listview.GetSelectedCount() != 1) {
        return 0;
    }
    int activeItemIdx = m_listview.GetSelectionMark();
    DWORD_PTR data = m_listview.GetItemData(activeItemIdx);

    g_DBManager.Lock();
    OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo((int)data);
    g_DBManager.UnLock();

    std::tstring itemlocation = pItemInfo->ownername;
    itemlocation += _T(" -> ");
    itemlocation += pItemInfo->containername;

    TCHAR buffer[1024];
    if (CommandId == ID_VIEW_ITEMSTATS_AUNO) {
        ATL::AtlLoadString(IDS_AUNO_ITEMREF_URL, buffer, 1024);
    }
    else if (CommandId == ID_VIEW_ITEMSTATS_JAYDEE) {
        ATL::AtlLoadString(IDS_JAYDEE_ITEMREF_URL, buffer, 1024);
    }
    else {
        assert(false); // Looks like you forgot to add a section to load the proper url for your command.
    }
    std::tstring url(buffer);

    if (!url.empty()) {
        boost::replace_all(url, _T("%lowid%"), pItemInfo->itemloid);
        boost::replace_all(url, _T("%hiid%"), pItemInfo->itemhiid);
        boost::replace_all(url, _T("%ql%"), pItemInfo->itemql);
    }

    ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_SHOWDEFAULT);

    return 0;
}


LRESULT InventoryView::OnExportToCSV(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    // Build the set of selected item indexes.
    std::set<int> selectedIndexes;
    if (m_listview.GetSelectedCount() < 1) {
        return 0;
    }
    else if (m_listview.GetSelectedCount() == 1) {
        selectedIndexes.insert(m_listview.GetNextItem(-1, LVNI_SELECTED));
    }
    else {
        int itemId = m_listview.GetNextItem(-1, LVNI_SELECTED);
        while (itemId >= 0) {
            selectedIndexes.insert(itemId);
            itemId = m_listview.GetNextItem(itemId, LVNI_SELECTED);
        }
    }

    ItemServer server = SERVER_AUNO;
    if (CommandId == ID_EXPORTTOCSV_JAYDEE) {
        server = SERVER_JAYDEE;
    }

    std::tstring itemTemplate = _T("%lowid%,%hiid%,%ql%,\"%itemname%\",%itemlocation%,");
    itemTemplate += GetServerItemURLTemplate(server);
    std::tstring prefix = _T("LowID,HighID,QL,Name,Location,Link\n");
    std::tstring separator = _T("\n");

    // Ask user for name of CSV file
    std::tstring filename = BrowseForOutputFile(GetTopLevelWindow(), _T("Select Name of Output File"), _T("CSV Files (Comma-Separated Values)\0*.csv\0\0"), _T("csv"));
    if (filename.empty()) {
        return 0;
    }

    // Output to the selected CSV file.
#ifdef UNICODE
    std::wofstream ofs(filename.c_str());
#else
    std::ofstream ofs(filename.c_str());
#endif  // UNICODE

    if (!ofs.is_open()) {
        return 0;
    }

    g_DBManager.Lock();
    ofs << prefix;
    for (std::set<int>::iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); ++it)
    {
        if (it != selectedIndexes.begin()) {
            ofs << separator;
        }

        DWORD_PTR data = m_listview.GetItemData(*it);
        OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo((unsigned int)data);

        std::tstring itemlocation = pItemInfo->ownername;
        itemlocation += _T(" -> ");
        itemlocation += pItemInfo->containername;

        std::tstring itemStr = itemTemplate;
        boost::replace_all(itemStr, _T("%lowid%"), pItemInfo->itemloid);
        boost::replace_all(itemStr, _T("%hiid%"), pItemInfo->itemhiid);
        boost::replace_all(itemStr, _T("%ql%"), pItemInfo->itemql);
        boost::replace_all(itemStr, _T("%itemname%"), pItemInfo->itemname);
        boost::replace_all(itemStr, _T("%itemlocation%"), itemlocation);

        ofs << itemStr;
    }
    g_DBManager.UnLock();

    return 0;
}


void InventoryView::HideFindWindow()
{
    if (::IsWindowVisible(m_findview))
    {
        m_listview.SetFocus();
        m_findview.ShowWindow(SW_HIDE);
    }

    RECT rect;
    GetClientRect(&rect);
    CSize newsize(rect.right, rect.bottom);
    UpdateLayout(newsize);

    m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, FALSE);
}


LRESULT InventoryView::OnCreate(LPCREATESTRUCT createStruct)
{
    m_findview.Create(m_hWnd);
    //m_findview.ShowWindow(SW_SHOWNOACTIVATE);
    m_findview.SetParent(this);
    m_findview.SetDlgCtrlID(IDW_FINDVIEW);

    m_infoview.Create(m_hWnd);
    m_infoview.SetParent(this);
    m_infoview.SetDlgCtrlID(IDW_INFOVIEW);

    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    m_splitter.Create(m_hWnd, rcDefault, NULL, style);
    m_splitter.SetSplitterExtendedStyle(0);

    m_treeview.Create(m_splitter.m_hWnd, rcDefault, NULL, style | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS, WS_EX_CLIENTEDGE);
    m_treeview.SetDlgCtrlID(IDW_TREEVIEW);

    m_listview.Create(m_splitter.m_hWnd, rcDefault, NULL, style | LVS_REPORT /*| LVS_SINGLESEL*/ | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
    m_listview.SetDlgCtrlID(IDW_LISTVIEW);

    //m_treeview.SetUnicodeFormat();
    m_treeRoot.SetOwner(this);
    m_treeview.SetRootItem(&m_treeRoot);

    m_splitter.SetSplitterPanes(m_treeview, m_listview);
    m_splitter.SetActivePane(SPLIT_PANE_LEFT);

    m_accelerators.LoadAccelerators(IDR_INV_ACCEL);

    TBBUTTON buttons[5];
    buttons[0].iBitmap = 0;
    buttons[0].idCommand = ID_INV_FIND_TOGGLE;
    buttons[0].fsState = TBSTATE_ENABLED;
    buttons[0].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_CHECK | BTNS_AUTOSIZE;
    buttons[0].dwData = NULL;
    buttons[0].iString = (INT_PTR)_T("Find Item");
    buttons[1].iBitmap = 1;
    buttons[1].idCommand = ID_INFO;
    buttons[1].fsState = TBSTATE_ENABLED;
    buttons[1].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_CHECK | BTNS_AUTOSIZE;
    buttons[1].dwData = NULL;
    buttons[1].iString = (INT_PTR)_T("Item Info");
    buttons[2].iBitmap = 3;
    buttons[2].idCommand = 0;
    buttons[2].fsState = 0;
    buttons[2].fsStyle = BTNS_SEP;
    buttons[2].dwData = NULL;
    buttons[2].iString = NULL;
    buttons[3].iBitmap = 3;
    buttons[3].idCommand = ID_SELL_ITEM_AOMARKET;
    buttons[3].fsState = 0;
    buttons[3].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    buttons[3].dwData = NULL;
    buttons[3].iString = (INT_PTR)_T("Sell Item");
    buttons[4].iBitmap = 2;
    buttons[4].idCommand = ID_HELP;
    buttons[4].fsState = TBSTATE_ENABLED;
    buttons[4].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    buttons[4].dwData = NULL;
    buttons[4].iString = (INT_PTR)_T("Help");

    CImageList imageList;
    imageList.CreateFromImage(IDB_INVENTORY_VIEW, 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

    m_toolbar.Create(GetTopLevelWindow(), NULL, _T("InventoryViewToolBar"), 
        ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 
        TBSTYLE_EX_MIXEDBUTTONS);
    m_toolbar.SetButtonStructSize();
    //m_toolbar.AddBitmap(2, IDB_INVENTORY_VIEW);
    m_toolbar.SetImageList(imageList);
    m_toolbar.AddButtons(ARRAYSIZE(buttons), buttons);
    m_toolbar.AutoSize();

    PostMessage(WM_POSTCREATE);

    return 0;
}


void InventoryView::UpdateLayout(CSize newSize)
{
    CRect r( CPoint( 0, 0 ), newSize );

    if (::IsWindowVisible(m_findview))
    {
        RECT fvRect;
        m_findview.GetWindowRect(&fvRect);
        int height = fvRect.bottom - fvRect.top;
        ::SetWindowPos(m_findview, 0, r.left, r.top, r.Width(), height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
        r.top += height;
    }

    if (::IsWindowVisible(m_infoview))
    {
        RECT fvRect;
        m_infoview.GetWindowRect(&fvRect);
        int width = fvRect.right - fvRect.left;
        ::SetWindowPos(m_infoview, 0, r.right - width, r.top, width, r.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
        r.right -= width;
    }

    m_splitter.SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
}


LRESULT InventoryView::OnSize(UINT wParam, CSize newSize)
{
    UpdateLayout(newSize);
    return 0;
}


LRESULT InventoryView::OnPostCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    //SetSplitterPos works best after the default WM_CREATE has been handled
    m_splitter.SetSplitterPos(200);

    CTreeItem item = m_treeview.GetRootItem();
    item = m_treeview.GetChildItem(item);
    if (!item.IsNull())
    {
        item = m_treeview.GetChildItem(item);
    }
    if (!item.IsNull())
    {
        m_treeview.SelectItem(item);
    }

    return 0;
}

void InventoryView::OnAOClientMessage(AOClientMessageBase &msg)
{


    switch(msg.messageId())
    {
	case 0x1b: //zone
		{
			
		}
		break;

	case AO::MSG_OPENBACKPACK:// 0x52526858://1196653092:
		{

			return;
			Native::AOOpenBackpackOperation moveOp((AO::OpenBackpackOperation*)msg.start());

			OutputDebugString(moveOp.print().c_str());
			
		}
		break;
	case AO::MSG_ITEM_OPERATION: //0x5e477770
		{

			Native::AOItemOperation itemOp((AO::ItemOperation*)msg.start());

			unsigned int opId = itemOp.operationId();

			std::tstringstream sql;

			//TODO: switch statement
			if (opId == 0x34)
			{
				//split! itemHigh is the count of the new item. this gets a new slotId
				//the new item gets the count of the existing.
				//the one with 

				g_DBManager.Lock();
				g_DBManager.Begin();
				unsigned int fromContainerId = GetFromContainerId(msg.characterId(), itemOp.fromType(), itemOp.fromContainerTempId());

				if (fromContainerId == 0)
				{
					OutputDebugString(_T("From container not found!"));
			 		return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
				}

				unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(), fromContainerId);

				std::tstringstream sqlInsert;

				sqlInsert << _T("INSERT INTO tItems (keylow, keyhigh, ql, stack, parent, slot, children, owner)")
				<< _T(" SELECT keylow, keyhigh, ql, ") << itemOp.itemId().High()
				<< _T(", parent, ") << nextFreeInvSlot << _T(", children, owner FROM tItems")
				<< _T(" WHERE owner = ") << msg.characterId() 
				<< _T(" AND parent = ") << fromContainerId
				<< _T(" AND slot = ") << itemOp.fromItemSlotId();

			//	OutputDebugString(sqlInsert.str().c_str());
				g_DBManager.Exec(sqlInsert.str());

				sql << _T("UPDATE tItems SET stack = stack - ") << itemOp.itemId().High()
				<< _T(" WHERE owner = ") << msg.characterId() 
				<< _T(" AND parent = ") << fromContainerId
				<< _T(" AND slot = ") << itemOp.fromItemSlotId();

			//	OutputDebugString(sql.str().c_str());

				g_DBManager.Exec(sql.str());
				g_DBManager.Commit();
				g_DBManager.UnLock();

			}
			else if (opId == 0x35)
			{
			//	OutputDebugString(itemOp.print().c_str());

				//user joined two stacks!
				//itemHigh is the container id and slot id of the other item (gets deleted)
				//itemLow is the fromType of the other item
				
				unsigned short removedItemContType	 = itemOp.itemId().Low() & 0xff;
				unsigned short removedItemContTempId = itemOp.itemId().High() >> 16;
				unsigned short removedItemSlotId	 = itemOp.itemId().High() & 0xff;
	
				unsigned int removedItemContainerId	 = GetFromContainerId(msg.characterId(), removedItemContType, removedItemContTempId);

				unsigned int fromContainerId = GetFromContainerId(msg.characterId(), itemOp.fromType(), itemOp.fromContainerTempId());

				g_DBManager.Lock();
				g_DBManager.Begin();
				
				if (fromContainerId == 0)
				{
					OutputDebugString(_T("From container not found!"));
			 		return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
				}

				std::tstringstream sqlUpd;
				sqlUpd << _T("UPDATE tItems SET stack = ")
				 << _T(" (stack + (SELECT tItems2.stack FROM tItems tItems2 WHERE")
				 << _T(" tItems2.owner = ") << msg.characterId() 
				 << _T(" AND tItems2.parent = ") << removedItemContainerId
				 << _T(" AND tItems2.slot = ") << removedItemSlotId << _T("))") 
				<< _T(" WHERE owner = ") << msg.characterId() 
				<< _T(" AND parent = ") << fromContainerId
				<< _T(" AND slot = ") << itemOp.fromItemSlotId();

				OutputDebugString(sqlUpd.str().c_str());
				g_DBManager.Exec(sqlUpd.str());

				sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId() 
				<< _T(" AND parent = ") << removedItemContainerId
				<< _T(" AND slot = ") << removedItemSlotId;//itemId().High();

				OutputDebugString(sql.str().c_str());
				g_DBManager.Exec(sql.str());
				g_DBManager.Commit();
				g_DBManager.UnLock();
			}
			else if (opId == 0x70)
			{
				g_DBManager.Lock();
				g_DBManager.Begin();
				unsigned int fromContainerId = GetFromContainerId(msg.characterId(), itemOp.fromType(), itemOp.fromContainerTempId());

				if (fromContainerId == 0)
				{
					OutputDebugString(_T("From container not found!"));
			 		return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
				}

				//user deleted an item!
				sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId() 
				<< _T(" AND parent = ") << fromContainerId
				<< _T(" AND keylow = ") << itemOp.itemId().Low()
				<< _T(" AND keyhigh = ") << itemOp.itemId().High()
				<< _T(" AND slot = ") << itemOp.fromItemSlotId();

				g_DBManager.Exec(sql.str());
				g_DBManager.Commit();
				g_DBManager.UnLock();

			}
			else if (opId == 0x13) //0x69  from = 0c350+ a char Id
			{
				//when you run a nano, this one fires
				return;
			}
			else if (opId == 0xB3) 
				//0x69  from = 0c350+ a char Id = ?
				//0xD2  =when you tradeskill? pb
			{
				//When you hit a perk, this one fires
				
				return;
			}
			else

			{
				sql << _T("Unknown operation Id:") <<std::hex << opId;
				OutputDebugString(sql.str().c_str());
				OutputDebugString(itemOp.print().c_str());
				return;
			}
			
		}
		break;
		
		
	case AO::MSG_ITEM_MOVE: //0x47537a24://1196653092:
		{
			return;
			//(AO::MoveOperation*)msg.start();
		/*	Native::AOMoveOperation moveOp((AO::MoveOperation*)msg.start());

			//{frominv}=00 00 00 68 fra inv
			//{frominv}= 00 69 fra bank
			//{frombp} =00 6b fra backpack
			//{fromWear1}= 00 65

			unsigned int newParent;
			//{toBp1} =00 00 c7 49 21 46 92 6e
			//{toBp2} =00 00 c7 49 21 b2 55 e9
			//{toBp}  =00 00 c7 49 + 4 byte container Id
			//{toBank}=00 00 de ad + siste 4 bytes i char id.
			//{toWear}=00 00 c3 50 44 bb 17 ae 

			if (moveOp.toContainer().Low() == 0xc749)
				newParent = moveOp.toContainer().High();
			else if (moveOp.toContainer().Low() == 0xdead)
				newParent = 1;
			else if (moveOp.toContainer().Low() == 0xc350)
				newParent = 2;
			else
				return;

			OutputDebugString(_T("move"));

			unsigned int fromContainerId = GetFromContainerId(moveOp.charid(), moveOp.fromType(), moveOp.fromContSlotId());

			if (fromContainerId == 0)
			{
				OutputDebugString(_T("From container not found!"));
			 	return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
			}

			unsigned short newSlot = moveOp.targetSlotId();

			g_DBManager.Lock();
            g_DBManager.Begin();

	//		g_DBManager.MoveItem(newParent, newSlot move everything here?

			if (newSlot = 0x6f)
				newSlot = g_DBManager.FindNextAvailableContainerSlot(moveOp.charid(), newParent);
			
			//TODO:if (newSlot > 63+30??) calc a new one!!

			//if (moveOp.fromType() == 0x006b) //from a backpack
            {
				
                // Remove old contents from BP
                std::tstringstream sql;
     //           sql << _T("UPDATE OR IGNORE tItems, tItems tContFrom SET tItems.parent = ") << newParent
					//<< _T(" AND tItems.slot = ") << newSlot
					//<< _T(" WHERE tItems.parent = tContFrom.children And tContFrom.parent = 2 And tContFrom.slot = ") << fromSlotId
					//<< _T(" AND tContFrom.owner = ") << moveOp.charid() << _T(" AND tItems.owner = ") << moveOp.charid()
					//<< _T(" AND tItems.slot = ") << moveOp.fromItemSlotId();

				sql << _T("UPDATE tItems SET parent = ") << newParent
					<< _T(", slot = ") << newSlot
				    << _T(" WHERE parent = ") << fromContainerId
					<< _T(" AND slot = ") << moveOp.fromItemSlotId()
					<< _T(" AND owner = ") << moveOp.charid();//tContFrom.children And tContFrom.parent = 2 And tContFrom.slot = ") << fromSlotId
					//<< _T(" AND tContFrom.owner = ") << moveOp.charid() << _T(" AND tItems.owner = ") << moveOp.charid()
					//<< _T(" AND tItems.slot = ") << moveOp.fromItemSlotId();
		    
				//std::tstringstream sql;
    //            sql << _T("SELECT * FROM tItems, tItems tContFrom ")
				//	<< _T(" WHERE tItems.parent = ") << fromContainerId << _T(" AND tItems.owner = ") << moveOp.charid()
				//	<< _T(" AND tItems.slot = ") << moveOp.fromItemSlotId();

				

				OutputDebugString(sql.str().c_str());
				OutputDebugString(moveOp.print().c_str());

				

                g_DBManager.Exec(sql.str());

				g_DBManager.Commit();
                g_DBManager.UnLock();
            }
*/
   //         
		}
		break;
	
	default:
        break;
	}
}

unsigned int InventoryView::GetFromContainerId(unsigned int charId, unsigned short fromType, unsigned short fromSlotId)
{
	if (fromType == 0x006b) //backpack
		return ServicesSingleton::Instance()->GetInvSlotIndex(charId, fromSlotId);

	else if (fromType == 0x0068//inv
			|| fromType == 0x0065  //utils, hud, ncu, weap pane
			|| fromType == 0x0073  //social pande
			|| fromType == 0x0067  //implants
			|| fromType == 0x0066) //wear neck,sleeve
//fromType = 6c => remove item from trade window
		return 2; 
	else if (fromType == 0x0069)//bank
		return 1;
	//else if (fromType == 0x006e) //to overflow
	//	return 5;//ServicesSingleton::Instance()->GetInvSlotIndex(charId, fromSlotId);
	else if (fromType == 0x006f)//shop/trade window
	{
		OutputDebugString(_T("Move from shop remote party ignored"));
		return 0;
	}
	else if (fromType == 0xc767)//shop inventory
	{
		OutputDebugString(_T("Move from shop inventory ignored"));
		return 0;
	}
	else if (fromType == 0x006c)//shop/trade window.. could be remote or local :O
	{
		OutputDebugString(_T("Move from shop"));
		return 4;
	}
	else
	{
		std::tstringstream tmp;
		tmp << _T("TODO: UNKNOWN FROM TYPE ") << std::hex << fromType;

		OutputDebugString(tmp.str().c_str());
		return 2; //we assume inventory o.O
	}
}

void InventoryView::OnAOServerMessage(AOMessageBase &msg)
{
    switch(msg.messageId())
    {
		//TODO: Trades
		//TODO: Sell to shops
		//TODO: Tradeskills
		//TODO: add/remove from playershop
		//TODO: overflow? is a container?, to/from id 0x6e
		//TODO: item move to inventory when stackable exists joins to lowest slot id stack
		//TODO: item rewards (check how you receive key/item on rk mish)
		//TODO: item depletion (ammo, use stims etc (last stim important))
	case AO::MSG_PARTNER_TRADE://0x35505644
		{
			//when trade partner adds/removes items to trade window
			Native::AOPartnerTradeItem item((AO::PartnerTradeItem*)msg.start());
			OutputDebugString(item.print().c_str());
			
			unsigned int otherTradeContainer = 5;//trade partner

			unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(), otherTradeContainer);


			if (item.operationId() == 0x55) //trade partner adds an item
			{
				g_DBManager.Lock();
				g_DBManager.Begin();

				g_DBManager.InsertItem(
                    item.itemid().Low(),
                    item.itemid().High(),
                    item.ql(),
                    item.stack(),
                    otherTradeContainer,  // 2 = inventory
                    nextFreeInvSlot,
                    0,
                    item.charid());

				g_DBManager.Commit();
				g_DBManager.UnLock();
			}
			else if (item.operationId() == 0x56) //trade partner removes an item
			{
				if (item.partnerFromType() == 0x6c)//remove from trade window
				{
					g_DBManager.Lock();
					g_DBManager.Begin();

					std::tstringstream sql;

					sql << _T("DELETE FROM tItems")
						<< _T(" WHERE parent = ") << otherTradeContainer
						<< _T(" AND slot = ") << item.partnerFromItemSlotId()
						<< _T(" AND owner = ") << item.charid();
					g_DBManager.Exec(sql.str());
					OutputDebugString(sql.str().c_str());

					g_DBManager.Commit();
					g_DBManager.UnLock();
				}
			}
		}
		break;
	case AO::MSG_SHOP_TRANSACTION: //0x36284f6e 
		{
			Native::AOTradeTransaction item((AO::TradeTransaction*)msg.start());
			OutputDebugString(item.print().c_str());

			std::tstringstream sql;

			unsigned int shopContainer = 4;
			unsigned int otherTradeContainer = 5;//trade partner

			//we move stuff to parent=4 when in a trade
			switch (item.operationId())
			{
				//00=start trade? 01=client accept, 02=decline,03=tempAccept, 04=commit,05=add item,06=remove item
				case 0x00:
				{
					g_DBManager.Lock();
		            g_DBManager.Begin();

					//start trade?
					sql << _T("DELETE FROM tItems WHERE (parent = ") << shopContainer
						<< _T(" OR parent = ") << otherTradeContainer << _T(")")
						<< _T(" AND owner = ") << item.charid();

					g_DBManager.Exec(sql.str());

					g_DBManager.Commit();
					g_DBManager.UnLock();
				}
				case 0x01:
				{
					//01=client accept //do nothing
				}
				break;
				case 0x02:
				{
					//02=decline //move stuff back
					g_DBManager.Lock();
		            g_DBManager.Begin();

					unsigned int shopCapacity = 35;

					for (unsigned int i=0;i<=shopCapacity;i++) //or is it the other direction?
					{
						unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(), 2);
						
						sql.flush();
						sql.str(_T(""));

						sql << _T("UPDATE tItems SET parent = 2")
						<< _T(", slot = ") << nextFreeInvSlot
						<< _T(" WHERE parent = ") << shopContainer
						<< _T(" AND slot = ") << i
						<< _T(" AND owner = ") << item.charid();
						g_DBManager.Exec(sql.str());
						OutputDebugString(sql.str().c_str());
					}

					//delete trade partner added items:
					sql.flush();
					sql.str(_T(""));
					sql << _T("DELETE FROM tItems WHERE parent = ") << otherTradeContainer
						<< _T(" AND owner = ") << item.charid();

					g_DBManager.Exec(sql.str());
					OutputDebugString(sql.str().c_str());

					g_DBManager.Commit();
					g_DBManager.UnLock();
				}
				break;
				case 0x03:
				{
					//03=tempAccept //do nothing
				}
				break;
				case 0x04:
				{
					g_DBManager.Lock();
		            g_DBManager.Begin();
					//commit/server accept: (bye-bye stuff!)
					sql << _T("DELETE FROM tItems WHERE parent = ") << shopContainer
						<< _T(" AND owner = ") << item.charid();
					g_DBManager.Exec(sql.str());

					OutputDebugString(sql.str().c_str());


					//grab the new stuff!
					unsigned int shopCapacity = 35;

					for (unsigned int i=0;i<=shopCapacity;i++) //or is it the other direction?
					{
						unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(), 2);
						
						sql.flush();
						sql.str(_T(""));

						sql << _T("UPDATE tItems SET parent = 2")
						<< _T(", slot = ") << nextFreeInvSlot
						<< _T(" WHERE parent = ") << otherTradeContainer
						<< _T(" AND slot = ") << i
						<< _T(" AND owner = ") << item.charid();
						g_DBManager.Exec(sql.str());
						OutputDebugString(sql.str().c_str());
					}


					g_DBManager.Commit();
					g_DBManager.UnLock();
				}
				break;
				case 0x05:
				{
					//add item to trade window

					//fromId contains the owner
					
					if (item.fromId().High() != item.charid())
					{
						OutputDebugString(_T("Added by the other part"));
						return;
					}

					unsigned int fromContainerId = GetFromContainerId(item.charid(), item.fromType(), item.fromContainerTempId());

					g_DBManager.Lock();
		            g_DBManager.Begin();

					unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(), shopContainer);
					
					sql << _T("UPDATE tItems SET parent = ") << shopContainer
						<< _T(", slot = ") << nextFreeInvSlot
						<< _T(" WHERE parent = ") << fromContainerId
						<< _T(" AND slot = ") << item.fromItemSlotId()
						<< _T(" AND owner = ") << item.fromId().High(); //TODO: remove from other monitored char with fromId?

					g_DBManager.Exec(sql.str());

					OutputDebugString(sql.str().c_str());

					g_DBManager.Commit();
					g_DBManager.UnLock();
				}
				break;
				case 0x06:
				{
					//remove item from trade window

					if (item.fromId().High() != item.charid())
					{
						OutputDebugString(_T("Removed by the other part"));
						return;
					}

					g_DBManager.Lock();
		            g_DBManager.Begin();

					//if (item.fromId) //who added this item??

					unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(), 2);

					//TODO: move to overflow?

					sql << _T("UPDATE tItems SET parent = 2, slot = ") << nextFreeInvSlot
						<< _T(" WHERE parent = ") << shopContainer
						<< _T(" AND slot = ") << item.fromItemSlotId()
						<< _T(" AND owner = ") << item.charid();
					g_DBManager.Exec(sql.str());
					OutputDebugString(sql.str().c_str());

					g_DBManager.Commit();
					g_DBManager.UnLock();
				}
				break;
				case 0x08:
				{
					//partner added a backpack to the trade window
					OutputDebugString(_T("Trade partner added a backpack to trade window, not supported yet."));
					return;
				}
				break;
				default:
				{
					std::tstringstream warning;
					warning << _T("Unknown trade operation: ") << std::hex << item.operationId();
					OutputDebugString(warning.str().c_str());
					return;
				}
				break;

			}

		}
		break;
	case AO::MSG_ITEM_BOUGHT: //0x052e2f0c: 
		{
			Native::AOBoughtItemFromShop item((AO::BoughtItemFromShop*)msg.start());

			OutputDebugString(item.print().c_str());

			g_DBManager.Lock();
            g_DBManager.Begin();

			unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(), 2);

			g_DBManager.InsertItem(
                    item.itemid().Low(),
                    item.itemid().High(),
                    item.ql(),
                    item.stack(),
                    2,                           // 2 = inventory
                    nextFreeInvSlot,
                    0,
                    item.charid());

			g_DBManager.Commit();
            g_DBManager.UnLock();
		}
		break;
		case AO::MSG_ITEM_MOVE: //0x47537A24:// what about 0x52526858:
		{
	//	The ItemMoved is identical to the client send version except the header is server type.
			Native::AOItemMoved moveOp((AO::ItemMoved*)msg.start());
			
			OutputDebugString(moveOp.print().c_str());

			unsigned int newParent;
			
			unsigned int toType = moveOp.toContainer().Low();
			if (toType == 0xc749) //to backpack
				newParent = moveOp.toContainer().High();
			else if (toType == 0xdead)
				newParent = 1; //to bank
			else if (toType == 0xc350)
				newParent = 2; //to inventory
			else if (toType == 0x006e) //to overflow
				newParent = 5;
			else
			{
				std::tstringstream tmp;
				tmp << _T("TODO: UNKNOWN TO TYPE ") << std::hex << toType;
				OutputDebugString(tmp.str().c_str());
				return;
			}

			unsigned int fromContainerId = GetFromContainerId(moveOp.charid(), moveOp.fromType(), moveOp.fromContainerTempId());
		

			if (fromContainerId == 0)
			{
				OutputDebugString(_T("FromContainerId not found in cache "));

			 	return; //we dont have the value cached, either a bug or ia was started after the bp was opened.
			}

			unsigned short newSlot = moveOp.targetSlotId();

			g_DBManager.Lock();
            g_DBManager.Begin();

			if (newSlot >= 0x6f || newSlot == 0x00)
				newSlot = g_DBManager.FindNextAvailableContainerSlot(moveOp.charid(), newParent);
			
            std::tstringstream sql;
			//with slot in DB:
 //           sql << _T("UPDATE OR IGNORE tItems, tItems tContFrom SET tItems.parent = ") << newParent
				//<< _T(" AND tItems.slot = ") << newSlot
				//<< _T(" WHERE tItems.parent = tContFrom.children And tContFrom.parent = 2 And tContFrom.slot = ") << fromSlotId
				//<< _T(" AND tContFrom.owner = ") << moveOp.charid() << _T(" AND tItems.owner = ") << moveOp.charid()
				//<< _T(" AND tItems.slot = ") << moveOp.fromItemSlotId();

			sql << _T("UPDATE tItems SET parent = ") << newParent
				<< _T(", slot = ") << newSlot
			    << _T(" WHERE parent = ") << fromContainerId
				<< _T(" AND slot = ") << moveOp.fromItemSlotId()
				<< _T(" AND owner = ") << moveOp.charid();

			OutputDebugString(sql.str().c_str());

            g_DBManager.Exec(sql.str());

			g_DBManager.Commit();
            g_DBManager.UnLock();
        
		}
		break;
	
    case AO::MSG_BANK:
        {
            Native::AOBank bank((AO::Bank*)msg.start());
            g_DBManager.Lock();
            g_DBManager.Begin();
            {  // Remove old stuff from the bank. Every update is a complete update.
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId() << _T(" AND parent = 1");
                g_DBManager.Exec(sql.str());
            }
            for (unsigned int i = 0; i < bank.numitems(); i++)
            {
                Native::AOItem item = bank.item(i);
                g_DBManager.InsertItem(
                    item.itemid().Low(),
                    item.itemid().High(),
                    item.ql(),
                    item.stack(),
                    1,                           // 1 = bank container
                    item.index(),
                    item.containerid().High(),
                    msg.characterId());
            }
            g_DBManager.Commit();
            g_DBManager.UnLock();
        }
        break;

    case AO::MSG_CONTAINER:
        {
            Native::AOContainer bp((AO::Header*)msg.start());

			//OutputDebugString(_T("MSG_CONTAINER"));
			//OutputDebugString(bp.print().c_str());

            g_DBManager.Lock();
            g_DBManager.Begin();
            {
                // Remove old contents from BP
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE parent = ") << bp.containerid().High();
                g_DBManager.Exec(sql.str());
            }
            // Register BP contents
            for (unsigned int i = 0; i < bp.numitems(); i++)
            {
                Native::AOItem item = bp.item(i);
                g_DBManager.InsertItem(
                    item.itemid().Low(),
                    item.itemid().High(),
                    item.ql(),
                    item.stack(),
                    bp.containerid().High(),
                    item.index(),
                    0,
                    msg.characterId());
            }
			
			ServicesSingleton::Instance()->UpdateInvSlotIndex(bp.charid(), bp.tempContainerId(), bp.containerid().High());
			

            g_DBManager.Commit();
            g_DBManager.UnLock();
            //AddToTreeView(msg.charid(), bp.containerid().High());
        }
        break;

    case AO::MSG_FULLSYNC:
        {
            Native::AOEquip equip((AO::Equip*)msg.start());
            g_DBManager.Lock();
            g_DBManager.Begin();
            {
                // Remove old contents from DB
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE parent = 2 AND owner = ") << equip.charid();
                g_DBManager.Exec(sql.str());
            }
			
			OutputDebugString(_T("zoneing"));
			ServicesSingleton::Instance()->ClearInvSlotCache(equip.charid());

#ifdef DEBUG
			OutputDebugString(_T("FullSync"));
#endif
            // Register items
            for (unsigned int i = 0; i < equip.numitems(); i++)
            {
                Native::AOItem item = equip.item(i);

                g_DBManager.InsertItem(
                    item.itemid().Low(),
                    item.itemid().High(),
                    item.ql(),
                    item.stack(),
                    2,             // parent 2 = equip
                    item.index(),
                    item.containerid().High(),
                    msg.characterId());
            }
            g_DBManager.Commit();
            g_DBManager.UnLock();

            CleanupDB(equip.charid());
        }
        break;

    case AO::MSG_MOB_SYNC:
        {
            // Make sure this is the message for the currently playing toon (and not other mobs in vicinity).
            if (msg.characterId() == msg.entityId())
            {
                AO::MobInfo* pMobInfo = (AO::MobInfo*)msg.start();
                std::string name(&(pMobInfo->characterName.str), pMobInfo->characterName.strLen - 1);

                g_DBManager.Lock();
                g_DBManager.SetToonName(msg.characterId(), from_ascii_copy(name));
                g_DBManager.UnLock();
            }
        }
        break;

    case AO::MSG_SHOP_INFO:
        {
            AOPlayerShopInfo shop(msg.start(), msg.size());
            if (shop.shopId() != 0 && shop.ownerId() != 0)
            {
                g_DBManager.UpdateToonShopId(shop.ownerId(), shop.shopId());
            }
        }
        break;

	/*case AO::MSG_ITEM_MOVE:
		{
			g_DBManager.MoveItem({8 c1} 00 {4 fromtype}{2 fromId} {2 fromItemId} {8 toContainer})
		}*/

    case AO::MSG_SHOP_ITEMS:
        {
            AOPlayerShopContent shop(msg.start(), msg.size());

            // This message should only update the shops for already registered shop IDs, and then use the character 
            // ID of who-ever that shop is registered to. This will allow you to update all your toons shops without 
            // logging them in. (You only need to visit the shop with one of them.)

            unsigned int owner = g_DBManager.GetShopOwner(shop.shopid());

            if (owner != 0)
            {
                g_DBManager.Lock();
                g_DBManager.Begin();
                {
                    // Remove old contents from container
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE parent = 3 AND owner = ") << owner;
                    g_DBManager.Exec(sql.str());
                }
                // Register container contents
                for (unsigned int i = 0; i < shop.numitems(); i++)
                {
                    AOContainerItem item = shop.item(i);
                    unsigned int price = shop.price(item.index());

                    g_DBManager.InsertItem(
                        item.itemId().low(),
                        item.itemId().high(),
                        item.ql(),
                        item.stack(),
                        3,
                        item.index(),
                        0,
                        owner);
                }
                g_DBManager.Commit();
                g_DBManager.UnLock();
            }
        }
        break;

    default:
        break;
    }
}


void InventoryView::CleanupDB(unsigned int charid)
{
    // Statement to delete all the orfaned containers for this toon.
    std::tstringstream sql;
    sql << _T("DELETE FROM tItems WHERE owner = ") << charid << _T(" AND parent > 3 AND parent NOT IN (SELECT DISTINCT children FROM tItems)");

    g_DBManager.Lock();
    g_DBManager.Exec(sql.str());
    g_DBManager.UnLock();
}


bool InventoryView::PreTranslateMsg(MSG* pMsg)
{
    if (m_accelerators.TranslateAccelerator(m_hWnd, pMsg))
    {
        return true;
    }
    if (m_findview.PreTranslateMsg(pMsg))
    {
        return true;
    }

    return false;
}


// Updates the list view with the results of the SQL query. 'where' is used as the expression after the WHERE keyword.
void InventoryView::UpdateListView(std::tstring const& where)
{
    m_listview.DeleteAllItems();

    std::string selectStr =
        "SELECT "
        "   tItems.itemidx, "
        "   tItems.owner, "
        "   CASE "
        "       WHEN ((SELECT ql FROM tblAO WHERE aoid = keyhigh)-(SELECT ql FROM tblAO WHERE aoid = keylow))/2+(SELECT ql FROM tblAO WHERE aoid = keylow) > ql "
        "       THEN (SELECT name FROM tblAO WHERE aoid = keylow) "
        "       ELSE (SELECT name FROM tblAO WHERE aoid = keyhigh) "
        "   END AS name, "
        "   tItems.ql AS QL, "
        "   tItems.stack AS Stack, "
        "   tToons.charname AS Character, "
        "   tItems.parent AS Container "
        "FROM "
        "   tItems JOIN tToons ON tToons.charid = tItems.owner ";

    std::tstringstream sql;
    sql << from_ascii_copy(selectStr);

    if (!where.empty())
    {
        sql << _T(" WHERE ") << where;
    }

    g_DBManager.Lock();
    SQLite::TablePtr pT = g_DBManager.ExecTable(sql.str());
    g_DBManager.UnLock();

    if (pT)
    {
        if (pT->Rows() > 0)
        {
            // Backup previous column widths and labels
            std::map<std::tstring, int> columnWidthMap;

            while(m_listview.GetHeader().GetItemCount() > 0)
            {
                // Get column label
                TCHAR buffer[1024];
                ZeroMemory(buffer, sizeof(buffer));
                LVCOLUMN columnInfo;
                columnInfo.mask = LVCF_TEXT;
                columnInfo.pszText = buffer;
                columnInfo.cchTextMax = sizeof(buffer);
                m_listview.GetColumn( 0, &columnInfo);

                // Store label an width
                columnWidthMap[buffer] = m_listview.GetColumnWidth(0);

                m_listview.DeleteColumn(0);
            }

            //// Rebuild columns with new labels
            //for (unsigned int col = 0; col < pT->Columns()-1; col++)
            //{
            //   m_listview.AddColumn(pT->Headers()[col+1].c_str(), col);
            //   int width = columnWidthList.size() > col ? columnWidthList[col] : 100;
            //   m_listview.SetColumnWidth(col, width);
            //}
            std::map<std::tstring, int> columnmap;

            int indx = INT_MAX - 1;
            for (unsigned int row = 0; row < pT->Rows(); row++)
            {
                for (unsigned int col = 2; col < pT->Columns(); col++)
                {
                    std::tstring column = from_ascii_copy(pT->Headers()[col]);
                    if (columnmap.find(column) == columnmap.end())
                    {
                        columnmap[column] = m_listview.AddColumn(column.c_str(), col-2);
                        int w = columnWidthMap.find(column) != columnWidthMap.end() ? columnWidthMap[column] : 100;
                        m_listview.SetColumnWidth(columnmap[column], w);
                    }

                    std::tstring data = from_ascii_copy(pT->Data(row, col));

                    // Find proper container name
                    if (column == _T("Container"))
                    {
                        unsigned int id = boost::lexical_cast<unsigned int>(pT->Data(row, col));
                        unsigned int charid = boost::lexical_cast<unsigned int>(pT->Data(row, 1));

                        data = ServicesSingleton::Instance()->GetContainerName(charid, id);
                    }
                    indx = m_listview.AddItem(indx, columnmap[column], data.c_str());
                }
                if (pT->Columns() > 0)
                {
                    int data = boost::lexical_cast<int>(pT->Data(row, 0));
                    m_listview.SetItemData(indx, data);
                }
            }
        }
    }

    m_listview.SortItemsEx(CompareStr, (LPARAM)this);

    OnSelectionChanged();
}


void InventoryView::OnSelectionChanged()
{
    int count = m_listview.GetSelectedCount();
    if (count == 1) {
        m_toolbar.EnableButton(ID_SELL_ITEM_AOMARKET, TRUE);
        DWORD_PTR data = m_listview.GetItemData(m_listview.GetNextItem(-1, LVNI_SELECTED));
        m_infoview.SetCurrentItem((unsigned int)data);
    }
    else {
        m_toolbar.EnableButton(ID_SELL_ITEM_AOMARKET, FALSE);
        m_infoview.SetCurrentItem(0);
    }
}


// Static callback function for sorting items.
int InventoryView::CompareStr(LPARAM param1, LPARAM param2, LPARAM sort)
{
    InventoryView* pThis = (InventoryView*)sort;

    int result = 0;

    TCHAR name1[MAX_PATH];
    TCHAR name2[MAX_PATH];

    ZeroMemory(name1, sizeof(name1));
    ZeroMemory(name2, sizeof(name2));

    pThis->m_listview.GetItemText((int)param1, pThis->m_sortColumn, name1, sizeof(name1) - 1);
    pThis->m_listview.GetItemText((int)param2, pThis->m_sortColumn, name2, sizeof(name2) - 1);

    switch (pThis->m_sortColumn)
    {
    case COL_QL:
    case COL_STACK:
        {
            int a = boost::lexical_cast<int>(to_ascii_copy(name1));
            int b = boost::lexical_cast<int>(to_ascii_copy(name2));
            result = pThis->m_sortDesc ? b - a : a - b;
        }
        break;
    default:
        result = pThis->m_sortDesc ? StrCmpI(name2, name1) : StrCmpI(name1, name2);
    }

    return result;
}


std::tstring InventoryView::GetServerItemURLTemplate( ItemServer server )
{
    std::tstring retval;

    switch (server) {
        case SERVER_AUNO:
            {
                TCHAR buffer[1024];
                ATL::AtlLoadString(IDS_AUNO_ITEMREF_URL, buffer, 1024);
                retval = std::tstring(buffer);
            }
            break;
        case SERVER_JAYDEE:
            {
                TCHAR buffer[1024];
                ATL::AtlLoadString(IDS_JAYDEE_ITEMREF_URL, buffer, 1024);
                retval = std::tstring(buffer);
            }
            break;
        case SERVER_AO:
            {
                TCHAR buffer[1024];
                ATL::AtlLoadString(IDS_AO_ITEMREF_URL, buffer, 1024);
                retval = std::tstring(buffer);
            }
            break;
    }

    return retval;
}






InfoView::InfoView()
: m_pParent(NULL) 
{
}


void InfoView::SetParent(InventoryView* parent)
{
    m_pParent = parent;
}


BOOL InfoView::PreTranslateMsg(MSG* pMsg)
{
    return IsDialogMessage(pMsg);
}


LRESULT InfoView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    this->SetWindowText(_T("Info View"));

    DlgResize_Init(false, true, WS_CLIPCHILDREN);
    return 0;
}


LRESULT InfoView::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    LPMSG pMsg = (LPMSG)lParam;
    return this->PreTranslateMsg(pMsg);
}


LRESULT InfoView::OnButtonClicked(WORD commandID, WORD buttonID, HWND hButton, BOOL &bHandled)
{
    if (m_currentItem == 0) {
        return 0;
    }

    InventoryView::ItemServer server;
    switch (buttonID) {
        case IDC_JAYDEE:
            server = InventoryView::SERVER_JAYDEE;
            break;
        case IDC_AUNO:
            server = InventoryView::SERVER_AUNO;
            break;
        default:
            assert(false);  // Wrong button ID?
            return 0;
    }
    std::tstring url = InventoryView::GetServerItemURLTemplate(server);

    g_DBManager.Lock();
    OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo(m_currentItem);
    g_DBManager.UnLock();

    boost::replace_all(url, _T("%lowid%"), pItemInfo->itemloid);
    boost::replace_all(url, _T("%hiid%"), pItemInfo->itemhiid);
    boost::replace_all(url, _T("%ql%"), pItemInfo->itemql);

    ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_SHOWDEFAULT);

    return 0;
}


void InfoView::SetCurrentItem(unsigned int item)
{
    m_currentItem = item;

    WTL::CListViewCtrl lv(GetDlgItem(IDC_LISTVIEW));

    lv.DeleteAllItems();

    std::tstringstream sql;
    sql << _T("SELECT * FROM tItems WHERE itemidx = ") << item;

    g_DBManager.Lock();
    SQLite::TablePtr pT = g_DBManager.ExecTable(sql.str());
    g_DBManager.UnLock();

    if (pT)
    {
        if (pT->Rows() > 0)
        {
            // Backup previous column widths
            std::vector<int> columnWidthList;
            while(lv.GetHeader().GetItemCount() > 0)
            {
                columnWidthList.push_back(lv.GetColumnWidth(0));
                lv.DeleteColumn(0);
            }

            // Rebuild columns with new labels
            lv.AddColumn(_T("Property"), 0);
            lv.AddColumn(_T("Value"), 1);

            int width = columnWidthList.size() > 0 ? columnWidthList[0] : 75;
            lv.SetColumnWidth(0, width);
            width = columnWidthList.size() > 1 ? columnWidthList[1] : 120;
            lv.SetColumnWidth(1, width);

            // Fill content
            int indx = INT_MAX - 1;
            for (unsigned int col = 0; col < pT->Columns(); col++)
            {
                indx = lv.AddItem(indx, 0, from_ascii_copy(pT->Headers()[col]).c_str());
                lv.AddItem(indx, 1, from_ascii_copy(pT->Data()[col]).c_str());
            }
        }
    }
}








FindView::FindView()
    : m_lastQueryChar(-1)
    , m_lastQueryQlMin(-1)
    , m_lastQueryQlMax(-1)
    , m_pParent(NULL)
{
}


void FindView::SetParent(InventoryView* parent)
{
    m_pParent = parent;
}


LRESULT FindView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    this->SetWindowText(_T("Find View"));

    DlgResize_Init(false, true, WS_CLIPCHILDREN);
    return 0;
}


LRESULT FindView::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    LPMSG pMsg = (LPMSG)lParam;
    return this->PreTranslateMsg(pMsg);
}


BOOL FindView::PreTranslateMsg(MSG* pMsg)
{
    return IsDialogMessage(pMsg);
}


LRESULT FindView::OnCbnBuildCharcombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    KillTimer(1);

    CComboBox cb = GetDlgItem(IDC_CHARCOMBO);

    int oldselection = cb.GetCurSel();

    cb.ResetContent();
    int item = cb.AddString(_T("-"));
    cb.SetItemData(item, 0);

    g_DBManager.Lock();
    SQLite::TablePtr pT = g_DBManager.ExecTable(_T("SELECT DISTINCT owner FROM tItems"));
    g_DBManager.UnLock();

    if (pT != NULL)
    {
        for (unsigned int i = 0; i < pT->Rows(); i++)
        {
            unsigned int id = boost::lexical_cast<unsigned int>(pT->Data(i,0));

            g_DBManager.Lock();
            std::tstring name = g_DBManager.GetToonName(id);
            g_DBManager.UnLock();

            if (name.empty())
            {
                name = from_ascii_copy(pT->Data()[pT->Columns()*i]);
            }

            if ((item = cb.AddString(name.c_str())) != CB_ERR)
            {
                cb.SetItemData(item, id);
            }
        }
    }

    if (oldselection >= 0)
    {
        cb.SetCurSel(oldselection);
    }

    return 0;
}


LRESULT FindView::OnEnChangeItemtext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SetTimer(1, 1500);
    return 0;
}


LRESULT FindView::OnCbnSelChangeCharcombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    UpdateFindQuery();
    return 0;
}


LRESULT FindView::OnTimer(UINT wParam, TIMERPROC lParam)
{
    if (wParam == 1)
    {
        UpdateFindQuery();
        KillTimer(1);
    }
    return 0;
}


void FindView::UpdateFindQuery()
{
    KillTimer(1);

    CComboBox cb = GetDlgItem(IDC_CHARCOMBO);
    CEdit eb = GetDlgItem(IDC_ITEMTEXT);
    CEdit qlmin = GetDlgItem(IDC_QLMIN);
    CEdit qlmax = GetDlgItem(IDC_QLMAX);

    unsigned int charid = 0;
    int item = -1;
    if ((item = cb.GetCurSel()) != CB_ERR)
    {
        charid = (unsigned int)cb.GetItemData(item);
    }

    TCHAR buffer[MAX_PATH];
    ZeroMemory(buffer, MAX_PATH);
    eb.GetWindowText(buffer, MAX_PATH);
    std::tstring text(buffer);

    int minql = -1;
    ZeroMemory(buffer, MAX_PATH);
    qlmin.GetWindowText(buffer, MAX_PATH);
    std::tstring qlminText(buffer);
    try {
        minql = boost::lexical_cast<int>(qlminText);
    }
    catch(boost::bad_lexical_cast &/*e*/) {
        // Go with the default value
    }

    int maxql = -1;
    ZeroMemory(buffer, MAX_PATH);
    qlmax.GetWindowText(buffer, MAX_PATH);
    std::tstring qlmaxText(buffer);
    try {
        maxql = boost::lexical_cast<int>(qlmaxText);
    }
    catch(boost::bad_lexical_cast &/*e*/) {
        // Go with the default value
    }

    if ( text.size() > 2
        && ( m_lastQueryText != text || m_lastQueryChar != charid || m_lastQueryQlMin != minql || m_lastQueryQlMax != maxql ) )
    {
        m_lastQueryText = text;
        m_lastQueryChar = charid;
        m_lastQueryQlMin = minql;
        m_lastQueryQlMax = maxql;
        std::tstringstream sql;

        if (charid > 0) {
            sql << _T("owner = ") << charid << _T(" AND ");
        }
        if (minql > -1) {
            sql << _T("titems.ql >= ") << minql << _T(" AND ");
        }
        if (maxql > -1) {
            sql << _T("titems.ql <= ") << maxql << _T(" AND ");
        }

        sql << _T("keylow IN (SELECT aoid FROM aodb.tblAO WHERE name LIKE \"%") << text << _T("%\")");

        m_pParent->UpdateListView(sql.str());
    }
}
