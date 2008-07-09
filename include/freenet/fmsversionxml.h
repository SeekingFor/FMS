#ifndef _fmsversionxml_
#define _fmsversionxml_

#include "../ifmsxmldocument.h"

class FMSVersionXML:public IFMSXMLDocument
{
public:
	FMSVersionXML();

	std::string GetXML();

	const bool ParseXML(const std::string &xml);

	const int GetMajor()				{ return m_major; }
	const int GetMinor()				{ return m_minor; }
	const int GetRelease()				{ return m_release; }
	const std::string GetNotes()		{ return m_notes; }
	const std::string GetChanges()		{ return m_changes; }
	const std::string GetPageKey()		{ return m_pagekey; }
	const std::string GetSourceKey()	{ return m_sourcekey; }

private:
	void Initialize();

	int m_major;
	int m_minor;
	int m_release;
	std::string m_notes;
	std::string m_changes;
	std::string m_pagekey;
	std::string m_sourcekey;

};

#endif	// _fmsversionxml_
