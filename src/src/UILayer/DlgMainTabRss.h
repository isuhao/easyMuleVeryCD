/*
 * $Id: DlgMainTabRss.h 20761 2010-11-09 01:54:12Z gaoyuan $
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
#pragma once


// CDlgMainTabRss �Ի���
#include "resource.h"

#include "rss/RssFeedUpdate.h"
#include "rss/RssFilter.h"
#include "WebImage.h"

class CTabWnd;
class CTbcRss;
class CRssFeedWnd;
class CRssFeedManager;
class CRssFeed;
class CFeedItemListCtrl;
class CRssFilterWnd;
class CPartFile;

class CDlgMainTabRss : public CDialog, 
					   public IRssFeedUpdateNotify,		// feed �������֪ͨ�ӿ�
					   public IRetrieveRssFilterResult,	// ��ȡ���˽��
					   public IPosterDownloadNotify		// �����������֪ͨ
{
	DECLARE_DYNAMIC(CDlgMainTabRss)

private:

#if !defined(_DEBUG)
	static const UINT		UPDATE_FEED_INTERVAL	= 1000 * 60 * 5;	///< 5min for release
#else
	static const UINT		UPDATE_FEED_INTERVAL	= 1000 * 60 * 2;	///< 2min for debug
#endif

	static const UINT_PTR	UPDATE_FEED_TIME_ID		= 0x100;			///< ��ʱ��id


public:
	CDlgMainTabRss(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgMainTabRss();

// �Ի�������
	enum { IDD = IDD_MAINTAB_RSS };

	struct UPDATETHREAD_PARAM
	{
		CRssFeedList	listFeed;
		BOOL 	bCheckOverdue;
		BOOL	bCreateNewThread;
	};

public: // �ⲿ���ù��ܽӿ�
	// ��Ӷ���
	void AddFeed(const CString & strFeedUrl, const CString & strSaveDir, BOOL bAutoDownload);

	void AddFeed(CRssFeed* pFeed, const CString & strSaveDir, BOOL bAutoDownload);

	// ���涩������
	void SaveFeed(const CRssFeed & feed);

	CString GetCurrentFileCommentString();

	// partfile��ɾ��ʱ����
	void ProcessDeletePartFile(const CKnownFile * pPartFile);

	// partfile״̬����ʱ����
	void ProcessPartFileStateChanged(const CPartFile * pPartFile);

	// ����feed�µ�������ʱ�䣨�����������������
	void SetFeedNewDownloadedTime(CRssFeed & feed);

	void	Localize();

public:	// from IRssFeedUpdateNotify
	// ֪ͨfeed�Ѹ��£����̵߳��ã�
	virtual void OnNotifyRssFeedUpdated(const CString & strUrl, const char * lpszStream);

public: // from IRetrieveRssFilterResult
	virtual void OnRetrieveRssFilterResult(const CRssFilter & filter, CRssFeed::Item & item);

public: // from IPosterDownloadNotify
	virtual void OnNotifyPosterDownload(CWebImage & webImage, const int & nCode, CxImage* pImage);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

private:
	// ��ʼ��UI
	void InitUI();

	// ��ʼ��feed������
	void InitFeedManager();

	// ��������feed����
	BOOL StartUpdateAllFeeds(BOOL bCheckOverdue);

	static UINT UpdateThread(LPVOID pParam);

	// ����һ��feed����
	BOOL StartUpdateFeed(CRssFeed & feed, BOOL bCheckOverdue, BOOL bCreateNewThread = TRUE);

	// �������Feed
	void AddAllFeeds();

	// ���������ʷ��Ŀ
	//void AddAllHistoryItems();

	// ������й�����
	void AddAllFilters();

	// ��������һ���ļ�
	BOOL RequestDownloadFile(CRssFeed::Item * pItem, BOOL bIgnoreHistoryFlag);

	// ����partfile����feed item
	CRssFeed::Item * GetFeedItem(const CPartFile * pPartFile);

	//// ��������
	//void SaveRssConfig();

	/// �����Դ
	void Clear();

public:
	CRssFeedManager *	m_pFeedManager;
	//CTbcRss * 			m_pToolBar;
	CRssFeedWnd *		m_pwndRssFeed;
	CWinThread*		m_pUpdateThread;
	//CFeedItemListCtrl * m_plistHistory;
	//CRssFilterWnd *		m_pwndRssFilter;
	//CTabWnd * 			m_pTabWnd;
	//POSITION			m_posRss;

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnFeedUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPosterUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReqDownloadFile(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnAddNewFeed();
	afx_msg void OnRefreshFeed();
	afx_msg void OnDeleteCommand();
};
