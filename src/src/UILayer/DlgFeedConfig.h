#pragma once
#include "afxwin.h"


// CDlgFeedConfig �Ի���

class CRssFeed;

class CDlgFeedConfig : public CDialog
{
	DECLARE_DYNAMIC(CDlgFeedConfig)

public:
	CDlgFeedConfig(CRssFeed & feed, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgFeedConfig();

// �Ի�������
	enum { IDD = IDD_RW_FEED_CONFIG };

private:
	// ����ui�ϵ�feed��Ϣ
	void UpdateFeedInfo();

	// ���ػ�
	void Localize();

private:
	// ����ui�ϵ�feed��С��Ϣ
	void UpdateFeedSizeInfo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

private:
	CRssFeed &		m_feed;
	CString			m_strFeedSize;
	CString			m_strFeedName;
	CComboBox		m_cboInterval;
	CButton			m_btnAutoDownload;
	CButton			m_btnAutoRefresh;
	CStatic			m_stcShowFeedName;
	CToolTipCtrl	m_tooltipShowFeedName;
	CFont *			m_pFontSaveDir;

public:
	UINT		m_uUpdateInterval;
	UINT		m_uAutoDownload;

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnEnChangeEditRwSaveDir();
	afx_msg void OnBnClickedCheckRwAutoRefresh();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticRwShowSaveDir();

protected:
	virtual void OnOK();	
	
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
