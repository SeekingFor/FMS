#ifndef _sonexml_
#define _sonexml_

#include "../ifmsxmldocument.h"

#include <Poco/DateTime.h>

class SoneXML:public IFMSXMLDocument
{
public:
	SoneXML();

	struct message
	{
		message(Poco::DateTime messagetime, std::string id, std::string replyto, std::string &messagetext):m_time(messagetime),m_id(id),m_replyto(replyto),m_message(messagetext)	{ }
		Poco::DateTime m_time;
		std::string m_id;
		std::string m_replyto;
		std::string m_message;
	};

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	const std::vector<message> &GetSones()	{ return m_messages; }
	const std::string GetAvatar()			{ return m_avatar; }

private:
	void Initialize();

	std::vector<message> m_messages;
	std::string m_avatar;

};

#endif	// _sonexml_
