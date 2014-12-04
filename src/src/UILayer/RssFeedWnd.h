/*
 * $Id: RssFeedWnd.h 20785 2010-11-12 02:44:52Z huangqing $
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

// CRssFeedWnd �Ի���

#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"
#include "ResizableLib\ResizableDialog.h"
#include "SplitterControlEx.h"
#include "FeedItemListCtrl.h"
#include "FeedIcoItemListCtrl.h"
#include "RssInfoWnd.h"
#include "WorkLayer\rss\RssFeed.h"

//class CFeedListBox : public CListBox
//{
//	DECLARE_DYNAMIC(CFeedListBox)
//
//public:
//	CFeedListBox();
//
//public:
//	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
//	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
//protected:
//	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
//
//private:
//	HICON	m_hIcon[CRssFeed::fsMaxState];
//	int		m_iItemHeight;
//};

enum ListShowMode
{
	LISTSTYLE_ITEM		= 0,
	LISTSTYLE_ICO	= 1,
	LISTSTYLE_SINGLE = 2
};

class CRssFeedWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CRssFeedWnd)

public:
	CRssFeedWnd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRssFeedWnd();

// �Ի�������
	enum { IDD = IDD_RW_FEED };

public:
	// �ػ�ȫ��feed
	void RedrawAllFeed()
	{
		m_listFeedItem.Invalidate(TRUE);
		m_listIcoFeedItem.Invalidate(TRUE);
	}

	// ����toolbar��ť״̬
	void UpdateToolBarState(CToolBarCtrl & toolbar)
	{
		//���ݵ�ǰ��ʾ��List����Toolbar״̬
		if ( m_ListShowMode == LISTSTYLE_ITEM )
		{
			m_listFeedItem.UpdateToolBarState(toolbar);
		}
		else if ( m_ListShowMode == LISTSTYLE_ICO )
		{
			m_listIcoFeedItem.UpdateToolBarState(toolbar);
		}
		else if ( m_ListShowMode == LISTSTYLE_SINGLE )
		{
			m_listSingleFeedItem.UpdateToolBarState(toolbar);
		}
	}

	// ���б����ɾ��һ��feed
	void RemoveFeed(CRssFeed & feed)
	{
		m_listFeedItem.RemoveFeed(feed);
	}

	// ���б����ɾ��һ��feed
	void RemoveICOFeed(CRssFeed & feed)
	{
		m_listIcoFeedItem.RemoveFeed(feed);
	}

	// ���б�������һ��feed
	void AddFeed(CRssFeed & feed, BOOL bIsNew)
	{
		m_listFeedItem.AddFeed(feed, bIsNew);
	}

	// ���б�������һ��feed
	void AddICOFeed(CRssFeed & feed, BOOL bIsNew)
	{
		m_listIcoFeedItem.AddFeed(feed, bIsNew);
	}

	// ������ʾһ��feed����item��Ϣ
	void RedrawFeed(CRssFeed & feed);

	// �ػ�feed���ڵ���
	void RedrawFeedLine(CRssFeedBase * pFeed)
	{
		m_listFeedItem.RedrawFeedLine(pFeed);
		m_listSingleFeedItem.RedrawFeedLine(pFeed);
	}

	// ����ICOlist ����
	//void RedrawFeedItemPoster(CRssFeed* item)
	//{
	//	m_listIcoFeedItem.UpdatePoster(item);
	//}

	// �ػ�feed item���ڵ���
	void RedrawFeedItem(CRssFeed::Item & item)
	{
		m_listFeedItem.RedrawFeedItem(item);
		m_listSingleFeedItem.RedrawFeedItem(item);
	}

	// ׼������feed
	void BeginUpdateFeed()
	{
		m_listFeedItem.SetRedraw(FALSE);
		m_listIcoFeedItem.SetRedraw(FALSE);
		m_listSingleFeedItem.SetRedraw(FALSE);
	}

	// ��������feed
	void EndUpdateFeed()
	{
		m_listFeedItem.SetRedraw(TRUE);
		m_listIcoFeedItem.SetRedraw(TRUE);
		m_listSingleFeedItem.SetRedraw(TRUE);
		//m_lsbFeed.Invalidate();
	}

	// ����ָ��feed��Ŀ
	BOOL DownloadFeedItem(int iItemIndex);

	//����feed��������Ŀ
	BOOL DownloadAllItem(int nIndex);

	//����feed����ͣ/ֹͣ����
	BOOL ResumeAllItem(int nIndex);

	// ��������������Ϣ
	BOOL SendDownloadRequestMessage(CRssFeed::Item & item);

	//��ָ���������feed��Ŀ
	BOOL OpenFeedItem(int iItemIndex);

	//�л�2��list
	void SwitchList(ListShowMode listmode);

	//����Toolbar��������ǰList
	void SetCurrentList();

	//����Toolbar ��������ӦList
	void SetToolbar2List(ListShowMode listmode);

	void JumpListInfo(CRssFeed * pFeed, ListShowMode listmode = LISTSTYLE_SINGLE/*LISTSTYLE_ITEM*/);

	//�Զ��л�List Show Mode
	void AutoSwitchListMode();//��ʱû�ط���

	//���singleList�е�����,���л���ͼ��ʱ��ʹ��
	void ClearSingleList();

	//��ȡinfoList��ǰFeed��ǰ��feed
	void GetCurFeedInfo(CRssFeed * pCurFeed);

	ListShowMode m_ListShowMode;

public: // from CResizableDialog
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	//afx_msg void OnVSplitterClicked(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//afx_msg void OnLbnSelchangeRwFeedList();
	afx_msg void OnNMDblclkRwFeedItemList(NMHDR* pNMHDR, LRESULT *pResult);
	//afx_msg void OnNMClickRwFeedItemList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnDownloadButtonClicked(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnItemchangedRwFeedItemList(NMHDR *pNMHDR, LRESULT *pResult);

	//�л�RSS LIST SHOW MODE;�� BUTTONST�� ����
	afx_msg LRESULT OnSwitchListShowModeClicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListInfoClicked(WPARAM wParam, LPARAM lParam);

private:
	// �����ָ���
	//void CreateSplitter();

	// �����������Ҳ���
	//void AdjustVLayout(UINT uLeftWidth = -1);

	//���ļ�
	void OpenFile(const CKnownFile* file);

public:
//	CFeedListBox		m_lsbFeed;			///< Feed�б��
	CFeedItemListCtrl	m_listFeedItem;		///< Feed��Ŀ�б�
	CFeedItemListCtrl	m_listSingleFeedItem;		///< ��������Feed��Ŀ�б�
	CFeedIcoItemListCtrl m_listIcoFeedItem;
//	CSplitterControlEx	m_wndVSplitter;		///< �ָ���

	CRssInfoWnd			m_wndRssInfo;

private:
	CRssFeed * m_pFeedPre;//InfoList��ǰFeed��ǰ������
	CRssFeed * m_pFeedNext;

	CString m_strGroupTitle;

	int m_nCurrFeedCatalog;//��ǰ�ڷ����λ��
	int m_nFeedCatalogCount;//����Feed��Ŀ
	//static const int DEFAULT_LEFT_LIST_WIDTH	= 168;	///< Ĭ������б���
	//static const int SPLITTER_RANGE_MAX			= 350;	///< ����϶���Χ
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
