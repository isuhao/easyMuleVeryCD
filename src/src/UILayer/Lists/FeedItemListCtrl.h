/*
 * $Id: FeedItemListCtrl.h 20807 2010-11-16 11:43:07Z huhonggang $
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
#include "MuleListCtrl.h"
#include "ButtonST.h"
#include <vector>
#include "WorkLayer\rss\RssFeed.h"
#include "TitleMenu.h"
#include "MatchString.h"

// CFeedItemListCtrl
class CxImage;
class CMenuXP;

typedef CMuleListCtrl CBaseListCtrl;

class CFeedItemListCtrl : public CBaseListCtrl
{
	DECLARE_DYNAMIC(CFeedItemListCtrl)

public:
	static const int COVER_EXPAND_WIDTH = 120;	///< ����չ��ʱ���
	static const int COVER_COLLAPSE_WIDTH = 22;	///< ����չ����ʱ���

private:
	static const int ITEM_HEIGHT = 24;							///< ��Ŀ�߶�
	static const int COVER_LINES = 6;							///< ��������
	static const int COVER_HEIGHT = ITEM_HEIGHT * COVER_LINES;	///< ����߶�
	static const int SORT_ASC_FLAG = 0;							///< ����
	static const int SORT_DESC_FLAG = 100;						///< ��������ڵ��ڴ�ֵ

	/// ��Ŀ����ɫ1
	static const COLORREF ITEM_BACKGROUND_COLOR1 = RGB(0xF0, 0xF0, 0xF0);

	/// ��Ŀ����ɫ2
	static const COLORREF ITEM_BACKGROUND_COLOR2 = RGB(0xFF, 0xFF, 0xFF);

	/// ���汳��ɫ
	static const COLORREF COVER_BACKGROUND_COLOR = RGB(0xF3,0xF3,0xF3);

	/// �������ɫ
	static const COLORREF SPLIT_LINE_COLOR = RGB(0xC0, 0xC0, 0xC0);

	/// ��ʱ��id
	static const int TIMER_ID = 32771;

public:
	// UTC �� ����ʱ��ʱ��
	static CTimeSpan s_timespan;

private:
	typedef std::vector<CButtonST *>		ItemButtons;

public:
	struct CtrlBase
	{
		virtual void DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct) = 0;
		virtual CRssFeed * GetRssFeed()
		{
			return NULL;
		}
		virtual CRssFeed::Item * GetRssFeedItem()
		{
			return NULL;
		}

		CtrlBase()
			//: m_pbtnDownload(NULL)
		{
		}

		//CButton * m_pbtnDownload;
	};

	struct CtrlFeed : public CtrlBase
	{
		virtual void DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct);
		virtual CRssFeed * GetRssFeed()
		{
			return m_pFeed;
		}

		CtrlFeed()
			: m_pFeed(NULL), 
			  m_bExpand(false),
			  m_bNew(false)
		{
		}

		CRssFeed *				m_pFeed;
		bool					m_bExpand;	///< ��ǰ���Ƿ�չ��
		bool					m_bNew;    ///��һ�μ���
	};

	struct CtrlFeedItem : public CtrlBase
	{	
		virtual void DrawItem(CDC & dc, CFeedItemListCtrl & list, LPDRAWITEMSTRUCT lpDrawItemStruct);
		virtual CRssFeed::Item * GetRssFeedItem()
		{
			return m_pFeedItem;
		}

		CtrlFeedItem()
			: m_pFeedItem(NULL),
			  m_bIsLast(FALSE)
		{
		}

		CRssFeed::Item *	m_pFeedItem;
		bool				m_bIsLast;	///< �Ƿ�Ϊ�����е����һ����Ŀ
	};


public:
	CFeedItemListCtrl();
	virtual ~CFeedItemListCtrl();

	/// ��ʼ��
	void Init(BOOL bSingle = FALSE);

public:

	/// ���ػ�
	void	Localize();

	/// ��Ӷ���
	int AddFeed(CRssFeed & feed, BOOL bIsNew)
	{
		m_stringMatcher.AddItem((void*)&feed, feed.GetDisplayName());

		int nNextIndex = GetItemCount();
		return AddFeed(nNextIndex, feed, bIsNew);
	}

	/// ���²���feed
	void ReAddFeed(CRssFeed & feed);

	/// ɾ������
	void RemoveFeed(CRssFeed & feed);

	/// ������ʾһ��feed����item��Ϣ
	void RedrawFeed(CRssFeed & feed);

	/// ������ʾfeed������������Ŀ��
	void RedrawFeedLine(CRssFeedBase * pFeed);

	/// ������ʾһ��feed item
	void RedrawFeedItem(CRssFeed::Item & item);

	/// ������ʾһ��feed item
	void RedrawFeedItem(int index);

	// ���feed��Ŀ
	//void AddFeedItem(const CRssFeed::Item & item)
	//{
	//	int nNextIndex = GetItemCount();
	//	AddFeedItem(nNextIndex, item);
	//}

	// ����feed item״̬��������ͼ��
	//void ResetItemImageIndex(int iItemIndex);

	/// ȡ��feed item
	CRssFeed::Item * GetFeedItem(int iItemIndex);

	/// ȡ��feed
	CRssFeed * GetFeed(int iItemIndex);

	/// ȡ����Ŀ����
	int GetItemIndex(const POINT & pt);

	// ȡ�õ���ѡ�е����ذ�ť��
	//int GetClickDownloadBtnIndex();

	/// ���ݿ�����Ϣȡ���к�
	int GetItemIndex(const CtrlBase * pCtrl);

	/// ����Feed��Ϣȡ���к�
	int GetItemIndex(const CRssFeed * pFeed);

	/// ����feedȡ��CtrlFeed
	CtrlFeed * GetCtrlFeed(const CRssFeed * pFeed);

	/// ��ť��תitem index
	int ButtonIndexToItemIndex(int iButtonIndex);

	/// ����toolbar��ť״̬
	void UpdateToolBarState(CToolBarCtrl & toolbar);

	/// ����toolbar��ť״̬
	void UpdateMenuState(CTitleMenu	& menuRSS);

	/// ȡ�õ�ǰѡ���index
	int GetCurrSelectIndex();

	/// �ֶ�����
	void UpdateSort(bool bUpdate = true);

	/// ȡ�ÿ���������feed
	CRssFeed * GetBlankLineFeed(int index);

	/// չ����ָ�����۵�״̬
	void SwitchToStat(int index, BOOL bExpand);

	/// ����ָ����Ŀ��List����
	void ScrollIndex2Top(int nIndex);

	//���õ�������ģʽ
	void SetSingleStyle(BOOL bSingle){ m_bSingleStyle = bSingle; }

	// �ַ���ƥ��ɸѡ
	void	FilterItemsBySearchKey(CString* searchKey);

private:
	/// ��ʼ��feed�б���Ŀ
	void InitFeedListView();

	// ��ʼ��ImageList
	//void InitImageList();

	/// ��Ӷ���
	int AddFeed(int index, CRssFeed & feed, BOOL bIsNew);

	/// ���feed��������Ŀ
	int AddFeedItems(int index, CRssFeed & feed);

	/// ���feed��һ����Ŀ
	void AddFeedItem(int index, const CRssFeed::Item & item, BOOL bIsLast);

	// ��ͼ��
	//void DrawItemIcon(CDC & dc, int iImageIndex, const RECT & rect, BOOL bSelected);

	/// ȡ�ð�ťλ��
	void GetDownloadButtonRect(int index, BOOL bItem, RECT & rectBtn);

	//ȡ����Ļͼ��λ��
	void GetSubtitleRect(CRect & rectSubTitle , int index = -1);

	//ȡ���������rect
	void GetCompleteNumRect(int index, CString strCompleteNum, CRect & rectCompleteNum);

	// �����ذ�ť
	//void DrawDownloadButton(CDC & dc, const RECT & rectBtn);

	/// ȡ�ù���ƫ��
	int GetScrollOffset(DWORD dwFlag);

	// ɾ�������µ�����Ŀ
	//void DeleteFeedItems(int index, const CRssFeed & feed);

	/// ��ʾ���ذ�ť
	BOOL DrawDownloadButton(int index, CtrlBase * pCtrl, HDWP & hWinPosInfo);

	/// �ػ���ť
	void RedrawAllButton();

	/// ����feed״̬ȡ�ö�Ӧ��CxImage
	CxImage * GetFeedCxImage(const CRssFeed & feed);

	/// ����feed��Դ��Ŀ״̬ȡ�ö�Ӧ��CxImage
	CxImage * GetFeedItemCxImage(const CRssFeed::Item & item);

	/// չ��/�۵�feed
	void ExpandFeed(int index, CRssFeed & feed, BOOL bExpand, BOOL bSetRedraw);

	/// �л�չ���۵�״̬
	void SwitchExpandStat(int index);

	/// ȡ�ÿ���ʾ����
	int GetCountPerPage2();

	/// �ж��뷽ʽת�ı��������
	DWORD ConverLvFmtToDtFmt(DWORD dwLvFmt);

	/// ���������Ƿ����һ�еı��
	int ResetIsLastFlag(int index);

	/// ���������Ƿ����һ�еı��
	void ResetIsLastFlag();

	BOOL IsResExit();

	//������ԴID��ȡCxImage
	CxImage* GetPng(LPCTSTR lpszPngResource);

	/// ȡ�õ�ǰҳ����β��
	BOOL GetPageRange(int & iFrom, int & iEnd);

	/// ɾ��ȫ����ť
	void DeleteAllButtons();

	/// ȡ���б���ɫ
	COLORREF GetItemBackColor(int index);

	/// ��������Ŀ
	void DrawItems(CDC & dcMem, const CRect & rectClip, const CRect & rectClient);

	/// ��������Ϣ
	void DrawCoverInfo(CDC & dc, CRect & rect, const CRssFeed & feed);

	/// ��������
	void DrawCoverColumnHead(BOOL bExpand);

	/// ��ǰ�����Ƿ�չ��
	BOOL IsCoverExpand() const;

	/// �л�������״̬
	void SwitchCoverColumnState()
	{
		DrawCoverColumnHead(!IsCoverExpand());
	}

	/// �����������ʵ���������ʾ����
	void FillBlankLine(int index);

	/// �����ж��������ʵ�����
	void FillAllFeedsBlankLine();

	/// ɾ��ָ�������еĿ���
	void DeleteBlankLine(int index);

	/// ɾ�����ж��Ŀ���
	void DeleteAllBlankLine();

	/// ����Ŀ����
	void DoSort(int iColIndex, BOOL bSortAscending);

	/// ȡ�÷�����������
	void GetCoverRect(CRect & rect);

	/// ���з�������
	BOOL IsHitCover(const CPoint & pt);
	

private:
	UINT					m_ButtonID; 
	//CImageList				m_ilsItemImage;			///< Feed��Ŀͼ���б�
	CxImage*				m_imageRSS_Normal;
	CxImage*				m_imageRSS_Disable;
	CxImage*				m_imageRSS_Downloading;
	CxImage*				m_imageRSS_Error;
	CxImage*				m_imageRSS_Update;
	CxImage*				m_imageTask_Complete;
	CxImage*				m_imageTask_Pause;
	CxImage*				m_imageTask_Error;
	CxImage*				m_imageTask_Stop;
	CxImage*				m_imageTask_Downloading1;
	CxImage*				m_imageTask_Downloading2;
	CxImage*				m_imageTask_Downloading3;

	CxImage*				m_imageRss_PosterBg;
	CxImage*				m_imageRss_PosterDefault;

	CxImage*				m_imageSubTitle_Green;
	CxImage*				m_imageSubTitle_Grey;
	UINT					m_nIcoState;
	ItemButtons				m_DownloadButtons;

	CFont                   m_FontBold, *m_pOldFont;

	CMenuXP*				m_pMenuXP;

	BOOL					m_bCanModifyCoverColumn;
	BOOL					m_bSingleStyle;//��������Style

	StringMatcher			m_stringMatcher; // �ַ���ƥ��ɸѡ

private:
	/// feed��ʾ�ı�
	static CString GetFeedText(UINT uColIndex, const CRssFeed & feed);

	/// feed��Ŀ��ʾ�ı�
	static CString GetFeedItemText(UINT uColIndex, const CRssFeed::Item & item);

	// feed��Ŀ��ʾͼ������
	//static UINT GetFeedItemImageIndex(const CRssFeed::Item & item);

	/// feed״̬����
	static CString GetFeedStatStr(const CRssFeed & feed);

	/// feed item״̬����
	static const CString & GetFeedItemStatStr(CRssFeed::EItemState state);

	/// �Ƚ�����
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	/// ȡ���б���ɫ
	static COLORREF GetLineBackColor(BOOL bIsEven);

private:
	//static CString s_strFeedStatStr[CRssFeed::fsMaxState];
	static CString s_strFeedItemStatStr[CRssFeed::isMaxState];

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemActivate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndScroll(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


