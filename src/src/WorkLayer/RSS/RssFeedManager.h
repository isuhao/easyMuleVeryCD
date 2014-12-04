/*
 * $Id: RssFeedManager.h 20761 2010-11-09 01:54:12Z gaoyuan $
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

#include "RssFeed.h"
#include "RssFilter.h"
#include "RssFeedUpdate.h"
#include "RssDao.h"


/**
 * @brief �����ά�����ж��ļ��������������
 */
class CRssFeedManager
{
public:
	/// ���캯��
	CRssFeedManager(void);

	/// ��������
	virtual ~CRssFeedManager(void);

public:
	/// �������ļ��д���RssFeed����ʷ��Ϣ
	void LoadConfig();

	/// ���浽�����ļ���
	//void SaveConfig(bool bSaveAlways);

	//// �����Ƿ����
	//bool IsModified() const
	//{
	//	return m_listFeed.m_bModified || m_listFeedHistory.m_bModified || m_listFilter.m_bModified;
	//}

	//// ���ø��±�־
	//void ResetModified()
	//{
	//	m_listFeed.m_bModified = m_listFeedHistory.m_bModified = m_listFilter.m_bModified = false;
	//}

	/// ����һ��feed
	CRssFeed & AddFeed(const CString & strFeedUrl, const CString & strSaveDir);

	/// ����һ��feed
	CRssFeed * FindFeed(const CString & strFeedUrl)
	{
		return m_listFeed.FindFeed(strFeedUrl);
	}

	// ɾ��һ��feed
	//void DeleteFeed(const CString & strFeedUrl);

	/// ɾ��һ��feed
	void DeleteFeed(CRssFeed & feed);

	/// ����һ��������
	CRssFilter & AddFilter(const CRssFilter & filter);

	/// ɾ��һ��������
	void DeleteFilter(CRssFilter & filter);

	// �����ĿΪ��ʷ
	//CRssFeed::Item * MarkHistory(const CString & strGuid);

	/// �����ĿΪ��ʷ
	CRssFeed::Item & MarkHistory(CRssFeed::Item & item);

	// ɾ��һ����ʷ��Ŀ
	//void DeleteHistory(const CString & strGuid);

	// ˢ������feed
	//void RefreshFeed();

	/// ��������feed�߳�
	void StartUpdateFeed(CRssFeed & feed, IRssFeedUpdateNotify & notify, BOOL bCreateNewThread = TRUE)
	{
		feed.m_iState |= CRssFeed::fsRefresh;
		feed.m_bLastUpdateSucc = false;
		m_FeedUpdate.StartUpdateFeed(feed.m_strFeedUrl, notify, bCreateNewThread);
	}

	/// ��������������feed��ɸѡ��Դ
	void StartFilter(CRssFeed & feed, IRetrieveRssFilterResult & result)
	{
		m_listFilter.StartFilter(feed, result);
	}

	/// ȡ��feed�б�
	CRssFeedList & GetFeedList()
	{
		return m_listFeed;
	}

	/// ����feed�µ�������ʱ�䣨�����������������
	void SetNewDownloadedTime(CRssFeed & feed)
	{
		CTime timeDownloaded = feed.m_timeDownloaded;
		feed.SetNewDownloadedTime();
		if ( timeDownloaded != feed.m_timeDownloaded )
		{
			m_dao.SaveRssFeed(feed);
		}
	}

	// ȡ��feed�б�
	CRssFeedList & GetHistoryFeedList()
	{
		return m_listFeedHistory;
	}

	// ȡ�ù���ȥ�б�
	//CRssFilterList & GetFilterList()
	//{
	//	return m_listFilter;
	//}

	/// ����feed
	void SaveFeed(const CRssFeed & feed)
	{
		m_dao.SaveRssFeed(feed);
	}

public:
	/// ����feed���������Ϣ������һ���ɱ��Զ����ص�feed
	afx_msg LRESULT OnFeedUpdateMsg(const CString & strUrl, const char * lpszStream);

private:
	/// ������ʷ����
	void SetHistoryAlias(CRssFeedBase & feedHistory, const CRssFeedBase & feedNormal);

	/// ������ʷ����
	void SetHistoryAlias(CRssFeedBase & feedHistory);

	/// ˢ��feed
	void RefreshFeed(const CString & strFeedUrl, const char * lpszUrlStream)
	{
		if ( CRssFeed * feed =  m_listFeed.FindFeed(strFeedUrl) )
		{
			RefreshFeed(*feed, lpszUrlStream);
		}
	}

	/// ˢ��feed
	bool RefreshFeed(CRssFeed & feed, const char * lpszUrlStream);

	/// ��ini�����ļ��д���RssFeed����ʷ��Ϣ
	void LoadIniConfig(const CString & strConfigFile);

	/// ���浽ini�����ļ���
	//void SaveIniConfig(bool bSaveAlways);

private:
	CRssFeedList	m_listFeed;				///< rss feed�б�
	CRssFeedList	m_listFeedHistory;		///< rss feed��ʷ�����б�
	CRssFilterList	m_listFilter;			///< feed������
	CRssFeedUpdate	m_FeedUpdate;			///< ����feed
	CRssDao			m_dao;					///< Rss���ݷ��ʶ���

private:
	/// ȡ��ini�����ļ�����
	static CString GetIniConfigFilename();

	/// ȡ��db�����ļ�����
	static CString GetDbConfigFilename();

public:
#if defined(_DEBUG)
	static void test();
#endif

};
