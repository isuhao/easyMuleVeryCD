/*
 * $Id: UpdateInfo.cpp 20689 2010-10-20 04:12:33Z dgkang $
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
// UpdateInfo.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "UpdateInfo.h"
#include "SharedFilesCtrl.h"
#include "SharedFilesWnd.h"


// CUpdateInfo

CUpdateInfo::CUpdateInfo()
{
	m_bUpdateFlag = FALSE;

}

CUpdateInfo::~CUpdateInfo()
{
}


// CUpdateInfo ��Ա����

void CUpdateInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar<<m_Hash;
	}
	else
	{
		ar>>m_Hash;
	}
}

// CUpdateInfo ��Ա����

void CUpdateInfo::Serialize1(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar<<m_Hash<<m_bUpdateFlag;
	}
	else
	{
		ar>>m_Hash>>m_bUpdateFlag;
	}
}

BOOL CUpdateInfo::OpenUpdateInfo(void)
{
	CFile file;

	if (file.Open(GetUpdateInfoFile(), CFile::modeRead))
	{
		int nLen = file.GetLength();
		try
		{
			CArchive ar(&file, CArchive::load);

			/*
			*�����ϰ汾��update.dat�ļ�
			*added by zhuhui 2010-5-13
			*begin
			*/
			switch(nLen)
			{
			case 68:
				Serialize(ar);
				break;
			default:
				Serialize1(ar);
			}
			//end �����ϰ汾��update.dat�ļ�
			
		}
		catch (CException* e)
		{
			e->Delete();
			ASSERT(0);
		}

		file.Close();
		return TRUE;
	}
	
	return FALSE;
}

void CUpdateInfo::SaveUpdateInfo(void)
{
	CFile file;
	
	if (file.Open(GetUpdateInfoFile(), CFile::modeCreate | CFile::modeReadWrite ))
	{
		try
		{
			CArchive ar(&file, CArchive::store);
			Serialize1(ar);
		}
		catch (CException* e)
		{
			e->Delete();
			ASSERT(0);
		}
	}	

	file.Close();
}

void CUpdateInfo::SetUpdateHash(CString hash)
{
	m_Hash = hash;
	SaveUpdateInfo();
}

CString CUpdateInfo::GetUpdateHash(void)
{
	if(OpenUpdateInfo())
	{
		return m_Hash;
	}

	return NULL;
}

CString CUpdateInfo::GetUpdateInfoFile(void)
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("update.dat");
}


BOOL CUpdateInfo::isUpdateFile(CString hash)
{
	CString oldhash = GetUpdateHash();

	if(oldhash.IsEmpty())
	{
		return FALSE;
	}

	if(oldhash.Compare(hash) == 0)
	{
		return TRUE;
	}

	return FALSE;
}


int CUpdateInfo::GetUpdateState(const uchar* hash)
{
	//�����ļ��б�
	const CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(hash);

	if(file)
	{
		if (file->IsPartFile())
		{
			return UPDATE_DOWNLOADING;	//�ڹ����б��У�����û��������ɡ�
		}
		else
		{
			return UPDATE_DOWNLOADED;	//�ڹ����б����У��Ѿ������ꡣ
		}
	}
	else
	{	
		//�����б�
		if ((file = CGlobalVariable::downloadqueue->GetFileByID(hash)) != NULL)
		{
			return UPDATE_DOWNLOADING;	//�������б��У��϶�û���������
		}
		else
		{
			return UPDATE_NODOWNLOAD;	//��δ����
		}
	}
}

BOOL CUpdateInfo::UpdateInfoFileExists(void)
{
	return PathFileExists(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("update.dat"));
}

int CUpdateInfo::Compare(CString hash)
{
	CString oldhash = GetUpdateHash();
	return oldhash.Compare(hash);
}

bool CUpdateInfo::IsUpdateTask(const CString& strTaskPath)
{//�ж��Ƿ�Ϊ��������
	int n = strTaskPath.ReverseFind('\\');
	CString strPath = strTaskPath.Mid(0, n);
	CString strName = strTaskPath.Mid(n+1, strTaskPath.GetLength());

	//����·��==����Ŀ¼ && �ļ�������"Update" �ж�Ϊ��������
	if( strPath.Compare(thePrefs.GetMuleDirectory(EMULE_UPDATEDIR)) == 0 && strName.Find(_T("Update")) >= 0)
		return true;

	return false;
}

BOOL CUpdateInfo::ClearUpdateHistory(const uchar * hashkey)//�����ʷ��������
{
	for (int i = 0; i < CGlobalVariable::sharedfiles->GetCount(); i++)
	{
		CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByIndex(i);
		CPartFile* pPartFile;

		try
		{
			//�����б����Ƿ���
			if(file)
			{
				if ( IsUpdateTask( file->GetFilePath() ) && file->GetFileHash() != hashkey )
				{
					if(file->IsPartFile())
					{
						//�����б��У���δ�����꣬�Ƴ�
						pPartFile = DYNAMIC_DOWNCAST(CPartFile,file);
						if( pPartFile )
						{
							pPartFile->DeleteFile();
						}
					}
					else
					{
						//�����б��У��Ѿ��������δ��װ���Ƴ�
						DeleteFile(file->GetFilePath());
						theApp.emuledlg->sharedfileswnd->sharedfilesctrl.RemoveFile(file);
						CGlobalVariable::sharedfiles->RemoveFile(file);

						CGlobalVariable::filemgr.RemoveFileItem(file);
					}
				}
				else
					continue;
			}
		}
		catch (CException* e)
		{
			e->Delete();
			//return FALSE;
		}
	}

	for (int i = 0; i < CGlobalVariable::downloadqueue->GetFileCount(); i++)
	{
		CPartFile* pFile = CGlobalVariable::downloadqueue->GetFileByIndex(i);
		try
		{
			if (pFile)
			{
				if ( IsUpdateTask( pFile->GetFilePath() ) && pFile->GetFileHash() != hashkey )
				{
					//�����б�û�У���δ�����꣬�Ƴ�
					pFile->DeleteFile();
				}
			}
		}
		catch (CException* e)
		{
			e->Delete();
			//return FALSE;
		}
	}

	return TRUE;
}

BOOL CUpdateInfo::DeleteUpdate(CString hash)
{
	uchar UpdateHash[16];
	CPartFile* pPartFile;

	if(!strmd4(hash,UpdateHash))
	{
		return FALSE;
	}

	CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(UpdateHash);

	try
	{
		//�����б����Ƿ���
		if(file)
		{
			if(file->IsPartFile())
			{
				//�����б��У���δ�����꣬�Ƴ�
				pPartFile = DYNAMIC_DOWNCAST(CPartFile,file);
				if( pPartFile )
				{
					pPartFile->DeleteFile();
				}
			}
			else
			{
				//�����б��У��Ѿ��������δ��װ���Ƴ�
				DeleteFile(file->GetFilePath());
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.RemoveFile(file);
				CGlobalVariable::sharedfiles->RemoveFile(file);
			}
		}

		if ((pPartFile = CGlobalVariable::downloadqueue->GetFileByID(UpdateHash)) != NULL)
		{
			//�����б�û�У���δ�����꣬�Ƴ�
			pPartFile->DeleteFile();
		}
	}
	catch (CException* e)
	{
		e->Delete();
		return FALSE;
	}

	return FALSE;
}

/*
*  ����ʱ���ݴ˱�־λ�ж��Ƿ��������°�
*  added by zhuhui 2010-5-12
*/
void CUpdateInfo::SetUpdateFlag(BOOL bUpdateFlag)
{
	m_bUpdateFlag = bUpdateFlag;
	SaveUpdateInfo();
}

BOOL CUpdateInfo::GetUpdateFlag(void)
{
	if(OpenUpdateInfo())
	{
		return m_bUpdateFlag;
	}
	return 0;
}
