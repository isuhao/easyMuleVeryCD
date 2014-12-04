// DlgMainTabSearch.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "DlgMainTabSearch.h"
#include "PageTabBkDraw.h"
#include "TabItem_Normal.h"
#include "TabItem_NormalCloseable.h"
#include "SearchListCtrl.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "HelpIDs.h"
#include "UserMsgs.h"
#include "CmdFuncs.h"
#include "TabItem_Wnd.h"
#include "DlgMainTabResource.h"
#include "kademlia/kademlia/kademlia.h"
#include ".\dlgmaintabsearch.h"

#include "WorkLayer/FileSearch.h"

#define UM_SEARCHTAB_EM_DESTROY	UM_END  + 10
#define IDC_SEARCH_TAB	IDC_INTERVAL_LAB + 0x100

// CDlgMainTabSearch �Ի���

IMPLEMENT_DYNAMIC(CDlgMainTabSearch, CDialog)
CDlgMainTabSearch::CDlgMainTabSearch(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDlgMainTabSearch::IDD, pParent)
{
	m_dwCounter = 0;
	m_SearchMap.InitHashTable(1031);
	m_dwTotalCount = 0;
}

CDlgMainTabSearch::~CDlgMainTabSearch()
{
	POSITION pos = m_SearchMap.GetStartPosition();
	while (pos != NULL) 
	{
		int key;
		SSearchParams * params;
		m_SearchMap.GetNextAssoc(pos, key, params);
		if(params != NULL)
			delete params;
	}
	m_SearchMap.RemoveAll();
}

void CDlgMainTabSearch::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMainTabSearch, CResizableDialog)
	ON_NOTIFY(NMC_TW_ACTIVE_TAB_CHANDED, IDC_SEARCH_TAB, OnNMActiveTabChanged)
	ON_NOTIFY(NMC_TW_TAB_DESTROY, IDC_SEARCH_TAB, OnNMTabDestroy)
	ON_NOTIFY(NMC_TW_TAB_CREATE, IDC_SEARCH_TAB, OnNMTabCreate)
	ON_MESSAGE(UM_SEARCHTAB_EM_DESTROY,OnEMTabDestroy)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CDlgMainTabSearch ��Ϣ�������

void CDlgMainTabSearch::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	OnOK();
}

void CDlgMainTabSearch::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	OnCancel();
}

int CDlgMainTabSearch::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CResizableDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN);
	return 0;
}

BOOL CDlgMainTabSearch::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CRect	rect;
	//GetDlgItem(IDC_TAB1)->GetWindowRect(&rect);
	//ScreenToClient(&rect);
	GetClientRect(&rect);

	m_tabWnd.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, rect, this, IDC_SEARCH_TAB );
	CPageTabBkDraw	*pBarBkDraw = new CPageTabBkDraw;
	m_tabWnd.SetBarBkDraw(pBarBkDraw);

	ShowEmuleSearch();
	AddAnchor(m_tabWnd,TOP_LEFT,BOTTOM_RIGHT);
	return TRUE;  
}

void CDlgMainTabSearch::ShowEmuleSearch()
{
	DWORD CustomData = 0xF0F00000;

	CString	strDisplayText;
#ifdef EMULE_SEARCH
	strDisplayText = GetResString(IDS_SEARCHEMULE);
#else
	strDisplayText = GetResString(IDS_SEARCHVERYCD);
#endif
	SSearchParams * pSearchParams = GetParameters(_T(""));

	CustomData += m_dwCounter;

	theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;
	int iCounter = m_dwCounter;
	m_dwCounter++;

	m_SearchMap.SetAt(iCounter, pSearchParams);

	CTabItem_NormalCloseable	*pTi = NULL;
	pTi = new CTabItem_NormalCloseable;
	pTi->SetCaption(strDisplayText);
	pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
	pTi->SetCustomData(CustomData);
	pTi->EnableClose(FALSE);
	pTi->SetDesireLength(150);

	m_tabWnd.AddTab(pTi,TRUE);
	m_dwTotalCount++;
}

void CDlgMainTabSearch::CreateNewSearch(LPCTSTR lpszCaption,ESearchType * pSearchType/* = NULL*/)
{
	DWORD	iCounter;
	DWORD CustomData = 0xF0F00000;

	SSearchParams * pSearchParams = GetParameters(lpszCaption);
	if(pSearchType)
		pSearchParams->eType = *pSearchType;
	else
		pSearchParams->eType = SearchTypeEasyMuleFile;

	CustomData          += m_dwCounter;
	theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;
	iCounter = m_dwCounter;	
	m_dwCounter++;
	m_SearchMap.SetAt(iCounter, pSearchParams);
	
	CString	strDisplayText;
	strDisplayText = GetResString(IDS_TABTITLE_SEARCH_RESULT);
	strDisplayText += lpszCaption;

	CTabItem_NormalCloseable	*pTi = NULL;
	pTi = new CTabItem_NormalCloseable;
	pTi->SetCaption(strDisplayText);
	pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
	pTi->SetCustomData(CustomData);
	pTi->SetDesireLength(150);
	POSITION Pos = m_tabWnd.GetActiveTab();
	m_tabWnd.AddTab(pTi,TRUE,Pos);

	if (!theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pSearchParams))
		m_SearchMap.RemoveKey(iCounter);

	m_dwTotalCount++;		
}

bool	CDlgMainTabSearch::UpdataJasonSearchState()
{
	bool	bUpdata = false;

	POSITION	pos = m_tabWnd.GetFirstTab();
	CTabItem	*pItem = NULL;
	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
				DWORD dwCounter = pItem->m_dwCustomData - 0xF0F00000;

				SSearchParams *pSearchParams=NULL;
				BOOL bSearch = m_SearchMap.Lookup(dwCounter, pSearchParams);

				if ( bSearch && pSearchParams && ( pSearchParams->nSeachStat == 0 || pSearchParams->nSeachStat == 1 ))//����״̬Ϊ�ǽ���
				{
					CFileSearch * pFileSearch = NULL;
					BOOL bSearchFile = theApp.emuledlg->searchwnd->m_pwndResults->m_FileSearchMap.Lookup(pSearchParams->dwSearchID, pFileSearch);

					if ( bSearchFile && pFileSearch )
					{
						bUpdata = true;//ˢ������״̬�ɹ�!
						BOOL bActive = pItem->GetActive();

						CFileSearchReqSocket * pSearchReqSocket = pFileSearch->m_pSearchReqSocket;
						if (pSearchReqSocket)
						{
							pSearchParams->nSeachStat	=	pSearchReqSocket->m_SeachStat;//��������״̬
							switch (pSearchReqSocket->m_SeachStat)
							{
							case -1:
								pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_FAILED)+pFileSearch->m_strKeyValue );
								if (bActive)
									theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(GetResString(IDS_STATIC_SEARCH_RESULT_FAILED));
								break;
							case 0:
								pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT) +pFileSearch->m_strKeyValue );
								if (bActive)
									theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(_T(""));
								break;
							case 1:
								pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCHING) + _T(":") +pFileSearch->m_strKeyValue );
								if (bActive)
								{
									static int nFlag = 1;
									CString strPoint;

									for (int i = 1; i<nFlag; i++)	
										strPoint += _T(".");

									if (nFlag++ > 6)
										nFlag = 1;

									theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_TABTITLE_SEARCHING) + strPoint);
								}
								break;
							case 2:
								pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_SUCCESS) +pFileSearch->m_strKeyValue );

								pSearchParams->uSeachResults	=	pSearchReqSocket->m_SeachNum;//�������������Ŀ

								if (bActive)
								{
									CString strResult;
									strResult.Format(GetResString(IDS_STATIC_SEARCH_RESULT_SUCCESS), pSearchReqSocket->m_SeachNum);
									theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(strResult);
								}
								break;
							default://jason��������������
								pSearchParams->nSeachStat	=	-1;
								pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_FAILED) +pFileSearch->m_strKeyValue );
								if (bActive)
									theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_STATIC_SEARCH_RESULT_FAILED) );
								break;
							}
						}
						
						pItem->Invalidate();
					}
				}
			}
		}
	}

	return bUpdata;
}

void	CDlgMainTabSearch::UpdataActiveJasonSearchState( SSearchParams* pParams )
{
	if (pParams	==	NULL)
		return;

	POSITION Pos = m_tabWnd.GetActiveTab();
	CTabItem* pItem	= NULL;
	bool	bActive = false;
	if (Pos != NULL)
	{
		pItem = m_tabWnd.GetTabItem(Pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
					bActive	=	true;
			}
		}
	}

	CString strResult;
	switch (pParams->nSeachStat)
	{
	case -1:
		if(bActive)
			pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_FAILED)+pParams->strExpression );
		theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_STATIC_SEARCH_RESULT_FAILED) );
		break;
	case 0:
		//if(bActive)
		//	pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT) +pParams->strKeyword );
		theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(_T(""));
		break;
	case 1:
		if(bActive)
			pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCHING) + _T(":") +pParams->strExpression );
		theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_TABTITLE_SEARCHING) );
		break;
	case 2:
		if(bActive)
			pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_SUCCESS) +pParams->strExpression );
		strResult.Format( GetResString(IDS_STATIC_SEARCH_RESULT_SUCCESS) , pParams->uSeachResults);
		theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(strResult);
		break;
	default://jason��������������
		if(bActive)
			pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_FAILED) +pParams->strExpression );
		theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_STATIC_SEARCH_RESULT_FAILED) );
		break;
	}
	if (bActive)
		pItem->Invalidate();
}

void	CDlgMainTabSearch::LocalizeJasonSearchState()
{
	POSITION	pos = m_tabWnd.GetFirstTab();
	CTabItem	*pItem = NULL;
	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
				DWORD dwCounter = pItem->m_dwCustomData - 0xF0F00000;

				SSearchParams *pParams=NULL;
				BOOL bSearch = m_SearchMap.Lookup(dwCounter, pParams);

				if ( bSearch && pParams )//����״̬Ϊ�ǽ���
				{
					BOOL bActive = pItem->GetActive();//�Ƿ�Ϊactive tab

					CString strResult;
					switch (pParams->nSeachStat)
					{
					case -1:
						pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_FAILED)+pParams->strExpression );
						if (bActive)
							theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_STATIC_SEARCH_RESULT_FAILED) );
						break;
					case 0:
						//if(bActive)
						//	pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT) +pParams->strKeyword );
						if (bActive)
							theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(_T(""));
						break;
					case 1:
						pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCHING) + _T(":") +pParams->strExpression );
						if (bActive)
							theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_TABTITLE_SEARCHING) );
						break;
					case 2:
						pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_SUCCESS) +pParams->strExpression );
						strResult.Format( GetResString(IDS_STATIC_SEARCH_RESULT_SUCCESS) , pParams->uSeachResults);
						if (bActive)
							theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText(strResult);
						break;
					default://jason��������������
						pItem->SetCaption( GetResString(IDS_TABTITLE_SEARCH_RESULT_FAILED) +pParams->strExpression );
						if (bActive)
							theApp.emuledlg->searchwnd->m_pwndResults->m_StcSeachInfo.SetWindowText( GetResString(IDS_STATIC_SEARCH_RESULT_FAILED) );
						break;
					}
					pItem->Invalidate();
				}
			}
		}
	}
}

//{begin}VC-dgkang 2008��7��11��

BOOL CDlgMainTabSearch::CreateNewShareFileTab(SSearchParams *pSS)
{
	if (!pSS)
		return FALSE;

	if (m_dwTotalCount > MAX_OPENED_TABS)
	{
		MessageBox(GetResString(IDS_ERR_MAXTABS),GetResString(IDS_CAPTION),MB_ICONWARNING);
		return FALSE;
	}

	POSITION Pos = m_SearchMap.GetStartPosition();
	SSearchParams * pNextSS;
	int i = 0;
	while(Pos != NULL)
	{
		m_SearchMap.GetNextAssoc(Pos,i,pNextSS);
		if (pNextSS->dwSearchID == pSS->dwSearchID)
		{
			return FALSE;
		}
	}

	CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_SEARCH);

	DWORD	iCounter = 0;
	DWORD	CustomData = 0xF0F00000;

	CustomData	+= m_dwCounter;

	theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;

	iCounter = m_dwCounter;
	m_dwCounter++;

	m_SearchMap.SetAt(iCounter,pSS);

	pSS->strSearchTitle = (pSS->strSpecialTitle.IsEmpty() ? pSS->strExpression : pSS->strSpecialTitle);
	CString	strDisplayText = pSS->strSearchTitle;

	CTabItem_NormalCloseable	*pTi = NULL;
	pTi = new CTabItem_NormalCloseable;
	pTi->SetCaption(strDisplayText);
	pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
	pTi->SetCustomData(CustomData);
	pTi->SetDesireLength(150);

	Pos = m_tabWnd.GetActiveTab();
	m_tabWnd.AddTab(pTi,TRUE,Pos);

	m_dwTotalCount++;
	return TRUE;
}

void CDlgMainTabSearch::OpenNewSearchResult(LPCTSTR lpszCaption,ESearchType * pSearchType/* = NULL */)
{
	if (m_dwTotalCount > MAX_OPENED_TABS)
	{
		MessageBox(GetResString(IDS_ERR_MAXTABS),GetResString(IDS_CAPTION),MB_ICONWARNING);
		return;
	}

	CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_SEARCH);
	
	if( !pSearchType || *pSearchType==SearchTypeEasyMuleFile )
	{
		CreateNewSearch(lpszCaption,pSearchType);
	}
	else if ( *pSearchType == SearchTypeEd2kGlobal )
	{
		if( CGlobalVariable::serverconnect->IsConnected() )
			CreateNewSearch(lpszCaption,pSearchType);
		else
			MessageBox(GetResString(IDS_ERR_NOTCONNECTED),GetResString(IDS_CAPTION),MB_ICONWARNING);
	}
	else if( *pSearchType == SearchTypeKademlia ) 
	{			
		if( Kademlia::CKademlia::IsConnected() )
		{
			DWORD	iCounter;
			DWORD CustomData = 0xF0F00000;

			if (m_dwCounter > 0x20)
			{
				return;
			}

			SSearchParams * pSearchParams = GetParameters(lpszCaption);
			pSearchParams->eType = SearchTypeKademlia;

			CustomData          += m_dwCounter;

			theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;
			iCounter = m_dwCounter;
			m_dwCounter++;
			m_SearchMap.SetAt(iCounter, pSearchParams);

			CString	strDisplayText;
			strDisplayText = GetResString(IDS_TABTITLE_SEARCH_RESULT);
			strDisplayText += lpszCaption;


			CTabItem_NormalCloseable	*pTi = NULL;
			pTi = new CTabItem_NormalCloseable;
			pTi->SetCaption(strDisplayText);
			pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
			pTi->SetCustomData(CustomData);
			pTi->SetDesireLength(150);
			POSITION Pos = m_tabWnd.GetActiveTab();
			m_tabWnd.AddTab(pTi,TRUE,Pos);

			/* VC-dgkang 2008��7��17��
			//Kademlia ���ش��󣬵���Delete  pParamsҲ��RemoveKey��m_SearchMap�У�������ɾ����ǩҳһ��ɾ��.
			//����Ϊ���ǩҳ�����޸ĵġ�
			if (!theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pSearchParams))
				m_SearchMap.RemoveKey(iCounter);
			*/
			theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pSearchParams);
			m_dwTotalCount++;
		}
		else
		{
			MessageBox(GetResString(IDS_ERR_NOTCONNECTEDKAD),GetResString(IDS_CAPTION),MB_ICONWARNING);	
		}
	}
}

void CDlgMainTabSearch::OnNMTabCreate(NMHDR* pNMHDR, LRESULT *pResult)
{
	NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);
	DWORD dwCustomData = m_tabWnd.GetTabData(pTabOp->posTab);
	
	if(dwCustomData >= 0xF0F00000)
		UpdateEMsClosableStatus();
	*pResult = 0;
}

void CDlgMainTabSearch::OnNMTabDestroy(NMHDR* pNMHDR, LRESULT *pResult)
{
	theApp.emuledlg->searchwnd->m_pwndResults->SetPos(0);
	NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);

	DWORD CustomData = m_tabWnd.GetTabData(pTabOp->posTab);
	DWORD LowData  = CustomData & 0x0000FFFF;
	DWORD HighData = CustomData & 0xFFFF0000;

	if (HighData ^ 0x0F0FFFFF)
	{
		SSearchParams *pSearchParams=NULL;
		BOOL bSearch = m_SearchMap.Lookup(LowData, pSearchParams);

		if ( bSearch && pSearchParams)
		{
			theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.RemoveResults(pSearchParams->dwSearchID);
			m_SearchMap.RemoveKey(LowData);
		}
		delete pSearchParams;
	}
	if (CustomData >= 0xF0F00000)
		PostMessage(UM_SEARCHTAB_EM_DESTROY);

	if (m_dwTotalCount > 0)
		m_dwTotalCount--;
	*pResult = 0;
}

void CDlgMainTabSearch::OnNMActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult)
{
	NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);
	DWORD CustomData = m_tabWnd.GetTabData(pTabOp->posTab);

	DWORD LowData  = CustomData & 0x0000FFFF;
	DWORD HighData = CustomData & 0xFFFF0000;

	if (HighData ^ 0x0F0FFFFF)
	{
		SSearchParams *pSearchParams=NULL;
		BOOL bSearch = m_SearchMap.Lookup(LowData, pSearchParams);

		if ( bSearch && pSearchParams )
		{
			theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.ShowResults(pSearchParams->dwSearchID);
			theApp.emuledlg->searchwnd->m_pwndResults->UpdateParamDisplay(pSearchParams);
			theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = LowData;

			// �л���ǩʱ���������
			if (theApp.emuledlg->m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.GetSafeHwnd() != NULL)
			{
				theApp.emuledlg->m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.SwitchSearchEditInfo(0); // 0:VC Search
				SetFocus();
			}
		}
	}
	*pResult = 0;
}

void CDlgMainTabSearch::OnMainTabChanged()
{
	POSITION Pos = m_tabWnd.GetActiveTab();
	if (Pos == NULL)
		return;

	m_tabWnd.ShowActiveTab();

	DWORD CustomData = m_tabWnd.GetTabData(Pos);
	DWORD LowData  = CustomData & 0x0000FFFF;
	DWORD HighData = CustomData & 0xFFFF0000;

	if( CustomData >= 0xF0F00000 )
	{
		CTabItem* pTabItem = m_tabWnd.GetTabItem(Pos);
		
		if (pTabItem)
			pTabItem->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
	}

	if (HighData ^ 0x0F0FFFFF)
	{
		SSearchParams *pSearchParams=NULL;
		BOOL bSearch = m_SearchMap.Lookup(LowData, pSearchParams);

		if ( bSearch && pSearchParams )
		{
			theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.ShowResults(pSearchParams->dwSearchID);
			theApp.emuledlg->searchwnd->m_pwndResults->UpdateParamDisplay(pSearchParams);
			theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = LowData;

			theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.SetFocus();
		}
	}
}

void CDlgMainTabSearch::UpdateEMsClosableStatus()
{
	int     iEMCount = 0;

	CTabItem_NormalCloseable * pEMItem  = NULL;
	POSITION	pos =m_tabWnd.GetFirstTab();
	CTabItem	*pItem = NULL;
	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
				pEMItem = (CTabItem_NormalCloseable *)pItem;
				pEMItem->EnableClose(TRUE);
				iEMCount ++;
			}
		}
	}
	if (1 == iEMCount && NULL != pEMItem && 
#ifdef EMULE_SEARCH
		pEMItem->GetCaption() == GetResString(IDS_SEARCHEMULE))
#else
		pEMItem->GetCaption() == GetResString(IDS_SEARCHVERYCD))
#endif		
	{
		pEMItem->EnableClose(FALSE);
	}
	if ( 0 == iEMCount) ShowEmuleSearch();
}


BOOL CDlgMainTabSearch::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;

		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
			return FALSE;
	}
	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CDlgMainTabSearch::Localize()
{
	const CString tcsEN = _T("easyMule Search"),tcsTW = _T("������H�W·"), tcsCN = _T("������¿����");
	const CString szEN = _T("Search:"),szTW = _T("����:"), szCN = _T("����:");

	CString tcs;
	CTabItem_NormalCloseable * pEMItem  = NULL;
	int nLen = -1;

	POSITION	pos = m_tabWnd.GetFirstTab();
	CTabItem	*pItem = NULL;
	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
				pEMItem = (CTabItem_NormalCloseable *)pItem;
				CString tcs = pEMItem->GetCaption();
				if (tcs == tcsEN || tcs == tcsTW || tcs == tcsCN)
#ifdef EMULE_SEARCH
					pEMItem->SetCaption(GetResString(IDS_SEARCHEMULE));	
#else
					pEMItem->SetCaption(GetResString(IDS_SEARCHVERYCD));
#endif
				else
				{
					if (tcs.Find(szEN) == 0) nLen = szEN.GetLength();
					else if(tcs.Find(szTW) == 0) nLen = szTW.GetLength();
					else if(tcs.Find(szCN) == 0) nLen = szCN.GetLength();
					else nLen = -1;
					if (nLen != -1)
					{
						tcs.Replace(tcs.Left(nLen),GetResString(IDS_TABTITLE_SEARCH_RESULT));
						pEMItem->SetCaption(tcs);
					}
				}
			}
		}
	}
}

void CDlgMainTabSearch::UpdateSearchParam(int iIndex, SSearchParams *pSearchParams)
{
	SSearchParams	*psp;
	if (m_SearchMap.Lookup(iIndex, psp))
	{
		delete psp;
		m_SearchMap.SetAt(iIndex, pSearchParams);
	}
}

LRESULT CDlgMainTabSearch::OnEMTabDestroy(WPARAM /*wParam */, LPARAM /*lParam */)
{
	UpdateEMsClosableStatus();
	return 0;
}
LRESULT CDlgMainTabSearch::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( WM_INPUTLANGCHANGEREQUEST == message )
	{
		if(GetKeyboardLayout(0) == (HKL)lParam)
			return 0;
		UINT count = GetKeyboardLayoutList(0,NULL);
		if(count == 0) return 0;
		HKL* lpList = new HKL[count];

		count = GetKeyboardLayoutList(count,lpList);
		BOOL bFound = FALSE;
		for(int i=0;i<count;i++)
		{
			if((HKL)lParam == lpList[i])
			{
				bFound = TRUE;
				break;
			}
		}
		if(lpList)
		{
			delete[] lpList;
			lpList = NULL;
		}
		if(!bFound)
		{
			CString sID;
			sID.Format( L"%08x",lParam );
			LoadKeyboardLayout(sID,KLF_ACTIVATE);
		}
		else
		{
			ActivateKeyboardLayout((HKL)lParam,KLF_REORDER);
		}

		return 0;
	}

	return CResizableDialog::WindowProc(message, wParam, lParam);
}
