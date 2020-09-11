// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include <regex>
#include <string>
#include <vector>
#include <unordered_map>
class StringConvertUtils {
#if !defined(_UNICODE) && !defined(UNICODE)
#define tstring std::string
#else
#define tstring std::wstring
#endif
public:
	//  ANSI to Unicode
	std::wstring AToW(const std::string& str)
	{
		int len = 0;
		len = str.length();
		int unicodeLen = ::MultiByteToWideChar(CP_ACP,
			0,
			str.c_str(),
			-1,
			NULL,
			0);
		wchar_t* pUnicode;
		pUnicode = new  wchar_t[(unicodeLen + 1)];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_ACP,
			0,
			str.c_str(),
			-1,
			(LPWSTR)pUnicode,
			unicodeLen);
		std::wstring rt;
		rt = (wchar_t*)pUnicode;
		delete pUnicode;
		return rt;
	}
	//Unicode to ANSI
	std::string WToA(const std::wstring& str)
	{
		char* pElementText;
		int iTextLen;
		iTextLen = WideCharToMultiByte(CP_ACP,
			0,
			str.c_str(),
			-1,
			NULL,
			0,
			NULL,
			NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
		::WideCharToMultiByte(CP_ACP,
			0,
			str.c_str(),
			-1,
			pElementText,
			iTextLen,
			NULL,
			NULL);
		std::string strText;
		strText = pElementText;
		delete[] pElementText;
		return strText;
	}
	//UTF-8 to Unicode
	std::wstring UTF8ToW(const std::string& str)
	{
		int len = 0;
		len = str.length();
		int unicodeLen = ::MultiByteToWideChar(CP_UTF8,
			0,
			str.c_str(),
			-1,
			NULL,
			0);
		wchar_t* pUnicode;
		pUnicode = new wchar_t[unicodeLen + 1];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_UTF8,
			0,
			str.c_str(),
			-1,
			(LPWSTR)pUnicode,
			unicodeLen);
		std::wstring rt;
		rt = (wchar_t*)pUnicode;
		delete pUnicode;
		return rt;
	}
	//Unicode to UTF-8
	std::string WToUTF8(const std::wstring& str)
	{
		char* pElementText;
		int iTextLen;
		iTextLen = WideCharToMultiByte(CP_UTF8,
			0,
			str.c_str(),
			-1,
			NULL,
			0,
			NULL,
			NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
		::WideCharToMultiByte(CP_UTF8,
			0,
			str.c_str(),
			-1,
			pElementText,
			iTextLen,
			NULL,
			NULL);
		std::string strText;
		strText = pElementText;
		delete[] pElementText;
		return strText;
	}
	std::string TToA(tstring tsT)
	{
		std::string str = "";
#if !defined(UNICODE) && !defined(_UNICODE)
		str = tsT;
#else
		str = WToA(tsT);
#endif
		return str;
	}
	std::wstring TToW(tstring tsT)
	{
		std::wstring wstr = L"";
#if !defined(UNICODE) && !defined(_UNICODE)
		wstr = AToW(tsT);
#else
		wstr = tsT;
#endif
		return wstr;
	}
	tstring AToT(std::string str)
	{
		tstring ts = TEXT("");
#if !defined(UNICODE) && !defined(_UNICODE)
		ts = str;
#else
		ts = AToW(str);
#endif
		return ts;
	}
	tstring WToT(std::wstring wstr)
	{
		tstring ts = TEXT("");
#if !defined(UNICODE) && !defined(_UNICODE)
		ts = WToA(wstr);
#else
		ts = wstr;
#endif
		return ts;
	}
public:
	__inline static bool string_regex_valid(const std::string& data, const std::string& pattern)
	{
		return std::regex_match(data, std::regex(pattern));
	}
	__inline static size_t string_regex_replace_all(std::string& result, std::string& data, const std::string& replace, const std::string& pattern, std::regex_constants::match_flag_type matchflagtype = std::regex_constants::match_default)
	{
		try
		{
			data = std::regex_replace(data, std::regex(pattern), replace, matchflagtype);
		}
		catch (const std::exception& e)
		{
			result = e.what();
		}
		return data.length();
	}
	__inline static size_t string_regex_find(std::string& result, std::vector<std::vector<std::string>>& svv, std::string& data, const std::string& pattern)
	{
		std::smatch smatch;
		std::string::const_iterator ite = data.end();
		std::string::const_iterator itb = data.begin();
		try
		{
			//如果匹配成功
			while (std::regex_search(itb, ite, smatch, std::regex(pattern)))
			{
				bool empty = true;
				for (auto it : smatch)
				{
					if (empty)
					{
						empty = false;
						svv.push_back({});
					}
					else
					{
						svv.rbegin()->emplace_back(std::string(it.first, it.second));
					}
				}
				if (!empty)
				{
					itb = smatch.suffix().first;
				}
			}
		}
		catch (const std::exception& e)
		{
			result = e.what();
		}
		return svv.size();
	}
	__inline static std::string::size_type string_reader(std::string& result, const std::string& strData,
		const std::string& strStart, const std::string& strFinal, std::string::size_type stPos = 0,
		bool bTakeStart = false, bool bTakeFinal = false)
	{
		std::string::size_type stRetPos = std::string::npos;
		std::string::size_type stStartPos = stPos;
		std::string::size_type stFinalPos = std::string::npos;
		stStartPos = strData.find(strStart, stStartPos);
		if (stStartPos != std::string::npos)
		{
			stRetPos = stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
			if (stFinalPos != std::string::npos)
			{
				if (!bTakeStart)
				{
					stStartPos += strStart.length();
				}
				if (bTakeFinal)
				{
					stFinalPos += strFinal.length();
				}
				result = strData.substr(stStartPos, stFinalPos - stStartPos);
			}
		}
		return stRetPos;
	}
	__inline static std::string string_reader(const std::string& strData,
		const std::string& strStart, const std::string& strFinal,
		bool bTakeStart = false, bool bTakeFinal = false)
	{
		std::string strRet = ("");
		std::string::size_type stStartPos = std::string::npos;
		std::string::size_type stFinalPos = std::string::npos;
		stStartPos = strData.find(strStart);
		if (stStartPos != std::string::npos)
		{
			stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
			if (stFinalPos != std::string::npos)
			{
				if (!bTakeStart)
				{
					stStartPos += strStart.length();
				}
				if (bTakeFinal)
				{
					stFinalPos += strFinal.length();
				}
				strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
			}
		}
		return strRet;
	}
	__inline static std::string string_replace_all(std::string& strData, const std::string& strDst, const std::string& strSrc, std::string::size_type stPos = 0)
	{
		while ((stPos = strData.find(strSrc, stPos)) != std::string::npos)
		{
			strData.replace(stPos, strSrc.length(), strDst);
			stPos += strDst.length();
		}
		return strData;
	}
	__inline static size_t string_split_to_vector(std::vector<std::string>& sv, const std::string& strData, const std::string& strSplitter, std::string::size_type stPos = 0)
	{
		std::string strTmp = ("");
		std::string::size_type stIdx = 0;
		std::string::size_type stSize = strData.length();
		while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
		{
			strTmp = strData.substr(stIdx, stPos - stIdx);
			if (!strTmp.length())
			{
				strTmp = ("");
			}
			sv.push_back(strTmp);
			stIdx = stPos + strSplitter.length();
		}
		if (stIdx < stSize)
		{
			strTmp = strData.substr(stIdx, stSize - stIdx);
			if (!strTmp.length())
			{
				strTmp = ("");
			}
			sv.push_back(strTmp);
		}
		return sv.size();
	}
	/*__inline static
		int enum_file(std::unordered_map<std::string, std::string>& file_list, const std::string& root)
	{
		std::error_code ec;
		for (auto& item : std::filesystem::recursive_directory_iterator(root, std::filesystem::directory_options::skip_permission_denied | std::filesystem::directory_options::follow_directory_symlink, ec))
		{
			try
			{
				if (item.is_regular_file())
				{
					std::string _p = item.path().string();
					file_list.insert(std::unordered_map<std::string, std::string>::value_type(_p, _p));
				}
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
		return 0;
	}*/
public:
	static StringConvertUtils* Instance()
	{
		static StringConvertUtils scuInstance;
		return &scuInstance;
	}
};
