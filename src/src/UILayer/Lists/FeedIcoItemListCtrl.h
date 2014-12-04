#pragma once
#include "MuleListCtrl.h"
#include "WorkLayer\rss\RssFeed.h"
#include "TitleMenu.h"
#include "MatchString.h"

// CFeedIcoItemListCtrl
class CxImage;
class CMenuXP;
//typedef CListCtrl CBaseListCtrl;

class CFeedIcoItemListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CFeedIcoItemListCtrl)

public:
	CFeedIcoItemListCtrl();
	virtual ~CFeedIcoItemListCtrl();

	LRESULT InsertTypeGroup(int nItem, int nGroupID, const CString& strHeader, DWORD dwState = LVGS_NORMAL, DWORD dwAlign = LVGA_HEADER_LEFT);

	//�Ե���FEED���з���
	BOOL GroupFeedByType(int nIndex, int nType);

	//������FEED���з���
	BOOL GroupAllFeedsByType(int nType);//Type��ʱ��û�õ�
	BOOL SetItemGroupId(int nItem, int nGroupID);

	int AddFeed(CRssFeed & feed, BOOL bIsNew);
	void RemoveFeed(CRssFeed & feed);

	//ɾ��ĳ��item��groupMap�е�����, bUpdate��ʶ �Ƿ� ���µ��µķ�����Ϣ, ɾ��IDS_DEFAULT�����е�����
	void RemoveItemforMap(CRssFeed & feed, int nIndex, bool bUpdate = false);

	BOOL UpdatePoster(CRssFeed* feed);

	int GetItemIndex(CRssFeed* feed);//����rss data�ҵ����item index

	/// ����toolbar��ť״̬
	void UpdateToolBarState(CToolBarCtrl & toolbar);

	/// ȡ�õ�ǰѡ���index
	int GetCurrSelectIndex();

	/// ȡ��feed
	CRssFeed * GetFeed(int iItemIndex);

	//�и��ַ�����Ӧ2��
	CString GetExtentString(const CString & strCaption);

	//����Group����ȡ��Group Num
	int GetGroupCounts(CString strGroupCaption);

	void Init();

	// �ַ���ƥ��ɸѡ
	void FilterItemsBySearchKey(CString* searchKey);

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg void OnDestroy();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	afx_msg void OnNMDblclkFeedIcoItemListCtrl(NMHDR *pNMHDR, LRESULT *pResult);

private:

	//������ԴID��ȡCxImage
	CxImage* GetPng(LPCTSTR lpszPngResource);

	//�ӷ���ͼƬ��imgList
	int	AddImg2List(CxImage* img);

	//����Item�ķ���ͼƬ��imgList�е�ID��
	BOOL SetItemPoster(int nItem, int nImageListId);
	
	//��õ�ǰGroup��Ŀ,��ʱֻ�����һ������
	int GetGroupCount();

	//ɾ���б����Լ�����ķ�������
	void RemoveAllGroupData();

	///ֻ��VISTA+ϵͳʹ��
	//group �Ƿ����չ��
	BOOL IsGroupStateEnabled();
	//���group rect 
	CRect GetRectbyGroupID(int nGroupID, UINT mask);
	//����group ID���group������Ϣ
	LVGROUP GetGroupInfoByIndex(int iGroupId, UINT mask);
	//���Group����
	BOOL HasGroupState(int nGroupId, DWORD dwState);
	///ֻ��VISTA+ϵͳʹ��

	//��ȡgroup����
	CString GetGroupCaption(int nGroupId);

	////����Groupע��
	//BOOL SetGroupFooter(int nGroupId, CString & strFooter);

	////��ȡGroupע��
	//CString GetGroupFooter(int nGroupId);

	//��ȡһ��item��group ID
	int GetGroupIDByItemIndex(int nIndex);

	//��ȡgroup��ϸ��Ϣ,����DrawGroup�������
	void DrawGroupInfo(CDC & dcMem, const CRect & rectClip, const CRect & rectClient);
	//�������group
	void DrawGroup(CDC & dcMem, const CRect & rectGroup, const CString & strGroupCaption, const CString & strGroupTotals);
	//����items
	void DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient);

	//����Group�߶�
	void SetGroupHeight(int nHeight);

	/// ����toolbar��ť״̬
	void UpdateMenuState(CTitleMenu	& menuRSS);
	
	//XPϵͳ��group�߶�
	int nGroupHeight;

	int m_nLastHotItemIndex; // Save the last hot item index

private:
	CImageList				m_PosterImageList;

	CxImage*				m_imageRss_PosterBg;//���汳��ͼ
	CxImage*				m_imageRss_PosterSelBg;//����ѡ��״̬����ͼ
	CxImage*				m_imageRss_GroupBg;//Rss Group���򱳾�
	CxImage*				m_imageRss_PosterDefault;//Ĭ�Ϸ���ͼ

	CxImage*				m_imageRss_Group_Drop_Up_Nor;//Vista WIN7 ������ť
	CxImage*				m_imageRss_Group_Drop_Up_Hov;
	CxImage*				m_imageRss_Group_Drop_Down_Nor;
	CxImage*				m_imageRss_Group_Drop_Down_Hov;

	CFont                 m_FontBold, *m_pOldFont;

	CMenuXP*				m_pMenuXP;

	//Group Name | Group ������items����
	CSimpleMap< CString, CSimpleArray<int> > m_mapGroups;

	// �ַ���ƥ��ɸѡ
	StringMatcher m_stringMatcher;

public:
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnHotTrack(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


