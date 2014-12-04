/*
 * $Id: DlgMainTabRss.cpp 20779 2010-11-11 02:15:21Z gaoyuan $
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

// UILayer\DlgMainTabRss.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TabItem_Normal.h"
#include "TabItem_Wnd.h"
#include "PageTabBkDraw.h"
#include "eMule.h"
#include "emuleDlg.h"
#include "TabWnd.h"
#include "UILayer\DlgMainTabRss.h"
#include "UILayer\RssFeedWnd.h"
#include "UILayer\RssFilterWnd.h"
#include "UILayer\ToolBarCtrls\TbcRss.h"
#include "WorkLayer\RSS\RssFeedManager.h"
#include ".\dlgmaintabrss.h"
#include "DlgAddTask.h"
#include "CmdFuncs.h"
#include "Version.h"
#include "StatForServer.h"
#include "ThreadsMgr.h"

// CDlgMainTabRss �Ի���

IMPLEMENT_DYNAMIC(CDlgMainTabRss, CDialog)
CDlgMainTabRss::CDlgMainTabRss(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMainTabRss::IDD, pParent)
{
	m_pFeedManager = new CRssFeedManager;
	m_pwndRssFeed = new CRssFeedWnd;
	m_pUpdateThread = NULL;
/*
	m_pToolBar = new CTbcRss;
	m_plistHistory = new CFeedItemListCtrl;
	m_pwndRssFilter = new CRssFilterWnd;
	m_pTabWnd = new CTabWnd;
	m_posRss = NULL;
*/
	CGlobalVariable::s_wndRssCtrl = this;
}

CDlgMainTabRss::~CDlgMainTabRss()
{
	Clear();
}

/// �����Դ
void CDlgMainTabRss::Clear()
{
	CGlobalVariable::s_wndRssCtrl = NULL;

	/*
	delete m_pTabWnd;
	m_pTabWnd = NULL;

	delete m_plistHistory;
	m_plistHistory = NULL;

	delete m_pwndRssFilter;
	m_pwndRssFilter = NULL;

	delete m_pToolBar;
	m_pToolBar = NULL;
	*/

	if ( m_pwndRssFeed != NULL )
	{
		m_pwndRssFeed->DestroyWindow();
		delete m_pwndRssFeed;
		m_pwndRssFeed = NULL;
	}

	delete m_pFeedManager;
	m_pFeedManager = NULL;
	m_pUpdateThread = NULL;
}

// ֪ͨfeed�Ѹ��£����̵߳��ã�
void CDlgMainTabRss::OnNotifyRssFeedUpdated(const CString & strUrl, const char * lpszStream)
{
	SendMessage(WM_RSS_FEED_UPDATE, (WPARAM)(&strUrl), (LPARAM)lpszStream);
}

// ��ȡRSS���˺�Ľ��
void CDlgMainTabRss::OnRetrieveRssFilterResult(const CRssFilter & /*filter*/, CRssFeed::Item & item)
{
	// ��Ҫ����Ƿ�Ϊ��ʷ��Ŀ
	BOOL bDownloadResult = RequestDownloadFile(&item, FALSE);
	if ( bDownloadResult )
	{
		// ��Ӧ��feed����Ϊ����״̬
		//ASSERT(m_pwndRssFeed != NULL);
		//if ( CFeedItemListCtrl::CtrlFeed * pCtrlFeed = m_pwndRssFeed->m_listFeedItem.GetCtrlFeed(item.m_pFeed) )
		//{
		//	pCtrlFeed->m_bNew = true;
		//}
	}
}

// ��������һ���ļ�
BOOL CDlgMainTabRss::RequestDownloadFile(CRssFeed::Item * pItem, BOOL bIgnoreHistoryFlag)
{
	ASSERT(m_pFeedManager != NULL);
//	ASSERT(m_plistHistory != NULL);

	if ( pItem == NULL ||								// ����Ŀ��������
		 (!bIgnoreHistoryFlag && pItem->m_bIsHistory) ||	// ����������ʷ��Ϊ��ʷ��Ŀ��������
		 pItem->m_strEnclosure.IsEmpty() ||				// ������url��������
		 !pItem->CanDownload()							// ��ǰ״̬�������أ�������
	   )
	{
		return FALSE;
	}

	ASSERT(pItem->m_pFeed != NULL);

	// ������Դ
	CPartFile * pPartFile = CDlgAddTask::SilenceAddNewTask(pItem->m_strEnclosure, pItem->m_pFeed->m_strSaveDir);
	if ( pPartFile == NULL )
	{
		pPartFile = CRssFeed::GetPartFile(pItem->m_strEnclosure);
	}

	if ( !pItem->m_bIsHistory )
	{
		// ��¼������ص�����
		if ( pItem->m_timePubDate > pItem->m_pFeed->m_timeLastDownload )
		{
			pItem->m_pFeed->m_timeLastDownload = pItem->m_timePubDate;

			// ����feed
			SaveFeed(*static_cast<CRssFeed*>(pItem->m_pFeed));
		}

		// ���Ϊ��ʷ
		//CRssFeed::Item & itemHistory = m_pFeedManager->MarkHistory(*pItem);
		m_pFeedManager->MarkHistory(*pItem);
		ASSERT(pItem->m_bIsHistory);
	}

	// ����partfile
	//ASSERT(pItem->GetPartFile() == NULL);
	pItem->SetPartFile(pPartFile);

	// ��ӵ���ʷҳ����
	//m_plistHistory->AddFeedItem(itemHistory);

	// ����������+1
	theStatForServer.m_statRssInfo.wStartRssTasks++;

	return TRUE;
}

// ��ʼ��UI
void CDlgMainTabRss::InitUI()
{
	//ASSERT(m_pTabWnd != NULL);
	//ASSERT(m_pToolBar != NULL);
	ASSERT(m_pwndRssFeed != NULL);
//	ASSERT(m_plistHistory != NULL);
//	ASSERT(m_pwndRssFilter != NULL);

	ModifyStyle(0, WS_CLIPCHILDREN, 0);

//	m_pTabWnd->Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, CRect(0, 0, 0, 0), this, 0);
//	m_pTabWnd->SetBarBkDraw(new CPageTabBkDraw);

	//	add Toolbar
	/*
	m_pToolBar->Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, AFX_IDW_TOOLBAR);
	m_pToolBar->SetOwner(this);
	m_pToolBar->SetIndent(8);
	m_pToolBar->SetBkDraw(new CPageTabBkDraw);

	CTabItem_Wnd	*pTabItemWnd = new CTabItem_Wnd;
	pTabItemWnd->SetItemWnd(m_pToolBar, FALSE);
	pTabItemWnd->SetDynDesireLength(TRUE);
	m_pTabWnd->AddTab(pTabItemWnd);
	pTabItemWnd = NULL;

	// add rss feed table
	m_pwndRssFeed->Create(m_pwndRssFeed->IDD, this);
	m_pwndRssFeed->SetOwner(this);
	CTabItem_Normal	*pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(_T("����"));
	pTI_Normal->SetRelativeWnd(m_pwndRssFeed->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_posRss = m_pTabWnd->AddTab(pTI_Normal, TRUE);
	pTI_Normal = NULL;

	// add history table
	m_plistHistory->Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | 
						   LVS_NOLABELWRAP | LVS_ALIGNLEFT | WS_TABSTOP, 
						   CRect(0, 0, 100, 100),
						   this, 
						   IDC_RW_LIST_HISTORY
						  );
	m_plistHistory->Init();
	m_plistHistory->SetOwner(this);
	pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(_T("��ʷ"));
	pTI_Normal->SetRelativeWnd(m_plistHistory->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_pTabWnd->AddTab(pTI_Normal);
	pTI_Normal = NULL;

	// add filter table
	m_pwndRssFilter->Create(m_pwndRssFilter->IDD, this);
	m_pwndRssFilter->SetOwner(this);
	pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(_T("������"));
	pTI_Normal->SetRelativeWnd(m_pwndRssFilter->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_pTabWnd->AddTab(pTI_Normal);
	pTI_Normal = NULL;*/

	m_pwndRssFeed->Create(m_pwndRssFeed->IDD, this);
	m_pwndRssFeed->SetOwner(this);
	m_pwndRssFeed->ShowWindow(SW_NORMAL);
}

// ����feed�µ�������ʱ�䣨�����������������
void CDlgMainTabRss::SetFeedNewDownloadedTime(CRssFeed & feed)
{
	if ( this == NULL )
	{
		return;
	}

	ASSERT(m_pFeedManager != NULL);
	m_pFeedManager->SetNewDownloadedTime(feed);
}

// ���涩������
void CDlgMainTabRss::SaveFeed(const CRssFeed & feed)
{
	if ( this == NULL )
	{
		return;
	}

	ASSERT(m_pFeedManager != NULL);
	m_pFeedManager->SaveFeed(feed);
}

// ��Ӷ���
void CDlgMainTabRss::AddFeed(const CString & strFeedUrl, const CString & strSaveDir, BOOL bAutoDownload)
{
	if ( this == NULL )
	{
		return;
	}

	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	CString strPrompt;
	if ( m_pFeedManager->FindFeed(strFeedUrl) )
	{
		strPrompt = GetResString(IDS_RW_FEED_EXISTS_PROMPT);
	}
	else
	{
		CRssFeed & feed = m_pFeedManager->AddFeed(strFeedUrl, strSaveDir);
		m_pwndRssFeed->AddFeed(feed, TRUE);
		m_pwndRssFeed->AddICOFeed(feed, TRUE);
		feed.SetAutoDownload(bAutoDownload);
		if ( bAutoDownload )
		{
			feed.SetUpdateInterval(30 * 60);
		}
		else
		{
			feed.SetUpdateInterval(CRssFeed::MANUAL_REFRESH_VALUE);
		}
		//SaveRssConfig();

		StartUpdateFeed(feed, FALSE);
		strPrompt.Format(GetResString(IDS_RW_ADD_FEED_SUCC_PROMPT), static_cast<LPCTSTR>(strFeedUrl));
	}
	
	CGlobalVariable::ShowNotifier(strPrompt, TBN_IMPORTANTEVENT);
}

// ���AddFeed()������Feed
void CDlgMainTabRss::AddFeed(CRssFeed* pFeed, const CString & strSaveDir, BOOL bAutoDownload)
{

	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	CString strPrompt;
	if ( m_pFeedManager->FindFeed(pFeed->m_strFeedUrl) )
	{
		strPrompt = GetResString(IDS_RW_FEED_EXISTS_PROMPT);
	}
	else
	{
		CRssFeed & feed = m_pFeedManager->AddFeed(pFeed->m_strFeedUrl, strSaveDir);
		feed = *pFeed;

		m_pwndRssFeed->AddFeed(feed, TRUE);
		m_pwndRssFeed->AddICOFeed(feed, TRUE);
		feed.SetAutoDownload(bAutoDownload);
		if ( bAutoDownload )
		{
			feed.SetUpdateInterval(30 * 60);
		}
		else
		{
			feed.SetUpdateInterval(CRssFeed::MANUAL_REFRESH_VALUE);
		}
			
		feed.m_iState |= CRssFeed::fsRefresh;
		feed.m_bLastUpdateSucc = false;

		// ���Id�Ƿ���������е�Ҷ��
		CFeedCatalogs::iterator it = CFeedCatalogs::Find(feed.m_uCatalogId);

		if ( it != CFeedCatalogs::GetEnd() && it->second.IsLeaf() )
		{			
			it->second.m_setFeeds.insert(&feed);
			TRACE(TEXT("\n--- insert to CFeedCatalogs in New Fn --- \n"));
		}
		
		CRssFeedList listHistory = m_pFeedManager->GetHistoryFeedList();
		for (CRssFeedBase::ItemList::iterator it = feed.GetItemIterBegin(); it != feed.GetItemIterEnd(); ++it)
		{
			if (listHistory.FindFirstItem(it->second.m_strGuid))
			{
				it->second.m_bIsHistory = TRUE;
			}

			if ( !it->second.m_strEnclosure.IsEmpty() )
			{
				if ( !it->second.m_bIsHistory)
				{
					if ( it->second.GetPartFile() || it->second.GetShareFile() )
					{
						m_pFeedManager->MarkHistory(it->second);
					}
				}
			}	
		}

		feed.m_iState &= ~int(CRssFeed::fsRefresh);
		m_pFeedManager->SaveFeed(feed);

		feed.GetPoster(*this);

		// feed���³ɹ����ػ�ָ����feed��������Ŀ
		m_pwndRssFeed->RedrawFeed(feed);

		m_pwndRssFeed->m_listFeedItem.UpdateSort();//��������

		//��Ҫ�����µ��Ķ�����������һ��,�Ա�ListCtrl����Caption ����
		int nIndex = m_pwndRssFeed->m_listIcoFeedItem.GetItemIndex(&feed);

		CString strCaption = m_pwndRssFeed->m_listIcoFeedItem.GetExtentString(feed.GetDisplayName());
		m_pwndRssFeed->m_listIcoFeedItem.SetItemText(nIndex, 0, strCaption);
		
		strPrompt.Format(GetResString(IDS_RW_ADD_FEED_SUCC_PROMPT), static_cast<LPCTSTR>(pFeed->m_strFeedUrl));
	}

	CGlobalVariable::ShowNotifier(strPrompt, TBN_IMPORTANTEVENT);	
}

// partfile��ɾ��ʱ����
void CDlgMainTabRss::ProcessDeletePartFile(const CKnownFile * pPartFile)
{
	if ( this == NULL )
	{
		return;
	}

	if ( pPartFile == NULL || !pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile)) )
	{
		return;
	}

	if ( CRssFeed::Item * pItem = GetFeedItem(static_cast<const CPartFile *>(pPartFile)) )
	{
		// ֪ͨɾ��
		pItem->OnDeletePartFile();

		// �ػ�feed item
		ASSERT(m_pwndRssFeed != NULL);
		m_pwndRssFeed->RedrawFeedItem(*pItem);
	}

}
// partfile״̬����ʱ����
void CDlgMainTabRss::ProcessPartFileStateChanged(const CPartFile * pPartFile)
{
	if ( this == NULL )
	{
		return;
	}

	if ( CRssFeed::Item * pItem = GetFeedItem(pPartFile) )
	{
		ASSERT(m_pwndRssFeed != NULL);

		//// ����Դ����Ļ������������ɣ������ö��ĵ���������ļ���־
		//if ( !pItem->IsSubtitle() && pPartFile != NULL && pPartFile->GetStatus(false) == PS_COMPLETE )
		//{
		//	ASSERT(pItem->m_pFeed != NULL);
		//	pItem->m_pFeed->m_bHasNewComplete = true;
		//}

		switch ( pItem->GetState() )
		{
			case CRssFeed::isDownloading:
				// ���������ļ�ʱ�����������ʱ����Ҫ���
				pItem->ResetDownloadedTime();
				break;

			case CRssFeed::isDownloaded:
				// ��������ɣ�����
				theStatForServer.m_statRssInfo.wDownloadedRssTasks++;
				break;
		}

		// �ػ�feed������
		m_pwndRssFeed->RedrawFeedLine(pItem->m_pFeed);

		// �ػ�feed item
		m_pwndRssFeed->RedrawFeedItem(*pItem);

		CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
		m_pwndRssFeed->UpdateToolBarState(wd.m_Toolbar);//����toolbar״̬
	}

}
// ����partfile����feed item
CRssFeed::Item * CDlgMainTabRss::GetFeedItem(const CPartFile * pPartFile)
{
	if ( pPartFile != NULL )
	{
		ASSERT(m_pFeedManager != NULL);
		if ( CRssFeed::Item *pItem = m_pFeedManager->GetFeedList().FindFirstItem(m_pFeedManager, pPartFile) )
		{
			ASSERT(pItem->GetPartFile() == pPartFile);
			return pItem;
		}
	}

	return NULL;
}
// ��ʼ��feed������
void CDlgMainTabRss::InitFeedManager()
{
	ASSERT(m_pFeedManager != NULL);

	m_pFeedManager->LoadConfig();

	// �������Feed
	AddAllFeeds();

	m_pwndRssFeed->m_listFeedItem.UpdateSort(false);//��ʼ������
	// ���������ʷ��Ŀ
	//AddAllHistoryItems();

	// ������й�����
	//AddAllFilters();
}

// �������Feed
void CDlgMainTabRss::AddAllFeeds()
{
	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	m_pwndRssFeed->BeginUpdateFeed();	
	// ���feed
	CRssFeedList & listFeed = m_pFeedManager->GetFeedList();
	for ( CRssFeedList::iterator it = listFeed.begin();
		  it != listFeed.end();
		  ++it
		)
	{
		CRssFeed & feed = it->second;
		feed.GetPoster(*this);
		m_pwndRssFeed->AddFeed(feed, FALSE);
	}

	//�����ļ���feed
	CFeedCatalogs & feedCatalogs = CFeedCatalogs::GetInstance();
	for ( CFeedCatalogs::iterator it = feedCatalogs.GetBegin(); it != feedCatalogs.GetEnd(); ++it )
	{
		FeedCatalog & feedCatlog = it->second;
		for ( CRssFeedSet::const_iterator it = feedCatlog.m_setFeeds.begin(); it != feedCatlog.m_setFeeds.end(); ++it )
		{
			CRssFeedBase* pFeed = *it;
			if ( pFeed != NULL )
			{		
				CRssFeed & Feed = *( (CRssFeed*)pFeed);
				m_pwndRssFeed->AddICOFeed(Feed, FALSE);
			}	
		}
	}
	
	StartUpdateAllFeeds(FALSE);

	//�������,һ���Խ��з���
	m_pwndRssFeed->m_listIcoFeedItem.GroupAllFeedsByType(0);
	m_pwndRssFeed->EndUpdateFeed();
}

//// ���������ʷ��Ŀ
//void CDlgMainTabRss::AddAllHistoryItems()
//{
//	ASSERT(m_pFeedManager != NULL);
//	ASSERT(m_plistHistory != NULL);
//
//	CRssFeedList & listFeed = m_pFeedManager->GetHistoryFeedList();
//	m_plistHistory->SetRedraw(FALSE);
//	for ( CRssFeedList::iterator it = listFeed.begin();
//		  it != listFeed.end();
//		  ++it
//		)
//	{
//		CRssFeed & feed = it->second;
//
//		// �ػ�ȫ��feed items
//		m_plistHistory->AddAllFeedItem(feed);
//	}
//	m_plistHistory->SetRedraw(TRUE);
//}

// ������й�����
//void CDlgMainTabRss::AddAllFilters()
//{
//	
//	ASSERT(m_pFeedManager != NULL);
////	ASSERT(m_pwndRssFilter != NULL);
//
//	CRssFilterList & listFilter = m_pFeedManager->GetFilterList();
//	m_pwndRssFilter->BeginUpdateFilter();
//	for ( CRssFilterList::iterator it = listFilter.begin();
//		  it != listFilter.end();
//		  ++it
//		)
//	{
//		CRssFilter & filter = it->second;
//
//		// �ػ�ȫ��feed items
//		m_pwndRssFilter->AddFilterToList(filter);
//	}
//	m_pwndRssFilter->EndUpdateFilter();
//
//}

// ��������feed����
BOOL CDlgMainTabRss::StartUpdateAllFeeds(BOOL bCheckOverdue)
{	
	if (m_pUpdateThread != NULL)
	{
		return	TRUE;
	}
	else
	{
		UPDATETHREAD_PARAM* pParam = new UPDATETHREAD_PARAM;
		pParam->bCheckOverdue = bCheckOverdue;
		pParam->bCreateNewThread = FALSE;
		pParam->listFeed = m_pFeedManager->GetFeedList();
		CThreadsMgr::BegingThreadAndRecDown(CThreadsMgr::CleanProc_WaitAndDelWinThd, (AFX_THREADPROC)&UpdateThread, pParam);
	}
	
	return TRUE;
}

UINT CDlgMainTabRss::UpdateThread(LPVOID pParam)
{
	CUnregThreadAssist uta(GetCurrentThreadId());

	BOOL bCheckOver = ((UPDATETHREAD_PARAM*)pParam)->bCheckOverdue;
	BOOL bCreateNewThread = ((UPDATETHREAD_PARAM*)pParam)->bCreateNewThread;

	CRssFeedList & listFeed = ((UPDATETHREAD_PARAM*)pParam)->listFeed;
	
	for (CRssFeedList::iterator it = listFeed.begin(); it != listFeed.end(); ++it)
	{
		CRssFeed & feed = it->second;
		CGlobalVariable::s_wndRssCtrl->StartUpdateFeed(feed, bCheckOver, bCreateNewThread);
	}

	delete (UPDATETHREAD_PARAM*)pParam;
	CGlobalVariable::s_wndRssCtrl->m_pUpdateThread = NULL;
	return 0;
}

// ����һ��feed����
BOOL CDlgMainTabRss::StartUpdateFeed(CRssFeed & feed, BOOL bCheckOverdue, BOOL bCreateNewThread)
{
	// ��̸���ʱ����Ϊ30����
	UINT uOverdue = UINT(-1);
	if ( feed.UpdateIntervalIsDefined() )
	{
		// feed��UpdateInterval�Ѷ��壬ʹ�õ������õ�ֵ
		uOverdue = feed.GetUpdateInterval();
	}
	else
	{
		// feed��UpdateIntervalδ���壬ʹ��ȫ�����õ�ֵ
		uOverdue = (thePrefs.m_uRssUpdateInterval >= 30 * 60 ? thePrefs.m_uRssUpdateInterval : 30 * 60);
	}

	if ( !bCheckOverdue || feed.IsOverdue(uOverdue) )
	{
		ASSERT(m_pFeedManager != NULL);
		m_pFeedManager->StartUpdateFeed(feed, *this, bCreateNewThread);
		return TRUE;
	}

	return FALSE;
}


void CDlgMainTabRss::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMainTabRss, CDialog)
	ON_WM_SIZE()
	ON_MESSAGE(WM_RSS_FEED_UPDATE, OnFeedUpdate)
	ON_MESSAGE(WM_RSS_POSTER_UPDATE, OnPosterUpdate)
	ON_MESSAGE(WM_RSS_REQ_DOWNLOAD_FILE, OnReqDownloadFile)
	ON_COMMAND(MP_NEW, OnAddNewFeed)
	ON_COMMAND(MP_REFRESH, OnRefreshFeed)
	ON_COMMAND(MP_CANCEL, OnDeleteCommand)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgMainTabRss ��Ϣ�������

void CDlgMainTabRss::OnSize(UINT nType, int cx, int cy)
{
	//ASSERT(m_pTabWnd != NULL);

	CDialog::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	
	if ( m_pwndRssFeed->GetSafeHwnd() != NULL )
	{
		m_pwndRssFeed->MoveWindow(0, 0, cx, cy, FALSE);
	}
	
}

BOOL CDlgMainTabRss::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if ( pMsg->message == WM_KEYDOWN )
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if ( pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0 )
		{
			return FALSE;
		}

		if ( VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam )
		{
			return FALSE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlgMainTabRss::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	
	InitUI();
	InitFeedManager();

	SetTimer(UPDATE_FEED_TIME_ID, UPDATE_FEED_INTERVAL, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgMainTabRss::OnNotifyPosterDownload(CWebImage & webImage, const int & /* nCode */, CxImage* pImage)
{
	SendMessage(WM_RSS_POSTER_UPDATE, reinterpret_cast<WPARAM>(&webImage), reinterpret_cast<LPARAM>(pImage));
}

LRESULT CDlgMainTabRss::OnPosterUpdate(WPARAM wParam, LPARAM lParam)
{
	CWebImage * pWeb = reinterpret_cast<CWebImage*>(wParam);
	CxImage * pImage = reinterpret_cast<CxImage*>(lParam);
	ASSERT(pWeb != NULL);

	// ���Ҷ�Ӧ��feed�Ƿ����
	CRssFeed * pFeed = NULL;
	if ( CString * pFeedUrl = static_cast<CString*>(pWeb->m_pAttachData) )
	{
		ASSERT(m_pFeedManager != NULL);
		pFeed = m_pFeedManager->FindFeed(*pFeedUrl);
		pWeb->m_pAttachData = NULL;
		delete pFeedUrl;
	}

	// ��feed�����ڣ�����������Դ
	if ( pFeed == NULL )
	{
		delete pWeb;
		delete pImage;
		return FALSE;
	}

	ASSERT(pFeed->m_pWebImage == pWeb);
	
	// ���º���ʱ�����ı�Ȼ�޺���
	ASSERT(pFeed->m_pPosterImage == NULL);

	// ���溣��
	delete pFeed->m_pPosterImage;	// ��ǰ����Ӧ��Ϊ��
	pFeed->m_pPosterImage = pImage;

	// ɾ��������ȡ����
	CWebImage * pWebImage = pFeed->m_pWebImage;
	pFeed->m_pWebImage = NULL;
	delete pWebImage;

	// �ػ�����
	if ( pImage != NULL )
	{
		ASSERT(m_pwndRssFeed != NULL);
		m_pwndRssFeed->RedrawFeedLine(pFeed);
		//ICOList ����Ҫ�ػ�.��Ϊfeed�Ѿ�����
		//m_pwndRssFeed->RedrawFeedItemPoster(pFeed);
	}

	return TRUE;
}

LRESULT CDlgMainTabRss::OnFeedUpdate(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pFeedManager != NULL);
	ASSERT(m_pwndRssFeed != NULL);

	CString * pStrUrl = reinterpret_cast<CString *>(wParam);
	const char * lpszStream = reinterpret_cast<const char *>(lParam);
	ASSERT(pStrUrl != NULL);
	ASSERT(lpszStream != NULL);

	CRssFeed * pFeed = m_pFeedManager->FindFeed(*pStrUrl);
	if ( pFeed == NULL )
	{
		return NULL;
	}
	//��ȡfeed�Ƿ��Ѿ�ȡ�÷�����Ϣ
	BOOL bHasCatalogId = (pFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID);

	// ��������
	LRESULT result = m_pFeedManager->OnFeedUpdateMsg(*pStrUrl, lpszStream);

	if ( result != NULL )
	{
		CRssFeed * pFeed = reinterpret_cast<CRssFeed*>(result);

		if ( pFeed->AutoDownloadIsDefined() )
		{
			// feed�������Ƿ��Զ����أ�������Դ�feed�ĵ�������
			if ( pFeed->GetAutoDownload() )
			{
				// feed�������Ƿ��Զ����أ���Ҫ���Զ�����
				// ���������������Զ�����
				m_pFeedManager->StartFilter(*pFeed, *this);
			}
		}
		else if ( thePrefs.m_bRssAutoDownload )
		{
			// feedδ�����Ƿ��Զ����أ�ȫ������Ҫ���Զ�����
			// ���������������Զ�����
			m_pFeedManager->StartFilter(*pFeed, *this);
		}

		// feed���³ɹ������Ի�ȡ����
		pFeed->GetPoster(*this);

		// feed���³ɹ����ػ�ָ����feed��������Ŀ
		m_pwndRssFeed->RedrawFeed(*pFeed);

		m_pwndRssFeed->m_listFeedItem.UpdateSort();//��������

		//��Ҫ�����µ��Ķ�����������һ��,�Ա�ListCtrl����Caption ����
		int nIndex = m_pwndRssFeed->m_listIcoFeedItem.GetItemIndex(pFeed);

		if ( !bHasCatalogId )//�״�ȡ�÷�����Ϣ,���з���
		{
			m_pwndRssFeed->m_listIcoFeedItem.RemoveItemforMap(*pFeed, nIndex, true);
			m_pwndRssFeed->m_listIcoFeedItem.GroupFeedByType(nIndex,0);
		}

		CString strCaption = m_pwndRssFeed->m_listIcoFeedItem.GetExtentString(pFeed->GetDisplayName());
		m_pwndRssFeed->m_listIcoFeedItem.SetItemText(nIndex, 0, strCaption);
	}
	else
	{
		// feedû�и��³ɹ�
		// ������ɾ������ӵķ�ʽ���£��Է�ֹ�����������
		m_pwndRssFeed->m_listFeedItem.ReAddFeed(*pFeed);
	}

	return result;
}

LRESULT CDlgMainTabRss::OnReqDownloadFile(WPARAM wParam, LPARAM lParam)
{
	CRssFeed::Item * pItem = reinterpret_cast<CRssFeed::Item *>(wParam);
	return RequestDownloadFile(pItem, (BOOL)lParam);
}

void CDlgMainTabRss::OnTimer(UINT nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if ( nIDEvent == UPDATE_FEED_TIME_ID )
	{
		StartUpdateAllFeeds(TRUE);


		//// ÿ��һ��ʱ�䱣��һ�Σ��Է�ֹ��¿�������˳���ʧ���ĵ�����
		//SaveRssConfig();
	}

	CDialog::OnTimer(nIDEvent);
}

//// ��������
//void CDlgMainTabRss::SaveRssConfig()
//{
//	ASSERT(m_pFeedManager != NULL);
//
//	// ���иĶ����򱣴�
//	m_pFeedManager->SaveConfig(false);
//}

void CDlgMainTabRss::OnDestroy()
{
	KillTimer(UPDATE_FEED_TIME_ID);

	Clear();

	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
}

void CDlgMainTabRss::OnAddNewFeed()
{
	CmdFuncs::PopupNewTaskDlg();
}

void CDlgMainTabRss::OnRefreshFeed()
{
	ASSERT(m_pwndRssFeed != NULL);

	// ˢ��ָ����feed
	int index = -1;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		index = m_pwndRssFeed->m_listFeedItem.GetCurrSelectIndex();
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		index = m_pwndRssFeed->m_listSingleFeedItem.GetCurrSelectIndex();
	}

	if ( index < 0 )
	{
		// û��ѡ�е���ˢ��ȫ��
		StartUpdateAllFeeds(FALSE);
		return;
	}

	CRssFeed * pFeed;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		pFeed = m_pwndRssFeed->m_listFeedItem.GetFeed(index);
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pFeed = m_pwndRssFeed->m_listSingleFeedItem.GetFeed(index);
	}
	
	if ( pFeed == NULL )
	{
		return;
	}

	if ( StartUpdateFeed(*pFeed, FALSE) )
	{
		// ����feed
		// �ɹ��������£����ػ�feed������
		m_pwndRssFeed->RedrawFeedLine(pFeed);
	}

}

void CDlgMainTabRss::OnDeleteCommand()
{
	ASSERT(m_pwndRssFeed != NULL);
	ASSERT(m_pFeedManager != NULL);

	int index = -1;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{
		index = m_pwndRssFeed->m_listIcoFeedItem.GetCurrSelectIndex();
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		index = m_pwndRssFeed->m_listFeedItem.GetCurrSelectIndex();
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		index = m_pwndRssFeed->m_listSingleFeedItem.GetCurrSelectIndex();
	}

	if ( index < 0 )
	{
		return;
	}

	CRssFeed * pFeed;
	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{
		pFeed = m_pwndRssFeed->m_listIcoFeedItem.GetFeed(index);
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		pFeed = m_pwndRssFeed->m_listFeedItem.GetFeed(index);
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		pFeed = m_pwndRssFeed->m_listSingleFeedItem.GetFeed(index);
	}

	if ( pFeed == NULL )
	{
		// ��Դ��Ŀ����ʱ������
		return;
	}

	CString strPrompt;
	strPrompt.Format(GetResString(IDS_RW_DELETE_FEED_CONFIRM_PROMPT), static_cast<LPCTSTR>(pFeed->GetDisplayName()));
	int iResp = AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION	| MB_DEFBUTTON2);
	if ( iResp == IDYES	)
	{
		// UI����
		m_pwndRssFeed->RemoveFeed(*pFeed);
		m_pwndRssFeed->RemoveICOFeed(*pFeed);
		if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
		{//��InfoListɾ��Ҫ��ת��ICOģʽ
			m_pwndRssFeed->SwitchList(LISTSTYLE_ICO);
		}

		// ��������
		m_pFeedManager->DeleteFeed(*pFeed);

		// ��ֹ��Ŀ���ɫ��ʾ��λ
		//m_pwndRssFeed->RedrawAllFeed();
	}
}

void CDlgMainTabRss::Localize()
{
	if (m_pwndRssFeed && ::IsWindow(m_pwndRssFeed->m_hWnd))
	{
		m_pwndRssFeed->m_listFeedItem.Localize();
		m_pwndRssFeed->m_listSingleFeedItem.Localize();
		m_pwndRssFeed->m_wndRssInfo.Localize();
	}
}

CString CDlgMainTabRss::GetCurrentFileCommentString()
{
	if (!m_pwndRssFeed || !::IsWindow(m_pwndRssFeed->m_hWnd))
	{
		return CString(_T(""));
	}
	
	POSITION Pos = NULL;

	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{
		Pos = m_pwndRssFeed->m_listIcoFeedItem.GetFirstSelectedItemPosition();
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		Pos = m_pwndRssFeed->m_listFeedItem.GetFirstSelectedItemPosition();
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		Pos = m_pwndRssFeed->m_listSingleFeedItem.GetFirstSelectedItemPosition();
	}

	if (!Pos)
	{
		return CString(_T(""));
	}

	int nIndex;
	CRssFeed::Item* pItem;

	if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ICO)
	{//ICOģʽû��item
		nIndex = m_pwndRssFeed->m_listIcoFeedItem.GetNextSelectedItem(Pos);
		pItem = NULL;
	}
	else if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
	{
		nIndex = m_pwndRssFeed->m_listFeedItem.GetNextSelectedItem(Pos);
		pItem =  m_pwndRssFeed->m_listFeedItem.GetFeedItem(nIndex);
	}
	else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
	{
		nIndex = m_pwndRssFeed->m_listSingleFeedItem.GetNextSelectedItem(Pos);
		pItem =  m_pwndRssFeed->m_listSingleFeedItem.GetFeedItem(nIndex);
	}

	if (pItem && pItem->GetPartFile())
	{
		CPartFile * pPartFile = pItem->GetPartFile();
		
		CString strFileEd2k = CreateED2kLink(pPartFile, false);
		if( strFileEd2k.IsEmpty() )
		{
			return CString(_T(""));
		}

		bool bFileisFinished = true;
		if( pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile)) )
		{
			if( ((CPartFile*)pPartFile)->GetStatus()!=PS_COMPLETE )
				bFileisFinished = false;
		}

		CString strCommentUrl = bFileisFinished ? thePrefs.m_strFinishedFileCommentUrl : thePrefs.m_strPartFileCommentUrl;
		strCommentUrl.Replace(_T("[ed2k]"),strFileEd2k);
		strCommentUrl.Replace(_T("|"), _T("%7C"));

		CString sVersion;
		sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
		strCommentUrl += sVersion;
		return strCommentUrl;
	}
	else if (pItem)
	{
		CString strED2KUrl = pItem->m_strEnclosure;
		if( strED2KUrl.IsEmpty() || strED2KUrl.Left(7).CompareNoCase(_T("ed2k://")))
		{
			return CString(_T(""));
		}

		CString strCommentUrl = thePrefs.m_strPartFileCommentUrl;
		strCommentUrl.Replace(_T("[ed2k]"),strED2KUrl);
		strCommentUrl.Replace(_T("|"), _T("%7C"));

		CString sVersion;
		sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
		strCommentUrl += sVersion;
		return strCommentUrl;
	}
	else
	{
		CRssFeed* pFeed;
		if (m_pwndRssFeed->m_ListShowMode == LISTSTYLE_ITEM)
		{
			pFeed = m_pwndRssFeed->m_listFeedItem.GetFeed(nIndex);
		}
		else if(m_pwndRssFeed->m_ListShowMode == LISTSTYLE_SINGLE)
		{
			pFeed = m_pwndRssFeed->m_listSingleFeedItem.GetFeed(nIndex);
		}
		
		if ( pFeed == NULL )
		{
			pFeed = m_pwndRssFeed->m_listFeedItem.GetBlankLineFeed(nIndex - 1);
		}

		if (pFeed)
		{
			CString strCommentUrl = pFeed->m_strFeedUrl;
			strCommentUrl.MakeLower();
			int nPos = strCommentUrl.Find(_T("/feed"));
			if (nPos != -1)
			{
				strCommentUrl = strCommentUrl.Left(nPos);
				strCommentUrl +=_T("/comments/");
				return strCommentUrl;	
			}
		}

		return CString();
		//CString strFilePath;
		//strFilePath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Default.htm");
		//return strFilePath;
	}
}
