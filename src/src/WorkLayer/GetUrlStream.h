/*
 * $Id: GetUrlStream.h
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

#include <afxstr.h>
#include <string>

 /**
  * ͨ��url��http��ftp����������
  */

class CGetUrlStream
{

public:
	CGetUrlStream(void);
	virtual ~CGetUrlStream(void);

public:
	// ��ָ��url����
	bool			OpenUrl( const CString & strUrl );

	// ���ض�ȡ������
	const char *	GetStream() const
	{ 
		return m_strStream.c_str();
	}

	// ���ض�ȡ�����ݴ�С
	size_t			GetSize() const			
	{ 
		return m_strStream.size();
	}

	// ���ش������
	unsigned int	GetLastError() const	
	{ 
		return m_uLastError;
	}

private:
	void			SetLastError();

private:
	std::string		m_strStream;		// �����ȡ������
	unsigned int	m_uLastError;		// ���һ�η��ʴ������
};
