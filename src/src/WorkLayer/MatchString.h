/* this file is part of easyMule
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

#include "getpinyin.h"
#include <string>
#include <vector>
#include <map>

typedef std::vector<CString> vec_cstr;
typedef std::vector<std::string> vec_str;

class StringMatcher {
public:
	StringMatcher() : m_isOriginal(true) {}
	~StringMatcher() {}

	typedef std::vector<void*> ListItemsNeedShow;

	void AddItem(void* toadd, const CString& name);
	void RemoveItem(void* toremove);
	void RenameItem(void* torename, const CString& name);
	void RemoveAllItems();
	const ListItemsNeedShow& GetMatchResult(CString*);
	CString GetItemName(void* toget);

	// �Ƿ�Ϊԭʼ�б�
	bool IsOriginal() const { return m_isOriginal; }
	
	// ������Ŀ�����Ƿ�����ؼ���
	bool GetOneItemMatchResult(CString& strItemName);

private:
	bool MatchString(CString& strMatchDestination, vec_cstr& vecMatchDestinationPinyin, vec_cstr& vec_strSearchKey);
	void GetPinyin(const CString& strSource, vec_cstr& vecPinyinContainer);
	void SpiltString(const CString& strInput, vec_cstr& vec_strOutput);

private:
	typedef std::map<void*, CString> ListItemNamesOrig;
	typedef std::map<void*, std::vector<CString> > ListItemNamesPinyin;
	ListItemNamesOrig m_ListItemNames; // ԭʼ����ӳ��
	ListItemNamesPinyin m_ListItemPinyin; // ƴ���ַ���ӳ��
	ListItemsNeedShow m_ListItemsNeedShow; // ƥ��ͨ������Ŀ�б�
	ListItemsNeedShow m_ListItemsNeedShowOrig; // ԭʼ��Ŀ�б�
	bool m_isOriginal;
	CString m_strSearchKey;
};