#ifndef _freenetkeys_
#define _freenetkeys_

#include <exception>
#include <string>

/*
	Exception
*/
class InvalidFreenetKeyException
{
public:
	InvalidFreenetKeyException(const std::string &what):m_what(what)	{}
private:
	std::string m_what;
};

/*
	Base class for Freenet keys
*/
class FreenetKey
{
public:
	FreenetKey():m_fullkey("")	{}
	virtual ~FreenetKey()		{}

	virtual const std::string GetBaseKey() const=0;
	virtual const std::string GetDataPath() const=0;
	virtual const std::string GetFullKey() const		{ return m_fullkey; }
	virtual const bool TryParse(const std::string &key)=0;

protected:
	std::string m_fullkey;

};


/*
	KSK key
*/
class FreenetKSKKey:public FreenetKey
{
public:
	FreenetKSKKey();
	FreenetKSKKey(const std::string &key);
	const std::string GetBaseKey() const	{ return GetFullKey(); }
	const std::string GetDataPath() const	{ return m_fullkey.size()>4 ? m_fullkey.substr(4) : "" ; }
	const bool TryParse(const std::string &key);
private:

};


/*
	USK key
*/
class FreenetUSKKey:public FreenetKey
{
public:
	FreenetUSKKey();
	FreenetUSKKey(const std::string &key);
	const bool TryParse(const std::string &key);

	const std::string GetBaseKey() const;
	const std::string GetDataPath() const;
	const std::string GetSiteName() const;
	const int GetEdition() const		{ return m_edition; }

private:
	int m_edition;
};



/*
	SSK Key
*/
class FreenetSSKKey:public FreenetKey
{
public:
	FreenetSSKKey();
	FreenetSSKKey(const std::string &key);
	const bool TryParse(const std::string &key);

	const std::string GetBaseKey() const;
	const std::string GetDataPath() const;

	FreenetSSKKey &operator=(const FreenetUSKKey &rhs);
};




/*
	CHK Key
*/
class FreenetCHKKey:public FreenetKey
{
public:
	FreenetCHKKey();
	FreenetCHKKey(const std::string &key);
	const bool TryParse(const std::string &key);

	const std::string GetBaseKey() const;
	const std::string GetDataPath() const;
};


#endif	// _freenetkeys_
