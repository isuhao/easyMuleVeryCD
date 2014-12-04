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

#include "stdafx.h"
#include "MatchString.h"


bool StringMatcher::MatchString(CString& strMatchDestination, vec_cstr& vecMatchDestinationPinyin, vec_cstr& vec_strSearchKey)
{
	vec_cstr::size_type total_match_count = 0;
	vec_cstr::size_type total_search_key = vec_strSearchKey.size();

	// ƥ��ÿ���ؼ���(key string)
	for (vec_cstr::iterator ix_key = vec_strSearchKey.begin(); ix_key != vec_strSearchKey.end(); ++ix_key)
	{
		CString& string_one_search_key = *ix_key;

		// ԭʼ�ַ���ƥ��
		if (strMatchDestination.MakeLower().Find(string_one_search_key.MakeLower()) != -1)
		{
			total_match_count++;
			continue;
		}

		int match_count = 0; // ��¼��ǰ�ؼ����б��ɹ�ƥ����ַ���
		int match_count_temp = 0; // ��ʱ����
		int last_match_index = -1; // ��һ��ƥ�䵽�ؼ����ַ���ƴ������
		int current_index = -1; // ��ǰ����ƥ���ƴ������
		const int search_key_str_count = string_one_search_key.GetLength(); // ��ǰ�ؼ��ʵ��ַ���

		// ԭʼ�ַ�����û���ҵ��ؼ���,����ƴ������ƥ��
		for (vec_cstr::iterator ix_pinyin = vecMatchDestinationPinyin.begin(); ix_pinyin != vecMatchDestinationPinyin.end(); ++ix_pinyin)
		{
			current_index++;

			CString& string_one_pinyin = *ix_pinyin; // һ���ֵ�ƴ��(�����ж��ֶ���)

			vec_cstr vec_one_pinyin_spilt;
			SpiltString(string_one_pinyin, vec_one_pinyin_spilt); // ���õ���ƴ����������(Ĭ�Ͻ��������Ƕ�����)

			// �������ֵ�ÿ����������ƥ��
			for (vec_cstr::iterator ix_pinyin_spilt = vec_one_pinyin_spilt.begin(); ix_pinyin_spilt != vec_one_pinyin_spilt.end(); ++ix_pinyin_spilt)
			{
				CString& string_temp = *ix_pinyin_spilt; // һ������
				int pinyin_index = 0;

				// ��ǰ����������ĸ��ؼ��ʵĵ�һ����ƥ���ַ��Ƚ�
				if (string_temp[0] != string_one_search_key[match_count])
				{
					if (string_temp[0] != string_one_search_key[0])
						continue;

					// �ؼ�����Ҫ����ƥ��:
					// eg:����->������
					// ƴ��->xia,hang_heng_xing,xian
					// �ؼ���->xingxian
					// ƥ�䵽"xing"ʱ���ؼ��ʳɹ�ƥ���ַ�������(����ƥ��)
					if (!match_count_temp)
						match_count = 0;
				}

				// ��ǰ����(���±�0��ʼ)��ؼ���(�ӵ�һ����ƥ���ַ���ʼ)����ַ��Ƚ�
				while (string_temp[pinyin_index] != '\0' && 
					string_temp[pinyin_index] == string_one_search_key[match_count])
				{
					match_count++;
					pinyin_index++;
				}

				last_match_index = current_index;

				// ��ȫƥ��
				if (pinyin_index == string_temp.GetLength() || match_count == search_key_str_count)
				{
					match_count_temp = 0;
					break;
				}

				// �ؼ����е��ַ���ƥ�䵽ʱ,����ʱ������¼
				if (pinyin_index > 0)
					match_count_temp = pinyin_index;

				// �ָ��ؼ��ʴ�ƥ���ַ�������
				match_count -= pinyin_index;
			}

			// ����ʵ��ƥ�䵽���ٸ��ַ�,��Ĭ��Ϊֻƥ�䵽����ĸ
			if (match_count_temp > 0)
			{
				match_count++;
				match_count_temp = 0;
			}

			// ��ȫƥ��ɹ�
			if (current_index == last_match_index && match_count == search_key_str_count)
			{
				total_match_count++;
				break;
			}

			// ����ǰƴ��û��ƥ�䵽,����һ��ƥ��ӹؼ��ʵĵ�һ���ַ���ʼƥ��
			if (current_index != last_match_index)
				match_count = 0;
		}

		if (total_match_count == total_search_key)
			return true;
	}

	return (total_match_count == total_search_key);
}

// Give a string and a pinyin string container, it will add the new pinyin string
// to the container.
void StringMatcher::GetPinyin(const CString& strSource, vec_cstr& vecPinyinContainer)
{
	int character_count = strSource.GetLength();

	wchar_t one_character;
	unsigned long ul_character;
	char utf8char[8] = {0};

	for (int ix = 0; ix != character_count; ++ix)
	{
		one_character = strSource[ix];
		ul_character = (unsigned long)one_character;

		// if not chinese
		if (ul_character < 0x4E00 || ul_character > 0x9fA5)
			continue;

		WideCharToMultiByte( CP_UTF8, 0, &one_character, -1, utf8char, 8, NULL, NULL );
		CString string_temp( get_pinyin( utf8char ) );

		//strPinyinContainer += string_temp;
		vecPinyinContainer.push_back(string_temp);
	}
} // END GetPinyin

void StringMatcher::SpiltString(const CString& strInput, vec_cstr& vec_strOutput)
{
	int nSpaceIndex = strInput.Find(L" ");

	if (nSpaceIndex == -1)
	{
		vec_strOutput.push_back(strInput);
		return;
	}

	CString strOneSearchKey = strInput.Mid(0, nSpaceIndex);
	CString strSubString = strInput.Mid(nSpaceIndex + 1);

	vec_strOutput.push_back(strOneSearchKey);

	while ((nSpaceIndex = strSubString.Find(L" ") ) != -1)
	{
		strOneSearchKey = strSubString.Mid(0, nSpaceIndex);
		vec_strOutput.push_back(strOneSearchKey);

		strSubString = strSubString.Mid(nSpaceIndex + 1);
	}

	if (!strSubString.IsEmpty())
		vec_strOutput.push_back(strSubString);
}

void StringMatcher::AddItem(void* toadd, const CString& name)
{
	m_ListItemNames.insert(std::make_pair(toadd, name));
	std::vector<CString> myVec;
	GetPinyin(name, myVec);
	m_ListItemPinyin.insert(std::make_pair(toadd, myVec));
	m_ListItemsNeedShowOrig.push_back(toadd);
}

void StringMatcher::RemoveItem(void* toremove)
{
	m_ListItemNames.erase(toremove);
	m_ListItemPinyin.erase(toremove);
	
	for (ListItemsNeedShow::iterator ix = m_ListItemsNeedShowOrig.begin(); ix != m_ListItemsNeedShowOrig.end(); ++ix)
	{
		if (*ix == toremove)
		{
			m_ListItemsNeedShowOrig.erase(ix);
			break;
		}
	}
}

const StringMatcher::ListItemsNeedShow& StringMatcher::GetMatchResult(CString* pSearchKey)
{
	if (!m_ListItemsNeedShow.empty())
		m_ListItemsNeedShow.clear();

	if ( m_ListItemNames.size() <= 0 )
		return m_ListItemsNeedShow;

	if ( pSearchKey == NULL || *pSearchKey == _T(""))
	{
		m_isOriginal = true;
		return m_ListItemsNeedShowOrig;
	}

	std::vector<CString> vecSearchKeys;
	m_strSearchKey = *pSearchKey;

	SpiltString(m_strSearchKey, vecSearchKeys);

	for (ListItemNamesOrig::iterator ix_ListItems = m_ListItemNames.begin(); ix_ListItems != m_ListItemNames.end(); ++ix_ListItems)
	{
		void* listKey = ix_ListItems->first;
		CString& strItemName = ix_ListItems->second;
		std::vector<CString>& vecStrPinyin = m_ListItemPinyin[listKey];

		if (MatchString(strItemName, vecStrPinyin, vecSearchKeys))
			m_ListItemsNeedShow.push_back(listKey);
	}

	m_isOriginal = false;

	return m_ListItemsNeedShow;
}

void StringMatcher::RemoveAllItems()
{
	m_ListItemNames.clear();
	m_ListItemPinyin.clear();
	m_ListItemsNeedShow.clear();
	m_ListItemsNeedShowOrig.clear();
}

bool StringMatcher::GetOneItemMatchResult(CString& strItemName)
{
	std::vector<CString> vecPinyin;
	GetPinyin(strItemName, vecPinyin);

	if (m_strSearchKey.IsEmpty())
		return true;

	std::vector<CString> vecSearchKeys;
	SpiltString(m_strSearchKey, vecSearchKeys);

	return MatchString(strItemName, vecPinyin, vecSearchKeys);
}

CString StringMatcher::GetItemName(void* toget)
{
	ListItemNamesOrig::iterator it = m_ListItemNames.find(toget);
	if (it != m_ListItemNames.end())
		return it->second;
	else
		return _T("");
}

void StringMatcher::RenameItem(void* torename, const CString& name)
{
	ListItemNamesOrig::iterator it = m_ListItemNames.find(torename);
	if (it == m_ListItemNames.end())
		return;

	m_ListItemNames[torename] = name;
	std::vector<CString>& myVec = m_ListItemPinyin[torename];
	myVec.clear();
	GetPinyin(name, myVec);
}