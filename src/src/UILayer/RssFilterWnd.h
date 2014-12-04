#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "ResizableLib\ResizableDialog.h"


// CRssFilterWnd �Ի���
class CRssFilter;
class CRssFilterWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CRssFilterWnd)

public:
	CRssFilterWnd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRssFilterWnd();

// �Ի�������
	enum { IDD = IDD_RW_FILTER };

public:
	// ��ӹ��������б�
	void AddFilterToList(CRssFilter & filter);

	// ���¿�ʼ
	void BeginUpdateFilter()
	{
		m_listFilter.SetRedraw(FALSE);
	}

	// ���½���
	void EndUpdateFilter()
	{
		m_listFilter.SetRedraw(TRUE);
	}

	// ȡ�ù�����
	CRssFilter * GetFilter(int iItemIndex)
	{
		return reinterpret_cast<CRssFilter *>(m_listFilter.GetItemData(iItemIndex));
	}

private:
	// ��ʾһ������������
	void DrawFilter(const CRssFilter * pFilter);

	// ����һ������������
	CRssFilter * SaveFilterContent(CRssFilter * pFilter);

	// ����һ���������Ƿ�����״̬
	CRssFilter * SaveFilterEnable(CRssFilter * pFilter, BOOL bEnable);

	// ����һ������������
	CRssFilter * SaveFilterName(CRssFilter * pFilter, LPCTSTR lpszName);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	// �������б�
	CListCtrl m_listFilter;
	// ��Ҫ�������Ӵ�
	CEdit m_edtFilterIn;
	// ��Ҫ�ų����Ӵ�
	CEdit m_edtFilterNot;
	// ��Դ����
	CComboBox m_cboQuality;
	// �ļ�����
	CEdit m_edtFileType;
	// ��Դ����Ŀ¼
	CEdit m_edtSaveDir;
	// ѡ����Դ����Ŀ¼
	CButton m_btnSelSaveDir;
	// ��Դ��С�ߴ�(MB)
	CEdit m_edtMinSize;
	// ��Դ���ߴ�(MB)
	CEdit m_edtMaxSize;
	// Ӧ�õ�ָ���Ķ���
	CComboBox m_cboApplyFeed;
	// ָ���Ƿ��Զ���ʼ����
	CButton m_btnNotAutoStart;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedRwFilterList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditRwFilterList(NMHDR *pNMHDR, LRESULT *pResult);
};
