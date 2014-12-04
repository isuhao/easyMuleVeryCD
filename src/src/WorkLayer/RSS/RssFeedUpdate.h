/*
 * $Id: RssFeedUpdate.h 20761 2010-11-09 01:54:12Z gaoyuan $
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

#include <map>


/**
 * @brief ���ڶ������ݸ�����ɵ�֪ͨ�ӿ�
 */
struct IRssFeedUpdateNotify
{
	/// ֪ͨfeed�Ѹ��£����̵߳��ã�
	virtual void OnNotifyRssFeedUpdated(const CString & strUrl, const char * lpszStream) = 0;
};

/**
 * @brief ���ڸ��¶�������
 */
class CRssFeedUpdate
{
private:
	/// �����߳�
	typedef std::map<CString, CWinThread *>	UpdateThreads;
	
	/// ���²���
	struct UPDATE_PARAM
	{
		CString			m_strUrl;		///< ���ĵ�url
		IRssFeedUpdateNotify *	m_pNotify;		///< ������ɺ��֪ͨ����
	};

public:
	/// ���캯��
	CRssFeedUpdate();

	/// ��������
	virtual ~CRssFeedUpdate();

public:
	/// ����һ���첽���¶��ĵĹ���
	void StartUpdateFeed(const CString & strUrl, IRssFeedUpdateNotify & notify, BOOL bCreateNewThread = TRUE);

	/// ���½��������������Դ
	void OnUpdateFeedStopped(const CString & strUrl);

private:
	UpdateThreads		m_mapUpdateThreads;

private:
	/// ���´�����
	static UINT UpdateProcess( LPVOID pUpdateParam );

};

