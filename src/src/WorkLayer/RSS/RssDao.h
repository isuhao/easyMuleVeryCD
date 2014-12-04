/*
 * $Id: RssDao.h 20462 2010-08-26 09:18:05Z yaveymao $
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

class CDatabase;
class CRssFilterList;

#include "rss/RssFeed.h"


/**
 * @brief ���ڱ���Ͷ�ȡ������������
 */
class CRssDao
{
public:
	CRssDao(void);
	virtual ~CRssDao(void);

public:
	/// �����ݿ�
	BOOL Open(const CString & strFilename);

	/// �Ƿ��Ѿ���
	BOOL IsOpen() const
	{
		return m_db != NULL;
	}

	/// �ر����ݿ�
	void Close();

	/// ����Rss Feed
	void LoadRssFeed(CRssFeedList & listFeed);

	/// ����FeedHistory
	void LoadFeedHistory(CRssFeedList & listHistory);

	/// ����FeedFilter
	void LoadFeedFilter(CRssFilterList & listFilter, CRssFeedList & listFeed);

	/// ɾ��Rss Feed
	void DeleteRssFeed(const CString & strFeedUrl);

	/// ����Rss Feed
	void SaveRssFeed(const CRssFeed & feed);

	/// �������е�Rss Feed
	void SaveRssFeeds(const CRssFeedList & listFeed);

	/// ������ʷ��Ŀ
	void SaveHistoryFeedItem(const CRssFeed::Item & item);

	/// ����feed��������ʷ��Ŀ
	void SaveHistoryFeed(const CRssFeed & feed);

	/// ����feed list��������ʷ��Ŀ
	void SaveHistoryFeeds(const CRssFeedList & listHistory);

	/// ���������
	void SaveFilter(const CRssFilter & filter);

	/// �����б������й�����
	void SaveFilters(const CRssFilterList & listFilter);

	/// ɾ��Rss Feed
	void DeleteFilter(const CString & strFilterName);

	/// ��ʼ��Catalogs���ݿ�
	void InitCatalogs();

	/// ����ȫ��catalog
	void LoadCatalogs(CFeedCatalogs & catalogs);

	/// ����һ��catalog
	void SaveCatalog(const FeedCatalog & catalog);

private:
	/// ִ��sql
	void ExecSql(CONST CHAR * strSql);

	/// ��ʼ����
	void BeginTrans()
	{
		ExecSql("begin transaction");
	}

	/// ��������
	void EndTrans()
	{
		ExecSql("end transaction");
	}

	/// ����insert catalog���sql
	void GenerateInsertCatalogSql(CStringA & strSql, unsigned int uId, const CString & strName, unsigned int uParentId);

private:
	/// ����ͨ�ַ���ת��Ϊsql����еĺϷ��ַ���
	static CStringA CStrToSqlStr(const CString & str);

private:
	CDatabase * m_db;
};
