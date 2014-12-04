/*
 * $Id: RssFeed.h 20761 2010-11-09 01:54:12Z gaoyuan $
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
#include <atltime.h>
#include <map>
#include <vector>
#include <set>

class TiXmlElement;
class CRssFeedList;
class CRssFilter;
class CPartFile;
class CKnownFile;
class CED2KFileLink;
class CRssFeedManager;
class CxImage;
class CWebImage;
struct IPosterDownloadNotify;
class CRssFeedBase;

/// VC���ļ���
typedef std::set<CRssFeedBase*>		CRssFeedSet;

/// ������Ϣ�ṹ
struct FeedCatalog
{
	static const UINT INVALID_CATALOG_ID = 0L;					///< ��Ч��catalog idֵ

	/// ���༯��
	typedef std::set<FeedCatalog*>	Childs;

	unsigned int	m_uId;			///< ����id
	CString			m_strName;		///< ������
	CRssFeedSet		m_setFeeds;		///< �÷����µ����ж���
	Childs			m_setChilds;	///< �÷����µ������ӷ���
	FeedCatalog *	m_pParent;		///< ����catalog

	/// ���캯��
	FeedCatalog()
		:
		m_uId(INVALID_CATALOG_ID),
		m_pParent(NULL)
	{
	}

	/// �Ƿ�Ҷ�ӽڵ�
	BOOL IsLeaf() const
	{
		// Ŀǰֻ��2��
		return ( m_pParent != NULL );
	}

	/// �Ƿ񶥼��ڵ�
	BOOL IsTop() const
	{
		// Ŀǰֻ��2��
		return ( m_pParent == NULL );
	}

	/// ȡ�ø÷��������ж��ĵ���Ŀ
	unsigned int GetFeedCount() const
	{
		unsigned int iResult = static_cast<unsigned int>(this->m_setFeeds.size());
		for ( Childs::const_iterator it = m_setChilds.begin();
			  it != m_setChilds.end();
			  ++it
			)
		{
			ASSERT(*it != NULL);
			iResult += static_cast<unsigned int>((*it)->m_setFeeds.size());
		}
		return iResult;
	}

	/// ȡ�õ�һ������
	CRssFeedBase * GetFirstFeed()
	{
		if ( !m_setFeeds.empty() )
		{
			return *m_setFeeds.begin();
		}

		for ( Childs::iterator it = m_setChilds.begin();
			  it != m_setChilds.end();
			  ++it
			)
		{
			FeedCatalog * pCatlog = *it;
			ASSERT(pCatlog != NULL);
			if ( !pCatlog->m_setFeeds.empty() )
			{
				return *pCatlog->m_setFeeds.begin();
			}
		}

		return NULL;
	}
};

/// VC������Ϣ����
class CFeedCatalogs : public std::map<unsigned int, FeedCatalog>
{
private:
	CFeedCatalogs() {}
	~CFeedCatalogs() {}

public:
	/// ����Ψһ�Ķ��������Ϣ����
	static CFeedCatalogs & GetInstance()
	{
		static CFeedCatalogs s_mapCatalogs;
		return s_mapCatalogs;
	}

	/// �������
	static CFeedCatalogs::iterator Find(unsigned int uId)
	{
		return GetInstance().find(uId);
	}

	/// ���ؼ����е�һ��Ԫ�ص�����
	static CFeedCatalogs::iterator GetBegin()
	{
		return GetInstance().begin();
	}

	/// ���ؼ���ĩβ
	static CFeedCatalogs::iterator GetEnd()
	{
		return GetInstance().end();
	}

};

/**
 * @brief ����ĳ�����ģ�����������ά��������Ŀ
 */
class CRssFeedBase
{
public:
	static const UINT UNDEFINED_VALUE = UINT(-1L);						///< δ�����ֵ
	static const UINT MANUAL_REFRESH_VALUE = UINT(-2L);					///< �ֶ�ˢ�¶���

private:
	static const int NEW_PUBDATE_OVERDUE_SECOND	= 14 * 24 * 60 * 60;	///< 14�������Ϊ���·�����Դ

public:

	/// ����״̬
	enum EFeedState
	{
		fsNormal		= 0x0001L,	///< ��ͨ״̬
		fsDisable		= 0x0002L,	///< �����ѽ���
		fsRefresh		= 0x0004L,	///< xml�ļ�������
	};

	/// ������Ŀ״̬
	enum EItemState
	{
		isRss = 0,		///< δ����
		isDownloading,	///< �Ѷ��ģ�partfile������
		isDownloaded,	///< �Ѷ��ģ�partfile�������
		isPaused,		///< �Ѷ��ģ�partfile����ͣ
		isStopped,		///< �Ѷ��ģ�partfile��ֹͣ
		isHistory,		///< �Ѷ��ģ���partfile�Ѿ���ɾ��

		isMaxState,		///< ��Ŀ״̬ö��ֵ����
	};

	/// ����ĳ��������Ŀ
	struct Item
	{
		//static const int COMPLETE_PARTFILE = -1;

		Item()
		{
			Clear();
		}

		//bool operator<(const Item & item) const
		//{
		//	return this->m_strGuid < item.m_strGuid;
		//}

		/// ָʾ��ǰ��Ŀ�Ƿ�ɱ�����
		bool CanDownload() const
		{
			switch ( GetState() )
			{
				case CRssFeedBase::isDownloading:	// �Ѷ��ģ�partfile������
				case CRssFeedBase::isDownloaded:	// �Ѷ��ģ�partfile�������
				case CRssFeedBase::isPaused:		// �Ѷ��ģ�partfile����ͣ
				case CRssFeedBase::isStopped:		// �Ѷ��ģ�partfile��ֹͣ
					return false;

				case CRssFeedBase::isRss:			// δ����
				case CRssFeedBase::isHistory:		// �Ѷ��ģ���partfile�Ѿ���ɾ��
				default:
					return true;
			}
		}

		/// �����Ŀ����
		void Clear()
		{
			m_pFeed = NULL;
			m_pAttachItem = NULL;
			m_strTitle.Empty();
			m_strLink.Empty();
			m_strDescription.Empty();
			m_strCategory.Empty();
			m_strGuid.Empty();
			m_strEnclosure.Empty();
			m_strEnclosureType.Empty();
			m_strFilename.Empty();
			m_timePubDate = 0;
			m_uEnclosureLength = 0;
			m_bIsHistory = false;
			m_bIsNewPub = false;
			m_pPartFile = NULL;
		}

		/// ȡ��feed��Ŀ״̬
		EItemState GetState() const;

		/// ����partfile
		void SetPartFile(CPartFile * pPartFile)
		{
			m_pPartFile = pPartFile;
		}

		/// partfile����ɾ��ʱ����
		void OnDeletePartFile();

		/// ȡ��partfile
		CPartFile * GetPartFile() const
		{
			return m_pPartFile;
		}		

		/// ��ȡshare file
		CKnownFile * GetShareFile() const
		{
			return CRssFeedBase::GetShareFile(m_strEnclosure);
		}

		/// �Ƿ�ǰ���·�������Դ
		bool IsLastPubItem() const;
		
		/// �Ƿ�ǰ���·�����ӰƬ��Դ
		bool IsLastPubFilmItem() const;

		/// �Ƿ�Ϊ��Ļ
		bool IsSubtitle() const;

		/// �Ƿ�С���ļ�
		bool IsSmallFile() const
		{
			return m_uEnclosureLength > 0 && m_uEnclosureLength <= 10 * 1024 * 1024;
		}

		/// �����������ʱ��
		void GetDownloadedTime(CTime & timeDownloaded) const;

		/// �������������ʱ��
		void ResetDownloadedTime()
		{
			m_timeDownloaded = 0;
		}

		CRssFeedBase *	m_pFeed;				///< ��Ŀ�����Ķ���
		Item *			m_pAttachItem;			///< ��Ŀ����������Ļ��ӰƬ
		CString			m_strTitle;				///< ��Ŀ����
		CString			m_strLink;				///< ��Ŀurl link
		CString			m_strDescription;		///< ��Ŀ��������
		CString			m_strCategory;			///< ��Ŀ���
		CString			m_strGuid;				///< ��Ŀguid
		CString			m_strEnclosure;			///< ��Ŀ������url
		CString			m_strEnclosureType;		///< ��Ŀ��Դ����
		CString			m_strFilename;			///< ��Ŀ�ļ���
		CTime			m_timePubDate;			///< ��Ŀ��������
		uint64			m_uEnclosureLength;		///< ��Ŀ��Դ����
		bool			m_bIsHistory;			///< ��Ŀ�Ƿ����������ع�
		bool			m_bIsNewPub;			///< ��Ŀ�Ƿ�Ϊ�·�������Դ

	private:
		CPartFile *	m_pPartFile;			///< ��Ŀ������partfile

		mutable CTime	m_timeDownloaded;	///< ��Ŀ�������ʱ��
	};

	typedef std::map<CString, Item> ItemList;				///< ��Ŀ�б�
	typedef ItemList::const_iterator ItemConstIterator;		///< ֻ����Ŀ������
	typedef ItemList::iterator ItemIterator;				///< �ɶ�д��Ŀ������
	typedef std::vector<Item *>	ItemArray;					///< ��Ŀ����
	typedef bool (*CompareItemFunc)(const Item *, const Item *);	///< ��Ŀ�ȽϺ���

public:
	CRssFeedBase();
	CRssFeedBase(const CString & strFeedUrl);
	virtual ~CRssFeedBase(void);

private:
	void Init();

public:
	/// ���ıȽϺ���
	bool operator<(const CRssFeedBase & feed) const
	{
		return this->m_strFeedUrl < feed.m_strFeedUrl;
	}

	CRssFeedBase& operator = (const CRssFeedBase& feed);

	/// ���ض����Ƿ񱻸��¹�
	bool IsUpdated() const
	{
		return (m_tmLastUpdate != 0);
	}

	/// �Ƿ�����������ص���Ŀ
	bool ExistDownloadingItem() const;

	bool ExistCanResumeItem() const;
	bool ExistUnloadItem() const;

	/// �Ƿ���ڣ����ں���Ҫ���£�
	bool IsOverdue(time_t tmOverdueSec) const;

	/// ˢ��feed
	bool Refresh(CRssFeedManager * pManager, const char * lpszUrlStream, const CRssFeedList * pHistoryList);

	/// ���ȫ��
	void Clear();

	/// ����һ��feed item
	const Item * FindItem(const CString & strGuid) const;

	/// ����һ��feed item
	Item * FindItem(const CString & strGuid);

	//// ����һ��feed item
	//const Item * FindItem(const CPartFile * pPartFile) const;

	/// ����һ��feed item
	Item * FindItem(CRssFeedManager * pManager, const CPartFile * pPartFile);

	/// ���һ��feed item
	Item & AddFeedItem(const CString & strGuid);

	/// ���һ��feed item
	Item & AddFeedItem(const Item & item);

	/// ������Ŀ�б�ͷ
	ItemConstIterator GetItemIterBegin() const
	{
		return m_listItem.begin();
	}

	/// ������Ŀ�б�β
	ItemConstIterator GetItemIterEnd() const
	{
		return m_listItem.end();
	}

	/// ������Ŀ�б�ͷ
	ItemIterator GetItemIterBegin()
	{
		return m_listItem.begin();
	}

	/// ������Ŀ�б�β
	ItemIterator GetItemIterEnd()
	{
		return m_listItem.end();
	}

	/// ����feed item��Ŀ
	size_t GetItemCount() const
	{
		return m_listItem.size();
	}

	/// ��ǰfeed item�Ƿ�Ϊ��
	bool IsEmpty() const
	{
		return m_listItem.empty();
	}

	/// ɾ��һ��feed item
	void DeleteItem(const CString & strGuid)
	{
		m_listItem.erase(strGuid);
	}

	/// ���ؿ���ʾ����
	const CString & GetDisplayName() const
	{
		const CString & strDisplayName = (m_strAlias.IsEmpty() ? m_strTitle : m_strAlias);
		return ( strDisplayName.IsEmpty() ? m_strFeedUrl : strDisplayName );
	}

	/// �õ�����ָ��˳���������Դ��Ŀ
	void GetOrderedItems(ItemArray & items, CompareItemFunc func);

	/// ȡ���·�������Ŀ��
	unsigned int GetNewPubCount() const;

	/// ȡ��RSS���ĺϼ�size
	uint64 GetRssTotalSize() const;

	/// ȡ��RSS�������������ӰƬ����ӰƬ������
	unsigned int GetDownloadedFilmCount(unsigned int * pFilmCount) const;

	/// ȡ���µ������������
	unsigned int GetNewDownloadedCount() const;

	/// �������µ������ʱ��
	void SetNewDownloadedTime();

	/// ���¼��ֵ�Ƿ���
	bool UpdateIntervalIsDefined() const
	{
		return m_uUpdateInterval != UNDEFINED_VALUE;
	}

	/// �Ƿ��ֶ�ˢ�¶���
	bool IsManualRefresh() const
	{
		return m_uUpdateInterval == MANUAL_REFRESH_VALUE;
	}

	//// ����Ϊ�ֶ�ˢ��
	//void SetManualRefresh()
	//{
	//	m_uUpdateInterval = MANUAL_REFRESH_VALUE;
	//}

	/// ��ȡ����ʱ����
	UINT GetUpdateInterval() const
	{
		return m_uUpdateInterval;
	}

	/// ���ø���ʱ����
	void SetUpdateInterval(UINT uUpdateInterval)
	{
		m_uUpdateInterval = uUpdateInterval;
	}

	/// �Զ�����ֵ�Ƿ���
	bool AutoDownloadIsDefined() const
	{
		return m_uAutoDownload != UNDEFINED_VALUE;
	}

	/// ȡ���Ƿ��Զ�����
	UINT GetAutoDownload() const
	{
		return m_uAutoDownload;
	}

	/// �����Զ�����
	void SetAutoDownload(UINT uAutoDownload)
	{
		m_uAutoDownload = uAutoDownload;
	}


private:
	/// ����һ��item
	Item * ParseItem(CRssFeedManager * pManager, TiXmlElement & elItem, bool bIsUtf, const CRssFeedList * pHistoryList);

	/// ����Ƶ��
	void ParseChannel(TiXmlElement & elChannel, bool bIsUtf, bool bAddToFeedCatalogs);

public:
	unsigned int	m_uCatalogId;		///< �������ID����catalog��ǲ���
	CString			m_strFeedUrl;		///< feed url
	CString			m_strAlias;			///< ����
	CString			m_strTitle;			///< ����
	CString			m_strCategory;		///< ���������Category��ǲ���
	CString			m_strDescription;	///< ������Ϣ
	CString			m_strSaveDir;		///< ����Ŀ¼
	CTime			m_timeLastDownload;	///< ������ص���Դ��������
	CTime			m_timeLastPub;		///< ��ǰ������Դ�ķ�������
	CTime			m_timeLastFilmPub;	///< ��ǰ����ӰƬ�ķ�������
	CTime			m_timeDownloaded;	///< ��ס��Դ�����ص�ʱ�䣨�������������
	int				m_iState;			///< ״̬
	time_t			m_tmLastUpdate;		///< �������ʱ��
	CRssFilter *	m_pFilter;			///< ʹ�õĹ�����
	//bool			m_bHasNewComplete;	///< �Ƿ�������ɵ���Ŀ
	bool			m_bLastUpdateSucc;	///< ���һ�θ����Ƿ�ɹ�

private:
	ItemList		m_listItem;			///< ��Դ��Ŀ�б�
	UINT			m_uUpdateInterval;	///< ���¼��
	UINT			m_uAutoDownload;	///< ���º��Ƿ��Զ����ض���

public:

#if defined(_DEBUG)
	static void test();
#endif

public:
	/// �����ĴӶ��������Ϣ������ɾ��
	static void RemoveFeedFromCatalogs(const CRssFeedBase & feed);

	/// ��ȡpartfile
	static CPartFile * GetPartFile(const CString & strUrl);

	/// ��ȡshare file
	static CKnownFile * GetShareFile(const CString & strUrl);

	/// ����Ŀ��������Ƚ�
	static bool CompareItemWithDateA(const Item * pItem1, const Item * pItem2)
	{
		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if ( pItem1 == NULL || pItem2 == NULL )
		{
			return false;
		}

		return pItem1->m_timePubDate < pItem2->m_timePubDate;
	}

	/// ����Ŀ���ڽ���Ƚ�
	static bool CompareItemWithDateD(const Item * pItem1, const Item * pItem2)
	{
		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if ( pItem1 == NULL || pItem2 == NULL )
		{
			return false;
		}

		return pItem1->m_timePubDate > pItem2->m_timePubDate;
	}

	/// ȡ����Ŀ�ļ���
	static CString GetItemFileNameNoExt(const Item & item);

	/// ȡ���ļ���
	static CString GetFileNameNoExt(const CString & strFileName);

private:
	/// ȡ������д��ʱ��
	static void GetLastWriteTime(CKnownFile & file, CTime & timeLastWrite);

	/// ��ȡpartfile
	static CPartFile * GetPartFile(const CED2KFileLink & link);

	/// ��ȡshare file
	static CKnownFile * GetShareFile(const uchar * pFileHash);


	static CString GetEncodeStr(const char * pXml);
	static CString GetElementText(const char * pText, bool bIsUtf);

	/// ��partfile�õ���Ŀ״̬
	static EItemState GetItemStateFromPartFile(const CPartFile & partfile);

	/// �������õ�ed2k
	static void GetEd2kInfoFromDesc(Item & item);

	/// �ַ���תʱ��
	static bool ConvertStringToTime(const CString & strTime, CTime & tmReturn);

	/// ��partfile��ȡ��url
	static void GetEnclosureUrl(CString & strEnclosureUrl, const CPartFile * pPartFile);

};

/**
 * @brief ����ĳ��VeryCD���ģ�������VeryCD���еĺ���
 */
class CRssFeed : public CRssFeedBase
{
public:
	CRssFeed() 
	{
		Init();
	}

	CRssFeed(const CString & strFeedUrl)
		:
		CRssFeedBase(strFeedUrl)
	{
		Init();
	}

	CRssFeed(const CRssFeed & feed)
	{
		Init();
		*this = feed;
	}

	virtual ~CRssFeed(void);

public:
	/// ��ֵ����
	CRssFeed & operator= (const CRssFeed & feed);

public:
	/// ���Ի�ȡ����
	bool GetPoster(IPosterDownloadNotify & notify);

public:
	CxImage *		m_pPosterImage;		///< ����ͼƬ
	CWebImage *		m_pWebImage;		///< ���ڻ�ȡ����

private:
	void Init();

};

/**
 * @brief ά��һ�������б�
 */
class CRssFeedList : public std::map<CString, CRssFeed>
{
public:
	CRssFeedList()
		//:
		//m_bModified(false)
	{
	}
	virtual ~CRssFeedList(void)
	{
	}

public:
	/// ����һ��feed
	CRssFeed & AddFeed(const CString & strFeedUrl)
	{
		CRssFeed & result = (*this)[strFeedUrl];
		if ( result.m_strFeedUrl.IsEmpty() )
		{
			result.m_strFeedUrl = strFeedUrl;
		}

		//m_bModified = true;
		return result;
	}

	/// ����һ��feed
	CRssFeed * FindFeed(const CString & strFeedUrl)
	{
		CRssFeedList::iterator it = find(strFeedUrl);
		if ( it != end() )
		{
			return &it->second;
		}
		return NULL;
	}

	/// ɾ��һ��feed
	void DeleteFeed(const CString & strFeedUrl)
	{
		erase(strFeedUrl);
		//m_bModified = true;
	}

	//// ���һ����Ŀ
	//void AddItem(const CRssFeedBase::Item & item, const CString & strFeedUrl)
	//{
	//	GetFeed(strFeedUrl).AddItem(item);
	//}

	/// ���ҵ�һ��ָ��guid��feed item
	const CRssFeed::Item * FindFirstItem(const CString & strGuid) const
	{
		for ( CRssFeedList::const_iterator it = this->begin();
			  it != this->end();
			  ++it
			)
		{
			if ( const CRssFeedBase::Item * item = it->second.FindItem(strGuid) )
			{
				return item;
			}
		}
		return NULL;
	}

	/// ����partfile����item
	CRssFeed::Item * FindFirstItem(CRssFeedManager * pManager, const CPartFile * pPartFile)
	{
		for ( CRssFeedList::iterator it = this->begin();
			  it != this->end();
			  ++it
			)
		{
			if ( CRssFeedBase::Item * item = it->second.FindItem(pManager, pPartFile) )
			{
				return item;
			}
		}
		return NULL;
	}

	/// ����feed��Ŀ
	size_t GetFeedCount() const
	{
		return this->size();
	}


public:
	//bool	m_bModified;
};

