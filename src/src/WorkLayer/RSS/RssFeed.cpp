/*
 * $Id: RssFeed.cpp 20761 2010-11-09 01:54:12Z gaoyuan $
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
#include <afxinet.h>
#include <algorithm>
#include <stdlib.h>
#include "RssFeed.h"
#include "../TinyXml/tinyxml.h"
#include "StringConversion.h"
#include "ED2KLink.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "GlobalVariable.h"
#include "langids.h"
#include "rss\RssFeedManager.h"
#include "WebImage.h"

/// �Ƿ�ǰ���·�������Դ
bool CRssFeedBase::Item::IsLastPubItem() const
{
	ASSERT(m_pFeed != NULL);
	if ( m_pFeed == NULL )
	{
		return false;
	}

	return (m_timePubDate >= m_pFeed->m_timeLastPub);
}

/// �Ƿ�ǰ���·�����ӰƬ��Դ
bool CRssFeedBase::Item::IsLastPubFilmItem() const
{
	ASSERT(m_pFeed != NULL);
	if ( m_pFeed == NULL )
	{
		return false;
	}

	// ����Ļ���ҷ�������Ϊ���
	return (!IsSubtitle() && m_timePubDate >= m_pFeed->m_timeLastFilmPub);
}


// �����������ʱ��
void CRssFeedBase::Item::GetDownloadedTime(CTime & timeDownloaded) const
{
	timeDownloaded = 0;

	// δ������ɷ���0
	if ( GetState() != isDownloaded )
	{
		return;
	}

	// ����Ƿ��ѱ�������ʱ��
	if ( m_timeDownloaded.GetTime() > 0 )
	{
		timeDownloaded = m_timeDownloaded;
		return;
	}

	// ����������ɵ����
	if ( CKnownFile * pKnowFile = GetShareFile() )
	{
		CRssFeedBase::GetLastWriteTime(*pKnowFile, m_timeDownloaded);
		timeDownloaded = m_timeDownloaded;
	}

}

// �Ƿ�Ϊ��Ļ
bool CRssFeedBase::Item::IsSubtitle() const
{	
	// ����С���ļ�������Ϊ����Ļ
	if ( !IsSmallFile() )
	{
		return false;
	}

	// VeryCD֧�ֵ���Ļ��ʽ��srt,ass,ssa (x.srt)
	if ( m_strFilename.GetLength() < 5 )
	{
		return false;
	}

	CString strExtName = m_strFilename.Right(4);
	if ( strExtName.CompareNoCase(_T(".srt")) == 0 ||
		 strExtName.CompareNoCase(_T(".ass")) == 0 ||
		 strExtName.CompareNoCase(_T(".ssa")) == 0
	   )
	{
		return true;
	}

	return false;
}

// ȡ��feed��Ŀ״̬
CRssFeedBase::EItemState CRssFeedBase::Item::GetState() const
{
	// ����ʷ��Ŀ
	if ( !m_bIsHistory )
	{
		return CRssFeedBase::isRss;
	}

	// ��ʷ��Ŀ
	if ( m_pPartFile != NULL )
	{
		// ����partfile�е�״̬
		return CRssFeedBase::GetItemStateFromPartFile(*m_pPartFile);
	}
	else if ( GetShareFile() != NULL )
	{
		// ��partfile�������
		return CRssFeedBase::isDownloaded;
	}
	else
	{
		// partfile�����
		return CRssFeedBase::isHistory;
	}
}

// partfile����ɾ��ʱ����
void CRssFeedBase::Item::OnDeletePartFile()
{
	// ����share file
	m_pPartFile = NULL;
	ResetDownloadedTime();
}

CRssFeedBase::CRssFeedBase()
{
	Init();
}

CRssFeedBase::CRssFeedBase(const CString & strFeedUrl)	
{
	Init();
	m_strFeedUrl = strFeedUrl;
}

CRssFeedBase::~CRssFeedBase(void)
{
}

void CRssFeedBase::Init()
{
	m_uCatalogId = FeedCatalog::INVALID_CATALOG_ID;
	m_iState = fsNormal;
	m_tmLastUpdate = 0;
	m_pFilter = NULL;
	//m_bHasNewComplete = false;

	m_uUpdateInterval = UNDEFINED_VALUE;
	m_uAutoDownload = UNDEFINED_VALUE;
	m_bLastUpdateSucc = false;
}

CRssFeedBase& CRssFeedBase::operator = (const CRssFeedBase& feed)
{
	m_strFeedUrl = feed.m_strFeedUrl;
	m_listItem = feed.m_listItem;

	m_bLastUpdateSucc = feed.m_bLastUpdateSucc;
	m_iState = feed.m_iState;
	m_pFilter = feed.m_pFilter;
	m_strAlias = feed.m_strAlias;
	m_strCategory = feed.m_strCategory;
	m_strDescription = feed.m_strDescription;
	m_strSaveDir = feed.m_strSaveDir;
	m_strTitle = feed.m_strTitle;
	m_timeDownloaded = feed.m_timeDownloaded;
	m_timeLastDownload = feed.m_timeLastDownload;
	m_timeLastFilmPub = feed.m_timeLastFilmPub;
	m_timeLastPub = feed.m_timeLastPub;
	m_tmLastUpdate = feed.m_tmLastUpdate;
	m_uAutoDownload = feed.m_uAutoDownload;
	m_uCatalogId = feed.m_uCatalogId;
	m_uUpdateInterval  = feed.m_uUpdateInterval;
	
	for (ItemIterator it = m_listItem.begin(); it != m_listItem.end(); ++it)
	{
		it->second.m_pFeed = this;
		if (it->second.m_pAttachItem != NULL)
		{
			BOOL bFound = FALSE;
			// m_pAttachItem ���򵽱� m_listItem �е� Item
			for (ItemIterator it2 = m_listItem.begin(); it2 != m_listItem.end(); ++it2)
			{
				if (it2->second.m_strGuid == it->second.m_pAttachItem->m_strGuid)
				{
					it->second.m_pAttachItem = &(it2->second);
					bFound = TRUE;
				}				
			}
			ASSERT(bFound);
		}
	}
	return *this;
}

// ���ȫ��
void CRssFeedBase::Clear()
{
	m_strTitle.Empty();
	m_strDescription.Empty();
	m_listItem.clear();
	m_timeLastFilmPub = 0;
}

// ��partfile�õ���Ŀ״̬
CRssFeedBase::EItemState CRssFeedBase::GetItemStateFromPartFile(const CPartFile & partfile)
{
	switch ( partfile.GetStatus(false) )
	{
		case PS_COMPLETE:
			return CRssFeedBase::isDownloaded;

		case PS_PAUSED:
			if ( partfile.IsStopped() )
			{
				return isStopped;
			}
			return CRssFeedBase::isPaused;

		default:
			if ( partfile.IsStopped() )
			{
				return isStopped;
			}
			if ( partfile.IsPaused() )
			{
				return isPaused;
			}
			return CRssFeedBase::isDownloading;
	}
}

// �Ƿ���ڣ����ں���Ҫ���£�
bool CRssFeedBase::IsOverdue(time_t tmOverdueSec) const
{
	return ( UINT64(time(NULL) - m_tmLastUpdate) >= UINT64(tmOverdueSec) );
}

/// ����Ƶ��
void CRssFeedBase::ParseChannel(TiXmlElement & elChannel, bool bIsUtf,bool bAddToFeedCatalogs)
{
	// parse title
	if ( TiXmlElement * el = elChannel.FirstChildElement("title") )
	{
		m_strTitle = GetElementText(el->GetText(), bIsUtf);
		//if ( m_strAlias.IsEmpty() && !m_strTitle.IsEmpty() )
		//{
		//	m_strAlias = m_strTitle;
		//}
	}

	// parse vc:catalogs
	if ( m_uCatalogId == FeedCatalog::INVALID_CATALOG_ID)
	{
		// û�����ʱ�Ž���
		if ( TiXmlElement * elCatalogs = elChannel.FirstChildElement("vc:catalogs") )
		{
			for ( TiXmlElement * el = elCatalogs->FirstChildElement("vc:catalog"); 
				  el != NULL;
				  el = el->NextSiblingElement("vc:catalog")
				)
			{
				const char * lpszId = el->Attribute("id");
				if ( lpszId == NULL )
				{
					continue;
				}

				// ��ID���붩�ĵ�catlog���Լ�����
				unsigned int uId = atoi(lpszId);

				// ���uId�Ƿ���������е�Ҷ��
				CFeedCatalogs::iterator it = CFeedCatalogs::Find(uId);
			
				// ����Ҷ�ӣ��򱣴��Ӧ��ϵ
				// �����ļ��뵽catlog�Ķ��ļ�����
				if ( it != CFeedCatalogs::GetEnd() && it->second.IsLeaf() )
				{
					if (bAddToFeedCatalogs)
					{
						it->second.m_setFeeds.insert(this);
					}			
					m_uCatalogId = uId;
					break;
				}

			}
		}
	}

	// parse category
	if ( TiXmlElement * el = elChannel.FirstChildElement("category") )
	{
		m_strCategory = GetElementText(el->GetText(), bIsUtf);
	}

	// parse description
	if ( TiXmlElement * el = elChannel.FirstChildElement("description") )
	{
		m_strDescription = GetElementText(el->GetText(), bIsUtf);
	}

}

/// �����ĴӶ��������Ϣ������ɾ��
void CRssFeedBase::RemoveFeedFromCatalogs(const CRssFeedBase & feed)
{
	CFeedCatalogs::iterator itCatlog = CFeedCatalogs::Find(feed.m_uCatalogId);
	if ( itCatlog != CFeedCatalogs::GetEnd() )
	{
		// �ҵ���Ӧ���ɾ������еĶ���
		itCatlog->second.m_setFeeds.erase(const_cast<CRssFeedBase*>(&feed));
	}
}


// ˢ��feed
bool CRssFeedBase::Refresh(CRssFeedManager * pManager, const char * lpszUrlStream, const CRssFeedList * pHistoryList)
{
	ASSERT(lpszUrlStream != NULL);
	
	if ( strstr(lpszUrlStream, "/rss>") == NULL )
	{
		return false;
	}

	// parse rss
	TiXmlDocument doc;
	doc.Parse(lpszUrlStream);
////////////////

	//FILE * fp = fopen("c:\\ttt.txt", "wb");
	//fwrite(sUrl.GetStream(), 1, sUrl.GetSize(), fp);
	//fclose(fp);

/////////////

	TiXmlHandle hDoc(&doc);
	TiXmlElement * elChannel = hDoc.FirstChild("rss").FirstChild("channel").ToElement();
	if ( elChannel == NULL )
	{
		return false;
	}

	// ���ԭ����Ϣ
	Clear();

	// ȷ���Ƿ�Ϊutf-8
	bool bIsUtf = false;
	CString strEncode = GetEncodeStr(lpszUrlStream);
	if ( StrStrI(strEncode, _T("UTF")) != NULL )
	{
		// found
		bIsUtf = true;
	}

	// channel
	ParseChannel(*elChannel, bIsUtf, pManager ? TRUE : FALSE);

	// items
	typedef std::vector<Item *> ItemPtrVector;
	typedef std::map<CString, Item *> ItemPtrMap;
	CTime timeOldLastPub = m_timeLastPub;
	CTime timeCurr = CTime::GetCurrentTime();
	Item * pLastPubFilmItem = NULL;
	ItemPtrVector vSubtitleItems;
	ItemPtrMap mapFilmItems;

	for ( TiXmlElement * elItem = elChannel->FirstChildElement("item"); 
		  elItem != NULL;
		  elItem = elItem->NextSiblingElement("item")
		)
	{
		if ( Item * item = ParseItem(pManager, *elItem, bIsUtf, pHistoryList) )
		{
			// ��¼���µ���Դ����
			if ( item->m_timePubDate > m_timeLastPub )
			{
				m_timeLastPub = item->m_timePubDate;
			}

			// ��¼���µ�ӰƬ��������
			if ( item->m_timePubDate > m_timeLastFilmPub && !item->IsSubtitle() )
			{
				m_timeLastFilmPub = item->m_timePubDate;

				// ��һ�θ���ʱ����Ҫ����Ϊ���µ���Ŀ
				pLastPubFilmItem = item;
			}

			// ����Ƿ�Ϊ����Դ
			if ( timeOldLastPub.GetTime() > 0 &&			// �ϴ����µķ������ڱ�����Ч
				 item->m_timePubDate > timeOldLastPub &&	// ��Դ�����ڴ����ϴ����µķ�������
				 timeCurr.GetTime() - item->m_timePubDate.GetTime() <= NEW_PUBDATE_OVERDUE_SECOND	// û�й���
			   )
			{
				item->m_bIsNewPub = true;
			}

			// ׼�������������Ļ����
			if ( item->IsSubtitle() )
			{
				vSubtitleItems.push_back(item);
			}
			else if ( !item->IsSmallFile() )
			{
				// ����С�ļ�ʱ������Ƿ���ƥ����Ļ
				CString strFileName = GetItemFileNameNoExt(*item);
				if ( !strFileName.IsEmpty() )
				{
					mapFilmItems[strFileName.MakeLower()] = item;
				}
			}
		}
	}

	// ��һ�μ���ʱ���������µ���ԴΪnew
	if ( !IsUpdated() )
	{
		// ��һ�μ���ʱ�������������������Ϊ������Դʱ�䣬�Ա��������κ���Դ
		m_timeLastDownload = m_timeLastPub;

		if ( pLastPubFilmItem != NULL )
		{
			pLastPubFilmItem->m_bIsNewPub = true;
		}
	}

	// �����Ļ
	for ( ItemPtrVector::iterator it = vSubtitleItems.begin();
		  it != vSubtitleItems.end();
		  ++it
		)
	{
		Item * pSubtitleItem = *it;

		ASSERT(pSubtitleItem != NULL);
		if ( pSubtitleItem == NULL )
		{
			continue;
		}

		if ( !pSubtitleItem->m_strFilename.IsEmpty() )
		{
			// ���verycd����Ļ ".chs.SRT" ".cht.srt" ".en.srt" ".eng.srt"
			CString strNoExtName = GetFileNameNoExt(pSubtitleItem->m_strFilename).MakeLower();	// "xx.chs" / "xx.cht" / "xx.en" / "xx.eng"
			CString strLang = strNoExtName.Right(3);	// chs / cht / .en / eng
			Item * pFilmItem = mapFilmItems[strNoExtName];
			if ( pFilmItem == NULL )
			{
				pFilmItem = mapFilmItems[GetFileNameNoExt(strNoExtName)];
			}
			if ( pFilmItem != NULL )
			{
				// ���Ļ���ƥ��������Ļ
				// ��������ƥ��Ӣ����Ļ
				// �������廷���������Ļ�����廷���䷱����Ļ
				BOOL bCanAttach = FALSE;
				switch ( thePrefs.GetLanguageID() )
				{
					case LANGID_ZH_CN:
						if ( strLang.CompareNoCase(_T("chs")) == 0 )
						{
							// ����ֱ��ƥ��
							bCanAttach = TRUE;
						}
						else if ( pFilmItem->m_pAttachItem == NULL && strLang.CompareNoCase(_T("cht")) == 0 )
						{
							// ����Ļʱ������ƥ�䷱��
							bCanAttach = TRUE;
						}
						break;

					case LANGID_ZH_TW:
						if ( strLang.CompareNoCase(_T("cht")) == 0 )
						{
							// ����ֱ��ƥ��
							bCanAttach = TRUE;
						}
						else if ( pFilmItem->m_pAttachItem == NULL && strLang.CompareNoCase(_T("chs")) == 0 )
						{
							// ����Ļʱ������ƥ�����
							bCanAttach = TRUE;
						}
						break;

					default:
						if ( pFilmItem->m_pAttachItem == NULL && 
							 (strLang.CompareNoCase(_T("eng")) == 0 || strLang.CompareNoCase(_T(".en")) == 0)
						   )
						{
							// �������Ի����£�����Ļʱ����ƥ��Ӣ����Ļ
							bCanAttach = TRUE;
						}
						break;
				}

				if ( bCanAttach )
				{				
					pFilmItem->m_pAttachItem = pSubtitleItem;
					pSubtitleItem->m_pAttachItem = pFilmItem;

					// ��Ϊ����Ļ�����Ӧ��ӰƬ����Ϊ��
					if ( pSubtitleItem->m_bIsNewPub )
					{
						pFilmItem->m_bIsNewPub = true;
					}
				}
			}
		}

		// ��Ļ������Ϊ�£���Ϊ��Ļ���ں�ӰƬ����һ����ʾ��
		pSubtitleItem->m_bIsNewPub = false;
	}

	m_tmLastUpdate = ::time(NULL);
	m_bLastUpdateSucc = true;

	return true;
}

// ȡ����Ŀ�ļ���
CString CRssFeedBase::GetItemFileNameNoExt(const Item & item)
{
	return GetFileNameNoExt(item.m_strFilename);
}

// ȡ���ļ���
CString CRssFeedBase::GetFileNameNoExt(const CString & strFileName)
{
	if ( strFileName.IsEmpty() )
	{
		return CString();
	}

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath(strFileName, 
				drive, //_MAX_DRIVE, 
				dir, //_MAX_DIR, 
				fname, //_MAX_FNAME, 
				ext	//, _MAX_EXT
			   );
	return CString(fname);
}


CString CRssFeedBase::GetEncodeStr(const char * pXml)
{
	ASSERT(pXml != NULL);
	
	const char * pBegin = StrStrIA(pXml, "encoding");
	if ( pBegin == NULL )
	{
		return CString();
	}

	pBegin = StrChrA(pBegin, '"');
	if ( pBegin == NULL )
	{
		return CString();
	}
	pBegin++;

	const char * pEnd = StrChrA(pBegin, '"');
	if ( pEnd == NULL )
	{
		return CString();
	}
	
	return CString(pBegin, pEnd - pBegin);
}

inline CString CRssFeedBase::GetElementText(const char * pText, bool bIsUtf)
{
	ASSERT(pText != NULL);

	if ( bIsUtf )
	{
		return OptUtf8ToStr(pText);
	}
	
	return CString(pText);
}

// �������õ�ed2k
void CRssFeedBase::GetEd2kInfoFromDesc(Item & item)
{
	// ����ed2k
	LPCTSTR lpszFind = StrStrI(item.m_strDescription, _T("ed2k://"));
	if ( lpszFind == NULL )
	{
		return;
	}
	int iEndPos = item.m_strDescription.ReverseFind('|');
	if (iEndPos == -1)
	{
		return;
	}
	item.m_strEnclosure = CString(lpszFind, iEndPos + 1);
	item.m_strEnclosure += '/';

	// ��������
	lpszFind = StrStrI(item.m_strEnclosure, _T("|file|"));
	if ( lpszFind == NULL )
	{
		return;
	}
	lpszFind = StrChr(lpszFind + 6, '|');
	if ( lpszFind == NULL )
	{
		return;
	}
	lpszFind++;
	LPCTSTR lpszEnd = StrChr(lpszFind, '|');
	if ( lpszEnd == NULL )
	{
		return;
	}
	item.m_uEnclosureLength = _tstoi64(CString(lpszFind, lpszEnd - lpszFind));
}

// ��ȡpartfile
CPartFile * CRssFeedBase::GetPartFile(const CString & strUrl)
{
	if ( strUrl.IsEmpty() )
	{
		return NULL;
	}

	CPartFile * pResult = NULL;
	switch ( strUrl.GetAt(0) )
	{
		case 'e':
		case 'E':
			// ����ed2k��ʽ����
			if ( CED2KFileLink * pFileLink = CreateFileLinkFromUrl(strUrl) )
			{
				pResult = GetPartFile(*pFileLink);
				delete pFileLink;
			}
			break;
		
		default:
			// http��ftp
			if ( CDownloadQueue * pQueue = CGlobalVariable::downloadqueue )
			{
				for ( POSITION pos = pQueue->filelist.GetHeadPosition(); pos != 0; )
				{
					CPartFile * cur_file = pQueue->filelist.GetNext(pos);
					if ( cur_file && cur_file->GetPartFileURL().CompareNoCase(strUrl) == 0 )
					{
						pResult = cur_file;
						break;
					}
				}
			}
			break;
	}

	return pResult;
}

// ��ȡpartfile
CPartFile * CRssFeedBase::GetPartFile(const CED2KFileLink & link)
{
	if ( CDownloadQueue * pQueue = CGlobalVariable::downloadqueue )
	{
		return pQueue->GetFileByID(link.GetHashKey());
	}
	return NULL;
}

// ��ȡshare file
CKnownFile * CRssFeedBase::GetShareFile(const uchar * pFileHash)
{
	if ( CSharedFileList * pList = CGlobalVariable::sharedfiles )
	{
		//return pList->FindKnownFileByID(pFileHash);
		return pList->GetFileByID(pFileHash);
	}
	return NULL;
}

// ��ȡshare file
CKnownFile * CRssFeedBase::GetShareFile(const CString & strUrl)
{
	CKnownFile * pResult = NULL;
	if ( CED2KFileLink * pFileLink = CreateFileLinkFromUrl(strUrl) )
	{
		pResult = GetShareFile(pFileLink->GetHashKey());
		delete pFileLink;
	}
	return pResult;
}

// ����һ��item
CRssFeedBase::Item * CRssFeedBase::ParseItem(CRssFeedManager * pManager, TiXmlElement & elItem, bool bIsUtf, const CRssFeedList * pHistoryList)
{
	CString strGuid;
	CString strLink;

	if ( TiXmlElement * el = elItem.FirstChildElement("link") )
	{
		strLink = el->GetText();
	}

	if ( TiXmlElement * el = elItem.FirstChildElement("guid") )
	{
		strGuid = el->GetText();
	}

	if ( strGuid.IsEmpty() )
	{
		strGuid = strLink;
	}

	if ( strGuid.IsEmpty() )
	{
		return NULL;
	}

	Item * item = &AddFeedItem(strGuid);
	//item->m_strGuid = strGuid;
	item->m_strLink = strLink;

	if ( TiXmlElement * el = elItem.FirstChildElement("title") )
	{
		item->m_strTitle = GetElementText(el->GetText(), bIsUtf);
	}

	if ( TiXmlElement * el = elItem.FirstChildElement("description") )
	{
		item->m_strDescription = GetElementText(el->GetText(), bIsUtf);
	}

	for ( TiXmlElement * el = elItem.FirstChildElement("category"); 
		  el != NULL;
		  el = el->NextSiblingElement("category")
		)
	{
		if ( !item->m_strCategory.IsEmpty() )
		{
			item->m_strCategory += ',';
		}
		item->m_strCategory += GetElementText(el->GetText(), bIsUtf);
	}

	if ( TiXmlElement * el = elItem.FirstChildElement("pubDate") )
	{
		ConvertStringToTime(CString(el->GetText()), item->m_timePubDate);
	}

	if ( TiXmlElement * el = elItem.FirstChildElement("enclosure") )
	{
		// ʹ��enclosure��ǵõ���Դurl
		if ( const char * url = el->Attribute("url") )
		{
			item->m_strEnclosure = url;
		}

		if ( const char * type = el->Attribute("type") )
		{
			item->m_strEnclosureType = type;
		}

		if ( const char * length = el->Attribute("length") )
		{
			item->m_uEnclosureLength = _atoi64(length);
		}
	}
	else
	{
		// �������н���ed2k
		GetEd2kInfoFromDesc(*item);
	}

	if ( pHistoryList != NULL && pHistoryList->FindFirstItem(item->m_strGuid) != NULL )
	{
		// ��ʷ��Ŀ
		item->m_bIsHistory = true;
	}

	// ȡ�ļ�����partfile
	if ( !item->m_strEnclosure.IsEmpty() )
	{
		if ( CED2KFileLink * pFileLink = CreateFileLinkFromUrl(item->m_strEnclosure) )
		{
			item->m_strFilename = pFileLink->GetName();
			item->SetPartFile(GetPartFile(*pFileLink));
			delete pFileLink;
		}
		else
		{
			DWORD dwServiceType;
			CString strServer, strObject;
			INTERNET_PORT nPort;
			if ( AfxParseURL(item->m_strEnclosure, dwServiceType, strServer, strObject, nPort) )
			{
				item->m_strFilename = strObject;
			}
			item->SetPartFile(GetPartFile(item->m_strEnclosure));
		}

		// ��partfile��sharefile��Ϊ��ʷ��Ŀ
		if ( !item->m_bIsHistory && pManager != NULL )
		{
			if ( item->GetPartFile() != NULL || 
				 item->GetShareFile() != NULL
			   )
			{
				pManager->MarkHistory(*item);
			}
		}
	}

	return item;
}

// �ַ���תʱ��
bool CRssFeedBase::ConvertStringToTime(const CString & strTime, CTime & tmReturn)
{
	static LPCTSTR s_lpszDayName[] = 
	{
		_T("Sun,"), _T("Mon,"), _T("Tue,"), _T("Wed,"), _T("Thu,"), _T("Fri,"), _T("Sat,")
	};

	if ( strTime.IsEmpty() )
	{
		return false;
	}

	int iSize;
	// ����+
	LPCTSTR lpszFind = StrChr(strTime, '+');
	if ( lpszFind == NULL )
	{
		iSize = strTime.GetLength();
	}
	else
	{
		iSize = lpszFind - static_cast<LPCTSTR>(strTime);
	}

	// ȷ���Ƿ��������ֿ�ͷ
	lpszFind = static_cast<LPCTSTR>(strTime);
	CString strLeft(strTime, 4);
	for ( int i = 0; i < sizeof(s_lpszDayName) / sizeof(s_lpszDayName[0]); ++i )
	{
		if ( StrCmpI(strLeft, s_lpszDayName[i]) == 0 )
		{
			lpszFind += 4;
			iSize -= 4;
			break;
		}
	}

	if ( iSize > 0 )
	{
		COleDateTime tm;
		if ( tm.ParseDateTime(CString(lpszFind, iSize)) )
		{
			SYSTEMTIME st;
			tm.GetAsSystemTime(st);
			if ( st.wYear > 1970 )
			{
				try 
				{
					tmReturn = st;
				}
				catch(...)
				{
					tmReturn = 0;
				}
			}
			else
			{
				tmReturn = 0;
			}
			return true;
		}
	}

	return false;
}

// ����һ��feed item
const CRssFeedBase::Item * CRssFeedBase::FindItem(const CString & strGuid) const
{
	ItemList::const_iterator it = m_listItem.find(strGuid);
	if ( it != m_listItem.end() )
	{
		return &it->second;
	}
	return NULL;
}

// ����һ��feed item
CRssFeedBase::Item * CRssFeedBase::FindItem(const CString & strGuid)
{
	return const_cast<Item*>(static_cast<const CRssFeedBase *>(this)->FindItem(strGuid));
}

// ��partfile��ȡ��url
void CRssFeedBase::GetEnclosureUrl(CString & strEnclosureUrl, const CPartFile * pPartFile)
{
	if ( pPartFile != NULL && pPartFile->m_pFileTaskItem != NULL )
	{
		/* 
			�����޸���һ����֪��m_pFileTaskItemҰָ��bug��
			���Ҳ���ȷ�ŵ�¿�������Ƿ񻹴�������ɾ����task item��
			ȴδ��m_pFileTaskItem����ΪNULL����ʵ��
			Ϊ�˰�ȫ������˴��ȼ���һ���쳣����
		*/
		__try
		{
			strEnclosureUrl = !pPartFile->m_pFileTaskItem->m_strEd2kLink.IsEmpty() ? 
							pPartFile->m_pFileTaskItem->m_strEd2kLink :
							pPartFile->m_pFileTaskItem->m_strUrl;
			return;
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}

	strEnclosureUrl.Empty();
}

// ����һ��feed item
CRssFeedBase::Item * CRssFeedBase::FindItem(CRssFeedManager * pManager, const CPartFile * pPartFile)
{
	CString strEnclosureUrl;
	GetEnclosureUrl(strEnclosureUrl, pPartFile);

	for ( ItemList::iterator it = m_listItem.begin();
		  it != m_listItem.end();
		  ++it
		)
	{
		CRssFeedBase::Item & item = it->second;
		if ( item.GetPartFile() == pPartFile )
		{
			return &item;
		}
		
		// ��item��part����Ա������Ƿ���ͬ������ͬ������Ϊpartfile
		if ( item.GetPartFile() == NULL && item.m_strEnclosure == strEnclosureUrl )
		{
			item.SetPartFile(const_cast<CPartFile*>(pPartFile));

			// ����Ϊ��ʷ
			if ( pManager != NULL )
			{
				pManager->MarkHistory(item);
			}
			return &item;
		}
	}

	return NULL;
}

// ����һ��feed item
//const CRssFeedBase::Item * CRssFeedBase::FindItem(const CPartFile * pPartFile) const
//{
//	return const_cast<CRssFeedBase *>(this)->FindItem(pPartFile);
//}

// ���һ��feed item
CRssFeedBase::Item & CRssFeedBase::AddFeedItem(const CString & strGuid)
{
	Item & result = m_listItem[strGuid];
	result.m_pFeed = this;
	if ( result.m_strGuid.IsEmpty() )
	{
		result.m_strGuid = strGuid;
	}
	return result;
}

// ���һ��feed item
CRssFeedBase::Item & CRssFeedBase::AddFeedItem(const Item & item)
{
	Item & result = (m_listItem[item.m_strGuid] = item);
	result.m_pFeed = this;
	return result;
}


// �õ�����ָ��˳���������Դ��Ŀ
void CRssFeedBase::GetOrderedItems(ItemArray & items, CompareItemFunc func)
{
	items.clear();
	items.reserve(m_listItem.size());
	
	for ( ItemIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		items.push_back(&it->second);
	}

	ASSERT(func != NULL);
	std::sort(items.begin(), items.end(), func);
}

// ȡ�������ص���Ŀ��
unsigned int CRssFeedBase::GetNewPubCount() const
{
	unsigned int iResult = 0;
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( it->second.m_bIsNewPub )
		{
			++iResult;
		}
	}

	return iResult;
}

//ȡ��RSS�������������ӰƬ����ӰƬ������
unsigned int CRssFeedBase::GetDownloadedFilmCount(unsigned int * pFilmCount) const
{
	unsigned int iFilmCount = 0;
	unsigned int iResult = 0;
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( !it->second.IsSubtitle() )
		{
			// ͳ��ӰƬ��
			++iFilmCount;
			if ( it->second.GetState() == CRssFeedBase::isDownloaded )
			{
				// ͳ���������
				++iResult;
			}
		}
	}

	if ( pFilmCount != NULL )
	{
		*pFilmCount = iFilmCount;
	}

	return iResult;
}

// �Ƿ�����������ص���Ŀ
bool CRssFeedBase::ExistDownloadingItem() const
{
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( it->second.GetState() == CRssFeedBase::isDownloading )
		{
			return true;
		}
	}

	return false;
}

// �Ƿ������ͣ/ֹͣ����Ŀ
bool CRssFeedBase::ExistCanResumeItem() const
{
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( it->second.GetState() == CRssFeedBase::isStopped || it->second.GetState() == CRssFeedBase::isPaused )
		{
			return true;
		}
	}
	return false;
}

// �Ƿ����δ��ɵ���Ŀ
bool CRssFeedBase::ExistUnloadItem() const
{
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( it->second.GetState() == CRssFeedBase::isHistory || it->second.GetState() == CRssFeedBase::isRss || 
			it->second.GetState() == CRssFeedBase::isStopped || it->second.GetState() == CRssFeedBase::isPaused )
		{
			return true;
		}
	}
	return false;
}

// ȡ������д��ʱ��
void CRssFeedBase::GetLastWriteTime(CKnownFile & file, CTime & timeLastWrite)
{
	timeLastWrite = 0;
	CString strFilePath = file.GetFilePath();
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFilePath);
	if ( bWorking )
	{
		finder.FindNextFile();
		finder.GetLastWriteTime(timeLastWrite);
	}
}

// ȡ���µ������������
unsigned int CRssFeedBase::GetNewDownloadedCount() const
{
	unsigned int iResult = 0;
	CTime timeCurrDownloaded;
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( !it->second.IsSubtitle() )
		{
			it->second.GetDownloadedTime(timeCurrDownloaded);
			if ( timeCurrDownloaded > m_timeDownloaded )
			{
				++iResult;
			}
		}
	}
	
	return iResult;
}

// �������µ������ʱ��
void CRssFeedBase::SetNewDownloadedTime()
{
	CTime timeCurrDownloaded;
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		if ( !it->second.IsSubtitle() )
		{		
			it->second.GetDownloadedTime(timeCurrDownloaded);
			if ( timeCurrDownloaded > m_timeDownloaded )
			{
				m_timeDownloaded = timeCurrDownloaded;
			}
		}
	}	
}

// ȡ��RSS���ĺϼ�size
uint64 CRssFeedBase::GetRssTotalSize() const
{
	uint64 iResult = 0;
	for ( ItemConstIterator it = m_listItem.begin(); it != m_listItem.end(); ++it )
	{
		iResult += it->second.m_uEnclosureLength;
	}

	return iResult;
}

#if defined(_DEBUG)
void CRssFeedBase::test()
{
	//CGetUrlStream url;
	//bool r = url.OpenUrl(_T("http://download.verycd.com/easyMule-Setup.exe"));
	////bool r = url.OpenUrl(_T("http://127.0.0.1:8888/easyMule-Setup.exe"));
	//assert(r);


	//CRssFeedBase rf(_T("http://www.mininova.org/rss.xml?sub=40"));
	//CRssFeedBase rf(_T("http://tech.163.com/digi/special/00161K7K/rss_digisyt.xml"));
	//CRssFeedBase rf(_T("http://192.168.2.42/subs/rss_film1.xml"));
	//rf.Refresh(NULL);

	CRssFeedList feeds, history;
	CRssFeedBase & rf = feeds.AddFeed(_T("http://www.mininova.org/rss.xml?sub=40"));
	//rf.Refresh(NULL);
	//rf.MarkHistory(_T("http://www.mininova.org/tor/3189427"), history);

	//rf.Refresh(&history);

	rf.Clear();

	//CGetUrlStream url;
	//bool r = url.OpenUrl(_T("http://download.verycd.com/easyMule-Setup.exe"));
	//bool r = url.OpenUrl(_T("http://127.0.0.1:8888/easyMule-Setup.exe"));
	//assert(r);
}
#endif

//---------------------------------------------------------------------------------------------

// CRssFeedʵ��

CRssFeed::~CRssFeed(void)
{
	//delete m_pWebImage;
	m_pWebImage = NULL;

	// ɾ������
	delete m_pPosterImage;
	m_pPosterImage = NULL;
}

void CRssFeed::Init()
{
	m_pPosterImage = NULL;
	m_pWebImage = NULL;
}

/// ��ֵ����
CRssFeed & CRssFeed::operator= (const CRssFeed & feed)
{
	CRssFeedBase::operator=(feed);
	
	m_pPosterImage = NULL;
	m_pWebImage = NULL;

	return *this;
}

/// ���Ի�ȡ����
bool CRssFeed::GetPoster(IPosterDownloadNotify & notify)
{
	if ( m_pPosterImage != NULL )
	{
		// �������Ѵ��ڣ����ٸ���
		return true;
	}

	if ( m_pWebImage != NULL )
	{
		// �ϴλ�ȡ������δ����
		return false;
	}

	// ��û��������Ϣ������url�����򲻴���
	if ( m_strDescription.IsEmpty() )
	{
		return false;
	}

	m_pWebImage = new CWebImage(m_strDescription, notify);
	if ( m_pWebImage != NULL )
	{
		CString * pFeedUrl = new CString(m_strFeedUrl);
		m_pWebImage->m_pAttachData = pFeedUrl;
		m_pWebImage->GetPoster();
		return true;
	}

	return false;
}

