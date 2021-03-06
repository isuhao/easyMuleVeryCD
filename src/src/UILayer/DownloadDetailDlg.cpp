/*
 * $Id: DownloadDetailDlg.cpp 19524 2010-05-20 10:09:21Z dgkang $
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
// DownloadDetailDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "DownloadDetailDlg.h"


// CDownloadDetailDlg 对话框

IMPLEMENT_DYNAMIC(CDownloadDetailDlg, CDialog)
CDownloadDetailDlg::CDownloadDetailDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadDetailDlg::IDD, pParent)
{
}

CDownloadDetailDlg::~CDownloadDetailDlg()
{
}

void CDownloadDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDownloadDetailDlg, CDialog)
END_MESSAGE_MAP()


// CDownloadDetailDlg 消息处理程序


void CDownloadDetailDlg::SetPartFileInfo(CKnownFile	*file)
{
	if(! file) return ;
	CPartFile	*pPartFile = NULL;
	if ( file->IsKindOf(RUNTIME_CLASS(CPartFile)) )
		pPartFile = (CPartFile*) file;



	SetDlgItemText(IDC_STATIC_FILENAME, file->GetFilePath());
	

	CString strTmp;
	time_t restTime;
	if ( NULL != pPartFile )
	{
		if (!thePrefs.UseSimpleTimeRemainingComputation())
			restTime = pPartFile->getTimeRemaining();
		else
			restTime = pPartFile->getTimeRemainingSimple();

		strTmp.Format(_T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes((pPartFile->GetFileSize() - pPartFile->GetCompletedSize()), false, false));
		SetDlgItemText(IDC_STATIC_LAST_SEEN_TIME, strTmp);
	}

}

LRESULT CDownloadDetailDlg::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
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

	return __super::WindowProc(message, wParam, lParam);
}