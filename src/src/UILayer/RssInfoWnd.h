#pragma once

#include "ButtonST.h"
// CRssInfoWnd

class CxImage;

class CRssInfoWnd : public CWnd
{
	DECLARE_DYNAMIC(CRssInfoWnd)

public:
	CRssInfoWnd();
	virtual ~CRssInfoWnd();

public:
	BOOL		Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();

public:
	CButtonST m_btBack;//���ذ�ť

	CButtonST m_btPre;//ǰһ��
	CButtonST m_btNext;//��һ��

	CString m_strGroupTitle;
	CString m_strInfo;

	int m_nCurrFeedCatalog;//��ǰ�ڷ����λ��
	int m_nFeedCatalogCount;//����Feed��Ŀ

	CxImage*	m_imageRss_Info_Bg;//����ͼ

	CFont  m_FontBold;
public:
	void SetNumData(CString strGroupTitle, CString strFeedName, int nCurr, int nCount);
	
	//������ԴID��ȡCxImage
	CxImage* GetPng(LPCTSTR lpszPngResource);

	void Localize(void);
};


