#include "../include/stringfunctions.h"
#include <Poco/UTF8String.h>

#include <algorithm>
#include <limits>

#ifdef XMEM
	#include <xmem.h>
#endif

namespace StringFunctions
{

int utf8safetolower(int ch)
{
	return (ch>64 && ch<91) ? ch+32 : ch;
}

void LowerCase(const std::string &str, std::string &output)
{
	//output=str;
	//std::transform(str.begin(),str.end(),output.begin(),utf8safetolower);
	output=Poco::UTF8::toLower(str);
}

#ifdef _DEBUG
/* This function is actually faster doing replaces in debug mode */
std::string Replace(const std::string &input, const std::string &find, const std::string &replace)
{
	std::string returnstr=input;
	std::string::size_type pos=returnstr.find(find);

	while(pos!=std::string::npos)
	{
		returnstr.replace(pos,find.size(),replace);
		pos=returnstr.find(find,pos+replace.size());
	}

	return returnstr;

}
#else
/* http://www.allquests.com/question/2561981/c-std-string-find-and-replace-all.html */
/*
template <class T, int n>
class MeteredAllocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	pointer address(reference value) const
	{
		return &value;
	}

	const_pointer address(const_reference value) const
	{
		return &value;
	}

	template <class U>
	struct rebind {
		typedef MeteredAllocator<U, -1> other;
	};

	size_type max_size() const 
	{
		return std::numeric_limits<size_t>::max() / sizeof(T);
	}

	pointer allocate(size_type num, std::allocator<void>::const_pointer /* hint *//* = 0)
	{
		m_nHits++;
		m_nVolume += num * sizeof(T);

		return (pointer) ::operator new(num*sizeof(T));
	}

	void deallocate(pointer p, size_type num)
	{
		delete( (value_type*) p );
	}

	void construct(pointer p, const T& value)
	{
		new((value_type*)p) T(value);
	}

	MeteredAllocator(): m_n(n), m_nHits(0), m_nVolume(0)
	{
	}

	template <class U, int o>
	MeteredAllocator(const MeteredAllocator<U,o> &ma): m_n(ma.m_n),m_nVolume(ma.m_nVolume),m_nHits(ma.m_nHits)
	{
	}

	~MeteredAllocator()
	{
	}

	template <class U, int o>
	const bool operator==(const MeteredAllocator<U,o> &ma)
	{
		return (m_n==ma.m_n && m_nVolume==ma.m_nVolume && m_nHits==ma.m_nHits);
	}

public:
	int m_n;
	size_t m_nVolume;
	size_t m_nHits;
};
*/


template <class T, int n>
class MeteredAllocator:public std::allocator<T>
{
public:
	typedef std::allocator<T> base;
	typedef typename base::size_type size_type;
	typedef typename base::difference_type difference_type;
	typedef typename base::pointer pointer;
	typedef typename base::const_pointer const_pointer;
	typedef typename base::reference reference;
	typedef typename base::const_reference const_reference;
	typedef typename base::value_type value_type;

	MeteredAllocator() throw():m_n(n),m_nVolume(0),m_nHits(0) {}
	MeteredAllocator(const MeteredAllocator<T,n> &ma) throw():m_n(n),m_nVolume(0),m_nHits(0),base(ma) {}
	template <class U, int o>
	MeteredAllocator(const MeteredAllocator<U,o> &ma) throw():m_n(o),m_nVolume(0),m_nHits(0),base(ma) {}
	~MeteredAllocator() throw() {}

	template<typename U>
	struct rebind
	{
		typedef MeteredAllocator<U,-1> other;
	};

	size_type max_size() const 
	{
		return std::numeric_limits<size_t>::max() / sizeof(T);
	}

	pointer allocate(size_type num, std::allocator<void>::const_pointer /* hint */ = 0)
	{
		m_nHits++;
		m_nVolume += num * sizeof(T);

		return (pointer) ::operator new(num*sizeof(T));
	}

	void deallocate(pointer p, size_type num)
	{
		delete( (value_type*) p );
	}

	void construct(pointer p, const T& value)
	{
		new((value_type*)p) T(value);
	}

private:
	int m_n;
	size_t m_nVolume;
	size_t m_nHits;
};


typedef std::basic_string<char, std::char_traits<char>, MeteredAllocator<char, 2> > metered_string;

inline metered_string replaceAll(const metered_string &s, const metered_string &f, const metered_string &r)
{
	if ( s.empty() || f.empty() || f == r || s.find(f) == metered_string::npos )
	{
		return s;
	}

	std::basic_stringstream<char, std::char_traits<char>, MeteredAllocator<char, 2> > build_it;
	metered_string::size_type i = 0;
	for (metered_string::size_type pos; ( pos = s.find( f, i ) ) != metered_string::npos; )
	{
		build_it.write( &s[i], pos - i );
		build_it << r;
		i = pos + f.size();
	}
	if ( i != s.size() )
	{
		build_it.write( &s[i], s.size() - i );
	}

	return build_it.str();
}

std::string Replace(const std::string &input, const std::string &find, const std::string &replace)
{
	metered_string ms=replaceAll(metered_string(input.begin(),input.end()),metered_string(find.begin(),find.end()),metered_string(replace.begin(),replace.end()));
	return std::string(ms.begin(),ms.end());
}
#endif	// _DEBUG



void Split(const std::string &str, const std::string &delim, std::vector<std::string> &output)
{
	std::string::size_type offset = 0;
	std::string::size_type delimIndex = 0;
    
    delimIndex = str.find(delim, offset);

    while (delimIndex != std::string::npos)
    {
        output.push_back(str.substr(offset, delimIndex - offset));
        offset += delimIndex - offset + delim.length();
        delimIndex = str.find(delim, offset);
    }

    output.push_back(str.substr(offset));
}

void SplitMultiple(const std::string &str, const std::string &delim, std::vector<std::string> &output)
{
	std::string::size_type offset = 0;
	std::string::size_type delimIndex = 0;
    
    delimIndex = str.find_first_of(delim, offset);

    while (delimIndex != std::string::npos)
    {
        output.push_back(str.substr(offset, delimIndex - offset));
        offset += delimIndex - offset + 1;
        delimIndex = str.find_first_of(delim, offset);
    }

    output.push_back(str.substr(offset));
}

std::string TrimWhitespace(const std::string &str)
{
	std::string returnstring=str;

	while(returnstring.size()>0 && returnstring.find_first_of(" \t\r\n")==0)
	{
		returnstring.erase(0,1);
	}
	while(returnstring.size()>0 && returnstring.find_last_of(" \t\r\n")==returnstring.size()-1)
	{
		returnstring.erase(returnstring.size()-1,1);
	}

	return returnstring;
}

int utf8safetoupper(int ch)
{
	return (ch>96 && ch<123) ? ch-32 : ch;
}

void UpperCase(const std::string &str, std::string &output)
{
	//output=str;
	//std::transform(str.begin(),str.end(),output.begin(),utf8safetoupper);
	output=Poco::UTF8::toUpper(str);
}

std::string UriDecode(const std::string & sSrc)
{
   // Note from RFC1630: "Sequences which start with a percent
   // sign but are not followed by two hexadecimal characters
   // (0-9, A-F) are reserved for future extension"
   
static const char HEX2DEC[256] = 
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    
    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

   const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
   const int SRC_LEN = sSrc.length();
   const unsigned char * const SRC_END = pSrc + SRC_LEN;
   // last decodable '%'
   const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

   char * const pStart = new char[SRC_LEN];
   char * pEnd = pStart;

   while (pSrc < SRC_LAST_DEC)
   {
      if (*pSrc == '%')
      {
         char dec1, dec2;
         if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
            && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
         {
            *pEnd++ = (dec1 << 4) + dec2;
            pSrc += 3;
            continue;
         }
      }

      *pEnd++ = *pSrc++;
   }

   // the last 2- chars
   while (pSrc < SRC_END)
      *pEnd++ = *pSrc++;

   std::string sResult(pStart, pEnd);
   delete [] pStart;
   return sResult;
}

std::string UriEncode(const std::string & sSrc)
{
	
// Only alphanum is safe.
static const char SAFE[256] =
{
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,1,
    /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
    
    /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    
    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    
    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};
	
   const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
   const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
   const int SRC_LEN = sSrc.length();
   unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
   unsigned char * pEnd = pStart;
   const unsigned char * const SRC_END = pSrc + SRC_LEN;

   for (; pSrc < SRC_END; ++pSrc)
   {
      if (SAFE[*pSrc])
         *pEnd++ = *pSrc;
      else
      {
         // escape this char
         *pEnd++ = '%';
         *pEnd++ = DEC2HEX[*pSrc >> 4];
         *pEnd++ = DEC2HEX[*pSrc & 0x0F];
      }
   }

   std::string sResult((char *)pStart, (char *)pEnd);
   delete [] pStart;
   return sResult;
}

}	// namespace
