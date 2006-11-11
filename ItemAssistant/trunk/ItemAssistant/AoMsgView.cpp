#include "StdAfx.h"
#include "aomsgview.h"
#include "AOMessageParsers.h"


AoMsgView::AoMsgView(void)
{
   m_mask.insert(AO::MSG_POS_SYNC);
   m_mask.insert(AO::MSG_UNKNOWN_1);
   m_mask.insert(AO::MSG_UNKNOWN_2);
   m_mask.insert(AO::MSG_UNKNOWN_3);
}


AoMsgView::~AoMsgView(void)
{
}


BOOL AoMsgView::PreTranslateMessage(MSG* pMsg)
{
   if (pMsg->message == WM_COMMAND && LOWORD(pMsg->wParam) == ID_EDIT_CLEAR)
   {
      pMsg->hwnd = m_hWnd;
   }
	return FALSE;
}


LRESULT AoMsgView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	//RECT rect;

	//GetClientRect(&rect);
	//dc.DrawText("AO MSG VIEW", -1, &rect, NULL);

	return 0;
}


LRESULT AoMsgView::OnCreate(LPCREATESTRUCT createStruct)
{
   //m_hWndClient = m_listview.Create(m_hWnd, rcDefault, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT, /*WS_EX_CLIENTEDGE*/ NULL );

   m_dlgview.Create(m_hWnd);

   m_listview = m_dlgview.GetDlgItem(IDC_LIST1);
   m_listview.ModifyStyle(0, LVS_REPORT, SWP_NOACTIVATE);
   m_listview.AddColumn(_T("Message ID"), 0);
   m_listview.AddColumn(_T("Size"), 1);
	return 0;
}


LRESULT AoMsgView::OnSize(UINT wParam, CSize newSize)
{
	::SetWindowPos(m_dlgview, NULL, 0, 0, newSize.cx, newSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	return 0;
}


LRESULT AoMsgView::OnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_listview.DeleteAllItems();
   return 0;
}


void AoMsgView::OnAOMessage(AO::Header* pMsg)
{
   CString msgString, sizeStr;
   unsigned int msgid = _byteswap_ulong(pMsg->msgid);

   if (m_mask.find(msgid) != m_mask.end())
   {
      return;
   }

   msgString.Format(_T("0x%08x"), msgid);
   sizeStr.Format(_T("%d"), _byteswap_ushort(pMsg->msgsize));

   int id = m_listview.AddItem(m_listview.GetItemCount(), 0, msgString);

   char* pdata = (char*)malloc(_byteswap_ushort(pMsg->msgsize));
   memcpy(pdata, pMsg, _byteswap_ushort(pMsg->msgsize));

   m_listview.SetItemData(id, (DWORD_PTR)pdata);
   m_listview.SetItemText(id, 1, sizeStr);
}






LRESULT DlgView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	this->SetWindowText(_T("Dialog View"));

	HICON hIcon = (HICON)::LoadImage(
				_Module.GetResourceInstance(),
				MAKEINTRESOURCE(IDR_MAINFRAME),
				IMAGE_ICON, 16, 16, LR_SHARED);
	this->SetIcon(hIcon, ICON_SMALL);

	DlgResize_Init(false, true, WS_CLIPCHILDREN);
	return 0;
}

LRESULT DlgView::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;

	return this->PreTranslateMessage(pMsg);
}


LRESULT DlgView::OnNMClickList1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
   WTL::CListViewCtrl listview(GetDlgItem(IDC_LIST1));

   int index = listview.GetSelectedIndex();
   if (index >= 0)
   {
      AO::Header *pMsg = (AO::Header*)listview.GetItemData(index);
      Native::AOMessageHeader msg(pMsg);

      char* pData = (char*)pMsg;
      unsigned int size = _byteswap_ushort(pMsg->msgsize);

      WTL::CString str;
      std::tstring text;
      unsigned char * p = (unsigned char*)pData;
      int linebreak = 0;

      text += msg.print();

      for (unsigned int offset = 0; offset < size; offset += 4)
      {
         p = (unsigned char*)(pData + offset);
         for (int i = 0; i < 4; i++)
         {
            str.Format(_T("%02X"), p[i]);
            text += str;
         }
         if (linebreak < 4)
         {
            text += _T("\t");
            linebreak++;
         }
         else
         {
            text += _T("\r\n");
            linebreak = 0;
         }
      }

      GetDlgItem(IDC_EDIT2).SetWindowText(text.c_str());
   }
   return 0;
}


BOOL DlgView::PreTranslateMessage(MSG* pMsg)
{
	return IsDialogMessage(pMsg);
}
