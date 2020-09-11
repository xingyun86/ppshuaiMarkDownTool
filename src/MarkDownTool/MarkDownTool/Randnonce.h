// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CRandomString {
public:
	CRandomString() { srand(time(nullptr)); }
	virtual ~CRandomString() {}
private:
	char _digit()
	{
		return static_cast<char>('0' + rand() % ('9' - '0' + 1));
	}
	char _lower()
	{
		return static_cast<char>('a' + rand() % ('z' - 'a' + 1));
	}
	char _upper()
	{
		return static_cast<char>('A' + rand() % ('Z' - 'A' + 1));
	}
public:
	std::string make_random_string(int length) {
		std::string str = ("");
		while (length-- != 0)
		{
			switch (rand() % 3)
			{
			case 0:str.push_back(_digit()); break;
			case 1:str.push_back(_lower()); break;
			default:str.push_back(_upper()); break;
			}
		}
		return (str);
	}
	std::string make_random_long(long min, long max) {
		return std::to_string(min + rand() % (max - min + 1));
	}
public:
	SINGLETON_INSTANCE(CRandomString)
};