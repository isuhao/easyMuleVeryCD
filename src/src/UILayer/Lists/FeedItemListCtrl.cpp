/*
 * $Id: FeedItemListCtrl.cpp 20807 2010-11-16 11:43:07Z huhonggang $
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

// FeedItemListCtrl.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "FeedItemListCtrl.h"
#include "emule.h"
#include ".\feeditemlistctrl.h"
#include "ximage.h"
#include <MemDC.h>
#include "MenuCmds.h"
#include "MenuXP.h"
#include "TitleMenu.h"
#include "DlgMainTabRss.h"
#include "CmdFuncs.h"
#include "emuleDlg.h"
#include "RssFeedWnd.h"
#include "DlgFeedConfig.h"

// CFeedItemListCtrl

#define IDC_BUTTON_ID 0x1235 //���ذ�ťID

// xpϵͳ���ô���ʽ
#if !defined(LVS_EX_DOUBLEBUFFER)
#	define LVS_EX_DOUBLEBUFFER     0x00010000
#endif

#if !defined(LVN_ENDSCROLL)
#	define LVN_ENDSCROLL            (LVN_FIRST-81)
	typedef struct tagNMLVSCROLL
	{
		NMHDR   hdr;
		int     dx;
		int     dy;
	} NMLVSCROLL, *LPNMLVSCROLL;
#endif

static const int COL_INDEX_COVER			= 0;
static const int COL_INDEX_TITLE			= 1;
static const int COL_INDEX_SIZE				= 2;
static const int COL_INDEX_STATUS			= 3;
//static const int COL_INDEX_CATEGORY			= 4;
static const int COL_INDEX_DATE				= 4;
//static const int COL_INDEX_FEED				= 5;
//static const int COL_NUM					= 6;

CTimeSpan CFeedItemListCtrl::s_timespan = 0;

struct CREATE_FEED_ITEM_COL
{
	CREATE_FEED_ITEM_COL(UINT uIndex, LPCTSTR uTitle, UINT uFormat, UINT uWidth)
		:
		m_uIndex(uIndex), 
		m_strTitle(uTitle), 
		m_uFormat(uFormat),
		m_uWidth(uWidth)
	{

	}

	UINT		m_uIndex;
	CString		m_strTitle;
	UINT		m_uFormat;
	UINT		m_uWidth;
};

CREATE_FEED_ITEM_COL s_CreateParam[] = 
{
	CREATE_FEED_ITEM_COL(COL_INDEX_COVER,	GetResString(IDS_RW_FEED_COVER), LVCFMT_LEFT, CFeedItemListCtrl::COVER_EXPAND_WIDTH),
	CREATE_FEED_ITEM_COL(COL_INDEX_TITLE,	GetResString(IDS_RSSLIST_RES), LVCFMT_LEFT, 760),
	CREATE_FEED_ITEM_COL(COL_INDEX_SIZE,	GetResString(IDS_RSSLIST_LENGTH), LVCFMT_RIGHT, 85),
	CREATE_FEED_ITEM_COL(COL_INDEX_STATUS,	GetResString(IDS_RSSLIST_STATE), LVCFMT_CENTER, 85),
	CREATE_FEED_ITEM_COL(COL_INDEX_DATE,	GetResString(IDS_RSSLIST_DATE), LVCFMT_CENTER, 140),
	// 		CREATE_FEED_ITEM_COL(COL_INDEX_CATEGORY, _T("���"), LVCFMT_LEFT, 100),
	// 		CREATE_FEED_ITEM_COL(COL_INDEX_FEED, _T("����"), LVCFMT_LEFT, 200)
};


IMPLEMENT_DYNAMIC(CFeedItemListCtrl, CMuleListCtrl)

//CString CFeedItemListCtrl::s_strFeedStatStr[CRssFeed::fsMaxState];
CString CFeedItemListCtrl::s_strFeedItemStatStr[CRssFeed::isMaxState];

CFeedItemListCtrl::CFeedItemListCtrl()
	:
	m_bCanModifyCoverColumn(TRUE)
{
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);	
	s_timespan = - (tzi.Bias * 60);
}

CFeedItemListCtrl::~CFeedItemListCtrl()
{

}

// ��ʼ��
void CFeedItemListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	if (m_bSingleStyle)
	{
		strRes = GetResString(IDS_RW_FEED_COVER);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(COL_INDEX_COVER, &hdi);

		this->SetColumnWidth(COL_INDEX_COVER, COVER_EXPAND_WIDTH);
	}

	strRes = GetResString(IDS_RSSLIST_RES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(COL_INDEX_TITLE, &hdi);

	strRes = GetResString(IDS_RSSLIST_LENGTH);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(COL_INDEX_SIZE, &hdi);


	strRes = GetResString(IDS_RSSLIST_STATE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(COL_INDEX_STATUS, &hdi);


	strRes = GetResString(IDS_RSSLIST_DATE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(COL_INDEX_DATE, &hdi);

	//s_strFeedStatStr[CRssFeed::fsNormal]			= GetResString(IDS_RSSSTATE_ENABLE);//_T("����");
	//s_strFeedStatStr[CRssFeed::fsDownloading]		= GetResString(IDS_RSSSTATE_DOWNLOADING);//_T("������");
	//s_strFeedStatStr[CRssFeed::fsDisable]			= GetResString(IDS_RSSSTATE_DISABLE);//_T("����");

	s_strFeedItemStatStr[CRssFeed::isRss]			= GetResString(IDS_RSSSTATE_NEWITEM);//_T("δ����");
	s_strFeedItemStatStr[CRssFeed::isDownloading]	= GetResString(IDS_RSSSTATE_DOWNLOADING);//_T("������");
	s_strFeedItemStatStr[CRssFeed::isDownloaded]	= GetResString(IDS_RSSSTATE_COMPLETE);//_T("������");
	s_strFeedItemStatStr[CRssFeed::isPaused]		= GetResString(IDS_RSSSTATE_PAUSE);//_T("����ͣ");	
	s_strFeedItemStatStr[CRssFeed::isStopped]		= GetResString(IDS_RSSSTATE_STOP);//_T("��ֹͣ");	
	s_strFeedItemStatStr[CRssFeed::isHistory]		= GetResString(IDS_RSSSTATE_DELETED);//_T("δ����"); // _T("�Ѷ���");

	// ��������ͷ
	DrawCoverColumnHead(IsCoverExpand());

	this->Invalidate();
}

void CFeedItemListCtrl::Init(BOOL bSingle)
{
	SetName(_T("RSSListCtrl"));

	InitFeedListView();

	m_imageRSS_Normal = GetPng(_T("PNG_RSSNORMAL"));
	m_imageRSS_Downloading = GetPng(_T("PNG_RSSDOWNLOADING"));
	m_imageRSS_Disable = GetPng(_T("PNG_RSSDISABLE"));
	m_imageRSS_Error = GetPng(_T("PNG_RSSERROR"));
	m_imageRSS_Update = GetPng(_T("PNG_RSSUPDATE"));

	m_imageTask_Complete = GetPng(_T("PNG_RSSTASK_COMPLETE"));
	m_imageTask_Pause = GetPng(_T("PNG_RSSTASK_PAUSE"));
	m_imageTask_Error = GetPng(_T("PNG_RSSTASK_ERROR"));
	m_imageTask_Stop = GetPng(_T("PNG_RSSTASK_STOP"));
	m_imageTask_Downloading1 = GetPng(_T("PNG_RSSTASK_DOWNLOADING1"));
	m_imageTask_Downloading2 = GetPng(_T("PNG_RSSTASK_DOWNLOADING2"));
	m_imageTask_Downloading3 = GetPng(_T("PNG_RSSTASK_DOWNLOADING3"));
	m_imageRss_PosterBg = GetPng(_T("PNG_RSS_POSTER_BG"));
	m_imageRss_PosterDefault = GetPng(_T("PNG_RSS_POSTER_DEFAULT"));
	m_imageSubTitle_Green = GetPng(_T("PNG_SUBTITLE_GREEN"));
	m_imageSubTitle_Grey = GetPng(_T("PNG_SUBTITLE_GREY"));

	m_ButtonID = IDC_BUTTON_ID;

	m_FontBold.CreateFont(12,0,0,0,FW_BOLD,FALSE,FALSE,0,0,0,0,0,0,_TEXT("tahoma"));//���� 

	m_nIcoState = 1;
	m_pMenuXP = NULL;
	m_bSingleStyle = bSingle;

	LoadSettings(TRUE);

	Localize();

	SetTimer(TIMER_ID, 500, NULL);
}

// չ��/�۵�feed
void CFeedItemListCtrl::ExpandFeed(int index, CRssFeed & feed, BOOL bExpand, BOOL bSetRedraw)
{
	if ( bSetRedraw )
	{
		SetRedraw(FALSE);
	}

	if ( bExpand )
	{
		// չ��
		AddFeedItems(index + 1, feed);
	}
	else
	{
		// �۵�
		int iCount = GetItemCount();
		int iDelIndex = index + 1;
		for ( int iDelCount = iDelIndex; iDelCount < iCount; ++iDelCount )
		{
			if ( GetFeedItem(iDelIndex) != NULL )
			{
				// Ϊ����Ŀ��ɾ��
				DeleteItem(iDelIndex);
			}
			else
			{
				// feed���˽���
				break;
			}
		}
	}

	// ���ָ��������
	FillBlankLine(index);

	//RedrawAllButton();
	// �ػ�feed���ڵ���
	RedrawItems(index, index);

	if ( bSetRedraw )
	{
		SetRedraw(TRUE);
	}
}

// ���ݿ�����Ϣȡ���к�
int CFeedItemListCtrl::GetItemIndex(const CtrlBase * pCtrl)
{
	int iCount = GetItemCount();
	for ( int i = 0; i < iCount; ++i )
	{
		if ( reinterpret_cast<CtrlBase *>(GetItemData(i)) == pCtrl )
		{
			return i;
		}
	}

	// û���ҵ�
	return -1;
}

int CFeedItemListCtrl::GetItemIndex(const CRssFeed * pFeed)
{
	int iCount = GetItemCount();
	for ( int i = 0; i < iCount; ++i )
	{
		CtrlBase * pCtrlfeed = reinterpret_cast<CtrlBase *>(GetItemData(i));
		if ( pCtrlfeed != NULL && pCtrlfeed->GetRssFeed() == pFeed )
		{
			return i;
		}
	}

	//// û���ҵ�
	return -1;
}

// ����feedȡ��CtrlFeed
CFeedItemListCtrl::CtrlFeed * CFeedItemListCtrl::GetCtrlFeed(const CRssFeed * pFeed)
{
	int iCount = GetItemCount();
	for ( int i = 0; i < iCount; ++i )
	{
		if ( CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(i)) )
		{
			if ( pCtrl->GetRssFeed() == pFeed )
			{
				return static_cast<CtrlFeed *>(pCtrl);
			}
		}
	}

	// û���ҵ�
	return NULL;
}

// ������ʾһ��feed����item��Ϣ
void CFeedItemListCtrl::RedrawFeed(CRssFeed & feed)
{
	int iCount = GetItemCount();
	for ( int i = 0; i < iCount; ++i )
	{
		CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(i));
		if ( pCtrl == NULL )
		{
			continue;
		}

		CRssFeed * pFeed = pCtrl->GetRssFeed();
		if ( pFeed == &feed )
		{
			// �ҵ�feed���ڵ���
			if ( static_cast<CtrlFeed*>(pCtrl)->m_bExpand )
			{
				// չ��״̬���ػ���������Ŀ
				SetRedraw(FALSE);
				ExpandFeed(i, feed, FALSE, FALSE);
				ExpandFeed(i, feed, TRUE, FALSE);
				RedrawAllButton();
				SetRedraw(TRUE);
			}
			else
			{
				// �۵�״̬��ֻ��Ҫ�ػ�feed���ڵ���
				RedrawItems(i, i);
			}
			break;
		}
	}
	
}

// ȡ�õ�ǰҳ����β��
BOOL CFeedItemListCtrl::GetPageRange(int & iFrom, int & iEnd)
{
	iFrom = GetTopIndex();
	iEnd = GetItemCount() - 1;
	if ( iEnd < 0 )
	{
		return FALSE;
	}
	
	int iBottomIndex = iFrom + GetCountPerPage2() - 1;
	if ( iBottomIndex < iEnd )
	{
		iEnd = iBottomIndex;
	}
	return TRUE;
}


// ������ʾһ��feed item
void CFeedItemListCtrl::RedrawFeedItem(CRssFeed::Item & item)
{
	int iFrom, iEnd;
	if ( !GetPageRange(iFrom, iEnd) )
	{
		return;
	}

	for ( ; iFrom <= iEnd; ++iFrom )
	{
		if ( CRssFeed::Item * pFilmItem = GetFeedItem(iFrom) )
		{
			// �ҵ�ӰƬ����Ļ
			if ( pFilmItem == &item ||
				 pFilmItem->m_pAttachItem == &item
			   )
			{
				//HDWP hWinPosInfo = NULL;
				//DrawDownloadButton(iFrom, TRUE, hWinPosInfo);
				//RedrawItems(iFrom, iFrom);
				RedrawFeedItem(iFrom);
				return;
			}
		}
	}

}

// ������ʾһ��feed item
void CFeedItemListCtrl::RedrawFeedItem(int index)
{
	// ����Ϊ��Ŀ	
	HDWP hWinPosInfo = NULL;
	DrawDownloadButton(index, 
					   reinterpret_cast<CtrlBase *>(GetItemData(index)), 
					   hWinPosInfo
					  );
	RedrawItems(index, index);
}


// ������ʾfeed������������Ŀ��
void CFeedItemListCtrl::RedrawFeedLine(CRssFeedBase * pFeed)
{
	int iFrom, iEnd;
	if ( !GetPageRange(iFrom, iEnd) )
	{
		return;
	}

	for ( ; iFrom <= iEnd; ++iFrom )
	{
		if ( CRssFeed * pCurFeed = GetFeed(iFrom) )
		{
			if ( pFeed == NULL )
			{
				// ˢ������feed
				RedrawItems(iFrom, iFrom);
			}
			else if ( pCurFeed == pFeed )
			{
				// ˢ��ָ����feed
				RedrawItems(iFrom, iFrom);
				return;
			}
		}
	}
}


// ����feed��Դ��Ŀ״̬ȡ�ö�Ӧ��CxImage
CxImage * CFeedItemListCtrl::GetFeedItemCxImage(const CRssFeed::Item & item)
{
	switch ( item.GetState() )
	{
		case CRssFeed::isRss:			// δ����
			return NULL;
			//return m_imageTask_Error;

		case CRssFeed::isDownloading:	// �Ѷ��ģ�partfile������
			{
				switch (m_nIcoState)
				{
					case 1: 
						return m_imageTask_Downloading1;

					case 2: 
						return m_imageTask_Downloading2;

					case 3: 
						return m_imageTask_Downloading3;

					default: 
						return m_imageTask_Downloading1;
				}
			}
		case CRssFeed::isPaused:		// �Ѷ��ģ�partfile����ͣ
			return m_imageTask_Pause;
		case CRssFeed::isStopped:		// �Ѷ��ģ�partfile��ֹͣ
			return NULL;//return m_imageTask_Stop;
		case CRssFeed::isDownloaded:	// �Ѷ��ģ�partfile�������
			return NULL;//return m_imageTask_Complete;
		case CRssFeed::isHistory:		// �Ѷ��ģ���partfile�Ѿ���ɾ��
			return NULL;//return m_imageTask_Error;

		default:
			// ��Ӧ�ó��ִ����
			ASSERT(FALSE);
			return NULL;//return m_imageTask_Error;
	}
}

CxImage* CFeedItemListCtrl::GetPng(LPCTSTR lpszPngResource)
{
	if (NULL == lpszPngResource)
		return NULL;

	CxImage* image = new CxImage(CXIMAGE_FORMAT_PNG);
	image->LoadResource(FindResource(NULL, lpszPngResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
	return image;
}

BOOL CFeedItemListCtrl::IsResExit()
{
	if (!m_imageRSS_Normal || !m_imageRSS_Disable || !m_imageRSS_Downloading || !m_imageRSS_Error || !m_imageRSS_Update)
	{
		return FALSE;
	}

	if ( !m_imageTask_Complete || !m_imageTask_Error || !m_imageTask_Pause || !m_imageTask_Stop ||
		!m_imageTask_Downloading1 || !m_imageTask_Downloading2 || !m_imageTask_Downloading3 || 
		!m_imageSubTitle_Green || !m_imageSubTitle_Grey )
	{
		return FALSE;
	}

	return TRUE;
}

// ��ʼ��ImageList
// void CFeedItemListCtrl::InitImageList()
// {
// 	int cx = GetSystemMetrics(SM_CXSMICON);
// 	int cy = GetSystemMetrics(SM_CYSMICON);
// 	//COLORREF crMask = RGB(0, 0, 0);
// 	m_ilsItemImage.Create(cx, cy, ILC_COLOR | ILC_MASK, 0, 1);
// 	//m_ilsItemImage.SetBkColor(CLR_NONE);
// 	m_ilsItemImage.Add(theApp.LoadIcon(_T("SHAREDFILES"), cx, cy));
// 	m_ilsItemImage.Add(theApp.LoadIcon(_T("FILTERCLEAR2"), cx, cy));
// }

// ��ʼ��feed�б���Ŀ
void CFeedItemListCtrl::InitFeedListView()
{
	ModifyStyle(0, WS_CLIPCHILDREN);
	DWORD dwExtStyle = 0;
	//DWORD dwVersion = ::GetVersion();
	//DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	//DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	//if ( dwWindowsMajorVersion > 5 || 
	//	 (dwWindowsMajorVersion == 5 && dwWindowsMinorVersion >= 1) 
	//   )
	//{
	//	dwExtStyle |= LVS_EX_DOUBLEBUFFER;
	//}
	dwExtStyle |= LVS_EX_FULLROWSELECT /*| LVS_EX_FLATSB | LVS_EX_ONECLICKACTIVATE */;
	SetExtendedStyle(GetExtendedStyle() | dwExtStyle);

	for ( int i = 0; i < sizeof(s_CreateParam) / sizeof(s_CreateParam[0]); ++i )
	{
		InsertColumn(s_CreateParam[i].m_uIndex, 
					 s_CreateParam[i].m_strTitle,
					 s_CreateParam[i].m_uFormat, 
					 s_CreateParam[i].m_uWidth
				    );
	}
}

// ���²���feed
void CFeedItemListCtrl::ReAddFeed(CRssFeed & feed)
{
	if ( CtrlFeed * pCtrlFeed = GetCtrlFeed(&feed) )
	{
		BOOL bNew = pCtrlFeed->m_bNew;
		BOOL bExpand = pCtrlFeed->m_bExpand;
		RemoveFeed(feed);
		int iNewIndex = AddFeed(feed, bNew);
		if ( bExpand )
		{
			// ����ȡ��������кţ���չ��
			ASSERT(iNewIndex == GetItemIndex(pCtrlFeed));
			SwitchExpandStat(iNewIndex);
		}
	}

}

// ��Ӷ���
int CFeedItemListCtrl::AddFeed(int index, CRssFeed & feed, BOOL bIsNew)
{
	CtrlFeed * pCtrl = new CtrlFeed;
	if ( pCtrl == NULL )
	{
		return -1;
	}

	pCtrl->m_pFeed = &feed;
	pCtrl->m_bExpand = false;
	pCtrl->m_bNew = (bIsNew != FALSE);

	SetRedraw(FALSE);

	index = InsertItem(LVIF_PARAM, index, NULL, 0, 0, 0,(LPARAM)pCtrl);

	// ������
	FillBlankLine(index);

	SetRedraw(TRUE);

	//RedrawAllButton();
	//HDWP hWinPosInfo = NULL;
	//DrawDownloadButton(index, FALSE, hWinPosInfo);

	//CRect rect;
	//GetSubItemRect(index, 0, LVIR_LABEL, rect);
	//if ( !rect.IsRectEmpty() )
	//{
	//	AddDownButton(index, TRUE, rect, *pCtrl);
	//}

	return index;
}

// ɾ������
void CFeedItemListCtrl::RemoveFeed(CRssFeed & feed)
{
	int iCount = GetItemCount();
	for ( int i = 0; i < iCount; ++i )
	{
		if ( GetFeed(i) == &feed )
		{
			m_stringMatcher.RemoveItem((void*)&feed);

			// �ҵ�feed��ɾ��������Ŀ���۵�������Ŀ�Ϳ�ɾ����
			SetRedraw(FALSE);
			ExpandFeed(i, feed, FALSE, FALSE);
			DeleteBlankLine(i);
			DeleteItem(i);
			SetRedraw(TRUE);
			if ( GetItemCount() <= 0 )
			{
				// ����Ŀ��ȫ��ɾ��
				DeleteAllButtons();
			}
			return;
		}
	}
}

// ��ť��תitem index
int CFeedItemListCtrl::ButtonIndexToItemIndex(int iButtonIndex)
{
	return iButtonIndex + GetTopIndex();
}

// ȡ�ÿ���ʾ����
int CFeedItemListCtrl::GetCountPerPage2()
{
	int iItemCount = GetItemCount();
	if ( iItemCount < 0 )
	{
		return GetCountPerPage();
	}

	ASSERT(GetHeaderCtrl() != NULL);

	CRect rectList, rectHeader;
	GetClientRect(rectList);
	GetHeaderCtrl()->GetClientRect(rectHeader);
	int iHeight = rectList.Height() - rectHeader.Height();
	int iResult = GetCountPerPage();
	if ( iResult * ITEM_HEIGHT < iHeight )
	{
		int iBottomIndex = GetTopIndex() + iResult - 1;
		int iLastIndex = iItemCount - 1;
		if ( iLastIndex > iBottomIndex )
		{
			iResult++;
		}
	}
	return iResult;
}

// ɾ��ȫ����ť
void CFeedItemListCtrl::DeleteAllButtons()
{
	for ( size_t i = 0; i < m_DownloadButtons.size(); ++i )
	{
		delete m_DownloadButtons[i];
	}

	m_DownloadButtons.clear();
}

void CFeedItemListCtrl::GetSubtitleRect(CRect & rectSubTitle , int index )
{
	//	POINT pt;
	//	int iItemIndex = GetClickItemIndex(&pt);
	int nIndex = index;
	if (index == -1)
	{
		nIndex = GetCurrSelectIndex();
	}

	if ( CRssFeed::Item* pItem = GetFeedItem(nIndex) )
	{
		int nStrWidth = GetStringWidth(pItem->m_strFilename);//���ⳤ��

		CRect rectButton;//���ذ�ťRECT
		GetDownloadButtonRect(nIndex,TRUE,rectButton);
		CRect rectSubItem;//��һ��RECT
		BOOL bResult = GetSubItemRect(nIndex, COL_INDEX_TITLE, LVIR_LABEL, rectSubItem);

		if ( !bResult || !rectButton )
			return;

		//21=��Ļͼ�곤��
		rectSubTitle.SetRect(rectSubItem.left+34+nStrWidth+10,rectSubItem.top,rectSubItem.left+34+nStrWidth+10+21,rectSubItem.bottom);

		if ( rectButton.left - 20 - 21 - 20 - (rectSubItem.left + 48 + nStrWidth) < 0 )
		{
			rectSubTitle.SetRect(rectButton.left -20-21,rectSubItem.top,rectButton.left-20,rectSubItem.bottom);
			if (rectButton.left-20-21 - (rectSubItem.left+34) < 0)
			{
				rectSubTitle.SetRect(rectSubItem.left + 34,rectSubItem.top,rectButton.left-20,rectSubItem.bottom);
				if ( rectButton.right - (rectSubItem.left + 34) < 0 )
				{
					rectSubTitle.SetRect(0,0,0,0);
				}
			}
		}
	}
	else
		return;
}

void CFeedItemListCtrl::GetCompleteNumRect(int index, CString strCompleteNum, CRect & rectCompleteNum)
{
	if (index < 0 || strCompleteNum.IsEmpty())
		return;

	int nIndex = index;

	if ( CRssFeed* pFeed = GetFeed(nIndex) )
	{
		int nItemTextWidth = GetStringWidth(pFeed->m_strTitle);//���ⳤ��

		CRect rectButton;//���ذ�ťRECT
		GetDownloadButtonRect(nIndex,TRUE,rectButton);
		CRect rectSubItem;//��һ��RECT
		BOOL bResult = GetSubItemRect(nIndex, COL_INDEX_TITLE, LVIR_LABEL, rectSubItem);

		if ( !bResult || !rectButton )
			return;

		int nCompleteNumWidth = GetStringWidth(strCompleteNum);
		int nCharNum = strCompleteNum.GetLength();//����֮��ļ��

		rectCompleteNum.SetRect(rectSubItem.left+34+nItemTextWidth+10, rectSubItem.top,
												  rectSubItem.left+34+nItemTextWidth+10+nCompleteNumWidth+6+nCharNum, rectSubItem.bottom);//Բ�Ǿ���������������������2*3=6

		if (rectButton.left-30 - ( rectSubItem.left+34+nItemTextWidth+10+(nCompleteNumWidth+6+nCharNum)) < 0 )//Խ��ItemText���Ҷ�
		{
			rectCompleteNum.SetRect(rectButton.left-15-(nCompleteNumWidth+6+nCharNum)-20,rectSubItem.top,rectButton.left-35,rectSubItem.bottom);
			if (rectCompleteNum.left-30 - rectSubItem.left-4 < 0)//Խ��ItemText�����
			{
				rectCompleteNum.SetRect( rectSubItem.left+34,rectSubItem.top,rectSubItem.left+34+(nCompleteNumWidth+6+nCharNum),rectSubItem.bottom);
				if (rectCompleteNum.right - rectButton.right > 0)
				{
					rectCompleteNum.SetRect(0,0,0,0);
				}
			}
		}
	}
}

// ��ʾ���ذ�ť
BOOL CFeedItemListCtrl::DrawDownloadButton(int index, CtrlBase * pCtrl, HDWP & hWinPosInfo)
{
	int iItemCount = GetItemCount();
	int iCountPerPage = GetCountPerPage2();
	int iTopIndex = GetTopIndex();
	int iBottomIndex = iTopIndex + iCountPerPage - 1;
	if ( index < iTopIndex || index > iBottomIndex )
	{
		return FALSE;
	}

	if ( index >= iItemCount )
	{
		return FALSE;
	}

	CRssFeed::Item * pItem = (pCtrl != NULL ? pCtrl->GetRssFeedItem() : NULL);

	// ɾ������İ�ť
	{
		int iMaxShowCount = (iCountPerPage < iItemCount ? iCountPerPage : iItemCount);
		while ( int(m_DownloadButtons.size()) > iMaxShowCount )
		{
			int iLast = m_DownloadButtons.size() - 1;
			if ( iLast >= 0 )
			{
				delete m_DownloadButtons[iLast];
				m_DownloadButtons.resize(iLast);
			}
		}
	}

	// ��ʾ��ť
	CString strCaption;
	LPCTSTR lpszImageName = NULL;
	if ( pItem != NULL )
	{
		strCaption = GetResString(IDS_RSSBTN_DL);
		lpszImageName = _T("PNG_RSSDOWNLOAD");
	}
	else
	{
		strCaption = GetResString(IDS_RSSBTN_ALLDL);
		lpszImageName = _T("PNG_RSSDOWNLOAD_ALL");
	}

	CRect rectButton;
	GetDownloadButtonRect(index, (pItem != NULL), rectButton);

	index -= iTopIndex;
	if ( index >= int(m_DownloadButtons.size()) )
	{
		DWORD dwStyle =  WS_CHILD | WS_VISIBLE;
		CButtonST * pButtonST = new CButtonST();
		if ( pButtonST == NULL )
		{
			return FALSE;
		}
		m_ButtonID++;
		
		pButtonST->Create(strCaption, dwStyle, rectButton, this, m_ButtonID);
		pButtonST->SetOwner(this->GetOwner());
		pButtonST->SetExtendData(index);
		pButtonST->SetTransparent(RGB(255,0,255));
		HCURSOR hCursor = NULL;
		hCursor = ::LoadCursor(NULL, IDC_HAND);
		pButtonST->SetBtnCursor(hCursor);
		pButtonST->SetTextColor(RGB(0x40,0x40,0x40));
		pButtonST->SetTextAlign(3);
		pButtonST->SetBitmaps(lpszImageName, 4, 2);

		m_DownloadButtons.resize(index + 1);
		m_DownloadButtons[index] = pButtonST;
	}
	
	CButtonST * pButtonST = m_DownloadButtons[index];
	ASSERT(pButtonST != NULL);	
	CString strCurCaption;
	pButtonST->GetWindowText(strCurCaption);
	if ( strCurCaption != strCaption )
	{
		pButtonST->SetWindowText(strCaption);
		pButtonST->SetBitmaps(lpszImageName, 4, 2);
	}

	BOOL bVisibled = (pItem != NULL ? pItem->CanDownload() : (pCtrl != NULL));

	if ( hWinPosInfo == NULL )
	{
		if ( bVisibled )
		{
			pButtonST->MoveWindow(&rectButton, TRUE);
			pButtonST->ShowWindow(SW_SHOW);
			pButtonST->RedrawWindow();
		}
		else
		{
			pButtonST->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		hWinPosInfo =
			DeferWindowPos(hWinPosInfo,
						   pButtonST->GetSafeHwnd(),
						   HWND_TOP,
						   rectButton.left,
						   rectButton.top,
						   rectButton.Width(),
						   rectButton.Height(),
						   SWP_NOZORDER | (bVisibled ? SWP_SHOWWINDOW : SWP_HIDEWINDOW)
						  );
	}
	
	return TRUE;
}

// ���feed��������Ŀ
int CFeedItemListCtrl::AddFeedItems(int index, CRssFeed & feed)
{
	if ( feed.GetItemCount() <= 0 )
	{
		return index;
	}

	int iFromIndex = index;

	//CRssFeed::ItemConstIterator itLast = (--feed.GetItemEnd());
	//for ( CRssFeed::ItemConstIterator it = feed.GetItemBegin(); 
	//	  it != feed.GetItemEnd();
	//	  ++it, ++index
	//	)
	//{
	//	AddFeedItem(index, it->second, it == itLast);
	//}

	// ��ΪĬ�ϰ������ڽ�������
	CRssFeed::ItemArray items;
	feed.GetOrderedItems(items, CRssFeed::CompareItemWithDateD);
	CRssFeed::ItemArray::iterator itLast = (--items.end());
	for ( CRssFeed::ItemArray::iterator it = items.begin(); 
		  it != items.end();
		  ++it
		)
	{
		// ��ӷ���Ļ��Դ
		CRssFeed::Item * pItem = *it;
		ASSERT(pItem != NULL);
		if ( !pItem->IsSubtitle() )
		{
			AddFeedItem(index, *pItem, it == itLast);
			++index;
		}
	}

	// �������ñ��
	ResetIsLastFlag(iFromIndex);

	return index;
}

// ���feed��һ����Ŀ
void CFeedItemListCtrl::AddFeedItem(int index, const CRssFeed::Item & item, BOOL bIsLast)
{
	CtrlFeedItem * pCtrlItem = new CtrlFeedItem;
	if ( pCtrlItem == NULL )
	{
		return;
	}

	pCtrlItem->m_pFeedItem = const_cast<CRssFeed::Item *>(&item);
	pCtrlItem->m_bIsLast = (bIsLast != FALSE);
	InsertItem(LVIF_PARAM, index, NULL, 0, 0, 0,(LPARAM)pCtrlItem);

	//CRect rect;
	//GetSubItemRect(index, 0, LVIR_LABEL, rect);
	//if ( !rect.IsRectEmpty() )
	//{
	//	AddDownButton(index, FALSE, rect, *pCtrlItem);
	//}

	//InsertItem(index, _T(""), GetFeedItemImageIndex(item));
	//SetItemData(index, reinterpret_cast<DWORD_PTR>(&item));
	//for ( int i = 0; i < COL_NUM; ++i )
	//{
	//	SetItemText(index, i, GetFeedItemText(i, item));
	//}
}

/// �����������ʵ���������ʾ����
void CFeedItemListCtrl::FillBlankLine(int index)
{
	// �����۵��򲻼ӿ���
	if ( !IsCoverExpand() )
	{
		return;
	}

	int iCount = GetItemCount();
	int iCurFeedLines = 1;
	index++;
	for ( ; index < iCount; ++index )
	{
		if ( CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(index)) )
		{
			if ( pCtrl->GetRssFeed() != NULL )
			{
				// feed����
				break;
			}
		}

		// ��Ŀ����
		iCurFeedLines++;

		// ��Ŀ���ﵽָ������
		if ( iCurFeedLines >= COVER_LINES )
		{
			// ����Ƿ��ж�����У�����ɾ��
			index++;
			while ( index < iCount ) 
			{
				if ( CtrlBase * pBase = reinterpret_cast<CtrlBase *>(GetItemData(index)) )
				{
					if ( pBase->GetRssFeed() != NULL )
					{
						// ���Ľ���
						return;
					}
					// Ϊ��Դ��Ŀ�������һ��
					index++;
				}
				else
				{
					// ������ɾ��֮
					DeleteItem(index);
					iCount--;
				}
			}
			return;
		}
	}

	// ������������
	ASSERT(iCurFeedLines < COVER_LINES);
	do
	{
		InsertItem(LVIF_PARAM, index, NULL, 0, 0, 0, NULL);
		iCurFeedLines++;
	} while ( iCurFeedLines < COVER_LINES );
}

/// ɾ�����ж��Ŀ���
void CFeedItemListCtrl::DeleteAllBlankLine()
{
	int index = GetItemCount() - 1;
	while ( index >= 0 )
	{
		if ( GetItemData(index) == NULL )
		{
			// ������ɾ��֮
			DeleteItem(index);
		}
		index--;
	}
	
}

/// ɾ��ָ�������еĿ���
void CFeedItemListCtrl::DeleteBlankLine(int index)
{
	int iCount = GetItemCount();
	index++;
	while ( index < iCount )
	{
		if ( GetItemData(index) == NULL )
		{
			// ������ɾ��֮
			DeleteItem(index);
			iCount--;
		}
		else
		{
			break;
		}
	}
}

/// �����ж��������ʵ�����
void CFeedItemListCtrl::FillAllFeedsBlankLine()
{
	// �����۵��򲻼ӿ���
	if ( !IsCoverExpand() )
	{
		return;
	}

	int iCount = GetItemCount();
	for ( int i = 0; i < iCount; ++i )
	{
		if ( GetFeed(i) != NULL )
		{
			DeleteBlankLine(i);
			FillBlankLine(i);
			iCount = GetItemCount();
			i += COVER_LINES - 1;
		}
	}
}

// ȡ��feed item
CRssFeed::Item * CFeedItemListCtrl::GetFeedItem(int iItemIndex)
{
	CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(iItemIndex));
	if ( pCtrl != NULL )
	{
		return pCtrl->GetRssFeedItem();
	}
	return NULL;
}

// ȡ��feed
CRssFeed * CFeedItemListCtrl::GetFeed(int iItemIndex)
{
	CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(iItemIndex));
	if ( pCtrl != NULL )
	{
		return pCtrl->GetRssFeed();
	}
	return NULL;
}

// feed��Ŀ��ʾͼ������
//UINT CFeedItemListCtrl::GetFeedItemImageIndex(const CRssFeed::Item & item)
//{
//	UINT nImage = (item.m_bIsHistory ? 1 : 0);
//	return nImage;
//}

// ����feed״̬ȡ�ö�Ӧ��CxImage
CxImage * CFeedItemListCtrl::GetFeedCxImage(const CRssFeed & feed)
{
	//switch ( feed.m_iState )
	//{
	//case CRssFeed::fsNormal:		// ��ͨ״̬
	//	return m_imageRSS_Normal;

	//case CRssFeed::fsRefresh:	// xml�ļ�������
	//	return m_imageRSS_Downloading;

	//case CRssFeed::fsDisable:		// �����ѽ���
	//	return m_imageRSS_Disable;

	//default:
	//	// ��Ӧ�ó��ִ����
	//	ASSERT(FALSE);
	//	return m_imageRSS_Normal;
	//}

	if ( (feed.m_iState & CRssFeed::fsRefresh) == CRssFeed::fsRefresh )
	{
		return m_imageRSS_Update;	// ����xmlˢ����
	}

	if ( feed.GetItemCount() <= 0 )
	{
		return m_imageRSS_Error;		// ����ʧ��
	}

	if ( feed.ExistDownloadingItem() )
	{
		return m_imageRSS_Downloading;	// �����ļ�������
	}

	if ( (feed.m_iState & CRssFeed::fsNormal) == CRssFeed::fsNormal )
	{
		return m_imageRSS_Normal;		// ����;
	}

	if ( (feed.m_iState & CRssFeed::fsDisable) == CRssFeed::fsDisable )
	{
		return m_imageRSS_Disable;		// ����
	}

	return m_imageRSS_Normal;

}

// feed״̬����
CString CFeedItemListCtrl::GetFeedStatStr(const CRssFeed & feed)
{
	//ASSERT(state < CRssFeed::fsMaxState);
	//return s_strFeedStatStr[state];

	if ( (feed.m_iState & CRssFeed::fsRefresh) == CRssFeed::fsRefresh )
	{
		return GetResString(IDS_RSSSTATE_REFRESH);		// ���ĸ�����
	}

	if ( feed.GetItemCount() <= 0 )
	{
		return GetResString(IDS_RSSSTATE_FAIL);			// ����ʧ��
	}

	if ( !feed.m_bLastUpdateSucc )
	{
		return GetResString(IDS_RSSSTATE_FAIL);			// �ϴθ���ʧ��
	}

	if ( feed.ExistDownloadingItem() )
	{
		return GetResString(IDS_RSSSTATE_DOWNLOADING);	// �����ļ�������
	}

	if ( (feed.m_iState & CRssFeed::fsNormal) == CRssFeed::fsNormal )
	{
		BOOL bIsAutoRefresh = TRUE;
		if ( feed.UpdateIntervalIsDefined() )
		{
			// ���¼���ж���
			bIsAutoRefresh = (feed.GetUpdateInterval() != CRssFeed::MANUAL_REFRESH_VALUE);
		}
		//return GetResString(IDS_RSSSTATE_ENABLE);		// ����;
		return GetResString(bIsAutoRefresh ? IDS_RW_RSS_STATE_AUTO_REFRESH : IDS_RW_RSS_STATE_MANUAL_REFRESH);
	}

	if ( (feed.m_iState & CRssFeed::fsDisable) == CRssFeed::fsDisable )
	{
		return GetResString(IDS_RSSSTATE_DISABLE);		// ����
	}

	return CString();
}

// feed item״̬����
const CString & CFeedItemListCtrl::GetFeedItemStatStr(CRssFeed::EItemState state)
{
	ASSERT(state < CRssFeed::isMaxState);
	return s_strFeedItemStatStr[state];
}

// feed��ʾ�ı�
CString CFeedItemListCtrl::GetFeedText(UINT uColIndex, const CRssFeed & feed)
{
	switch ( uColIndex )
	{
		case COL_INDEX_TITLE:
			return feed.GetDisplayName();

		case COL_INDEX_STATUS:
			return GetFeedStatStr(feed);

		//case COL_INDEX_FEED:
		//	return feed.GetDisplayName();
		case COL_INDEX_DATE:
			{
				// �޸� Feed ���·���ʱ��Ϊ����ʱ��
				return (feed.m_timeLastPub + s_timespan).Format(_T("%Y/%m/%d %H:%M:%S"));
			}
			

		case COL_INDEX_SIZE:
			return CastItoXBytes(feed.GetRssTotalSize(), false, false);
		//case COL_INDEX_CATEGORY:

		default:
			return CString();
	}
}

// feed��Ŀ��ʾ�ı�
CString CFeedItemListCtrl::GetFeedItemText(UINT uColIndex, const CRssFeed::Item & item)
{
	switch ( uColIndex )
	{
		case COL_INDEX_TITLE:
			return item.m_strTitle;

		case COL_INDEX_SIZE:
			return CastItoXBytes(item.m_uEnclosureLength, false, false);

		case COL_INDEX_STATUS:
			return GetFeedItemStatStr(item.GetState());

		//case COL_INDEX_CATEGORY:
		//	return item.m_strCategory;

		case COL_INDEX_DATE:
			{
				//// �޸� Item ���·���ʱ��Ϊ����ʱ��
				//SYSTEMTIME sysTime;
				//item.m_timePubDate.GetAsSystemTime(sysTime);

				//TIME_ZONE_INFORMATION tzi;
				//GetTimeZoneInformation(&tzi);
				//SystemTimeToTzSpecificLocalTime( &tzi, &sysTime, &sysTime);
				//CTime local(sysTime);
				//return local.Format(_T("%Y/%m/%d %H:%M:%S"));
				return (item.m_timePubDate + s_timespan).Format(_T("%Y/%m/%d %H:%M:%S"));
			}
			

		//case COL_INDEX_FEED:
		//	ASSERT(item.m_pFeed != NULL);
		//	return item.m_pFeed->GetDisplayName();

		default:
			return CString();
	}
}

// ȡ����Ŀ����
int CFeedItemListCtrl::GetItemIndex(const POINT & pt)
{
	LVHITTESTINFO TestInfo;
	//DWORD dwPos = GetMessagePos();
	//TestInfo.pt.x = LOWORD(dwPos);
	//TestInfo.pt.y = HIWORD(dwPos);
	//ScreenToClient(&TestInfo.pt);
	TestInfo.pt = pt;
	SubItemHitTest(&TestInfo);
	return TestInfo.iItem;
}

//// ȡ�õ���ѡ�е����ذ�ť��
//int CFeedItemListCtrl::GetClickDownloadBtnIndex()
//{
//	POINT pt;
//	int iItemIndex = GetClickItemIndex(&pt);
//	if ( iItemIndex == -1 )
//	{
//		return -1;
//	}
//
//	CRect rect;
//	ASSERT(GetHeaderCtrl() != NULL);
//	GetHeaderCtrl()->GetClientRect(rect);
//
//	rect.top = rect.bottom + iItemIndex * ITEM_HEIGHT;
//	rect.bottom = rect.top + ITEM_HEIGHT;
//	rect.right = rect.left + GetColumnWidth(0);
//	rect.left = 0;
//	GetDownloadButtonRect(TRUE, rect, rect);
//	
//	int iOffsetX = -GetScrollOffset(SB_HORZ);
//	int iOffsetY = -GetScrollOffset(SB_VERT);
//	rect.OffsetRect(iOffsetX, iOffsetY);
//
//	if ( rect.PtInRect(pt) )
//	{
//		return iItemIndex;
//	}
//	return -1;
//}

// ȡ�ù���ƫ��
int CFeedItemListCtrl::GetScrollOffset(DWORD dwFlag)
{	
	SCROLLINFO info;
	if ( !GetScrollInfo(dwFlag, &info, SIF_POS) )
	{
		return 0;
	}
	
	switch ( dwFlag )
	{
		case SB_HORZ:
			return info.nPos;

		case  SB_VERT:
			return info.nPos * ITEM_HEIGHT;

		default:
			return 0;
	}
}

// ɾ�������µ�����Ŀ
//void CFeedItemListCtrl::DeleteFeedItems(int index, const CRssFeed & feed)
//{
//	DWORD dwSize = feed.GetItemSize();
//	for ( DWORD i = 0; i < dwSize; ++i )
//	{
//		DeleteItem(index);
//	}
//}

//// ����feed item״̬��������ͼ��
//void CFeedItemListCtrl::ResetItemImageIndex(int iItemIndex)
//{
//	if ( CRssFeed::Item * pItem = GetFeedItem(iItemIndex) )
//	{
//		SetItem(iItemIndex,
//			    0, 
//			    LVIF_IMAGE, 
//			    NULL, 
//			    GetFeedItemImageIndex(*pItem),
//			    0,
//			    0,
//			    NULL
//			   );
//		//RedrawItems(iItemIndex, iItemIndex);
//	}
//}

BEGIN_MESSAGE_MAP(CFeedItemListCtrl, CBaseListCtrl)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteitem)
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnLvnItemActivate)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(LVN_ENDSCROLL, OnLvnEndScroll)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_NOTIFY(HDN_BEGINTRACKA, 0, OnHdnBegintrack)
	ON_NOTIFY(HDN_BEGINTRACKW, 0, OnHdnBegintrack)
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, OnHdnItemchanging)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// CFeedItemListCtrl ��Ϣ�������

void CFeedItemListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO:  ������Ĵ�����ȷ��ָ����Ĵ�С
	ASSERT(lpMeasureItemStruct != NULL);
	lpMeasureItemStruct->itemHeight = ITEM_HEIGHT;
}

// ��ͼ��
//void CFeedItemListCtrl::DrawItemIcon(CDC & dc, int iImageIndex, const RECT & rect, BOOL bSelected)
//{
//	int cy = GetSystemMetrics(SM_CYSMICON);
//	CPoint pt(rect.left, rect.top + (ITEM_HEIGHT - cy) / 2);
//	m_ilsItemImage.Draw(&dc, 
//						iImageIndex, 
//						pt, 
//						bSelected ? (ILD_SELECTED | ILD_TRANSPARENT) : ILD_TRANSPARENT
//					   );
//}

// ȡ�ð�ťλ��
void CFeedItemListCtrl::GetDownloadButtonRect(int index, BOOL bItem, RECT & rectBtn)
{
	CRect rectSubItem;
	BOOL bResult = GetSubItemRect(index, COL_INDEX_TITLE, LVIR_LABEL, rectSubItem);
	if ( !bResult )
	{
		return;
	}

	if ( bItem )
	{
		rectBtn.right = rectSubItem.right - 4;
		rectBtn.left = rectSubItem.right - 44;
		rectBtn.top = rectSubItem.top + 4;
		rectBtn.bottom = rectSubItem.bottom - 4;
	}
	else
	{
		rectBtn.right = rectSubItem.right - 4;
		rectBtn.left = rectSubItem.right - 68;
		rectBtn.top = rectSubItem.top + 4;
		rectBtn.bottom = rectSubItem.bottom - 4;
	}
}

void CFeedItemListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  ������Ĵ����Ի���ָ����
	ASSERT(lpDrawItemStruct != NULL);
	CDC * pDrawDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if ( pDrawDC == NULL )
	{
		return;
	}

	int iSaveDC = pDrawDC->SaveDC();
	int iCoverColWidth = GetColumnWidth(COL_INDEX_COVER);
	CRect rectBackground = lpDrawItemStruct->rcItem;
	rectBackground.left += iCoverColWidth;

	if ( lpDrawItemStruct->itemID != UINT(-1) && 
		 lpDrawItemStruct->itemData != NULL 
	   )
	{
		CFeedItemListCtrl::CtrlBase * pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(lpDrawItemStruct->itemData);
		//HDWP hWinPosInfo = NULL;
		//DrawDownloadButton(lpDrawItemStruct->itemID, (pCtrl->GetRssFeedItem() != NULL), hWinPosInfo);

		BOOL bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED);
		if ( bSelected )
		{
			BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
			if (lpDrawItemStruct->itemState & ODS_SELECTED) 
			{
				if (bCtrlFocused)
				{
					pDrawDC->SetBkColor(m_crHighlight);
				}
				else
				{
					pDrawDC->SetBkColor(m_crNoHighlight);
				}
			}
			else
			{
				pDrawDC->SetBkColor(GetBkColor());
			}
			CMemDC dc(pDrawDC, rectBackground);

			int iOldBkMode;
			if (m_crWindowTextBk == CLR_NONE)
			{
				//DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
				iOldBkMode = dc.SetBkMode(TRANSPARENT);
			}
			else
			{
				iOldBkMode = OPAQUE;
			}

			RECT outline_rec = rectBackground;

			outline_rec.top--;
			outline_rec.bottom++;
			dc.FrameRect(&outline_rec, &CBrush(m_crWindow));
			outline_rec.top++;
			outline_rec.bottom--;
			outline_rec.left++;
			outline_rec.right--;

			if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			{
				outline_rec.top--;
			}

			if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			{
				outline_rec.bottom++;
			}

			if(bCtrlFocused)
			{
				dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
			}
			else
			{
				dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
			}

			if (m_crWindowTextBk == CLR_NONE)
			{
				dc.SetBkMode(iOldBkMode);
			}
		}
		else
		{
			if ( pCtrl->GetRssFeed() != NULL && static_cast<CtrlFeed*>(pCtrl)->m_bNew )
			{
				pDrawDC->FillSolidRect(&rectBackground, RGB(255,245,208));
			}
			else
			{
				// ����Ҫ�ٻ������ˣ�����ǰ�Ѿ����ñ�����
				//pDrawDC->FillSolidRect(&rectBackground, GetItemBackColor(lpDrawItemStruct->itemID));
			}
		}

		pCtrl->DrawItem(*pDrawDC, *this, lpDrawItemStruct);
	}
	else
	{
		// ����Ҫ�ٻ������ˣ�����ǰ�Ѿ����ñ�����
		//pDrawDC->FillSolidRect(&rectBackground, GetItemBackColor(lpDrawItemStruct->itemID));
	}

	pDrawDC->RestoreDC(iSaveDC);
}

// �ж��뷽ʽת�ı��������
DWORD CFeedItemListCtrl::ConverLvFmtToDtFmt(DWORD dwLvFmt)
{
	switch ( dwLvFmt )
	{
		case LVCFMT_LEFT:
			return DT_LEFT;

		case LVCFMT_RIGHT:
			return DT_RIGHT;

		case LVCFMT_CENTER:	
			return DT_CENTER;

		default:
			return DT_LEFT;
	}
}

void CFeedItemListCtrl::CtrlFeed::DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct != NULL);
	RECT & rectDraw = lpDrawItemStruct->rcItem;
	COLORREF crTextColor = dc.GetTextColor();
	dc.SetTextColor(crTextColor);

	ASSERT(m_pFeed != NULL);

	CHeaderCtrl *pHeaderCtrl = list.GetHeaderCtrl();
	ASSERT(pHeaderCtrl != NULL);
	int iCount = pHeaderCtrl->GetItemCount();
	CString strItemText;

	for( int iCurrent = 0; iCurrent < iCount; iCurrent++ )
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);		
		if (list.IsColumnHidden(iColumn))
			continue;

		strItemText = GetFeedText(iColumn, *m_pFeed);
		
		rectDraw.right = rectDraw.left + list.GetColumnWidth(iColumn);
		int iNextLeft = rectDraw.right;

		CString strCompleteNum;
		CRect rectCompleteNum;

		if ( iColumn == COL_INDEX_TITLE )
		{
			if ( !list.m_bSingleStyle )//����ģʽ����Ҫ����+��
			{
				//����"��"
				CPen penGrey, penBlack, *oldpn;
				penGrey.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
				penBlack.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWTEXT));

				oldpn = dc.SelectObject(&penGrey);
				CRect rectSquare(rectDraw.left+4,rectDraw.top+8,rectDraw.left+13,rectDraw.top+17);
				dc.SelectStockObject(NULL_BRUSH);//����
				dc.Rectangle(rectSquare);

				int nMiddleH = rectSquare.Height()/2;
				int nMiddleW = rectSquare.Width()/2;

				oldpn = dc.SelectObject(&penBlack);
				int OffsetX1 = 2;
				int OffsetX2 = 2;
				dc.MoveTo( rectSquare.left+OffsetX1, rectSquare.top+nMiddleH );
				dc.LineTo( rectSquare.right-OffsetX1, rectSquare.top+nMiddleH );
				if (!m_bExpand)
				{
					dc.MoveTo( rectSquare.left+nMiddleW, rectSquare.top+OffsetX2 );
					dc.LineTo( rectSquare.left+nMiddleW, rectSquare.bottom-OffsetX2 );
				}

				dc.SelectObject(oldpn);
				penGrey.DeleteObject();
				penBlack.DeleteObject();
			}

			rectDraw.left += 5;
			if (list.IsResExit())
			{
					if ( CxImage * pImage = list.GetFeedCxImage(*m_pFeed) )
					{
						pImage->Draw(dc, rectDraw.left + 13, rectDraw.top + 5, 16, 16);
					}
			}
			
			rectDraw.left += GetSystemMetrics(SM_CXSMICON) + 16;
			CRect rectButton;
			list.GetDownloadButtonRect(lpDrawItemStruct->itemID, FALSE, rectButton);
			rectDraw.right = rectButton.left - 28;

			UINT nNewCompleteNum = m_pFeed->GetNewDownloadedCount(); // m_pFeed->GetCompleteCount();
			if ( nNewCompleteNum > 0 )//��ʾ�����count
			{
				strCompleteNum.Format( _T("%d"),  nNewCompleteNum);
				list.GetCompleteNumRect(lpDrawItemStruct->itemID,strCompleteNum,rectCompleteNum);

				if (!rectCompleteNum.IsRectEmpty())
				{
					rectDraw.right = rectCompleteNum.left - 6;
				}
			}
		}

		DWORD uFlag = list.ConverLvFmtToDtFmt(s_CreateParam[iColumn].m_uFormat);
		if ( uFlag == DT_RIGHT )
		{
			rectDraw.right -= 10;
		}
		dc.DrawText(strItemText, &rectDraw, uFlag | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX);
		if ( iColumn == COL_INDEX_TITLE && !rectCompleteNum.IsRectEmpty() )
		{
			CRect rectRGN(rectCompleteNum.left, rectCompleteNum.top+4, rectCompleteNum.right, rectCompleteNum.bottom-4);
			CRgn rgnFrame;//��Բ�Ǿ���
			rgnFrame.CreateRoundRectRgn(rectRGN.left+1,rectRGN.top+3,rectRGN.right,rectRGN.bottom-1,3,3);

			CBrush* brushFrame;

			//if (m_pFeed->m_bHasNewComplete)
			{
				dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
				brushFrame = new CBrush( RGB(0xb7,0x58,0x58) );//��ɫ
				//brushFrame = new CBrush( RGB(0x84,0x84,0x84) );//��ɫ
			}

			//{//����һ ��ѡ�� "����C++�쳣"-�ǣ����� SEH �쳣(/EHa) ��ͻ ����ԭ����
			//	CRgn rgnFrame;//��Բ�Ǿ���
			//	rgnFrame.CreateRoundRectRgn(rectRGN.left+1,rectRGN.top+3,rectRGN.right,rectRGN.bottom-1,3,3);
			//	dc.FillRgn(&rgnFrame,brushFrame);
			//}

			{//������
				CPen penRound(PS_NULL, 1, RGB(0, 0, 0));
				CPen * pOrgPen = dc.SelectObject(&penRound);
				CBrush * pOrgBrush = dc.SelectObject(brushFrame);
				dc.RoundRect(rectRGN.left+1,rectRGN.top+3,rectRGN.right,rectRGN.bottom-1,3,3);
				dc.SelectObject(pOrgBrush);
				dc.SelectObject(pOrgPen);
			}

			delete brushFrame;

			list.m_pOldFont = dc.SelectObject(&list.m_FontBold);
			dc.DrawText(strCompleteNum, &rectRGN, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX);
			dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
			dc.SelectObject(list.m_pOldFont);
		}
		rectDraw.left = iNextLeft;
	}

}

void CFeedItemListCtrl::CtrlFeedItem::DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct != NULL);
	RECT & rectDraw = lpDrawItemStruct->rcItem;
	COLORREF crTextColor = dc.GetTextColor();

	ASSERT(m_pFeedItem != NULL);
	ASSERT(!m_pFeedItem->IsSubtitle());

	if ( !m_pFeedItem->m_bIsHistory )
	{
		crTextColor = ::GetSysColor(COLOR_BTNSHADOW);
	}
	dc.SetTextColor(crTextColor);

	CHeaderCtrl *pHeaderCtrl = list.GetHeaderCtrl();
	ASSERT(pHeaderCtrl != NULL);
	int iCount = pHeaderCtrl->GetItemCount();
	CString strItemText;

	for( int iCurrent = 0; iCurrent < iCount; iCurrent++ )
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);		
		if (list.IsColumnHidden(iColumn))
			continue;

		strItemText = GetFeedItemText(iColumn, *m_pFeedItem);
		rectDraw.right = rectDraw.left + list.GetColumnWidth(iColumn);
		int iNextLeft = rectDraw.right;

		CString strSubtitle;
		CRect rectSubtitle;

		if ( iColumn == COL_INDEX_TITLE )
		{
			//����������
			/*CPen pn, *oldpn;
			pn.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
			oldpn = dc.SelectObject(&pn);
			rectDraw.left += 12;
			int nMiddle = rectDraw.top+(rectDraw.bottom-rectDraw.top)/2;
			int OffsetX1 = 14;
			int OffsetX2 = 27;
			if ( this->m_bIsLast )
			{
				dc.MoveTo(rectDraw.left+OffsetX1,rectDraw.top);
				dc.LineTo(rectDraw.left+OffsetX1,nMiddle);
				dc.MoveTo(rectDraw.left+OffsetX1,nMiddle);
				dc.LineTo(rectDraw.left+OffsetX2,nMiddle);
			}
			else
			{
				dc.MoveTo(rectDraw.left+OffsetX1,rectDraw.top);
				dc.LineTo(rectDraw.left+OffsetX1,rectDraw.bottom);
				dc.MoveTo(rectDraw.left+OffsetX1,nMiddle);
				dc.LineTo(rectDraw.left+OffsetX2,nMiddle);
			}
			dc.SelectObject(oldpn);
			pn.DeleteObject();*/

			//rectDraw.left += nOffset;
			if (list.IsResExit())
			{
				if ( CxImage * pImage = list.GetFeedItemCxImage(*m_pFeedItem) )
				{
					pImage->Draw(dc,rectDraw.left+2, rectDraw.top + 5, 16, 16);
				}
			}

			
			ASSERT(m_pFeedItem != NULL);
			int iImage = theApp.GetFileTypeSystemImageIdx(m_pFeedItem->m_strFilename);
			if (theApp.GetSystemImageList() != NULL)
			{
				::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), rectDraw.left+18, rectDraw.top+5, ILD_NORMAL|ILD_TRANSPARENT);
			}

			rectDraw.left += GetSystemMetrics(SM_CXSMICON) + 12;
			CRect rectButton;
			list.GetDownloadButtonRect(lpDrawItemStruct->itemID, TRUE, rectButton);			
			rectDraw.right = rectButton.left - 5;
			dc.SetTextColor(crTextColor);
			
			if (m_pFeedItem->m_pAttachItem != NULL)//����Ļ
			{
				list.GetSubtitleRect(rectSubtitle, lpDrawItemStruct->itemID);
				if (!rectSubtitle.IsRectEmpty())
				{
					rectDraw.right = rectSubtitle.left - 10;
				}
			}
		}


		DWORD uFlag = list.ConverLvFmtToDtFmt(s_CreateParam[iColumn].m_uFormat);
		if ( uFlag == DT_LEFT )
		{
			rectDraw.left += 10;
		}
		if ( uFlag == DT_RIGHT )
		{
			rectDraw.right -= 10;
		}

		COLORREF           rgbThis = ::GetSysColor(COLOR_GRAYTEXT);
		UINT nState =  m_pFeedItem->GetState() ;
		if (nState >= 0)
		{
			switch ( nState )
			{
			case CRssFeed::isRss:			// δ����
			case CRssFeed::isDownloading:	// �Ѷ��ģ�partfile������
			case CRssFeed::isPaused:		// �Ѷ��ģ�partfile����ͣ
			case CRssFeed::isStopped:		// �Ѷ��ģ�partfile��ֹͣ
				{
					//dc.SelectObject( GetStockObject(DEFAULT_GUI_FONT) );
					break;
				}

			case CRssFeed::isDownloaded:	// �Ѷ��ģ�partfile�������
				rgbThis = ::GetSysColor(COLOR_WINDOWTEXT);
				break;

			case CRssFeed::isHistory:		// �Ѷ��ģ���partfile�Ѿ���ɾ��
				rgbThis = ::GetSysColor(COLOR_GRAYTEXT);
				break;

			default:
				rgbThis = ::GetSysColor(COLOR_GRAYTEXT);
				break;
			}
		}

		//if (m_pFeedItem->m_bIsNewPub)//���µ�item ������ʾ
		//{
		//	dc.SelectObject(list.m_FontBold);
		//}

		
		dc.SetTextColor( rgbThis );

		dc.DrawText(strItemText, &rectDraw, uFlag | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX);

		if (  iColumn == COL_INDEX_TITLE && m_pFeedItem->m_pAttachItem != NULL && !rectSubtitle.IsRectEmpty()/*&& m_pFeedItem->m_pAttachItem->GetState() == CRssFeed::isDownloaded*/  )
		{
			if (list.IsResExit())
			{
				if (m_pFeedItem->m_pAttachItem->GetState() == CRssFeed::isDownloaded)
					list.m_imageSubTitle_Green->Draw(dc,rectSubtitle.left, rectSubtitle.top+5, 21, 12);
				else
					list.m_imageSubTitle_Grey->Draw(dc,rectSubtitle.left, rectSubtitle.top+5, 21, 12);
			}
		}
		rectDraw.left = iNextLeft;
	}
	CHeaderCtrl* pHeadCtr = list.GetHeaderCtrl();
	pHeadCtr->Invalidate();
}

void CFeedItemListCtrl::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(pNMLV != NULL);
	ASSERT(pResult != NULL);
	*pResult = 0;

	if ( CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(pNMLV->lParam) )
	{
		//delete pCtrl->m_pbtnDownload;
		delete pCtrl;
	}

	//int index = pNMLV->iItem;
	//int iTopIndex = GetTopIndex();
	//int iBottomIndex = iTopIndex + GetCountPerPage() - 1;
	//if ( index < iTopIndex && index > iBottomIndex )
	//{
	//	return;
	//}
}

void CFeedItemListCtrl::OnLvnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(pNMIA != NULL);
	ASSERT(pResult != NULL);
	*pResult = 0;

	if (!m_bSingleStyle)
	{
		int index = pNMIA->iItem;	//GetClickItemIndex();
		SwitchExpandStat(index);
	}
}

/// ����ָ����Ŀ��List����
void CFeedItemListCtrl::ScrollIndex2Top(int nIndex)
{
	EnsureVisible(nIndex, FALSE);//������Ŀ���ɼ�

	int iFrom = GetTopIndex();
	if (nIndex > iFrom)
	{
		CSize nScrollSize;
		nScrollSize.cy = ITEM_HEIGHT * (nIndex-iFrom);//������붥��ƫ����
		Scroll(nScrollSize);
	}
}

/// չ����ָ�����۵�״̬
void CFeedItemListCtrl::SwitchToStat(int index, BOOL bExpand)
{
	CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(index));
	if ( pCtrl == NULL )
	{
		return;
	}

	if ( CRssFeed * pFeed = pCtrl->GetRssFeed() )
	{
		CtrlFeed * pCtrlFeed = static_cast<CtrlFeed *>(pCtrl);
		if ( pCtrlFeed->m_bExpand == bool(bExpand) )
		{
			return;
		}
		SwitchExpandStat(index);
	}
}

// �л�չ���۵�״̬
void CFeedItemListCtrl::SwitchExpandStat(int index)
{
	if ( index == -1 )
	{
		return;
	}

	CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(index));
	if ( pCtrl == NULL )
	{
		return;
	}

	if ( CRssFeed * pFeed = pCtrl->GetRssFeed() )
	{
		CtrlFeed * pCtrlFeed = static_cast<CtrlFeed *>(pCtrl);
		pCtrlFeed->m_bExpand = !pCtrlFeed->m_bExpand;
		pCtrlFeed->m_bNew = false;
		CGlobalVariable::s_wndRssCtrl->SetFeedNewDownloadedTime(*pFeed);
		//pFeed->m_bHasNewComplete = false;
		ExpandFeed(index, *pFeed, pCtrlFeed->m_bExpand, TRUE);
		RedrawAllButton();
	}
}

// �ػ���ť
void CFeedItemListCtrl::RedrawAllButton()
{
	int iCount = GetItemCount();
	if ( iCount <= 0 )
	{
		return;
	}

	HDWP hWinPosInfo = BeginDeferWindowPos(m_DownloadButtons.size());

	for ( int i = 0; i < iCount; ++i )
	{
		CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(i));
		DrawDownloadButton(i, pCtrl, hWinPosInfo);
	}

	if ( hWinPosInfo != NULL )
	{
		EndDeferWindowPos(hWinPosInfo);
	}

	Invalidate(FALSE);
	return;
}
void CFeedItemListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//RedrawAllButton();
	CBaseListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CFeedItemListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//RedrawAllButton();
	CBaseListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFeedItemListCtrl::OnDestroy()
{
	KillTimer(TIMER_ID);

	if (m_imageRSS_Normal)
	{
		delete m_imageRSS_Normal;
	}
	if (m_imageRSS_Disable)
	{
		delete m_imageRSS_Disable;
	}
	if (m_imageRSS_Downloading)
	{
		delete m_imageRSS_Downloading;
	}
	if (m_imageRSS_Update)
	{
		delete m_imageRSS_Update;
	}
	if (m_imageRSS_Error)
	{
		delete m_imageRSS_Error;
	}


	if (m_imageTask_Complete)
	{
		delete m_imageTask_Complete;
	}
	if (m_imageTask_Pause)
	{
		delete m_imageTask_Pause;
	}
	if (m_imageTask_Error)
	{
		delete m_imageTask_Error;
	}
	if (m_imageTask_Stop)
	{
		delete m_imageTask_Stop;
	}
	if (m_imageTask_Downloading1)
	{
		delete m_imageTask_Downloading1;
	}
	if (m_imageTask_Downloading2)
	{
		delete m_imageTask_Downloading2;
	}
	if (m_imageTask_Downloading3)
	{
		delete m_imageTask_Downloading3;
	}
	if (m_imageSubTitle_Grey)
	{
		delete m_imageSubTitle_Grey;
	}
	if (m_imageSubTitle_Green)
	{
		delete m_imageSubTitle_Green;
	}
	if (m_imageRss_PosterBg)
	{
		delete m_imageRss_PosterBg;
	}
	if (m_imageRss_PosterDefault)
	{
		delete m_imageRss_PosterDefault;
	}

	if(m_pMenuXP)
	{
		delete m_pMenuXP;
	}

	DeleteAllButtons();

	m_FontBold.DeleteObject();
	CBaseListCtrl::OnDestroy();
	// TODO: Add your message handler code here
}


BOOL CFeedItemListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	BOOL bResult = CBaseListCtrl::OnMouseWheel(nFlags, zDelta, pt);
	//RedrawAllButton();
	return bResult;
}

void CFeedItemListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch ( nChar )
	{
		case VK_DOWN:
		case VK_UP:
			//RedrawAllButton();
			break;
	}

	CBaseListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CFeedItemListCtrl::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	if (nIDEvent == TIMER_ID)
	{
		int iCount = GetItemCount();
		for ( int i = 0; i < iCount; ++i )
		{
			if ( CRssFeed::Item * pFeedItem = GetFeedItem(i) )
			{
				switch ( pFeedItem->GetState() )
				{
				case CRssFeed::isDownloading:	// �Ѷ��ģ�partfile������
				//case CRssFeed::isPaused:		// �Ѷ��ģ�partfile����ͣ
				//case CRssFeed::isStopped:		// �Ѷ��ģ�partfile��ֹͣ
					RedrawItems(i, i);
					break;
				}
			}
		}

		if ( m_nIcoState++ >= 3 )
		{
			m_nIcoState = 1;
		}
	}
	else
	{
		CBaseListCtrl::OnTimer(nIDEvent);
	}
}

void CFeedItemListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != 0);
	*pResult = 0;

	// Version 4.71. Pointer to an NMITEMACTIVATE structure 
	LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if ( lpnmitem->iItem == -1 )
	{
		return;
	}

	CtrlBase * pCtrl = reinterpret_cast<CtrlBase *>(GetItemData(lpnmitem->iItem));
	if ( pCtrl == NULL )
	{
		return;
	}

	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));
	ScreenToClient(&point);

	if ( CRssFeed::Item * pItem = pCtrl->GetRssFeedItem() )
	{
		if (pItem->m_bIsNewPub)//ȡ������
		{
			pItem->m_bIsNewPub = false;
			RedrawItems(lpnmitem->iItem,lpnmitem->iItem);
			RedrawFeedLine(pItem->m_pFeed);
		}

		// ����������Ļ�������
		if ( pItem->m_pAttachItem != NULL )
		{
			CRect rect;
			GetSubtitleRect(rect, lpnmitem->iItem);
			if ( !rect.IsRectEmpty() && rect.PtInRect(point) )
			{
				//SendDownloadRequestMessage
				if ( CGlobalVariable::s_wndRssCtrl != NULL && CGlobalVariable::s_wndRssCtrl->m_pwndRssFeed != NULL )
				{
					CGlobalVariable::s_wndRssCtrl->m_pwndRssFeed->SendDownloadRequestMessage(*pItem->m_pAttachItem);
				}
			}
		}
		return;
	}

	if (!m_bSingleStyle)
	{
		// ������������Ŀ�ġ�+���ŵ����
		CRect rect;
		if ( GetSubItemRect(lpnmitem->iItem, COL_INDEX_TITLE, LVIR_LABEL, rect) )
		{
			rect.SetRect(rect.left + 4, rect.top + 8, rect.left + 13, rect.top + 17);
			if ( rect.PtInRect(point) )
			{
				SwitchExpandStat(lpnmitem->iItem);
				return;
			}
		}
	}
}

void CFeedItemListCtrl::OnLvnEndScroll(NMHDR *pNMHDR, LRESULT *pResult)
{
	// �˹���Ҫ�� Internet Explorer 5.5 ����߰汾��
	// ���� _WIN32_IE ������ >= 0x0560��
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != NULL);

	//LPNMLVSCROLL pStateChanged = reinterpret_cast<LPNMLVSCROLL>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	RedrawAllButton();
	//Invalidate();
}

int CALLBACK CFeedItemListCtrl::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	if ( lParam1 == NULL || lParam2 == NULL )
	{
		// �п���Ŀ
		return 0;
	}

	BOOL bIsAsc = (lParamSort < SORT_DESC_FLAG);
	CtrlBase * pBase1 = reinterpret_cast<CtrlBase *>(lParam1);
	CtrlBase * pBase2 = reinterpret_cast<CtrlBase *>(lParam2);
	CRssFeed::Item * pFeedItem1 = pBase1->GetRssFeedItem();
	CRssFeed::Item * pFeedItem2 = pBase2->GetRssFeedItem();

	CRssFeed * pFeed1 = pBase1->GetRssFeed();
	if ( pFeed1 == NULL )
	{
		ASSERT(pFeedItem1 != NULL);
		pFeed1 = static_cast<CRssFeed*>(pFeedItem1->m_pFeed);
	}

	CRssFeed * pFeed2 = pBase2->GetRssFeed();
	if ( pFeed2 == NULL )
	{
		ASSERT(pFeedItem2 != NULL);
		pFeed2 = static_cast<CRssFeed*>(pFeedItem2->m_pFeed);
	}

	ASSERT(pFeed1 != NULL &&  pFeed2 != NULL);

	if ( pFeed1 == NULL ||  pFeed2 == NULL )
	{
		// ���ó��ִ����
		return 0;
	}

	// ������Դ��Ŀ
	if ( lParamSort >= SORT_DESC_FLAG )
	{
		lParamSort -= SORT_DESC_FLAG;
	}

	// �ȱȽ�feed
	if ( pFeed1 != pFeed2 )
	{
		INT64 iResult = 0;
		// ��ͬfeed���Ȱ�feed��������
		switch ( lParamSort )
		{
		case COL_INDEX_TITLE:
			iResult = pFeed1->GetDisplayName().CompareNoCase(pFeed2->GetDisplayName());
			break;

		case COL_INDEX_SIZE:
			iResult = pFeed1->GetRssTotalSize()- pFeed2->GetRssTotalSize();
			break;

		case COL_INDEX_STATUS:
			iResult = GetFeedStatStr(*pFeed1).Compare(GetFeedStatStr(*pFeed2));
			break;

		case COL_INDEX_DATE:
			iResult = pFeed1->m_timeLastPub.GetTime() - pFeed2->m_timeLastPub.GetTime();
			break;
		}
		//iResult = pFeed1->GetDisplayName().CompareNoCase(pFeed2->GetDisplayName());
		//if ( iResult != 0 )
		//{
		//	//return bIsAsc ? iResult : -iResult;
		//	return iResult;
		//}
		// ��ͬ��feed����������ͬ
		if ( !bIsAsc )
		{
			iResult = -iResult;
		}
		return ( iResult != 0 ? (iResult > 0 ? 1 : -1) : 0 );
		//return pFeed1 < pFeed2;
	}

	// feed��ͬ
	if ( pFeedItem1 == NULL && pFeedItem2 != NULL )
	{
		// 1Ϊ������Ŀ��2Ϊ��Դ��Ŀ��1��ǰ
		return -1;
	}
	else if ( pFeedItem1 != NULL && pFeedItem2 == NULL ) 
	{
		// 1Ϊ��Դ��Ŀ��2Ϊ������Ŀ��2��ǰ
		return 1;
	}
	else if ( pFeedItem1 == NULL && pFeedItem2 == NULL  )
	{
		// �����ܷ��������
		ASSERT(FALSE);
		return 0;
	}

	INT64 iResult = 0;
	switch ( lParamSort )
	{
		case COL_INDEX_TITLE:
			iResult = pFeedItem1->m_strTitle.CompareNoCase(pFeedItem2->m_strTitle);
			break;

		case COL_INDEX_SIZE:
			iResult = pFeedItem1->m_uEnclosureLength - pFeedItem2->m_uEnclosureLength;
			break;

		case COL_INDEX_STATUS:
			iResult = int(pFeedItem1->GetState()) - int(pFeedItem2->GetState());
			break;
			
		case COL_INDEX_DATE:
			iResult = pFeedItem1->m_timePubDate.GetTime() - pFeedItem2->m_timePubDate.GetTime();
			break;
	}
	
	
	if ( iResult == 0 )
	{
		return 0;
	}

	if ( !bIsAsc )
	{
		iResult = -iResult;
	}

	//return (iResult > 0 ? 1 : -1);
	return 1L | int(iResult >> (sizeof(iResult) * 8 - 1));
}

void CFeedItemListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ASSERT(pNMLV != NULL);
	ASSERT(pResult != NULL);
	*pResult = 0;

	if ( pNMLV->iSubItem == COL_INDEX_COVER )
	{
		SetRedraw(FALSE);

		// �л�������״̬
		SwitchCoverColumnState();

		// �������۵�������Ҫ�������
		// ������չ��������Ҫ������
		DeleteAllBlankLine();
		FillAllFeedsBlankLine();

		SetRedraw(TRUE);
		return;
	}

	bool sortAscending = (GetSortItem() != pNMLV->iSubItem) ? false : !GetSortAscending();//false: ��һ�ε���Ϊ����
	DoSort(pNMLV->iSubItem, sortAscending);
}

void CFeedItemListCtrl::UpdateSort(bool bUpdate)
{
	int nIndex = COL_INDEX_DATE;//��ʼ��ʱǿ�ư��ո���ʱ������
	bool sortAscending = false;
	if ( bUpdate )
	{
		nIndex = GetSortItem();
		sortAscending = GetSortAscending();

		if ( nIndex < 0)
			return;
	}

	DoSort(nIndex, sortAscending);
}

/// ����Ŀ����
void CFeedItemListCtrl::DoSort(int iColIndex, BOOL bSortAscending)
{
	SetRedraw(FALSE);

	// ɾ�����п��У�����Ӱ������
	DeleteAllBlankLine();

	// Sort table
	UpdateSortHistory(iColIndex + (bSortAscending ? SORT_ASC_FLAG : SORT_DESC_FLAG));
	SetRssArrow(iColIndex, !!bSortAscending);
	SortItems(CompareFunc, iColIndex + (bSortAscending ? SORT_ASC_FLAG : SORT_DESC_FLAG));

	// ����������
	FillAllFeedsBlankLine();

	// �����β��ʶ
	ResetIsLastFlag();

	SetRedraw(TRUE);
}

// ���������Ƿ����һ�еı��
void CFeedItemListCtrl::ResetIsLastFlag()
{
	int index = 0;
	int iCount = GetItemCount();
	do 
	{
		index = ResetIsLastFlag(index);
	} while ( index < iCount && index != -1 );
}

// ���������Ƿ����һ�еı��
int CFeedItemListCtrl::ResetIsLastFlag(int index)
{
	if ( index < 0 )
	{
		return -1;
	}

	int iCount = GetItemCount();
	CtrlFeedItem * pLastItem = NULL;

	for ( ; index < iCount; ++index )
	{
		CtrlBase * pBase = reinterpret_cast<CtrlBase*>(GetItemData(index));
		if ( pBase == NULL || 
			 pBase->GetRssFeed() != NULL 
		   )
		{
			// ������Ŀ�����Ŀ
			index++;
			break;
		}
		else
		{
			pLastItem = static_cast<CtrlFeedItem*>(pBase);
			pLastItem->m_bIsLast = FALSE;
		}
	}
	
	if ( pLastItem != NULL )
	{
		pLastItem->m_bIsLast = TRUE;
	}

	return index;
}

void CFeedItemListCtrl::UpdateMenuState(CTitleMenu	& menuRSS)
{
	if( !CGlobalVariable::IsRunning() )
	{
		return;
	}

	//���ò˵�״̬
	int iSelectedItems = GetSelectedCount();
	if(iSelectedItems && iSelectedItems == 1)
	{
		int nIndex = GetCurrSelectIndex();
		if ( nIndex >= 0 )
		{
			if ( CRssFeed* pFeed = GetFeed(nIndex) )
			{
				//ɾ�����õ�menu��
				menuRSS.DeleteMenu(MP_OPEN,MF_STRING);
				menuRSS.DeleteMenu(MP_RESUME,MF_STRING);

				//��������||��ͣ
				if (pFeed->ExistDownloadingItem())
				{
					menuRSS.EnableMenuItem(MP_PAUSE, MF_ENABLED);
				}

				if (pFeed->ExistUnloadItem())
				{
					menuRSS.EnableMenuItem(MP_DOWNLOAD_ALL, MF_ENABLED);
				}

				menuRSS.EnableMenuItem(MP_CANCEL, MF_ENABLED);
				menuRSS.EnableMenuItem(MP_REFRESH, MF_ENABLED);
				menuRSS.EnableMenuItem(MP_OPTIONS, MF_ENABLED);
				menuRSS.EnableMenuItem(MP_OPENFOLDER, MF_ENABLED);
			}
			else if ( CRssFeed::Item * pItem = GetFeedItem(nIndex) )
			{
				menuRSS.EnableMenuItem(MP_OPENFOLDER, MF_ENABLED);

				//ɾ�����õ�menu��
				menuRSS.DeleteMenu(MP_DOWNLOAD_ALL,MF_STRING);
				menuRSS.DeleteMenu(MP_CANCEL,MF_STRING);
				menuRSS.DeleteMenu(MP_REFRESH,MF_STRING);
				menuRSS.DeleteMenu(MP_OPTIONS,MF_STRING);

				switch ( pItem->GetState() )
				{
				case CRssFeed::isDownloading:	// �Ѷ��ģ�partfile������
					{
						menuRSS.EnableMenuItem(MP_PAUSE, MF_ENABLED);
						break;
					}
				case CRssFeed::isDownloaded:	// �Ѷ��ģ�partfile�������
					{
						menuRSS.EnableMenuItem(MP_OPEN, MF_ENABLED);
						break;
					}
				case CRssFeed::isPaused:		// �Ѷ��ģ�partfile����ͣ
					{
						menuRSS.EnableMenuItem(MP_RESUME, MF_ENABLED);
						break;
					}
				case CRssFeed::isStopped:		// �Ѷ��ģ�partfile��ֹͣ
					{
						menuRSS.EnableMenuItem(MP_RESUME, MF_ENABLED);
						break;
					}
				case CRssFeed::isHistory:		// �Ѷ��ģ���partfile�Ѿ���ɾ��
					{
						menuRSS.EnableMenuItem(MP_RESUME, MF_ENABLED);
						break;
					}
				case CRssFeed::isRss:			// δ����
					{
						menuRSS.EnableMenuItem(MP_RESUME, MF_ENABLED);
						break;
					}
				}
			}
		}
	}
	else//�հ״��Ҽ�
	{
		menuRSS.DeleteMenu(MP_OPEN,MF_STRING);
		menuRSS.DeleteMenu(MP_OPENFOLDER,MF_STRING);
		menuRSS.DeleteMenu(MP_DOWNLOAD_ALL,MF_STRING);
		menuRSS.DeleteMenu(MP_RESUME,MF_STRING);
		menuRSS.DeleteMenu(MP_PAUSE,MF_STRING);
		menuRSS.DeleteMenu(MP_CANCEL,MF_STRING);
		menuRSS.DeleteMenu(MP_OPTIONS,MF_STRING);
		//menuRSS.DeleteMenu(MF_SEPARATOR,MF_STRING);

		if ( this->GetItemCount() > 0 )
		{
			menuRSS.EnableMenuItem(MP_REFRESH, MF_ENABLED);
		}
		else
		{
			menuRSS.DeleteMenu(MP_REFRESH,MF_STRING);
		}
	}
}

// ����toolbar��ť״̬
void CFeedItemListCtrl::UpdateToolBarState(CToolBarCtrl & toolbar)
{
	if( !CGlobalVariable::IsRunning() )
	{
		return;
	}

	toolbar.EnableButton(MP_PAUSE, FALSE);
	toolbar.EnableButton(MP_RESUME, FALSE);
	toolbar.EnableButton(MP_STOP, FALSE);
	toolbar.EnableButton(MP_OPENFOLDER, FALSE);
	toolbar.EnableButton(MP_CANCEL, FALSE);

	int iSelectedItems = GetSelectedCount();
	if(iSelectedItems && iSelectedItems == 1)
	{
		int nIndex = GetCurrSelectIndex();
		if ( nIndex >= 0 )
		{
			if ( CRssFeed* pFeed = GetFeed(nIndex) )
			{
				toolbar.EnableButton(MP_OPENFOLDER, TRUE);

				//��������||��ͣ
				if (pFeed->ExistDownloadingItem())
				{
					toolbar.EnableButton(MP_PAUSE, TRUE);
				}

				if (pFeed->ExistCanResumeItem())
				{
					toolbar.EnableButton(MP_RESUME, TRUE);
				}

				toolbar.EnableButton(MP_CANCEL, TRUE);
			}

			if ( CRssFeed::Item * pItem = GetFeedItem(nIndex) )
			{
				switch ( pItem->GetState() )
				{
				case CRssFeed::isDownloading:	// �Ѷ��ģ�partfile������
					{
						toolbar.EnableButton(MP_PAUSE, TRUE);
						break;
					}
				case CRssFeed::isDownloaded:	// �Ѷ��ģ�partfile�������
					{
						toolbar.EnableButton(MP_OPENFOLDER, TRUE);
						break;
					}
				case CRssFeed::isPaused:		// �Ѷ��ģ�partfile����ͣ
					{
						toolbar.EnableButton(MP_RESUME, TRUE);
						break;
					}
				case CRssFeed::isStopped:		// �Ѷ��ģ�partfile��ֹͣ
					{
						toolbar.EnableButton(MP_RESUME, TRUE);
						break;
					}
				case CRssFeed::isHistory:		// ����״ֻ̬����������ͣ/ֹͣ������
					{
						//toolbar.EnableButton(MP_RESUME, TRUE);
						break;
					}
				case CRssFeed::isRss:			// δ����
					{
						//toolbar.EnableButton(MP_RESUME, TRUE);
						break;
					}

				}
			}
		}
	}
}

// ȡ�õ�ǰѡ���index
int CFeedItemListCtrl::GetCurrSelectIndex()
{
	POSITION pos = GetFirstSelectedItemPosition();
	if ( pos != NULL )
	{
		return  GetNextSelectedItem(pos);
	}
	return -1;
}

void CFeedItemListCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	if (lpDrawItemStruct->CtlType == ODT_MENU)
		m_pMenuXP->DrawItem(lpDrawItemStruct);
	else
		CBaseListCtrl::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CFeedItemListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
		m_pMenuXP->MeasureItem(lpMeasureItemStruct);
	else
		CBaseListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

BOOL CFeedItemListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if (  /*LOWORD(wParam) == MP_STOP
		||*/LOWORD(wParam) == MP_DOWNLOAD_ALL
		||LOWORD(wParam) == MP_OPEN
		||LOWORD(wParam) == MP_OPENFOLDER
		||LOWORD(wParam) == MP_RESUME
		||LOWORD(wParam) == MP_PAUSE
		||LOWORD(wParam) == MP_CANCEL
		||LOWORD(wParam) == MP_OPTIONS)
	{
		POSITION Pos = this->GetFirstSelectedItemPosition();
		if (Pos == NULL)
		{
			return 0;
		}

		CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;

		int nIndex = this->GetNextSelectedItem(Pos);
		if ( nIndex == -1 )
		{
			return 0;
		}

		CRssFeed::Item* pItem = this->GetFeedItem(nIndex);
		if (pItem == NULL)
		{//���ڵ�
			if( CRssFeed* pFeed = this->GetFeed(nIndex) )
			{
				if ( LOWORD(wParam) == MP_OPENFOLDER)
				{
					if(PathFileExists(pFeed->m_strSaveDir))
						ShellExecute(NULL, _T("open"), _T("explorer.exe"), pFeed->m_strSaveDir, NULL, SW_SHOW);
				}
				if ( LOWORD(wParam) == MP_DOWNLOAD_ALL )
				{
					((CRssFeedWnd*)GetParent())->DownloadAllItem(nIndex);
				}
				if ( LOWORD(wParam) == MP_RESUME )
				{
					((CRssFeedWnd*)GetParent())->ResumeAllItem(nIndex);
				}
				if ( LOWORD(wParam) == MP_PAUSE)
				{
					for ( CRssFeed::ItemIterator it = pFeed->GetItemIterBegin();
						it != pFeed->GetItemIterEnd();
						++it
						)
					{
						CPartFile* pPartFile = it->second.GetPartFile();
						if (pPartFile != NULL)
						{
							if (pPartFile->CanPauseFile())
								pPartFile->PauseFile();		
						}
					}
				}
				if ( LOWORD(wParam) == MP_CANCEL)
				{
					::SendMessage(CGlobalVariable::s_wndRssCtrl->GetSafeHwnd(), WM_COMMAND, wParam, lParam);
				}
				if ( LOWORD(wParam) == MP_OPTIONS)
				{
					CDlgFeedConfig dlgFeedConfig(*pFeed);
					if ( dlgFeedConfig.DoModal() == IDOK )
					{
						// ��������
						pFeed->SetUpdateInterval(dlgFeedConfig.m_uUpdateInterval);
						pFeed->SetAutoDownload(dlgFeedConfig.m_uAutoDownload);
						CGlobalVariable::s_wndRssCtrl->SaveFeed(*pFeed);
						RedrawItems(nIndex, nIndex);
					}
				}
				UpdateToolBarState(wd.m_Toolbar);
			}
			return 0;
		}

		//�ӽڵ�
		CKnownFile* pKnownFile = pItem->GetShareFile();
		CPartFile* pPartFile = pItem->GetPartFile();

		if ( LOWORD(wParam) == MP_OPEN )
		{
			((CRssFeedWnd*)GetParent())->OpenFeedItem(nIndex);
		}
		if ( LOWORD(wParam) == MP_OPENFOLDER )
		{
			if ( pKnownFile != NULL)
			{
				CmdFuncs::OpenFolder(pKnownFile);
				UpdateToolBarState(wd.m_Toolbar);
				return 0;
			}
			if ( pPartFile != NULL)
			{
				CmdFuncs::OpenFolder(pPartFile);
				UpdateToolBarState(wd.m_Toolbar);
				return 0;
			}
			if ( pPartFile == NULL && pKnownFile == NULL )
			{
				if(PathFileExists((pItem->m_pFeed)->m_strSaveDir))
					ShellExecute(NULL, _T("open"), _T("explorer.exe"), (pItem->m_pFeed)->m_strSaveDir, NULL, SW_SHOW);
				UpdateToolBarState(wd.m_Toolbar);
				return 0;
			}
		}
		
		if ( pPartFile != NULL && LOWORD(wParam) == MP_PAUSE)
		{
			if (pPartFile->CanPauseFile())
				pPartFile->PauseFile();			
		}
		else if ( LOWORD(wParam) == MP_RESUME)
		{
			if (pItem->GetState() == CRssFeed::isHistory || pItem->GetState() == CRssFeed::isRss)
			{//������
				((CRssFeedWnd*)GetParent())->DownloadFeedItem(nIndex);
			}
			else if( pPartFile != NULL )
			{//��ͣ����
				if (pPartFile->CanResumeFile())
				{
					if (pPartFile->GetStatus() == PS_INSUFFICIENT)
						pPartFile->ResumeFileInsufficient();
					else
						pPartFile->ResumeFile();
				}
			}
		}

		/*		if (LOWORD(wParam) == MP_STOP)
		{
			if (pPartFile->CanStopFile())
				pPartFile->StopFile();			
		}
		else*/

		UpdateToolBarState(wd.m_Toolbar);

		return 0;
	}
	::SendMessage(CGlobalVariable::s_wndRssCtrl->GetSafeHwnd(), WM_COMMAND, wParam, lParam);
	return CBaseListCtrl::OnCommand(wParam, lParam);
}

void CFeedItemListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here

	CTitleMenu	menuRSS;
	menuRSS.CreatePopupMenu();

	menuRSS.AppendMenu(MF_STRING,MP_OPEN,GetResString(IDS_OPEN),_T("OPEN"));
	menuRSS.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	//menuRSS.AppendMenu(MF_STRING|MF_SEPARATOR);
	menuRSS.AppendMenu(MF_STRING,MP_DOWNLOAD_ALL, GetResString(IDS_RSSBTN_ALLDL), _T("RESUME"));//��������
	menuRSS.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DOWNLOAD), _T("RESUME"));
	menuRSS.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_PAUSE), _T("PAUSE"));
	menuRSS.AppendMenu(MF_STRING,MP_CANCEL, GetResString(IDS_DELETE_FILE), _T("DELETE"));
	//menuRSS.AppendMenu(MF_STRING|MF_SEPARATOR);
	menuRSS.AppendMenu(MF_STRING,MP_REFRESH, GetResString(IDS_SV_UPDATE), NULL );
	menuRSS.AppendMenu(MF_STRING,MP_OPTIONS, GetResString(IDS_RW_RSS_CONFIG), NULL );
	//menuRSS.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));

	menuRSS.EnableMenuItem(MP_OPEN, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_OPENFOLDER, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_DOWNLOAD_ALL, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_RESUME, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_PAUSE, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_CANCEL, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_REFRESH, MF_GRAYED);
	menuRSS.EnableMenuItem(MP_OPTIONS, MF_GRAYED);
	//menuRSS.EnableMenuItem(MP_STOP, MF_GRAYED);

	UpdateMenuState(menuRSS);

	m_pMenuXP = new CMenuXP();
	m_pMenuXP->AddMenu(&menuRSS, TRUE);
	menuRSS.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	menuRSS.DestroyMenu();

	delete m_pMenuXP;
	m_pMenuXP = NULL;
}

void CFeedItemListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CMuleListCtrl::OnMouseMove(nFlags, point);

	HCURSOR hCurHandle = LoadCursor(NULL, IDC_ARROW);

	int index = GetItemIndex(point);
	if ( index >= 0 )
	{
		if ( CRssFeed::Item * pFilmItem = GetFeedItem(index) )
		{
			if ( pFilmItem->m_pAttachItem != NULL &&		// ����Ļ
				 pFilmItem->m_pAttachItem->CanDownload()	// ��������
			   )	
			{
				CRect rect;
				GetSubtitleRect(rect, index);
				if ( !rect.IsRectEmpty() && rect.PtInRect(point) )
				{
					// ��Ϊ���͹��
					hCurHandle = LoadCursor(NULL, IDC_HAND);
				}
			}
		}
	}
	SetCursor(hCurHandle);
}

// ȡ���б���ɫ
inline COLORREF CFeedItemListCtrl::GetLineBackColor(BOOL bIsEven)
{
	return bIsEven ? ITEM_BACKGROUND_COLOR1 : ITEM_BACKGROUND_COLOR2;
}

// ȡ���б���ɫ
inline COLORREF CFeedItemListCtrl::GetItemBackColor(int index)
{
	return GetLineBackColor((index - GetTopIndex()) % 2 == 0);
}

BOOL CFeedItemListCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	return TRUE;
	//return CBaseListCtrl::OnEraseBkgnd(pDC);
}

void CFeedItemListCtrl::OnPaint()
{
	CRect rectClip;
	{	
		// TODO: �ڴ˴������Ϣ����������
		CPaintDC dcPaint(this); // device context for painting
		// ��Ϊ��ͼ��Ϣ���� CMuleListCtrl::OnPaint()
		//CMuleListCtrl::OnPaint();

		dcPaint.GetClipBox(rectClip);
	}

	{
		CRect rectClient;
		GetClientRect(rectClient);

		CClientDC dcClient(this);	
		CMemDC dcMem(&dcClient, rectClient);
		dcMem.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dcClient, 0, 0, SRCCOPY);

		CFont * pListFont = GetFont();
		CFont * pOrgMemFont = NULL;
		if ( pListFont != NULL )
		{
			pOrgMemFont = dcMem.SelectObject(pListFont);
		}
		dcMem.SetBkMode(TRANSPARENT);

		// ��������Ŀ
		DrawItems(dcMem, rectClip, rectClient);

		if ( pOrgMemFont != NULL )
		{
			dcMem.SelectObject(pOrgMemFont);
		}
		dcMem.Flush();
	}
}

/// ȡ�ÿ���������feed
CRssFeed * CFeedItemListCtrl::GetBlankLineFeed(int index)
{
	while ( index >= 0 )
	{
		if ( CRssFeed * pFeed = GetFeed(index) )
		{
			return pFeed;
		}
		index--;
	}
	return NULL;
}

// ��������Ŀ
void CFeedItemListCtrl::DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient)
{
	CPen penSplit(PS_SOLID, 1, SPLIT_LINE_COLOR);
	CRect rectLine;
	CRect rectIntersect;
	int iLine = 0;
	int iCoverColWidth = GetColumnWidth(COL_INDEX_COVER);
	BOOL bIsCoverExpand = (iCoverColWidth > COVER_COLLAPSE_WIDTH);
	
	// ������Ŀ�����λ��
	int iCurrItemLeft = rectClient.left;

	int iFrom, iEnd;
	if ( GetPageRange(iFrom, iEnd) )
	{
		// չ������ģʽ�£���iFrom��Ϊfeed����λiFromΪ��һ����Ч�Ķ���
		if ( bIsCoverExpand )
		{
			if (  GetFeed(iFrom) == NULL )
			{
				while ( iFrom > 0 )
				{
					iFrom--;
					iLine--;
					if ( GetFeed(iFrom) != NULL )
					{
						break;
					}
				}
			}

			// ���ػ������һ���Ƕ����У��������+1
			if ( GetFeed(iEnd) != NULL )
			{
				if ( iEnd + 1 < GetItemCount() )
				{
					iEnd++;
				}
			}

			// չ������ģʽ�£���iEnd��λ������ĩβ��
			//int iLastIndex = GetItemCount() - 1;
			//while ( iEnd < iLastIndex )
			//{
			//	if ( GetFeed(iEnd + 1) != NULL )
			//	{
			//		// ��ǰiEndΪ���ĵ����һ�У���Ϊ��һ����һ���¶��ģ�
			//		break;
			//	}
			//	iEnd++;
			//}
		}

		// ������Ŀ���Ҷ�λ��
		int iCurrItemRigth = rectClient.right;
		{
			CRect rectRight;
			ASSERT(GetHeaderCtrl() != NULL);
			for ( int i = GetHeaderCtrl()->GetItemCount() - 1; i >= 0; i-- )
			{
				if ( !IsColumnHidden(i) )
				{
					if ( GetSubItemRect(iFrom, i, LVIR_LABEL, rectRight) )
					{
						iCurrItemRigth = rectRight.right;
					}
					break;
				}
			}
		}

		// ������Ŀ�����λ��
		{
			CRect rectLeft;
			if ( GetItemRect(iFrom, rectLeft, LVIR_BOUNDS) )
			{
				iCurrItemLeft = rectLeft.left;
			}
		}

		// ��������Ŀ
		CRect rectCover;
		DRAWITEMSTRUCT DrawItemStruct;
		memset(&DrawItemStruct, 0, sizeof(DrawItemStruct));
		DrawItemStruct.hDC = dcMem;
		int iSelectIndex = GetCurrSelectIndex();
		int iLastCoverTop = 0;
		CFeedItemListCtrl::CtrlBase * pLastCtrl = NULL;
		for ( ; iFrom <= iEnd; ++iFrom, ++iLine )
		{
			DrawItemStruct.itemID = iFrom;
			DrawItemStruct.itemState = (iFrom == iSelectIndex ? ODS_SELECTED : 0);
			DrawItemStruct.itemData = GetItemData(iFrom);
			GetItemRect(iFrom, &DrawItemStruct.rcItem, LVIR_BOUNDS);
			DrawItemStruct.rcItem.right = rectClient.right;
			int iCurrItemTop = DrawItemStruct.rcItem.top;

			CFeedItemListCtrl::CtrlBase * pCtrl = reinterpret_cast<CFeedItemListCtrl::CtrlBase *>(DrawItemStruct.itemData);

			// �������һ����Ŀ��λ��
			if ( iFrom == iEnd )
			{
				rectLine = DrawItemStruct.rcItem;
				rectLine.right = rectClient.right;
				rectLine.OffsetRect(0, ITEM_HEIGHT);
			}

			// ����Ҫ�ػ������ػ���Ŀ����������Ԫ��
			if ( iFrom == iSelectIndex || 
				 rectIntersect.IntersectRect(rectClip, &DrawItemStruct.rcItem) 
			   )
			{
				// ����ť
				HDWP hWinPosInfo = NULL;			
				DrawDownloadButton(iFrom, pCtrl, hWinPosInfo);

				// ��������
				COLORREF crBack = GetLineBackColor(iLine % 2 == 0);
				dcMem.FillSolidRect(DrawItemStruct.rcItem.left + iCoverColWidth, 
									DrawItemStruct.rcItem.top, 
									DrawItemStruct.rcItem.right - (DrawItemStruct.rcItem.left + iCoverColWidth),
									ITEM_HEIGHT,
									crBack
								   );

				// �����ֺ�ͼƬ
				DrawItemStruct.rcItem.right = iCurrItemRigth;
				DrawItem(&DrawItemStruct);
			}

			// չ���������Ҫ������
			if ( bIsCoverExpand )
			{
				BOOL bCanDrawCover = FALSE;
				int iCoverBottom = 0;
				if ( pCtrl != NULL && pCtrl->GetRssFeed() != NULL )
				{
					// ��ǰ���¶���
					iCoverBottom = iCurrItemTop - 1;
					bCanDrawCover = TRUE;
				}
				else if ( iFrom == iEnd )
				{
					// ��ǰ�����һ��
					iCoverBottom = iCurrItemTop + ITEM_HEIGHT - 1;
					bCanDrawCover = TRUE;
				}

				// ��Ϊ�µĶ����У�����һ�����ĵķ���
				// ��Ϊ���һ�У��������һ�����ĵķ���
				if ( bCanDrawCover )
				{
					if ( pLastCtrl != NULL )
					{
						CRssFeed * pFeed = pLastCtrl->GetRssFeed();
						ASSERT(pFeed != NULL);

						// ���㻭ͼ����
						rectCover.left = iCurrItemLeft;
						rectCover.top = iLastCoverTop;
						rectCover.right = rectCover.left + iCoverColWidth - 1;
						rectCover.bottom = iCoverBottom;

						// ��Ҫ�ػ�����ʱ���ػ�����
						if ( rectIntersect.IntersectRect(rectClip, rectCover) )
						{
							DrawCoverInfo(dcMem, rectCover, *pFeed);
						}

						// ����ָ���
						CPen *pPenOld = dcMem.SelectObject(&penSplit);
						dcMem.MoveTo(rectClient.left, iCoverBottom);
						dcMem.LineTo(rectClient.right, iCoverBottom);
						dcMem.SelectObject(pPenOld);
					}

					// ���浱ǰ������Ϣ��Ϊ�´λ�������׼��
					pLastCtrl = pCtrl;
					iLastCoverTop = iCurrItemTop;
				} // end if ( bCanDraw )
			} // end if ( bIsCoverExpand )
		} // end for (������Ŀ)
	}
	else
	{
		// ����Ŀʱ�����һ��λ��
		ASSERT( GetHeaderCtrl() != NULL );
		CRect rectHeader;
		GetHeaderCtrl()->GetClientRect(rectHeader);

		rectLine.left = rectClient.left;
		rectLine.top = rectHeader.bottom;
		rectLine.right = rectClient.right;
		rectLine.bottom = rectLine.top + ITEM_HEIGHT;
	}

	{
		// �����µķ��汳��
		CRect rectCoverBg;
		rectCoverBg.left = iCurrItemLeft;
		rectCoverBg.right = rectCoverBg.left + iCoverColWidth - 1;
		rectCoverBg.bottom = rectClient.bottom;

		// �������۵���û����Ŀʱ����շ��汳����
		if ( !bIsCoverExpand || iFrom < 0 || iEnd < 0 )
		{
			rectCoverBg.top = rectClient.top;	
		}
		else
		{
			// ���µ�����
			rectCoverBg.top = rectLine.top;
		}

		if ( rectIntersect.IntersectRect(rectClip, rectCoverBg) )
		{
			dcMem.FillSolidRect(rectCoverBg, COVER_BACKGROUND_COLOR);
		}

	}

	// �����µı�����
	int iCountPerPage = GetCountPerPage2();
	rectLine.left += iCoverColWidth;
	for ( ; iLine <= iCountPerPage; ++iLine )
	{
		// ��������
		COLORREF crBack = GetLineBackColor(iLine % 2 == 0);
		dcMem.FillSolidRect(rectLine, crBack);
		rectLine.OffsetRect(0, ITEM_HEIGHT);
	}

	// �����ָ���
	{
		CPen *pPenOld = dcMem.SelectObject(&penSplit);
		CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
		ASSERT(pHeaderCtrl != NULL);
		int iColCount = pHeaderCtrl->GetItemCount();
		for( int iCurrent = 0, x = rectClient.left + iCurrItemLeft - 1; iCurrent < iColCount; iCurrent++ )
		{
			if ( IsColumnHidden(iCurrent) )
			{
				continue;
			}

			x += GetColumnWidth(iCurrent);
			dcMem.MoveTo(x, rectClient.top);
			dcMem.LineTo(x, rectClient.bottom);
		}
		dcMem.SelectObject(pPenOld);
	}

}

/// ��������Ϣ
void CFeedItemListCtrl::DrawCoverInfo(CDC & dc, CRect & rect, const CRssFeed & feed)
{
	// ��䱳��ɫ
	dc.FillSolidRect(rect, COVER_BACKGROUND_COLOR);

	if (m_imageRss_PosterBg)
	{
		m_imageRss_PosterBg->Draw(dc,rect.left+3,rect.top+3);
	}

	// ������
	if (feed.m_pPosterImage)
	{
		feed.m_pPosterImage->Draw(dc,rect.left+3+10/2,rect.top+3+10/2);
	}
	else
	{
		if (m_imageRss_PosterDefault)
			m_imageRss_PosterDefault->Draw(dc,rect.left+3+10/2,rect.top+3+10/2);
	}

		//RGB(162,162,162) ->RGB(232,232,232) ����
	/*CRect rectRight(CPoint(rect.left+10+feed.m_pPosterImage->GetWidth(),rect.top+5+2),CPoint(rect.left+10+feed.m_pPosterImage->GetWidth()+3,rect.top+5+feed.m_pPosterImage->GetHeight()));
	CRect rectRight(CPoint(rect.left+10+feed.m_pPosterImage->GetWidth(),rect.top+5+2),CPoint(rect.left+10+feed.m_pPosterImage->GetWidth()+3,rect.top+5+feed.m_pPosterImage->GetHeight()));
	int nColor = 162;
	CPoint nPoint1,nPoint2;
	nPoint1 = rectRight.TopLeft();
	nPoint2 = CPoint(rectRight.left,rectRight.bottom);
	for (int i = 0;i<3;i++)
	{
		CPen pen;
		pen.CreatePen(PS_SOLID,   1,   RGB( nColor,nColor,nColor ));   
		CPen *pOldPen   =   dc.SelectObject(&pen);4

		dc.MoveTo(nPoint1);
		dc.LineTo(nPoint2);

		dc.SelectObject(pOldPen);   
		pen.DeleteObject();  

		nPoint1.Offset(1,0);
		nPoint2.Offset(1,0);

		nColor += 35;
	}*/
}

/// ��������
void CFeedItemListCtrl::DrawCoverColumnHead(BOOL bExpand)
{
	if ( m_bSingleStyle )
	{
		return;
	}

	CString strPosterName = _T("");
	if ( bExpand )//���ú����п��
	{
		this->SetColumnWidth(COL_INDEX_COVER,COVER_EXPAND_WIDTH);
		if (GetResString(IDS_RW_FEED_COVER) == _T("����") || GetResString(IDS_RW_FEED_COVER) == _T("����"))
			strPosterName = GetResString(IDS_RW_FEED_COVER)+_T("                                 ");
		else
			strPosterName = GetResString(IDS_RW_FEED_COVER)+_T("                   ");
		
	}
	else
	{
		this->SetColumnWidth(COL_INDEX_COVER,COVER_COLLAPSE_WIDTH);
	}
	//���ú����м�ͷ
	SetRssArrow(COL_INDEX_COVER, bExpand ? arrowExpand : arrowCollapse, strPosterName);	
}

/// ��ǰ�����Ƿ�չ��
inline BOOL CFeedItemListCtrl::IsCoverExpand() const
{
	return GetColumnWidth(COL_INDEX_COVER) > COVER_COLLAPSE_WIDTH;
}

void CFeedItemListCtrl::OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	ASSERT(phdr != NULL);
	ASSERT(pResult != NULL);

	switch ( phdr->iItem )
	{
		case COL_INDEX_COVER:
			*pResult = TRUE;
			break;

		default:
			*pResult = FALSE;
			break;
	}
}
void CFeedItemListCtrl::OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	ASSERT(phdr != NULL);
	ASSERT(pResult != NULL);

	switch ( phdr->iItem )
	{
		case COL_INDEX_COVER:
			*pResult = !m_bCanModifyCoverColumn;
			break;

		default:
			*pResult = FALSE;
			break;
	}
}

/// ȡ�÷�����������
void CFeedItemListCtrl::GetCoverRect(CRect & rect)
{
	rect.SetRectEmpty();

	if ( GetItemCount() > 0 )
	{
		if ( GetItemRect(0, rect, LVIR_LABEL) )
		{
			CRect rectClient;
			GetClientRect(rectClient);
			rect.bottom = rectClient.bottom;

			if ( GetItemRect(0, rectClient, LVIR_BOUNDS) )
			{
				rect.left = rectClient.left;
			}
		}
	}
}

/// ���з�������
BOOL CFeedItemListCtrl::IsHitCover(const CPoint & pt)
{
	CRect rectCover;
	GetCoverRect(rectCover);
	return rectCover.PtInRect(pt);	
}

void CFeedItemListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	// ��������ʱ�����ƶ�ѡ������
	if ( IsHitCover(point) )
	{
		return;
	}

	CMuleListCtrl::OnLButtonDown(nFlags, point);
}

void CFeedItemListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	// ��������ʱ�����ƶ�ѡ������
	if ( IsHitCover(point) )
	{
		return;
	}

	CMuleListCtrl::OnRButtonDown(nFlags, point);
}

void CFeedItemListCtrl::FilterItemsBySearchKey(CString* pSearchKey)
{
	if (!IsWindowVisible())
		return;

	if (pSearchKey == NULL || pSearchKey->IsEmpty())
	{
		if (m_stringMatcher.IsOriginal())
			return;
	}

	const StringMatcher::ListItemsNeedShow& listItemsNeedShow = m_stringMatcher.GetMatchResult(pSearchKey);

	SetRedraw(FALSE);

	DeleteAllItems();

	for (StringMatcher::ListItemsNeedShow::const_iterator ix = listItemsNeedShow.begin(); ix != listItemsNeedShow.end(); ++ix)
	{
		CRssFeed* feed = (CRssFeed*)*ix;
		AddFeed(GetItemCount(), *feed, FALSE);
	}

	if ( GetItemCount() <= 0 )
	{
		// ����Ŀ��ȫ��ɾ��
		DeleteAllButtons();
	}

	SetRedraw(TRUE);

}
BOOL CFeedItemListCtrl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	if (pMsg->message == WM_KEYDOWN)
	{
		if ( VK_ESCAPE == pMsg->wParam )
		{
			FilterItemsBySearchKey(NULL);
		}
	}

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}
