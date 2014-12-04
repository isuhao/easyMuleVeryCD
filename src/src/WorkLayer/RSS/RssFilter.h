/*
 * $Id: RssFilter.h 19972 2010-06-25 06:32:16Z yaveymao $
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

#include <afxstr.h>
#include "RssFeed.h"

class CRssFilter;


/**
 * @brief ���ڻ�ȡ����������������Դ�Ľӿ�
 */
struct IRetrieveRssFilterResult
{
	/// ���ڻ�ȡ����������������Դ
	virtual void OnRetrieveRssFilterResult(const CRssFilter & filter, CRssFeed::Item & item) = 0;
};

/**
 * @brief ���Ĺ����������ڴӶ����й��˳�������������Դ
 */
class CRssFilter
{
public:
	/// ���캯��
	CRssFilter();

	/// ��������
	virtual ~CRssFilter(void);

public:
	/// �����������
	void Clear();

	// ȡ�÷�����������Դ
	//void StartFilter(IRetrieveRssFilterResult & result);

	/// ȡ�÷�����������Դ
	bool StartFilter(CRssFeed & rfApplyFeed, IRetrieveRssFilterResult & result) const;

private:
	/// �ж���Ŀ�Ƿ���Ϲ�������
	bool IsRequest(const CRssFeed::Item & item) const;

public:
	CString				m_strName;			///< ����������
	CString				m_strFilter;		///< ���˹ؼ���
	CString				m_strNot;			///< �����ų��ؼ���
	CString				m_strQuality;		///< ��������
	CString				m_strFileType;		///< �����ļ�����
	CRssFeed *			m_pApplyFeed;		///< ָ��������Ӧ�����ĸ����ģ�Ϊ����Ӧ�������ж���
	DWORD				m_uMinSizeKB;		///< ������Դ����С��С��KB��
	DWORD				m_uMaxSizeKB;		///< ������Դ������С��KB��
	bool				m_bAutoStart;		///< ָ������������Ƿ��Զ���ʼ����
	bool				m_bEnable;			///< �������Ƿ���Ч
};

/**
 * @brief ά�����Ĺ������б�
 */
class CRssFilterList : public std::map<CString, CRssFilter>
{
public:
	/// ���캯��
	CRssFilterList()
		//:
		//m_bModified(false)
	{
	}

	/// ��������
	virtual ~CRssFilterList(void)
	{
	}

public:
	/// ����һ��filter
	CRssFilter & AddFilter(const CString & strName)
	{
		CRssFilter & result = (*this)[strName];
		if ( result.m_strName.IsEmpty() )
		{
			result.m_strName = strName;
		}
		//m_bModified = true;
		return result;
	}

	/// ɾ��һ��������
	void DeleteFilter(const CString & strName)
	{
		erase(strName);
		//m_bModified = true;
	}

	/// ȡ�÷�����������Դ
	void StartFilter(CRssFeed & rfApplyFeed, IRetrieveRssFilterResult & result)
	{
		// feed�й���������ʹ��ָ���Ĺ�����
		if ( CRssFilter * pFilter = rfApplyFeed.m_pFilter )
		{
			ASSERT(pFilter->m_pApplyFeed == &rfApplyFeed);
			pFilter->StartFilter(rfApplyFeed, result);
			return;
		}

		// ʹ��Ĭ�Ϲ�����
		for ( CRssFilterList::iterator it = this->begin();
			  it != this->end();
			  ++it
			)
		{
			if ( it->second.StartFilter(rfApplyFeed, result) )
			{
				break;
			}
		}
	}

public:
	//bool	m_bModified;
};