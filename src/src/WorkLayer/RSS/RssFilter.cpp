/*
 * $Id: RssFilter.cpp 19972 2010-06-25 06:32:16Z yaveymao $
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

#include "StdAfx.h"
#include "RssFilter.h"


CRssFilter::CRssFilter()
{
	Clear();
}

CRssFilter::~CRssFilter(void)
{
}

// �����������
void CRssFilter::Clear()
{
	m_strName.Empty();
	m_strFilter.Empty();
	m_strNot.Empty();
	//m_strSaveDir.Empty();
	m_strQuality.Empty();
	m_strFileType.Empty();
	m_pApplyFeed = NULL;
	m_uMinSizeKB = 0;
	m_uMaxSizeKB = 0;
	m_bAutoStart = true;
	m_bEnable = true;
}

// ȡ�÷�����������Դ
// void CRssFilter::StartFilter(IRetrieveRssFilterResult & result)
// {
// 	if ( CRssFeed * feed = m_pApplyFeed )
// 	{
// 		StartFilter(*feed, result);
// 	}
// 	else
// 	{
// 		if ( CRssFeedList * list = result.OnGetAllRssFeed() )
// 		{
// 			for ( CRssFeedList::iterator it = list->begin();
// 				  it != list->end();
// 				  ++it
// 				)
// 			{
// 				StartFilter(it->second, result);
// 			}
// 		}
// 	}
// }

// ȡ�÷�����������Դ
bool CRssFilter::StartFilter(CRssFeed & rfApplyFeed, IRetrieveRssFilterResult & result) const
{
	if ( !m_bEnable )
	{
		// �ѽ���
		return false;
	}

	if ( m_pApplyFeed != NULL && m_pApplyFeed != &rfApplyFeed )
	{
		// ��������������ָ���Ķ���
		return false;
	}

	// ��������������ڣ�OnRetrieveRssFilterResult���ú��ֵ���ܱ�����
	CTime timeLastDownload = rfApplyFeed.m_timeLastDownload;
	if ( timeLastDownload >= rfApplyFeed.m_timeLastPub )
	{
		// ���µ���Դ�Ѿ���������
		return true;
	}

	// �������µ�ӰƬ��Դ����ӰƬ������Ļ����ͬʱ������Ļ
	for ( CRssFeed::ItemIterator it = rfApplyFeed.GetItemIterBegin();
		  it != rfApplyFeed.GetItemIterEnd();
		  ++it
		)
	{
		CRssFeed::Item & item = it->second;

		/*
			����������
			1) ��Դ�������� > �ϴ�������ص�����
			2-1) Ϊ����ӰƬ�������ͬ���ڵ�����ӰƬ���ɱ����أ�
			2-1-1) �������������
			�� 2-2) Ϊ��Ļ,������ص�ӰƬ�ѱ����أ���Ҳ�Զ�����֮
		*/

		// 1) ����Ƿ����㡰��Դ�������� > �ϴ�������ص����ڡ�
		if ( item.m_timePubDate > timeLastDownload )
		{
			// 2-1) Ϊ����ӰƬ�������ͬ���ڵ�����ӰƬ���ɱ����أ�
			if ( item.IsLastPubFilmItem() )
			{
				// 2-1-1) ����Ƿ��������������
				if ( IsRequest(item) )
				{
					result.OnRetrieveRssFilterResult(*this, item);

					// ������Ļ����ͬʱ������Ļ
					if ( item.m_pAttachItem != NULL )
					{
						result.OnRetrieveRssFilterResult(*this, *item.m_pAttachItem);
					}
				}
			}
			// �� 2-2) Ϊ��Ļ,������ص�ӰƬ�ѱ����أ���Ҳ�Զ�����֮
			else if ( !item.m_bIsHistory && item.IsSubtitle() &&
					  item.m_pAttachItem != NULL && item.m_pAttachItem->m_bIsHistory 
					)
			{
				result.OnRetrieveRssFilterResult(*this, item);
			}
		}
	}

	return true;
}

// �ж���Ŀ�Ƿ���Ϲ�������
bool CRssFilter::IsRequest(const CRssFeed::Item & item) const
{
	// �Ƿ�Ϊ��ʷ��Ŀ
	if ( item.m_bIsHistory )
	{
		return false;
	}

	// �Ƿ����ָ���ַ����������ִ�Сд��
	if ( !m_strFilter.IsEmpty() && StrStrI(item.m_strTitle, m_strFilter) == NULL )
	{
		// not found
		return false;
	}

	// �Ƿ�����ų��ַ����������ִ�Сд��
	if ( !m_strNot.IsEmpty() && StrStrI(item.m_strTitle, m_strNot) != NULL )
	{
		// found
		return false;
	}
	
	// ����С
	if ( m_uMinSizeKB != 0 && item.m_uEnclosureLength / 1024 < m_uMinSizeKB )
	{
		return false;
	}
	if ( m_uMaxSizeKB != 0 && item.m_uEnclosureLength / 1024 > m_uMaxSizeKB )
	{
		return false;
	}

	return true;
}

