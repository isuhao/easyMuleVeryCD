/*
 * $Id: RssFeedWnd.cpp 20790 2010-11-12 08:51:48Z huangqing $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
// UILayer\RssFeedWnd.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "emule.h"
#include "resource.h"
#include "UILayer\RssFeedWnd.h"
#include "UserMsgs.h"
#include ".\rssfeedwnd.h"
#include "DlgMaintabDownload.h"
#include "emuleDlg.h"
#include "CollectionViewDialog.h"

//--------------------------------------------------------------------
// CFeedListBox Feed�б��
//IMPLEMENT_DYNAMIC(CFeedListBox, CListBox)
//
//CFeedListBox::CFeedListBox()
//{
//	int cx = GetSystemMetrics(SM_CXSMICON);
//	int cy = GetSystemMetrics(SM_CYSMICON);
//	m_hIcon[CRssFeed::fsNormal] = theApp.LoadIcon(_T("SHAREDFILES"), cx, cy);
//	m_hIcon[CRssFeed::fsDownloading] = theApp.LoadIcon(_T("CHECKNEWVERSION"), cx, cy);
//	m_hIcon[CRssFeed::fsDisable] = theApp.LoadIcon(_T("FILTERCLEAR2"), cx, cy);
//	m_iItemHeight = cy + 6;
//}
//
//void CFeedListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
//{
//	// TODO:  ������Ĵ�����ȷ��ָ����Ĵ�С
//	ASSERT(lpMeasureItemStruct != NULL);
//	lpMeasureItemStruct->itemHeight = m_iItemHeight;
//}
//
//void CFeedListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
//{
//	// TODO:  ������Ĵ����Ի���ָ����
//	ASSERT(lpDrawItemStruct != NULL);
//
//	CDC * dc = CDC::FromHandle(lpDrawItemStruct->hDC);
//	if ( dc == NULL )
//	{
//		return;
//	}
//
//	int iSaveDc = dc->SaveDC();
//
//	COLORREF crTextColor, crBkColor;
//	dc->SetBkMode(TRANSPARENT);
//	BOOL bIsSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
//
//	if( bIsSelected )
//	{
//		crTextColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
//		dc->SetTextColor(crTextColor);
//		crBkColor = ::GetSysColor(COLOR_HIGHLIGHT);
//	}
//	else
//	{
//		crTextColor = dc->GetTextColor();
//		crBkColor = dc->GetBkColor();
//	}
//
//	dc->FillSolidRect(&lpDrawItemStruct->rcItem, crBkColor);
//	//dc->SetBkColor(crBkColor);
//
//	if ( lpDrawItemStruct->itemID != UINT(-1) )
//	{
//		CString strDisplayName;
//		HICON hIcon = NULL;
//		const CRssFeed * pFeed = (const CRssFeed *)(lpDrawItemStruct->itemData);
//		if ( pFeed != NULL )
//		{
//			strDisplayName = pFeed->GetDisplayName();
//			ASSERT(pFeed->m_State < CRssFeed::fsMaxState);
//			hIcon = m_hIcon[pFeed->m_State];
//		}
//		else
//		{
//			strDisplayName = _T("ȫ��");
//			hIcon = m_hIcon[CRssFeed::fsNormal];
//		}
//
//		int cx = GetSystemMetrics(SM_CXSMICON);
//		int cy = GetSystemMetrics(SM_CYSMICON);
//		//dc->DrawIcon(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, hIcon);
//		::DrawIconEx(*dc,
//					 lpDrawItemStruct->rcItem.left,
//					 lpDrawItemStruct->rcItem.top + (m_iItemHeight - cy) / 2,
//					 hIcon,
//					 cx,
//					 cy,
//					 0,
//					 NULL,
//					 DI_NORMAL
//					);
//		lpDrawItemStruct->rcItem.left += cx + 3;
//		dc->DrawText(strDisplayName, &lpDrawItemStruct->rcItem, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
//		if ( bIsSelected )
//		{
//			lpDrawItemStruct->rcItem.left -= cx + 3;
//			dc->DrawFocusRect(&lpDrawItemStruct->rcItem);
//		}
//
//	}
//	dc->RestoreDC(iSaveDc);
//
//	//TRACE(_T("DrawItem at %u\n"), GetTickCount());
//}
//
//BOOL CFeedListBox::PreCreateWindow(CREATESTRUCT& cs)
//{
//	// TODO: �ڴ����ר�ô����/����û���
//	cs.style &= ~(LBS_SORT | LBS_OWNERDRAWVARIABLE);
//	cs.style |= LBS_OWNERDRAWFIXED;
//	return CListBox::PreCreateWindow(cs);
//}


//--------------------------------------------------------------------
// CRssFeedWnd �Ի���

IMPLEMENT_DYNAMIC(CRssFeedWnd, CDialog)
CRssFeedWnd::CRssFeedWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CRssFeedWnd::IDD, pParent)
{
	m_pFeedPre = NULL;
	m_pFeedNext = NULL;

	m_nCurrFeedCatalog = 0;
	m_nFeedCatalogCount = 0;
}

CRssFeedWnd::~CRssFeedWnd()
{
	m_pFeedPre = NULL;
	m_pFeedNext = NULL;

	m_nCurrFeedCatalog = 0;
	m_nFeedCatalogCount = 0;
}

BOOL CRssFeedWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	ModifyStyle(0, WS_CLIPCHILDREN, 0);

	m_listFeedItem.Init();
	m_listFeedItem.SetOwner(this);

	m_listIcoFeedItem.Init();
	m_listIcoFeedItem.SetOwner(this);

	m_listSingleFeedItem.Init(TRUE);
	m_listSingleFeedItem.SetOwner(this);

	CRect Rect;
	GetClientRect(&Rect);
	m_listFeedItem.MoveWindow(Rect);
	AddAnchor(IDC_RW_FEED_ITEM_LIST, TOP_LEFT, BOTTOM_RIGHT);

	m_listIcoFeedItem.MoveWindow(Rect);
	AddAnchor(IDC_RW_FEED_ICO_ITEM_LIST, TOP_LEFT, BOTTOM_RIGHT);

	int nHeight = 25;
	CRect rcInfoWnd(Rect.left,Rect.top,Rect.right,Rect.top+nHeight);
	m_wndRssInfo.Create(WS_CHILD | WS_VISIBLE, rcInfoWnd, this, 0);
	m_wndRssInfo.SetOwner(this);
	
	m_wndRssInfo.MoveWindow(rcInfoWnd);
	AddAnchor(m_wndRssInfo.m_hWnd, TOP_LEFT, TOP_RIGHT);

	CRect rcListInfo(Rect.left,Rect.top+nHeight+1,Rect.right,Rect.bottom);
	m_listSingleFeedItem.MoveWindow(rcListInfo);
	AddAnchor(IDC_RW_FEED_SINGLE_ITEM_LIST, TOP_LEFT, BOTTOM_RIGHT);

	//��ʼ����ICOģʽ��ʾ
	m_ListShowMode = LISTSTYLE_ICO;

	//CreateSplitter();
	//AdjustVLayout(DEFAULT_LEFT_LIST_WIDTH);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

// ������ʾһ��feed����item��Ϣ����pRedrawFeedΪ�գ��ػ���ǰfeed
//void CRssFeedWnd::RedrawFeed(const CRssFeed * pRedrawFeed)

// ������ʾһ��feed����item��Ϣ
void CRssFeedWnd::RedrawFeed(CRssFeed & feed)
{
	m_listFeedItem.RedrawFeed(feed);

	if ( m_listSingleFeedItem.GetItemCount() > 0 )
	{
		m_listSingleFeedItem.RedrawFeed(feed);
	}

	//if ( pRedrawFeed != NULL )
	//{
	//	RedrawFeedLine();
	//}

	//int iSel = m_lsbFeed.GetCurSel();
	//if ( iSel < 0 )
	//{
	//	return;
	//}

	//const CRssFeed * pSelFeed = static_cast<const CRssFeed *>(m_lsbFeed.GetItemDataPtr(iSel));
	//if ( pSelFeed != NULL )
	//{
	//	if ( pRedrawFeed != NULL && pRedrawFeed != pSelFeed )
	//	{
	//		// �����ػ���feed���ǵ�ǰѡ����Ŀ
	//		return;
	//	}
	//	// �ػ�ָ��feed��items
	//	m_listFeedItem.SetRedraw(FALSE);
	//	m_listFeedItem.DeleteAllItems();
	//	m_listFeedItem.AddFeed(*pSelFeed);
	//	m_listFeedItem.SetRedraw(TRUE);
	//}
	//else
	//{
	//	// �ػ�ȫ��feed items
	//	m_listFeedItem.SetRedraw(FALSE);
	//	m_listFeedItem.DeleteAllItems();
	//	for ( int i = 1, count = m_lsbFeed.GetCount(); i < count; ++i )
	//	{
	//		const CRssFeed * pFeed = static_cast<const CRssFeed *>(m_lsbFeed.GetItemDataPtr(i));
	//		ASSERT(pFeed != NULL);
	//		m_listFeedItem.AddFeed(*pFeed);
	//	}
	//	m_listFeedItem.SetRedraw(TRUE);

	//}
}

// �����ָ���
//void CRssFeedWnd::CreateSplitter()
//{
//	
//	CRect	rcLeft;
//	m_lsbFeed.GetWindowRect(rcLeft);
//	ScreenToClient(rcLeft);
//
//	CRect rcSpl;
//	m_listFeedItem.GetWindowRect(rcSpl);
//	ScreenToClient(rcSpl);
//	rcSpl.left = rcLeft.right;
//	rcSpl.right = rcSpl.left;
//
//	m_wndVSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_RW_FEED_SPLITTER);
//	m_wndVSplitter.SetNormalDraw(FALSE);
//	m_wndVSplitter.m_nflag = TRUE;
//		
//}

// �����������Ҳ���
//void CRssFeedWnd::AdjustVLayout(UINT uLeftWidth /*= -1*/)
//{
//	
//	RemoveAnchor(IDC_RW_FEED_LIST);
//	RemoveAnchor(IDC_RW_FEED_SPLITTER);
//	RemoveAnchor(IDC_RW_FEED_ITEM_LIST);
//
//	// ����m_lsbFeed
//	CRect rcLeft;
//	m_lsbFeed.GetWindowRect(rcLeft);
//	ScreenToClient(rcLeft);
//	if ( uLeftWidth != -1 )
//	{
//		rcLeft.right = rcLeft.left + uLeftWidth;
//		m_lsbFeed.Invalidate();
//		m_lsbFeed.MoveWindow(rcLeft);
//	}
//	AddAnchor(IDC_RW_FEED_LIST, TOP_LEFT, BOTTOM_LEFT);
//
//	if ( ::IsWindow(m_wndVSplitter.m_hWnd) )
//	{	
//		// ����splitter
//		CRect rcSpl;
//		m_wndVSplitter.GetWindowRect(rcSpl);
//		ScreenToClient(rcSpl);
//		rcSpl.left = rcLeft.right - 1;
//		rcSpl.right = rcSpl.left + m_wndVSplitter.GetHBreadth();
//		rcSpl.top = rcLeft.top;
//		rcSpl.bottom = rcLeft.bottom;
//		m_wndVSplitter.MoveWindow(rcSpl);
//
//		// ����m_listFeedItem
//		CRect rcRight;
//		m_listFeedItem.GetWindowRect(rcRight);
//		ScreenToClient(rcRight);
//		rcRight.left = rcSpl.right - 1;
//		//m_listFeedItem.Invalidate();
//		m_listFeedItem.MoveWindow(rcRight);
//
//		AddAnchor(IDC_RW_FEED_SPLITTER, TOP_LEFT, BOTTOM_LEFT);
//		AddAnchor(IDC_RW_FEED_ITEM_LIST, TOP_LEFT, BOTTOM_RIGHT);
//	}
//}

// ������������
BOOL CRssFeedWnd::SendDownloadRequestMessage(CRssFeed::Item & item)
{
	item.m_bIsNewPub = false;

	if ( CWnd * pwndNotify = GetOwner() )
	{
		// ������������
		BOOL bResult = pwndNotify->SendMessage(WM_RSS_REQ_DOWNLOAD_FILE, (WPARAM)(&item), TRUE);
		return bResult;
	}

	return FALSE;
}

void CRssFeedWnd::OpenFile(const CKnownFile* file)
{
	if(file->m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(file->m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

//��ָ���������feed��Ŀ
BOOL CRssFeedWnd::OpenFeedItem(int iItemIndex)
{
	if ( iItemIndex == -1 )
	{
		return FALSE;
	}

	CRssFeed::Item * pItem;
	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		pItem = m_listFeedItem.GetFeedItem(iItemIndex);
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pItem = m_listSingleFeedItem.GetFeedItem(iItemIndex);
	}

	if ( pItem == NULL )
	{
		return FALSE;
	}

	CKnownFile* pKnownFile = pItem->GetShareFile();
	if(PathFileExists(pKnownFile->GetPath()))
	{
		if (!pKnownFile->IsPartFile())
		{
			if(!PathFileExists(pKnownFile->GetFilePath()))
			{
				MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
				return FALSE;
			}

			OpenFile(pKnownFile);
			return TRUE;
		}
		else
		{
			MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK);
		}
	}
	else
	{
		MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK);
	}

	return FALSE;
}

// ����ָ��feed��Ŀ
BOOL CRssFeedWnd::DownloadFeedItem(int iItemIndex)
{
	if ( iItemIndex == -1 )
	{
		return FALSE;
	}

	CRssFeed::Item * pItem;
	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		pItem = m_listFeedItem.GetFeedItem(iItemIndex);
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pItem = m_listSingleFeedItem.GetFeedItem(iItemIndex);
	}
	
	if ( pItem == NULL )
	{
		return FALSE;
	}
	
	// ���ض�Ӧ��Ļ
	if ( pItem->m_pAttachItem != NULL )
	{
		SendDownloadRequestMessage(*pItem->m_pAttachItem);
	}

	// ����ӰƬ
	SendDownloadRequestMessage(*pItem);
	
	// �ػ�����Ŀ
	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		m_listFeedItem.RedrawFeedItem(iItemIndex);
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		m_listSingleFeedItem.RedrawFeedItem(iItemIndex);
	}
	
	return TRUE;
}

void CRssFeedWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RW_FEED_ITEM_LIST, m_listFeedItem);
	DDX_Control(pDX, IDC_RW_FEED_ICO_ITEM_LIST, m_listIcoFeedItem);
	DDX_Control(pDX, IDC_RW_FEED_SINGLE_ITEM_LIST, m_listSingleFeedItem);
}


BEGIN_MESSAGE_MAP(CRssFeedWnd, CResizableDialog)
	ON_WM_SIZE()
	ON_WM_MEASUREITEM()
	//ON_NOTIFY(UM_SPLITTER_CLICKED, IDC_RW_FEED_SPLITTER, OnVSplitterClicked)
	//ON_NOTIFY(UM_SPN_SIZED, IDC_RW_FEED_SPLITTER, OnSplitterMoved)
	//ON_LBN_SELCHANGE(IDC_RW_FEED_LIST, OnLbnSelchangeRwFeedList)
	ON_NOTIFY(NM_DBLCLK, IDC_RW_FEED_ITEM_LIST, OnNMDblclkRwFeedItemList)
	ON_NOTIFY(NM_DBLCLK, IDC_RW_FEED_SINGLE_ITEM_LIST, OnNMDblclkRwFeedItemList)
	//ON_NOTIFY(NM_CLICK, IDC_RW_FEED_ITEM_LIST, OnNMClickRwFeedItemList)
	ON_MESSAGE(WM_LISTCTRLCLICKED, OnDownloadButtonClicked)
	ON_MESSAGE(WM_LISTCTRL_SWITCH_SHOWMODE_CLICKED, OnSwitchListShowModeClicked)
	ON_MESSAGE(WM_LISTCTRL_INFO_CLICKED, OnListInfoClicked)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RW_FEED_ITEM_LIST, OnLvnItemchangedRwFeedItemList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RW_FEED_SINGLE_ITEM_LIST, OnLvnItemchangedRwFeedItemList)
END_MESSAGE_MAP()


// CRssFeedWnd ��Ϣ�������
//void CRssFeedWnd::OnVSplitterClicked(NMHDR* pNMHDR, LRESULT* /*pResult*/)
//{
//	
//	ASSERT(pNMHDR != NULL);
//	SPCEX_NMHDR* pHdr = (SPCEX_NMHDR*)pNMHDR;
//
//	UINT uLeftListWidth = (pHdr->flag ? 0 : DEFAULT_LEFT_LIST_WIDTH);
//
//	// ��������
//	AdjustVLayout(uLeftListWidth);
//
//	// �ı���
//	m_wndVSplitter.m_nflag = !pHdr->flag;
//	
//}

//void CRssFeedWnd::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
//{
//	ASSERT(pNMHDR != NULL);
//	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
//
//	UINT uNewLeftWidth = (UINT)(-1);
//	if ( pHdr->delta != 0 )
//	{
//		CRect rcLeft;
//		m_lsbFeed.GetWindowRect(rcLeft);
//		uNewLeftWidth = rcLeft.Width() + pHdr->delta;
//	}
//	AdjustVLayout(uNewLeftWidth);
//}

//void CRssFeedWnd::OnSize(UINT nType, int cx, int cy)
//{
//	CResizableDialog::OnSize(nType, cx, cy);
//
//	// TODO: �ڴ˴������Ϣ����������
//	/*
//	if ( ::IsWindow(m_wndVSplitter.m_hWnd) )
//	{
//		CRect rc;
//		GetWindowRect(rc);
//		ScreenToClient(rc);
//		m_wndVSplitter.SetRange(rc.left + 10, rc.left + SPLITTER_RANGE_MAX);
//	}*/
//	if (m_listFeedItem.GetSafeHwnd() != NULL)
//		m_listFeedItem.MoveWindow(0, 0, cx, cy, FALSE);
//}

void CRssFeedWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch ( nIDCtl )
	{
		//case IDC_RW_FEED_LIST:
		//	m_lsbFeed.MeasureItem(lpMeasureItemStruct);
		//	break;
		case IDC_RW_FEED_ITEM_LIST:
			m_listFeedItem.MeasureItem(lpMeasureItemStruct);
			break;
		case IDC_RW_FEED_SINGLE_ITEM_LIST:
			m_listSingleFeedItem.MeasureItem(lpMeasureItemStruct);
			break;
		default:
			CResizableDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
	}
}

//void CRssFeedWnd::OnLbnSelchangeRwFeedList()
//{
//	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	RedrawFeed(NULL);
//}

void CRssFeedWnd::OnNMDblclkRwFeedItemList(NMHDR* pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != NULL);
	*pResult = 0;

	// Version 4.71. Pointer to an NMITEMACTIVATE structure 
	LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	//int iItem = m_listFeedItem.GetClickItemIndex(NULL);
	//m_listFeedItem.SetItemState(TestInfo.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	//m_listFeedItem.SetFocus();

	if ( lpnmitem->iItem == -1 )
	{
		return;
	}

	CRssFeed::Item * pItem;
	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		pItem = m_listFeedItem.GetFeedItem(lpnmitem->iItem);
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pItem = m_listSingleFeedItem.GetFeedItem(lpnmitem->iItem);
	}
	
	if ( pItem == NULL )
	{
		return;
	}

	if ( pItem->GetState() == CRssFeed::isDownloaded )
	{
		OpenFeedItem(lpnmitem->iItem);
	}
	else
	{
		DownloadFeedItem(lpnmitem->iItem);
	}
}

//void CRssFeedWnd::OnNMClickRwFeedItemList(NMHDR * /*pNMHDR*/, LRESULT *pResult)
//{
//	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	ASSERT(pResult != NULL);
//	*pResult = 0;
//
//	int iItem = m_listFeedItem.GetClickDownloadBtnIndex();
//	SendDownloadRequest(iItem);
//}

LRESULT CRssFeedWnd::OnDownloadButtonClicked(WPARAM wParam, LPARAM /*lParam*/)
{
	int index;
	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		index = m_listFeedItem.ButtonIndexToItemIndex(wParam);
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		index = m_listSingleFeedItem.ButtonIndexToItemIndex(wParam);
	}
	
	return DownloadAllItem(index);
}

LRESULT CRssFeedWnd::OnSwitchListShowModeClicked(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)
	{
		SwitchList(LISTSTYLE_ICO);
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.m_btIco.EnableWindow(FALSE);
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.m_btItem.EnableWindow(TRUE);
		m_listIcoFeedItem.FilterItemsBySearchKey(NULL); // �л���ǩʱ�ָ�ɸѡǰ���
	}
	else
	{
		SwitchList(LISTSTYLE_ITEM);
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.m_btIco.EnableWindow(TRUE);
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.m_btItem.EnableWindow(FALSE);
		m_listFeedItem.FilterItemsBySearchKey(NULL); // �л���ǩʱ�ָ�ɸѡǰ���
	}

	//�л���ǩʱ ���������
	if (theApp.emuledlg->m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.GetSafeHwnd() != NULL)
		theApp.emuledlg->m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.SwitchSearchEditInfo(1); // 1:Smart Filter

	return 0;
}

LRESULT CRssFeedWnd::OnListInfoClicked(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)//������һ��
	{
		SwitchList(LISTSTYLE_ICO);
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.m_btIco.EnableWindow(FALSE);
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.m_btItem.EnableWindow(TRUE);
	}
	else if(wParam == 2)//ǰһ��
	{
		ClearSingleList();

		if ( m_pFeedPre != NULL )
		{
			JumpListInfo(m_pFeedPre);
		}
	}
	else if(wParam == 3)//��һ��
	{
		ClearSingleList();

		if ( m_pFeedNext != NULL )
		{
			JumpListInfo(m_pFeedNext);
		}
	}

	return 0;
}

BOOL CRssFeedWnd::DownloadAllItem(int nIndex)
{
	int index = nIndex;
	CFeedItemListCtrl::CtrlBase * pCtrl;

	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(m_listFeedItem.GetItemData(index));
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(m_listSingleFeedItem.GetItemData(index));
	}
	else if (m_ListShowMode == LISTSTYLE_ICO)
	{
		//Ϊ����Ӧ���´���,������Ҫ��ICO INDEX->ITEM INDEX ת��
		CRssFeed* pFeed = m_listIcoFeedItem.GetFeed(index);//ȡ��ICO FEED
		index = m_listFeedItem.GetItemIndex(pFeed);//ȡ�õ�ǰFEED��itemList�е�index

		pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(m_listFeedItem.GetItemData(index));
	}

	if ( pCtrl == NULL )
	{
		return FALSE;
	}

	// ����һ����Դ
	if ( CRssFeed::Item * pItem = pCtrl->GetRssFeedItem() )
	{
		DownloadFeedItem(index);
		return TRUE;
	}

	// ����ָ��rss�µ�ȫ����Դ
	if ( CRssFeed * pFeed = pCtrl->GetRssFeed() )
	{		
		for ( CRssFeed::ItemIterator it = pFeed->GetItemIterBegin();
			it != pFeed->GetItemIterEnd();
			++it
			)
		{
			CPartFile* pPartFile = it->second.GetPartFile();
			if( pPartFile != NULL )//��ͣ����
			{
				if (pPartFile->CanResumeFile())
				{
					if (pPartFile->GetStatus() == PS_INSUFFICIENT)
						pPartFile->ResumeFileInsufficient();
					else
						pPartFile->ResumeFile();
				}
			}
			else
			{
				SendDownloadRequestMessage(it->second);
			}
		}
		RedrawFeed(*pFeed);
	}

	return TRUE;
}

BOOL CRssFeedWnd::ResumeAllItem(int nIndex)
{
	int index = nIndex;
	CFeedItemListCtrl::CtrlBase * pCtrl;

	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(m_listFeedItem.GetItemData(index));
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(m_listSingleFeedItem.GetItemData(index));
	}
	else if (m_ListShowMode == LISTSTYLE_ICO)
	{
		//Ϊ����Ӧ���´���,������Ҫ��ICO INDEX->ITEM INDEX ת��
		CRssFeed* pFeed = m_listIcoFeedItem.GetFeed(index);//ȡ��ICO FEED
		index = m_listFeedItem.GetItemIndex(pFeed);//ȡ�õ�ǰFEED��itemList�е�index

		pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(m_listFeedItem.GetItemData(index));
	}

	if ( pCtrl == NULL )
	{
		return FALSE;
	}

	// ����һ����Դ
	if ( CRssFeed::Item * pItem = pCtrl->GetRssFeedItem() )
	{
		DownloadFeedItem(index);
		return TRUE;
	}

	// ����ָ��rss�µ���ͣ/ֹͣ��Ŀ
	if ( CRssFeed * pFeed = pCtrl->GetRssFeed() )
	{		
		for ( CRssFeed::ItemIterator it = pFeed->GetItemIterBegin();
			it != pFeed->GetItemIterEnd();
			++it
			)
		{
			CPartFile* pPartFile = it->second.GetPartFile();
			if( pPartFile != NULL )//��ͣ����
			{
				if (pPartFile->CanResumeFile())
				{
					if (pPartFile->GetStatus() == PS_INSUFFICIENT)
						pPartFile->ResumeFileInsufficient();
					else
						pPartFile->ResumeFile();
				}
			}
		}
		RedrawFeed(*pFeed);
	}

	return TRUE;
}

void CRssFeedWnd::OnLvnItemchangedRwFeedItemList(NMHDR * /* pNMHDR */, LRESULT *pResult)
{
	//LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(pResult != NULL);
	*pResult = 0;

	// ���¹�����
	CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
	UpdateToolBarState(wd.m_Toolbar);

	// ��������
	if( theApp.emuledlg->m_mainTabWnd.m_dlgDownload.IsRemarkTabActived())
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.RefreshRSSComment();
	}

	int index = -1; 
	if (m_ListShowMode == LISTSTYLE_ITEM)
	{
		index = m_listFeedItem.GetCurrSelectIndex();
	}
	else if (m_ListShowMode == LISTSTYLE_SINGLE)
	{
		index = m_listSingleFeedItem.GetCurrSelectIndex();
	}
	
	if ( index >= 0 )	
	{
		if (m_ListShowMode == LISTSTYLE_ITEM)
		{
			if ( CRssFeed::Item * pItem = m_listFeedItem.GetFeedItem(index) )
			{
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateRssItem(pItem);
			}
			else if ( CRssFeed * pFeed = m_listFeedItem.GetFeed(index) )
			{
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateRssFeed(pFeed);
			}
		}
		else if (m_ListShowMode == LISTSTYLE_SINGLE)
		{
			if ( CRssFeed::Item * pItem = m_listSingleFeedItem.GetFeedItem(index) )
			{
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateRssItem(pItem);
			}
			else if ( CRssFeed * pFeed = m_listSingleFeedItem.GetFeed(index) )
			{
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateRssFeed(pFeed);
			}
		}
	}
}

void CRssFeedWnd::SetCurrentList()
{
	SwitchList(m_ListShowMode);
}

void CRssFeedWnd::AutoSwitchListMode()
{
	//if (m_ListShowMode == LISTSTYLE_ICO)
	//	m_ListShowMode = LISTSTYLE_ITEM;
	//else
	//	m_ListShowMode = LISTSTYLE_ICO;

	//SetCurrentList();
}

void CRssFeedWnd::SetToolbar2List(ListShowMode listmode)
{
	if (listmode == LISTSTYLE_ICO)
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_Toolbar.SetOwner(&m_listIcoFeedItem);
	}
	else if (listmode == LISTSTYLE_ITEM)
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_Toolbar.SetOwner(&m_listFeedItem);
	}
	else if (listmode == LISTSTYLE_SINGLE)
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_Toolbar.SetOwner(&m_listSingleFeedItem);
	}
}

void CRssFeedWnd::SwitchList(ListShowMode listmode)
{
	if (listmode == LISTSTYLE_ICO)
	{
		ClearSingleList();//�л���ͼ��ʱ��������Ҫ���singleList�е�����

		m_listFeedItem.ShowWindow(SW_HIDE);
		m_listSingleFeedItem.ShowWindow(SW_HIDE);
		m_wndRssInfo.ShowWindow(SW_HIDE);
		m_listIcoFeedItem.ShowWindow(SW_SHOW);

		m_listIcoFeedItem.SetFocus();

		m_ListShowMode = LISTSTYLE_ICO;
	}
	else if (listmode == LISTSTYLE_ITEM)
	{
		ClearSingleList();//�л���ͼ��ʱ��������Ҫ���singleList�е�����

		m_listIcoFeedItem.ShowWindow(SW_HIDE);
		m_listSingleFeedItem.ShowWindow(SW_HIDE);
		m_wndRssInfo.ShowWindow(SW_HIDE);
		m_listFeedItem.ShowWindow(SW_SHOW);

		m_listFeedItem.SetFocus();

		m_ListShowMode = LISTSTYLE_ITEM;
	}
	else if (listmode == LISTSTYLE_SINGLE)
	{
		m_listIcoFeedItem.ShowWindow(SW_HIDE);
		m_listFeedItem.ShowWindow(SW_HIDE);
		m_listSingleFeedItem.ShowWindow(SW_SHOW);
		m_wndRssInfo.ShowWindow(SW_SHOW);

		m_listSingleFeedItem.SetFocus();

		m_ListShowMode = LISTSTYLE_SINGLE;
	}

	SetToolbar2List(m_ListShowMode);

	CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
	UpdateToolBarState(wd.m_Toolbar);//����Toolbar״̬
}

void CRssFeedWnd::ClearSingleList()
{
	if ( m_listSingleFeedItem.GetItemCount() > 0 )
	{
		CRssFeed* OriginalFeed = m_listSingleFeedItem.GetFeed(0);
		if ( OriginalFeed != NULL )
		{
			m_listSingleFeedItem.RemoveFeed(*OriginalFeed);
		}
	}
}

void CRssFeedWnd::GetCurFeedInfo(CRssFeed * pCurFeed)
{
	if (pCurFeed == NULL)
		return;

	if ( pCurFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
	{
		//���������
		m_pFeedPre = NULL;
		m_pFeedNext = NULL;

		m_strGroupTitle = _T("");
		m_nCurrFeedCatalog = 0;
		m_nFeedCatalogCount = 0;

		//��ǰ���ӷ���,����ŷ����Ӱ
		FeedCatalog & catalog = CFeedCatalogs::GetInstance()[pCurFeed->m_uCatalogId];
		ASSERT(catalog.m_uId == pCurFeed->m_uCatalogId);
		
		if ( catalog.IsLeaf() )
		{
			//��ǰ�ĸ�����,�����Ӱ\����
			FeedCatalog * catalogParent = catalog.m_pParent;
			m_strGroupTitle = catalogParent->m_strName;

			int nCurrFeedCatalog = 0;
			m_nFeedCatalogCount = catalogParent->GetFeedCount();//������
			//m_nFeedCatalogCount = m_listIcoFeedItem.GetGroupCounts(m_strGroupTitle);

			if ( catalogParent->IsTop() && m_nFeedCatalogCount > 0 )
			{
				//��ʶ�Ƿ��Ѿ�ƥ���
				bool bMatched = false;
				//��ʶ�Ƿ�ƥ�����,Ϊtrue��ʾ�����˳�ѭ��
				bool bAllMatch = false;

				//��ʱ��סǰһ��feed
				CRssFeed * pFeedPre = NULL;

				for ( FeedCatalog::Childs::const_iterator it = catalogParent->m_setChilds.begin();
					it != catalogParent->m_setChilds.end();
					++it	)
				{
					FeedCatalog * pLeaf = *it;
					ASSERT(pLeaf != NULL);
					for ( CRssFeedSet::iterator itFeed = pLeaf->m_setFeeds.begin();
						itFeed != pLeaf->m_setFeeds.end();
						++itFeed	)
					{
						CRssFeedBase* pFeedBase = *itFeed;
						CRssFeed* pFeed = (CRssFeed*)pFeedBase;
						if ( pFeed != NULL )
						{
							nCurrFeedCatalog++;
							if ( !bMatched )
							{
								if ( pFeed == pCurFeed )//ƥ��ɹ�
								{
									m_nCurrFeedCatalog = nCurrFeedCatalog;//��¼��ǰλ�� 5/10
									m_pFeedPre = pFeedPre;//������¼ǰһ��
									bMatched = true;
								}
								else
									pFeedPre = (CRssFeed*)pFeed;//��ʱ��¼ǰһ��
							}
							else
							{
								m_pFeedNext = (CRssFeed*)pFeed;//��¼��һ��
								bAllMatch = true;
								break;
							}
						}
					}

					if (bAllMatch)
					{//�˳�ѭ��
						break;
					}
				}
			}
		}
	}
}

void CRssFeedWnd::JumpListInfo(CRssFeed * pFeed, ListShowMode listmode/* = LISTSTYLE_SINGLE*/)
{
	if (listmode == LISTSTYLE_SINGLE)
	{
		SwitchList(listmode);

		m_listSingleFeedItem.AddFeed(*pFeed, FALSE);
		m_listSingleFeedItem.SwitchToStat(0, TRUE);//չ��

		GetCurFeedInfo(pFeed);//��ȡ��ǰfeedǰ����Ϣ
		BOOL bHasPre = !(m_pFeedPre == NULL);
		BOOL bHasNext = !(m_pFeedNext == NULL);

		//����ǰ��ť״̬
		m_wndRssInfo.m_btPre.EnableWindow(bHasPre);
		m_wndRssInfo.m_btNext.EnableWindow(bHasNext);

		m_wndRssInfo.SetNumData(m_strGroupTitle, pFeed->GetDisplayName(), m_nCurrFeedCatalog, m_nFeedCatalogCount);

		m_wndRssInfo.Invalidate(TRUE);

		//����2��RSS��ť״̬
		//theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd.m_BtWnd.ShowWindow(SW_HIDE);

		//ת��Item�б�,��ʱ����
		//int nIndex = m_listFeedItem.GetItemIndex( pFeed );
		//if (nIndex != -1)
		//{
		//	m_listFeedItem.ScrollIndex2Top(nIndex);//����
		//	m_listFeedItem.SetItemState(nIndex, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		//	m_listFeedItem.SwitchToStat(nIndex, TRUE);//չ��

		//	m_listFeedItem.SetFocus();
		//}
	}
}

BOOL CRssFeedWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (VK_ESCAPE == pMsg->wParam)
			return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}
