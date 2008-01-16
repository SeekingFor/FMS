#ifndef _introductionpuzzlexml_
#define _introductionpuzzlexml_

#include "../ifmsxmldocument.h"

class IntroductionPuzzleXML:public IFMSXMLDocument
{
public:
	IntroductionPuzzleXML();

	std::string GetXML();

	const bool ParseXML(const std::string &xml);

	void SetType(const std::string &type)		{ m_type=type; }
	void SetUUID(const std::string &uuid)		{ m_uuid=uuid; }
	void SetPuzzleData(const std::string &puzzledata)	{ m_puzzledata=puzzledata; }
	void SetMimeType(const std::string &mimetype)		{ m_mimetype=mimetype; }

	const std::string GetType() const			{ return m_type; }
	const std::string GetUUID() const			{ return m_uuid; }
	const std::string GetPuzzleData() const		{ return m_puzzledata; }
	const std::string GetMimeType() const		{ return m_mimetype; }

private:
	void Initialize();

	std::string m_type;
	std::string m_uuid;
	std::string m_puzzledata;
	std::string m_mimetype;
	
};

#endif	// _introductionpuzzlexml_
