// DlgFeedConfig.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "resource.h"
#include "DlgFeedConfig.h"
#include "rss/RssFeed.h"
#include "otherfunctions.h"
#include ".\dlgfeedconfig.h"
#include "PPgGeneral.h"
#include "Preferences.h"

// CDlgFeedConfig �Ի���

IMPLEMENT_DYNAMIC(CDlgFeedConfig, CDialog)
CDlgFeedConfig::CDlgFeedConfig(CRssFeed & feed, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFeedConfig::IDD, pParent),
	m_feed(feed),
	m_uUpdateInterval(0),
	m_uAutoDownload(0),
	m_pFontSaveDir(NULL)
{
}

CDlgFeedConfig::~CDlgFeedConfig()
{
	if ( m_pFontSaveDir != NULL )
	{
		m_pFontSaveDir->DeleteObject();
		delete m_pFontSaveDir;
		m_pFontSaveDir = NULL;
	}
}

void CDlgFeedConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_RSS_INTERVAL, m_cboInterval);
	DDX_Control(pDX, IDC_CHECK_RSS_AUTO_DOWNLOAD, m_btnAutoDownload);
	DDX_Control(pDX, IDC_CHECK_RW_AUTO_REFRESH, m_btnAutoRefresh);
	DDX_Control(pDX, IDC_STATIC_RW_SHOW_FEED_NAME, m_stcShowFeedName);
}

// ����ui�ϵ�feed��С��Ϣ
void CDlgFeedConfig::UpdateFeedSizeInfo()
{
	// ����ʣ��ռ�
	CString strSaveDir;
	GetDlgItemText(IDC_EDIT_RW_SAVE_DIR, strSaveDir);
	uint64 uFreeSpace;
	if ( strSaveDir.GetLength() < 3 || strSaveDir.GetAt(1) != _T(':') || strSaveDir.GetAt(2) != _T('\\') )
	{
		uFreeSpace = 0;
	}
	else
	{
		uFreeSpace = GetFreeDiskSpaceX(strSaveDir.Left(3));
	}

	// ���Ĵ�С������ʣ��ռ䣩
	CString strText;
	strText.Format(_T("%s (%s%s)"),
				   m_strFeedSize,
				   GetResString(IDS_ADDTASKDLG_FREE_SPACE),
				   CastItoXBytes(uFreeSpace)
				  );
	SetDlgItemText(IDC_EDIT_RW_FEED_SIZE, strText);

}


// ����ui�ϵ�feed��Ϣ
void CDlgFeedConfig::UpdateFeedInfo()
{
	// ���涩�Ĵ�С��Ϣ
	m_strFeedSize = CastItoXBytes(m_feed.GetRssTotalSize(), false, false);

	// ��ʾ��������
	m_strFeedName = m_feed.GetDisplayName();
	m_stcShowFeedName.SetWindowText(m_strFeedName);

	//��ʼ��tooltip
	m_tooltipShowFeedName.Create(this, TTS_NOPREFIX);
	m_tooltipShowFeedName.AddTool(&m_stcShowFeedName, m_strFeedName);
	//m_tooltipShowFeedName.SetTipTextColor(RGB(0, 0, 255));
	//m_tooltipShowFeedName.UpdateTipText(m_feed.GetDisplayName(), &m_stcShowFeedName);
	m_tooltipShowFeedName.SetDelayTime(500);
	m_tooltipShowFeedName.Activate(TRUE);

	// ���ı���Ŀ¼
	SetDlgItemText(IDC_STATIC_RW_SHOW_SAVE_DIR, m_feed.m_strSaveDir);

	// ������ʾ�Ķ��Ĵ�С��Ϣ
	UpdateFeedSizeInfo();

	// ��/δ������Ŀ
	unsigned int uFilmCount;
	unsigned int uDownloadedCount = m_feed.GetDownloadedFilmCount(&uFilmCount);
	CString strText;
	strText.Format(_T("%u/%u"), uDownloadedCount, uFilmCount - uDownloadedCount);
	SetDlgItemText(IDC_EDIT_RW_FEED_DOWNLOAD_INFO, strText);

	// ����ˢ�¼��ʱ��
	m_uUpdateInterval = m_feed.GetUpdateInterval();

	// ���ж��壬��ʹ��feed��ֵ�����򣬰�ȫ����������ʾ
	if ( m_feed.UpdateIntervalIsDefined() )
	{
		CPPgGeneral::InitRssIntervalComboBox(m_cboInterval, m_uUpdateInterval);
		if ( m_feed.IsManualRefresh() )
		{
			// �ֶ�ˢ�£�����ѡ���
			m_btnAutoRefresh.SetCheck(BST_UNCHECKED);
			m_cboInterval.SetCurSel(0);
			m_cboInterval.EnableWindow(FALSE);
		}
		else
		{
			// �Զ�ˢ�£� ����ѡ���
			m_btnAutoRefresh.SetCheck(BST_CHECKED);
			m_cboInterval.EnableWindow(TRUE);
		}
	}
	else
	{
		// ʹ��ȫ������
		CPPgGeneral::InitRssIntervalComboBox(m_cboInterval, thePrefs.m_uRssUpdateInterval);
		m_btnAutoRefresh.SetCheck(BST_CHECKED);
		m_cboInterval.EnableWindow(TRUE);
	}

	// �����Զ�����
	m_uAutoDownload = m_feed.GetAutoDownload();
	
	// ���ж��壬��ʹ��feed��ֵ�����򣬰�ȫ����������ʾ
	if ( m_feed.AutoDownloadIsDefined() )
	{
		m_btnAutoDownload.SetCheck(m_uAutoDownload ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_btnAutoDownload.SetCheck(thePrefs.m_bRssAutoDownload ? BST_CHECKED : BST_UNCHECKED);
	}
}

// ���ػ�
void CDlgFeedConfig::Localize()
{
	// ����
	SetWindowText(GetResString(IDS_RW_RSS_CONFIG));

	// ������Ϣ
	SetDlgItemText(IDC_STATIC_RW_FEED_INFO, GetResString(IDS_RW_FEED_INFO));
	SetDlgItemText(IDC_STATIC_RW_FEED_NAME, GetResString(IDS_RW_CONFIG_RSS_NAME) + CString(":"));
	SetDlgItemText(IDC_STATIC_RW_FEED_SIZE, GetResString(IDS_RSSLIST_LENGTH) + CString(":"));
	SetDlgItemText(IDC_STATIC_RW_FEED_DOWNLOAD_INFO, GetResString(IDS_RW_FEED_DOWNLOAD_INFO) + CString(":"));
	SetDlgItemText(IDC_STATIC_RW_FEED_SAVE_DIR, GetResString(IDS_ADDTASKDLG_SAVE_LOCATION) + CString(":"));

	// ����
	SetDlgItemText(IDC_STATIC_RW_FEED_CONFIG, GetResString(IDS_RW_FEED_CONFIG));
	m_btnAutoRefresh.SetWindowText(GetResString(IDS_RW_AUTO_REFRESH));
	SetDlgItemText(IDC_STATIC_RSS_INTERVAL, GetResString(IDS_PPGRSS_INTERVAL));
	m_btnAutoDownload.SetWindowText(GetResString(IDS_PPGRSS_AUTO_DOWNLOAD));
}

BEGIN_MESSAGE_MAP(CDlgFeedConfig, CDialog)
	ON_EN_CHANGE(IDC_EDIT_RW_SAVE_DIR, OnEnChangeEditRwSaveDir)
	ON_BN_CLICKED(IDC_CHECK_RW_AUTO_REFRESH, OnBnClickedCheckRwAutoRefresh)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_RW_SHOW_SAVE_DIR, OnStnClickedStaticRwShowSaveDir)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CDlgFeedConfig ��Ϣ�������


void CDlgFeedConfig::OnEnChangeEditRwSaveDir()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateFeedSizeInfo();
}

BOOL CDlgFeedConfig::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	EnableToolTips(TRUE);
	GetDlgItem(IDC_EDIT_RW_FEED_NAME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_RW_SAVE_DIR)->ShowWindow(SW_HIDE);
	
	if ( CWnd *pWndSaveDir = GetDlgItem(IDC_STATIC_RW_SHOW_SAVE_DIR) )
	{
		if ( CFont * pFont = pWndSaveDir->GetFont() )
		{
			LOGFONT logfont;
			pFont->GetLogFont(&logfont);
			logfont.lfUnderline = TRUE;
			
			m_pFontSaveDir = new CFont();
			m_pFontSaveDir->CreateFontIndirect(&logfont);
			pWndSaveDir->SetFont(m_pFontSaveDir);
		}
	}

	// ����ui�ϵ�feed��Ϣ
	UpdateFeedInfo();

	// ���ػ�
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgFeedConfig::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���

	// �������ʱ����
	switch ( m_btnAutoRefresh.GetCheck() )
	{
		case BST_UNCHECKED:
			// ȡ���Զ�ˢ�£���Ϊ�ֶ�ˢ��
			m_uUpdateInterval = CRssFeed::MANUAL_REFRESH_VALUE;
			break;

		case BST_CHECKED:
			// ѡ���Զ�ˢ�£�ʹ��ѡ����ڵ�ֵ
			{
				int iCurSel = m_cboInterval.GetCurSel();
				if ( iCurSel >= 0 )
				{
					m_uUpdateInterval = m_cboInterval.GetItemData(iCurSel);
				}
			}
			break;
	}

	// �����Ƿ��Զ�����
	m_uAutoDownload = (m_btnAutoDownload.GetCheck() == BST_CHECKED);

	CDialog::OnOK();
}

void CDlgFeedConfig::OnBnClickedCheckRwAutoRefresh()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	// ѡ���Զ�ˢ�£�����ѡ��ȡ���Զ�ˢ�£�����ѡ��
	m_cboInterval.EnableWindow(m_btnAutoRefresh.GetCheck() == BST_CHECKED);
}

void CDlgFeedConfig::OnPaint()
{
	CDialog::OnPaint();

	//CPaintDC dc(this); // device context for painting
	//// TODO: �ڴ˴������Ϣ����������
	//// ��Ϊ��ͼ��Ϣ���� CDialog::OnPaint()

	//CRect rectFeedName;
	//GetDlgItem(IDC_STATIC_RW_SHOW_FEED_NAME)->GetWindowRect(rectFeedName);
	//ScreenToClient(rectFeedName);
	//dc.SetBkMode(TRANSPARENT);
	//CFont * pOrgFont = dc.SelectObject(GetFont());

	//dc.DrawText(m_feed.GetDisplayName(), 
	//			rectFeedName, 
	//			DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_TOP | DT_NOPREFIX
	//		   );
	//if ( pOrgFont != NULL )
	//{
	//	dc.SelectObject(pOrgFont);
	//}
	
}

BOOL CDlgFeedConfig::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	if( m_tooltipShowFeedName.m_hWnd != NULL ) 
	{
		m_tooltipShowFeedName.RelayEvent(pMsg);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CDlgFeedConfig::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if ( nCtlColor == CTLCOLOR_STATIC )
	{
		ASSERT(pDC != NULL);
		ASSERT(pWnd != NULL);
		if ( pWnd->GetDlgCtrlID() == IDC_STATIC_RW_SHOW_SAVE_DIR )
		{
			pDC->SetTextColor(RGB(0, 0, 255));
		}
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CDlgFeedConfig::OnStnClickedStaticRwShowSaveDir()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ShellExecute(GetSafeHwnd(), 
				 _T("open"),
				 m_feed.m_strSaveDir,
				 NULL,
				 NULL,
				 SW_SHOWNORMAL
				);
}

BOOL CDlgFeedConfig::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	ASSERT(pWnd != NULL);

	if ( pWnd->GetDlgCtrlID() != IDC_STATIC_RW_SHOW_SAVE_DIR )
	{
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
	}
	else
	{
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
		return TRUE;
	}
}
