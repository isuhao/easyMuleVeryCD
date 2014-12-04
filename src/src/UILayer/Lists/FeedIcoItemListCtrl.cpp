// FeedIcoItemListCtrl.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "FeedIcoItemListCtrl.h"
#include "ximage.h"
#include "OtherFunctions.h"
#include "RssFeedWnd.h"
#include "CmdFuncs.h"
#include "DlgMaintabDownload.h"
#include "emuleDlg.h"
#include "DlgFeedConfig.h"
#include <MemDC.h>

#define ELLIPSIS_STR _T(" ... ")//������� �м��ʡ�Ժ�
#define TITLE_DOUBLE_LENGTH 320//���ⳤ�ȳ����˳��Ⱦͻ��и�
#define TITLE_SINGLE_LENGTH 160//���ⵥ�г���

#ifndef LVM_GETGROUPINFOBYINDEX
#define LVM_GETGROUPINFOBYINDEX   (LVM_FIRST + 153)
#endif
#ifndef LVM_GETGROUPCOUNT
#define LVM_GETGROUPCOUNT         (LVM_FIRST + 152)
#endif
#ifndef LVM_GETGROUPRECT
#define LVM_GETGROUPRECT          (LVM_FIRST + 98)
#endif
#ifndef LVGGR_HEADER
#define LVGGR_HEADER		      (1)
#endif
// CFeedIcoItemListCtrl

IMPLEMENT_DYNAMIC(CFeedIcoItemListCtrl, CListCtrl)

CFeedIcoItemListCtrl::CFeedIcoItemListCtrl()
{
	nGroupHeight = -1;
	m_nLastHotItemIndex = -1;
}

CFeedIcoItemListCtrl::~CFeedIcoItemListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFeedIcoItemListCtrl, CListCtrl)
	ON_WM_KEYUP()
	//ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkFeedIcoItemListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CFeedIcoItemListCtrl::OnLvnItemchanged)
	ON_WM_CONTEXTMENU()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_NOTIFY_REFLECT(LVN_HOTTRACK, &CFeedIcoItemListCtrl::OnLvnHotTrack)
END_MESSAGE_MAP()


namespace {
	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR classList, LPCWSTR subApp, LPCWSTR idlist)
	{
		LRESULT lResult = S_FALSE;

		HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		HMODULE hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, "OpenThemeData");
			(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, "CloseThemeData");
			(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, "SetWindowTheme");
			if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
			{
				HANDLE theme = pOpenThemeData(hwnd,classList);
				if (theme!=NULL)
				{
					VERIFY(pCloseThemeData(theme)==S_OK);
					lResult = pSetWindowTheme(hwnd, subApp, idlist);
				}
			}
			::FreeLibrary(hinstDll);
		}
		return lResult;
	}

	bool IsCommonControlsEnabled()
	{
		bool commoncontrols = false;

		// Test if application has access to common controls
		HMODULE hinstDll = ::LoadLibrary(_T("comctl32.dll"));
		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");
			if (pDllGetVersion != NULL)
			{
				DLLVERSIONINFO dvi = {0};
				dvi.cbSize = sizeof(dvi);
				HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
				if (SUCCEEDED(hRes))
					commoncontrols = dvi.dwMajorVersion >= 6;
			}
			::FreeLibrary(hinstDll);
		}
		return commoncontrols;
	}

	bool IsThemeEnabled()
	{
		bool XPStyle = false;
		bool (__stdcall *pIsAppThemed)();
		bool (__stdcall *pIsThemeActive)();

		// Test if operating system has themes enabled
		HMODULE hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			(FARPROC&)pIsAppThemed = ::GetProcAddress(hinstDll, "IsAppThemed");
			(FARPROC&)pIsThemeActive = ::GetProcAddress(hinstDll,"IsThemeActive");
			if (pIsAppThemed != NULL && pIsThemeActive != NULL)
			{
				if (pIsAppThemed() && pIsThemeActive())
				{
					// Test if application has themes enabled by loading the proper DLL
					XPStyle = IsCommonControlsEnabled();
				}
			}
			::FreeLibrary(hinstDll);
		}
		return XPStyle;
	}
}


// CFeedIcoItemListCtrl ��Ϣ�������

void CFeedIcoItemListCtrl::PreSubclassWindow()
{
//	// Focus retangle is not painted properly without double-buffering
//#if (_WIN32_WINNT >= 0x501)//���Ѿ��Ի�������������˫�����ǻ�������
//	//SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
//#endif
	//if ( thePrefs.GetWindowsVersion() == _WINVER_XP_ )
	//{
	//	SetExtendedStyle(GetExtendedStyle() | LVS_EX_TRACKSELECT);
	//}
	
//
//	// Enable Vista-look if possible
	EnableWindowTheme(GetSafeHwnd(), L"ListView", L"Explorer", NULL);
//
	CListCtrl::PreSubclassWindow();
}

void CFeedIcoItemListCtrl::Init()
{
	//SetName(_T("RSSIcoListCtrl"));

	ModifyStyle(0, WS_CLIPCHILDREN);

	m_PosterImageList.Create(100,100,ILC_COLORDDB,1,1);

	m_imageRss_PosterBg = GetPng(_T("PNG_RSS_POSTER_BG"));
	m_imageRss_PosterDefault = GetPng(_T("PNG_RSS_POSTER_DEFAULT"));
	m_imageRss_PosterSelBg = GetPng(_T("PNG_RSS_POSTER_SELECTED_BG"));
	m_imageRss_GroupBg = GetPng(_T("PNG_RSS_GROUP_BG"));

	m_imageRss_Group_Drop_Up_Nor = GetPng(_T("RSS_GROUP_DROP_UP_NORMAL"));
	m_imageRss_Group_Drop_Up_Hov = GetPng(_T("RSS_GROUP_DROP_UP_HOVER"));
	m_imageRss_Group_Drop_Down_Nor = GetPng(_T("RSS_GROUP_DROP_DOWN_NORMAL"));
	m_imageRss_Group_Drop_Down_Hov = GetPng(_T("RSS_GROUP_DROP_DOWN_HOVER"));

	this->SetImageList(&m_PosterImageList,LVSIL_NORMAL);

	//AddImg2List(m_imageRss_PosterDefault);

	SetIconSpacing(150,150);

	m_FontBold.CreateFont(16,0,0,0,FW_BOLD,FALSE,FALSE,0,0,0,0,0,0,_TEXT("tahoma"));//���� 

	m_pMenuXP = NULL;

	EnableGroupView(TRUE);

	if ( thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
	{
		SetGroupHeight(38);
	}
	else
	{
		SetGroupHeight(28);
	}
}

void CFeedIcoItemListCtrl::OnDestroy()
{
	if (m_imageRss_PosterBg)
		delete m_imageRss_PosterBg;

	if (m_imageRss_PosterDefault)
		delete m_imageRss_PosterDefault;

	if (m_imageRss_PosterSelBg)
		delete m_imageRss_PosterSelBg;

	if (m_imageRss_GroupBg)
		delete m_imageRss_GroupBg;

	if (m_imageRss_Group_Drop_Up_Nor)
		delete m_imageRss_Group_Drop_Up_Nor;

	if (m_imageRss_Group_Drop_Up_Hov)
		delete m_imageRss_Group_Drop_Up_Hov;

	if (m_imageRss_Group_Drop_Down_Nor)
		delete m_imageRss_Group_Drop_Down_Nor;

	if (m_imageRss_Group_Drop_Down_Hov)
		delete m_imageRss_Group_Drop_Down_Hov;

	if(m_pMenuXP)
		delete m_pMenuXP;

	m_FontBold.DeleteObject();
	CListCtrl::OnDestroy();
}

int	CFeedIcoItemListCtrl::AddImg2List(CxImage* img)
{
	int nRet = -1;

	if (img)
	{
		CImageList* mImgList = GetImageList(LVSIL_NORMAL);

		CDC* hdc = this->GetDC();

		HBITMAP m_bitmap = img->MakeBitmap(hdc->m_hDC);
		nRet = mImgList->Add(CBitmap::FromHandle(m_bitmap),CLR_NONE);

		if (hdc)
			ReleaseDC(hdc);

		if (m_bitmap) 
			DeleteObject(m_bitmap);
	}

	return nRet;
}

LRESULT CFeedIcoItemListCtrl::InsertTypeGroup(int nItem, int nGroupId, const CString& strHeader, DWORD dwState /* = LVGS_NORMAL */, DWORD dwAlign /*= LVGA_HEADER_LEFT*/)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupId;
	lg.state = dwState;
	lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	lg.uAlign = dwAlign;

	// Header-title must be unicode (Convert if necessary)
#ifdef UNICODE
	lg.pszHeader = (LPWSTR)(LPCTSTR)strHeader;
	lg.cchHeader = strHeader.GetLength();
#else
	CComBSTR header = strHeader;
	lg.pszHeader = header;
	lg.cchHeader = header.Length();
#endif

	return InsertGroup(nItem, (PLVGROUP)&lg );
}

void CFeedIcoItemListCtrl::SetGroupHeight(int nHeight)
{
	LVGROUPMETRICS metrics;
	ZeroMemory( &metrics, sizeof(LVGROUPMETRICS) );
	metrics.cbSize = sizeof( LVGROUPMETRICS );
	metrics.mask = LVGMF_BORDERSIZE;
	metrics.Top = nHeight;
	SetGroupMetrics( &metrics );
}

BOOL CFeedIcoItemListCtrl::GroupFeedByType(int nIndex, int nType)
{
	if (!IsCommonControlsEnabled())
		return FALSE;

	SetRedraw(FALSE);

	EnableGroupView(TRUE);
	if (IsGroupViewEnabled())
	{
		CString cellText = GetResString( IDS_UNKNOWN );
		bool	bNew = false;//Is new Group?

		CRssFeed* pFeed = (CRssFeed*)GetItemData(nIndex);
		
		if (pFeed == NULL)
			return FALSE;

		if ( pFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
		{
			FeedCatalog & catalog = CFeedCatalogs::GetInstance()[pFeed->m_uCatalogId];
			ASSERT(catalog.m_uId == pFeed->m_uCatalogId);
			ASSERT(catalog.IsLeaf());

			FeedCatalog * catalogParent;
			if (catalog.m_pParent != NULL)
			{
				catalogParent = catalog.m_pParent;
				cellText = catalogParent->m_strName;
			}
		}

		int nGroupId = m_mapGroups.FindKey(cellText);
		if (nGroupId == -1)
		{
			CSimpleArray<int> nItems;
			m_mapGroups.Add(cellText, nItems);
			nGroupId = m_mapGroups.FindKey(cellText);

			bNew = true;
		}

		//������Ӳ��ظ�����
		if (m_mapGroups.GetValueAt(nGroupId).Find(nIndex) == -1)
		{
			m_mapGroups.GetValueAt(nGroupId).Add(nIndex);
		}

		if (bNew)//new Group
		{
			DWORD dwState = LVGS_NORMAL;

			#ifdef LVGS_COLLAPSIBLE
					if (IsGroupStateEnabled())
						dwState = LVGS_COLLAPSIBLE;
			#endif

			//InsertTypeGroup(nGroupId, nGroupId, cellText, dwState);
			VERIFY(InsertTypeGroup(nGroupId, nGroupId, cellText, dwState) != -1);
		}

		VERIFY( SetItemGroupId(nIndex, nGroupId) );
	}

	SetRedraw(TRUE);
	Invalidate(TRUE);
	

	return FALSE;
}

int CFeedIcoItemListCtrl::GetGroupCounts(CString strGroupCaption)
{
	if ( m_mapGroups.GetSize() > 0 )
	{
		int nGroupId = m_mapGroups.FindKey(strGroupCaption);
		const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(nGroupId);

		return groupRows.GetSize();
	}

	return 0;
}

void CFeedIcoItemListCtrl::RemoveAllGroupData()
{
	if ( m_mapGroups.GetSize() > 0 )
	{
		for (int nGroupId = 0; nGroupId < m_mapGroups.GetSize();  nGroupId++)
		{
			m_mapGroups.GetValueAt(nGroupId).RemoveAll();
		}
		m_mapGroups.RemoveAll();
	}
}

BOOL CFeedIcoItemListCtrl::GroupAllFeedsByType(int nType)
{
	if (!IsCommonControlsEnabled())
		return FALSE;

	SetRedraw(FALSE);

	RemoveAllGroups();
	RemoveAllGroupData();
	EnableGroupView( GetItemCount() > 0 );

	if (IsGroupViewEnabled())
	{
		// Loop through all rows and find possible groups
		for(int nItem = 0; nItem < GetItemCount(); nItem++ )
		{
			CString strCatalogName = GetResString( IDS_UNKNOWN );

			CRssFeed* pFeed = (CRssFeed*)GetItemData(nItem);

			if (pFeed == NULL)
				continue;

			if ( pFeed->m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
			{
				FeedCatalog & catalog = CFeedCatalogs::GetInstance()[pFeed->m_uCatalogId];
				ASSERT(catalog.m_uId == pFeed->m_uCatalogId);
				ASSERT(catalog.IsLeaf());

				FeedCatalog * catalogParent;
				if (catalog.m_pParent != NULL)
				{
					catalogParent = catalog.m_pParent;
					strCatalogName = catalogParent->m_strName;
				}
			}

			int nGroupId = m_mapGroups.FindKey(strCatalogName);
			if (nGroupId == -1)
			{
				CSimpleArray<int> nItems;
				m_mapGroups.Add(strCatalogName, nItems);
				nGroupId = m_mapGroups.FindKey(strCatalogName);
			}

			//������Ӳ��ظ�����
			if (m_mapGroups.GetValueAt(nGroupId).Find(nItem) == -1)
			{
				m_mapGroups.GetValueAt(nGroupId).Add(nItem);
			}
			
		}

		// Look through all groups and assign rows to group
		for(int nGroupId = 0; nGroupId < m_mapGroups.GetSize(); nGroupId++)
		{
			const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(nGroupId);
			DWORD dwState = LVGS_NORMAL;

#ifdef LVGS_COLLAPSIBLE
			if (IsGroupStateEnabled())
				dwState = LVGS_COLLAPSIBLE;
#endif

			VERIFY( InsertTypeGroup(nGroupId, nGroupId, m_mapGroups.GetKeyAt(nGroupId), dwState) != -1);

			for(int groupRow = 0; groupRow < groupRows.GetSize(); ++groupRow)
			{
				VERIFY( SetItemGroupId(groupRows[groupRow], nGroupId) );
			}
		}

	}

	SetRedraw(TRUE);

	Invalidate(TRUE);
	return FALSE;
}

BOOL CFeedIcoItemListCtrl::IsGroupStateEnabled()
{
	if (!IsGroupViewEnabled())
		return FALSE;

	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
		return TRUE;

	return FALSE;
}

BOOL CFeedIcoItemListCtrl::SetItemGroupId(int nItem, int nGroupId)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	lvItem.iGroupId = nGroupId;
	return SetItem( &lvItem );
}

int CFeedIcoItemListCtrl::GetGroupIDByItemIndex(int nIndex)
{
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nIndex;
	lvItem.iSubItem = 0;

	int nGroupId = -1;
	if ( GetItem( &lvItem ) )
		nGroupId = lvItem.iGroupId;

	return nGroupId;
}

BOOL CFeedIcoItemListCtrl::SetItemPoster(int nItem, int nImageListId)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	lvItem.iImage = nImageListId;
	return SetItem( &lvItem );
}

BOOL CFeedIcoItemListCtrl::UpdatePoster(CRssFeed* feed)
{
	if (feed->m_pPosterImage)
	{
		//int nItemIndex = GetItemIndex(feed);
		//int nImageId = AddImg2List(feed->m_pPosterImage);

		//SetItemPoster(nItemIndex,nImageId);

		return TRUE;
	}
	else
		return FALSE;
}

int CFeedIcoItemListCtrl::AddFeed(CRssFeed & feed, BOOL bIsNew)
{
	int nRet = -1;

	CString strCaption = GetExtentString( feed.GetDisplayName() );
	//if (feed.m_pPosterImage)
	//{
	//	int nIndex = AddImg2List(feed.m_pPosterImage);//����ʹ��ImgList
	//	nRet = InsertItem(GetItemCount(), strCaption, nIndex);
	//}

	nRet = InsertItem(GetItemCount(), strCaption);
	m_stringMatcher.AddItem((void*)&feed, feed.GetDisplayName());

	if (nRet >= 0)
	{
		this->SetItemData(nRet, LPARAM(&feed));

		if ( bIsNew )//�¼ӵ�feed,����UI��"Default"չʾ����
		{
			GroupFeedByType(nRet,0);
		}
	}

	return nRet;
}

CString CFeedIcoItemListCtrl::GetExtentString(const CString & strCaption)
{
	CString strCaptionCopy = strCaption;

	CDC* dc = this->GetDC();
	CSize szCaption = dc->GetTextExtent(strCaptionCopy);

	if (szCaption.cx > TITLE_DOUBLE_LENGTH)//���ⳤ�ȴ���2���ܿ��
	{
		int nLength = strCaptionCopy.GetLength();
		int nMidPos = nLength/2;//�е�

		//ʡ�Ժſ��
		CSize szEllipsis = dc->GetTextExtent(ELLIPSIS_STR);

		//������ȷ�Ϊ2��
		CString strCaptionStart = strCaptionCopy.Mid(0, nMidPos);
		CString strCaptionEnd   = strCaptionCopy.Mid(nMidPos, nLength-1);

		//�����1���ַ���
		for (int i = nMidPos - 1; i > 0; i--)
		{
			strCaptionStart = strCaptionStart.Mid(0, i);
			CSize szMid = dc->GetTextExtent(strCaptionStart);
			if (szMid.cx + szEllipsis.cx <= TITLE_SINGLE_LENGTH)
				break;
		}

		//�����2���ַ���
		int nCaptionEndLength = strCaptionEnd.GetLength();
		for (int i = nCaptionEndLength - 1; i > 0; i--)
		{
			strCaptionEnd = strCaptionEnd.Mid(1, i);//�ӿ�ͷ��ʼ��ȡ
			CSize szMid = dc->GetTextExtent(strCaptionEnd);
			if (szMid.cx <= TITLE_SINGLE_LENGTH)
				break;
		}

		strCaptionCopy = strCaptionStart + ELLIPSIS_STR + strCaptionEnd;
	}

	return strCaptionCopy;
}

void CFeedIcoItemListCtrl::RemoveItemforMap(CRssFeed & feed, int nIndex, bool bUpdate/*=false*/)
{
	if ( m_mapGroups.GetSize() > 0 )
	{
		CString strCatalogName = GetResString( IDS_UNKNOWN );//���Ǹ��µĻ�ֱ�Ӳ���IDS_UNKNOWN����
		if ( !bUpdate && feed.m_uCatalogId != FeedCatalog::INVALID_CATALOG_ID )
		{
			//���ҷ�������
			FeedCatalog & catalog = CFeedCatalogs::GetInstance()[feed.m_uCatalogId];
			ASSERT(catalog.m_uId == feed.m_uCatalogId);
			ASSERT(catalog.IsLeaf());

			FeedCatalog * catalogParent;
			if (catalog.m_pParent != NULL)
			{
				catalogParent = catalog.m_pParent;
				strCatalogName = catalogParent->m_strName;
			}
		}


		//���Ҳ�ɾ��
		int nGroupId = m_mapGroups.FindKey(strCatalogName);
		if (nGroupId == -1)
			return;

		if (m_mapGroups.GetValueAt(nGroupId).Find(nIndex) != -1)
		{
			m_mapGroups.GetValueAt(nGroupId).Remove(nIndex);

			if ( m_mapGroups.GetValueAt(nGroupId).GetSize() <= 0 && bUpdate )
			{//����˷�����û��itemʱӦ��ɾ��, ��Ϊvista+ ʹ��ϵͳ��ȡ��group���л���
				RemoveGroup(nGroupId);//��List��ɾ����Group
			}
		}
	}
}

void CFeedIcoItemListCtrl::RemoveFeed(CRssFeed & feed)
{
	int nIndex = GetItemIndex(&feed);

	m_stringMatcher.RemoveItem((void*)&feed);

	RemoveItemforMap(feed, nIndex);

	DeleteItem(nIndex);
}

int CFeedIcoItemListCtrl::GetItemIndex(CRssFeed* feed)
{
	int nIndex = -1;

	for (int i=0; i < GetItemCount(); i++)
	{
		CRssFeed* pFeed = (CRssFeed*)GetItemData(i);

		if (pFeed != NULL && pFeed->m_strFeedUrl == feed->m_strFeedUrl)
		{
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

CxImage* CFeedIcoItemListCtrl::GetPng(LPCTSTR lpszPngResource)
{
	if (NULL == lpszPngResource)
		return NULL;

	CxImage* image = new CxImage(CXIMAGE_FORMAT_PNG);
	image->LoadResource(FindResource(NULL, lpszPngResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
	return image;
}

// ȡ�õ�ǰѡ���index
int CFeedIcoItemListCtrl::GetCurrSelectIndex()
{
	POSITION pos = GetFirstSelectedItemPosition();
	if ( pos != NULL )
	{
		return  GetNextSelectedItem(pos);
	}
	return -1;
}

// ȡ��feed
CRssFeed * CFeedIcoItemListCtrl::GetFeed(int iItemIndex)
{
	CRssFeed * pFeed = reinterpret_cast<CRssFeed *>(GetItemData(iItemIndex));
	if ( pFeed != NULL )
	{
		return pFeed;
	}
	return NULL;
}

int CFeedIcoItemListCtrl::GetGroupCount()
{
	int groupCount = 0;
	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
	{
		groupCount = SNDMSG((m_hWnd), LVM_GETGROUPCOUNT, (WPARAM)0, (LPARAM)0);
	}
	else
	{
		CFeedCatalogs & feedCatalogs = CFeedCatalogs::GetInstance();

		for (	CFeedCatalogs::iterator it = feedCatalogs.GetBegin();
				it != feedCatalogs.GetEnd();
				++it	)
		{
			FeedCatalog & feedCatlog = it->second;
			if ( feedCatlog.IsTop() && feedCatlog.GetFeedCount() > 0 )
			{
				groupCount++;
			}
		}
	}

	return groupCount;
}

LVGROUP CFeedIcoItemListCtrl::GetGroupInfoByIndex(int iGroupId, UINT mask)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = mask;//����ֻ��ҪID

	VERIFY( SNDMSG((m_hWnd), LVM_GETGROUPINFOBYINDEX, (WPARAM)(iGroupId), (LPARAM)(&lg)) );

	return lg;
}

// Vista SDK - ListView_GetGroupState / LVM_GETGROUPSTATE
BOOL CFeedIcoItemListCtrl::HasGroupState(int nGroupId, DWORD dwState)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.stateMask = dwState;
	if ( GetGroupInfo(nGroupId, (PLVGROUP)&lg) == -1)
		return FALSE;

	return lg.state == dwState;
}

CRect CFeedIcoItemListCtrl::GetRectbyGroupID(int nGroupID, UINT mask)
{
	if (nGroupID >= GetGroupCount() || nGroupID < 0)
	{
		return CRect(0,0,0,0);
	}

	CRect rect(0,mask,0,0);
	VERIFY( SNDMSG((m_hWnd), LVM_GETGROUPRECT, (WPARAM)(nGroupID), (LPARAM)(RECT*)(&rect)) );

	return rect;
}

CString CFeedIcoItemListCtrl::GetGroupCaption(int nGroupId)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupId;
	lg.mask = LVGF_HEADER | LVGF_GROUPID;
	VERIFY( GetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 );

#ifdef UNICODE
	return lg.pszHeader;
#else
	CComBSTR header( lg.pszHeader );
	return (LPCTSTR)COLE2T(header);
#endif
}
//
//CString CFeedIcoItemListCtrl::GetGroupFooter(int nGroupId)
//{
//	LVGROUP lg = {0};
//	lg.cbSize = sizeof(lg);
//	lg.iGroupId = nGroupId;
//	lg.mask = LVGF_SUBTITLE | LVGF_GROUPID;
//	VERIFY( GetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 );
//
//#ifdef UNICODE
//	return lg.pszSubsetTitle;
//#else
//	CComBSTR header( lg.pszSubsetTitle );
//	return (LPCTSTR)COLE2T(header);
//#endif
//}
//
//BOOL CFeedIcoItemListCtrl::SetGroupFooter(int nGroupId, CString & strFooter)
//{
//	if (!IsGroupStateEnabled())
//		return FALSE;
//
//#if _WIN32_WINNT >= 0x0600
//	LVGROUP lg = {0};
//	lg.cbSize = sizeof(lg);
//	lg.mask = LVGF_SUBTITLE;
//#ifdef UNICODE
//	lg.pszSubtitle = (LPWSTR)(LPCTSTR)strFooter;
//	lg.cchSubtitle = strFooter.GetLength();
//#else
//	CComBSTR bstrSubtitle = strFooter;
//	lg.pszSubtitle = bstrSubtitle;
//	lg.cchSubtitle = bstrSubtitle.Length();
//#endif
//
//	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
//		return FALSE;
//
//	return TRUE;
//#else
//	return FALSE;
//#endif
//
////	LVGROUP lg = {0};
////	lg.cbSize = sizeof(lg);
////	lg.mask = LVGF_FOOTER | LVGF_ALIGN;
////	lg.uAlign = LVGA_FOOTER_CENTER;
////
////#ifdef UNICODE
////	lg.pszFooter = (LPWSTR)(LPCTSTR)strFooter;
////	lg.cchFooter = strFooter.GetLength();
////#else
////	CComBSTR bstrFooter = strFooter;
////	lg.pszFooter = bstrFooter;
////	lg.cchFooter = bstrFooter.Length();
////#endif
////
////	if( SetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 )
////		return TRUE;
////
////	return FALSE;
//}

void CFeedIcoItemListCtrl::OnNMDblclkFeedIcoItemListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	ASSERT(pNMHDR != NULL);
	ASSERT(pResult != 0);
	*pResult = 0;

	// Version 4.71. Pointer to an NMITEMACTIVATE structure 
	LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if ( lpnmitem->iItem == -1 )
	{
		return;
	}

	CRssFeed * pFeed = reinterpret_cast<CRssFeed *>(GetItemData(lpnmitem->iItem));
	if ( pFeed == NULL )
	{
		return;
	}

	((CRssFeedWnd*)GetParent())->JumpListInfo(pFeed, ListShowMode::LISTSTYLE_SINGLE);
}

BOOL CFeedIcoItemListCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE; 
}

void CFeedIcoItemListCtrl::DrawGroupInfo(CDC & dcMem, const CRect & rectClip, const CRect & rectClient)
{
	dcMem.SelectObject(&m_FontBold);

	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ )
	{
		int n = GetGroupCount();
		for(int i = 0 ; i < n; ++i)
		{
			CString  strGroupCaption = GetGroupCaption( i );

			CString strGroupTotals;
			
			if ( strGroupCaption == m_mapGroups.GetKeyAt(i) )
			{
				const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(i);
				strGroupTotals.Format(_T("(��%d��)"), groupRows.GetSize());
			}

			LVGROUP  lg = GetGroupInfoByIndex(i, LVGF_GROUPID);
			CRect    rcGroup = GetRectbyGroupID(lg.iGroupId, LVGGR_HEADER);
			
			CRect rcIntersect;
			rcIntersect.IntersectRect(rectClip, rcGroup);

			//rcGroup.DeflateRect(0, 7);//��С7���ظ߶�
			//rcGroup.OffsetRect(0, -7);//����ƫ��7����

			if ( !rcIntersect.IsRectEmpty() )
			{
				DrawGroup(dcMem, rcGroup, strGroupCaption, strGroupTotals);
			}

#ifdef LVGS_COLLAPSIBLE
			// Maintain LVGS_COLLAPSIBLE state
			CRect rcDropDown(rcGroup.right - 20, rcGroup.top + 10, rcGroup.right - 5, rcGroup.bottom - 3);
			if (HasGroupState(i, LVGS_COLLAPSED))
			{
				if (m_imageRss_Group_Drop_Up_Nor)
				{
					m_imageRss_Group_Drop_Up_Nor->Draw(dcMem, rcDropDown.left, rcDropDown.top);
				}
			}
			else if (HasGroupState(i, LVGS_NORMAL))
			{
				if (m_imageRss_Group_Drop_Down_Nor)
				{
					m_imageRss_Group_Drop_Down_Nor->Draw(dcMem, rcDropDown.left, rcDropDown.top);
				}
			}
#endif

			//CRect rcDropDown(rcGroup.right - 20, rcGroup.top + 7, rcGroup.right - 5, rcGroup.bottom - 3);
			//CRgn rgDropDown;//��Բ�Ǿ���
			//rgDropDown.CreateRoundRectRgn( rcDropDown.left, rcDropDown.top, rcDropDown.right, rcDropDown.bottom, 3, 3 );
			//
			//CBrush* brushFrame; 
			//brushFrame = new CBrush( GetSysColor(COLOR_GRAYTEXT) );
			//dcMem.FrameRgn(&rgDropDown,brushFrame,1,1);//������߿�
			//delete brushFrame;
		}
	}
	else
	{
		if ( nGroupHeight == -1 )
		{
			CRect rcFirstItem;//�㷨�д��Ż�
			GetItemRect(0, rcFirstItem, LVIR_BOUNDS);
			if ( !rcFirstItem.IsRectEmpty() )
			{
				nGroupHeight = rcFirstItem.top;
			}
		}//����group �߶�

		CFeedCatalogs & feedCatalogs = CFeedCatalogs::GetInstance();
		//����Catlogs ���ÿ��һ�������Start RssData[ֻҪ�õ�һ��������....��һ������ƫ�ƻ���]
		for (	CFeedCatalogs::iterator it = feedCatalogs.GetBegin();
				it != feedCatalogs.GetEnd();
				++it	)
		{
			FeedCatalog & feedCatlog = it->second;
			if ( feedCatlog.IsTop() && feedCatlog.GetFeedCount() > 0 )
			{
				CRssFeedBase* feedStart = feedCatlog.GetFirstFeed();

				int nStartIndex = GetItemIndex( (CRssFeed*)feedStart );//ȷ����һ��itemλ��
				
				if ( nStartIndex > -1 )
				{
					int nGroupID = GetGroupIDByItemIndex( nStartIndex );
					CString strGroupCaption = GetGroupCaption( nGroupID );

					CString strGroupTotals;
					const CSimpleArray<int>& groupRows = m_mapGroups.GetValueAt(nGroupID);
					strGroupTotals.Format(_T("(��%d��)"), groupRows.GetSize());
					//strGroupTotals.Format(_T("(�� %n ��)"), GetGroupCount());

					CRect itemRect;
					GetItemRect(nStartIndex, &itemRect, LVIR_BOUNDS);

					//����item rect���group rect
					CRect rcGroup(rectClient.left, itemRect.top - nGroupHeight, rectClient.right, itemRect.top - 6);
					
					CRect rtIntersect;
					rtIntersect.IntersectRect(rectClip, rcGroup);

					if ( !rtIntersect.IsRectEmpty() )
					{
						DrawGroup(dcMem, rcGroup, strGroupCaption, strGroupTotals);
					}
				}
			}
		}

	}
}

void CFeedIcoItemListCtrl::DrawGroup(CDC & dcMem, const CRect & rectGroup, const CString & strGroupCaption, const CString & strGroupTotals)
{
	CRgn rgnFrame;//��Բ�Ǿ���
	rgnFrame.CreateRoundRectRgn( rectGroup.left, rectGroup.top, rectGroup.right, rectGroup.bottom, 3, 3 );

	if (m_imageRss_GroupBg)
	{
		m_imageRss_GroupBg->Draw(dcMem.m_hDC, rectGroup.left,  rectGroup.top, rectGroup.Width(),  24);
	}

	//CBrush* brushFrame; 
	//brushFrame = new CBrush( GetSysColor(COLOR_GRAYTEXT) );
	//dcMem.FrameRgn(&rgnFrame,brushFrame,1,1);//������߿�
	//delete brushFrame;

	CRect rectCaption  ( rectGroup.left + 7, rectGroup.top + 4, rectGroup.left + 40, rectGroup.bottom - 3   ) ;

	dcMem.SetTextColor( RGB(0,0,0) );
	dcMem.DrawText(strGroupCaption, -1, &rectCaption, DT_SINGLELINE|DT_LEFT/*|DT_CENTER*/ );

	CRect rectTotals  ( rectCaption.right + 4, rectGroup.top + 5, rectCaption.right + 80, rectGroup.bottom - 3   ) ;

	CFont * pListFont = GetFont();
	CFont* pOld = dcMem.SelectObject(pListFont);
	dcMem.SetTextColor( RGB(169,47,47) );
	dcMem.DrawText(strGroupTotals, -1, &rectTotals, DT_SINGLELINE|DT_LEFT/*|DT_CENTER*/ );
	dcMem.SelectObject(pOld);
}

void CFeedIcoItemListCtrl::DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient)
{
	//CImageList* pImgList = GetImageList(LVSIL_NORMAL);

	CFont * pListFont = GetFont();
	dcMem.SelectObject(pListFont);
	dcMem.SetTextColor(RGB(0,0,0));

	CRect itemRect(0,0,0,0);

	CString strTaskName;

	int nItemCount  = GetItemCount() - 1;
	int nHoverIndex = GetHotItem();//Hover
	for ( ; nItemCount >= 0; nItemCount-- )
	{
		int nItemIndex = nItemCount;
		GetItemRect(nItemIndex, &itemRect, LVIR_BOUNDS);

		CRect rtIntersect;
		rtIntersect.IntersectRect(rectClip,itemRect);
		if (rtIntersect.IsRectEmpty())//�жϴ�ITEM�Ƿ����ػ�������,���ڵĻ���ֱ��continue
			continue;

		CRect rectPoster;//����RECT
		GetItemRect(nItemIndex, &rectPoster, LVIR_ICON);

		//���ƺ���
		if( nHoverIndex == nItemIndex || GetItemState(nItemIndex, ODA_SELECT) == ODA_SELECT )
		{//Hover
			if (m_imageRss_PosterSelBg)
				m_imageRss_PosterSelBg->Draw(dcMem.m_hDC, rectPoster.left + 11, rectPoster.top - 8);
		}
		else
		{//Normal
			if (m_imageRss_PosterBg)
				m_imageRss_PosterBg->Draw(dcMem.m_hDC, rectPoster.left + 15, rectPoster.top - 4);
		}

		CRssFeed* pFeed = (CRssFeed*)GetItemData(nItemIndex);

		if (pFeed->m_pPosterImage)//���ƺ���
			pFeed->m_pPosterImage->Draw(dcMem.m_hDC, rectPoster.left + 20, rectPoster.top + 1);
		else if (m_imageRss_PosterDefault)//����Ĭ�Ϻ���
				m_imageRss_PosterDefault->Draw(dcMem.m_hDC, rectPoster.left + 20, rectPoster.top + 1);

		//strTaskName = pFeed->m_strTitle;
		strTaskName = GetItemText(nItemCount, 0);

		CRect rectNameStr;//���146
		GetItemRect(nItemIndex, &rectNameStr, LVIR_LABEL);
		rectNameStr.OffsetRect(0,5);//���������Poster�ľ��� 

		dcMem.DrawText(strTaskName, -1, &rectNameStr, DT_CENTER | DT_WORDBREAK );

		//if (pImgList)
		//	pImgList->Draw(&MemDC,nItem,CPoint(itemRect.left+5,itemRect.top+5),ILD_TRANSPARENT);
	}
}

void CFeedIcoItemListCtrl::OnPaint()
{
	CRect rectClip;
	{	
		CPaintDC dcPaint(this); // device context for painting
		//dcPaint.GetClipBox(rectClip);
	}

	CRect rectClient; 
	GetClientRect(&rectClient); //��ȡ�ͻ�����С

	CClientDC dc(this); //����ͼ ��ȡ�ͻ���DC
	dc.GetClipBox(rectClip);

	CDC dcMem; //����һ����ʾ�豸����
	dcMem.CreateCompatibleDC(NULL);//���������Ļ��ʾ���ݵ��ڴ���ʾ�豸

	CBitmap MemBitmap;//����һ��λͼ����(����)
	MemBitmap.CreateCompatibleBitmap(&dc,rectClient.Width(),rectClient.Height());

	//��λͼѡ�뵽�ڴ���ʾ�豸��
	CBitmap *pOldBit = dcMem.SelectObject(&MemBitmap);//�൱�ڽ���λͼ����Ϊ����
	dcMem.SetBkMode(TRANSPARENT);

	CBrush *pBKBrush = new CBrush( RGB(0xf3,0xf3,0xf3/*246, 252, 251*/) );
	dcMem.FillRect( rectClient, pBKBrush );//���ͻ�������ɫ
	delete pBKBrush;

	
	//CMemDC dcMem(&dc, rectClient);
	//dcMem.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	//CFont * pListFont = GetFont();
	//CFont * pOrgMemFont = NULL;
	//if ( pListFont != NULL )
	//{
	//	pOrgMemFont = dcMem.SelectObject(pListFont);
	//}
	//dcMem.SetBkMode(TRANSPARENT);

	if ( GetItemCount() > 0 )
	{
		DrawGroupInfo(dcMem, rectClip, rectClient);
		DrawItems(dcMem, rectClip, rectClient);
	}

	//if ( pOrgMemFont != NULL )
	//{
	//	dcMem.SelectObject(pOrgMemFont);
	//}
	//dcMem->Flush();
	dc.BitBlt( rectClient.left, rectClient.top, rectClient.Width(),rectClient.Height(), &dcMem, 0,0, SRCCOPY );//������ͼ
}

void CFeedIcoItemListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	// ���¹�����
	CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
	UpdateToolBarState(wd.m_Toolbar);
}

// ����toolbar��ť״̬
void CFeedIcoItemListCtrl::UpdateToolBarState(CToolBarCtrl & toolbar)
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
		}
	}
}

BOOL CFeedIcoItemListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: �ڴ����ר�ô����/����û���

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
		int nIndex = this->GetNextSelectedItem(Pos);

		if( CRssFeed* pFeed = this->GetFeed(nIndex) )
		{
			if ( LOWORD(wParam) == MP_OPENFOLDER)
			{
				if( CRssFeed* pFeed = this->GetFeed(nIndex) )
				{
					if(PathFileExists(pFeed->m_strSaveDir))
						ShellExecute(NULL, _T("open"), _T("explorer.exe"), pFeed->m_strSaveDir, NULL, SW_SHOW);
				}
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
					//RedrawItems(nIndex, nIndex);
				}
			}

			CDownloadTabWnd & wd = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_DownloadTabWnd;
			UpdateToolBarState(wd.m_Toolbar);

			return 0;
		}
	}

	::SendMessage(CGlobalVariable::s_wndRssCtrl->GetSafeHwnd(), WM_COMMAND, wParam, lParam);
	
	return CListCtrl::OnCommand(wParam, lParam);
}

void CFeedIcoItemListCtrl::UpdateMenuState(CTitleMenu	& menuRSS)
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


void CFeedIcoItemListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: �ڴ˴������Ϣ����������
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

void CFeedIcoItemListCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (lpDrawItemStruct->CtlType == ODT_MENU)
		m_pMenuXP->DrawItem(lpDrawItemStruct);
	else
		CListCtrl::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CFeedIcoItemListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
		m_pMenuXP->MeasureItem(lpMeasureItemStruct);
	else
		CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CFeedIcoItemListCtrl::OnLvnHotTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	// vista+ ϵͳ�ľ��������vista-ϵͳ�� ����Ҫ�Ӵ���Ϣ
	if (thePrefs.GetWindowsVersion() >= _WINVER_VISTA_ && IsThemeEnabled())
		return;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nCurrentHotItemIndex = GetHotItem();

	if (nCurrentHotItemIndex == m_nLastHotItemIndex)
		return;

	if (-1 != nCurrentHotItemIndex)
	{// move into the hot item
		m_nLastHotItemIndex = nCurrentHotItemIndex;
	}
	else
	{// move out of the hot item
		nCurrentHotItemIndex = m_nLastHotItemIndex;
		m_nLastHotItemIndex = -1;
	}

	CRect itemRect(0,0,0,0);
	GetItemRect(nCurrentHotItemIndex, &itemRect, LVIR_BOUNDS);

	InvalidateRect(&itemRect);
}

void CFeedIcoItemListCtrl::FilterItemsBySearchKey(CString* pSearchKey)
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

	RemoveAllGroups();//ɾ������
	RemoveAllGroupData();//ɾ��m_mapGroups
	DeleteAllItems();//ɾ��items

	for (StringMatcher::ListItemsNeedShow::const_iterator ix = listItemsNeedShow.begin(); ix != listItemsNeedShow.end(); ++ix)
	{
		CRssFeed* listKey = (CRssFeed*)*ix;
		CString strCaption = GetExtentString( listKey->GetDisplayName() );

		int nRet = InsertItem(GetItemCount(), strCaption);
		this->SetItemData(nRet, LPARAM(listKey));
	}
	GroupAllFeedsByType(0);

	SetRedraw(TRUE);
}
BOOL CFeedIcoItemListCtrl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	if (pMsg->message == WM_KEYDOWN)
	{
		if ( VK_ESCAPE == pMsg->wParam )
		{
			FilterItemsBySearchKey(NULL);
		}
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}
