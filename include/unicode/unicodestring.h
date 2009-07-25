#ifndef _unicodestring_
#define _unicodestring_

/*
http://gcc.gnu.org/ml/libstdc++/2000-07/msg00001.html
*/

#include <string>

class UnicodeString
{
public:
	UnicodeString();
	UnicodeString(const std::string &utf8string);
	UnicodeString(const std::wstring &widestring);
	
	const std::string NarrowString() const;
	const std::string &NarrowString();
	const std::wstring WideString() const;
	const std::wstring &WideString();
	
	const size_t CharacterCount() const;
	const size_t CharacterCount();

	void Trim(const size_t charpos);
	
	UnicodeString &operator=(const std::string &utf8string);
	UnicodeString &operator=(const std::wstring &widestring);
	UnicodeString &operator+=(const std::string &utf8string);
	UnicodeString &operator+=(const std::wstring &widestring);

private:
	void Widen();		// UTF-8 to UTF-16/32
	void Narrowen();	// UTF-16/32 to UTF-8
	void CheckAndReplaceInvalid();
	
	enum Flag
	{
		FLAG_WIDE_OK=1,
		FLAG_NARROW_OK=2
	};
	int m_flags;
	std::wstring m_widestring;		// UTF-16 or 32 depending on size of wchar_t
	std::string m_narrowstring;		// UTF-8

};

#endif	// _unicodestring_
