#ifndef _boardlistxml_
#define _boardlistxml_

#include "../ifmsxmldocument.h"

class BoardListXML:public IFMSXMLDocument
{
public:
	BoardListXML();

	std::string GetXML();

	const bool ParseXML(const std::string &xml);

	void AddBoard(const std::string &name, const std::string &description);
	const long GetCount()		{ return m_boards.size(); }
	const std::string GetName(const long index);
	const std::string GetDescription(const long index);

private:
	struct board
	{
		board(const std::string &name, const std::string &description):m_name(name),m_description(description)	{}
		std::string m_name;
		std::string m_description;
	};

	void Initialize();

	std::vector<board> m_boards;
	
};

#endif	// _boardlistxml_
