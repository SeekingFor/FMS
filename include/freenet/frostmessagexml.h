#ifndef _frost_message_xml_
#define _frost_message_xml_

#include "messagexml.h"

class FrostMessageXML:public MessageXML
{
public:

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	void Initialize();

	struct frostfileattachment
	{
		frostfileattachment(const std::string &key, const int size, const std::string &name):m_key(key),m_size(size),m_name(name)	{}
		std::string m_key;
		int m_size;
		std::string m_name;
	};

	struct frostboardattachment
	{
		frostboardattachment(const std::string &name, const std::string &description, const std::string &publickey, const std::string &privatekey):m_name(name),m_description(description),m_publickey(publickey),m_privatekey(privatekey)	{}
		std::string m_name;
		std::string m_description;
		std::string m_publickey;
		std::string m_privatekey;
	};

	const long GetFrostIDLinePos() const			{ return m_frostidlinepos; }
	const long GetFrostIDLineLen() const			{ return m_frostidlinelen; }
	const std::string GetFrostDate() const			{ return m_frostdate; }
	const std::string GetFrostTime() const			{ return m_frosttime; }
	const std::string GetFrostAuthor() const		{ return m_frostauthor; }
	const std::string GetFrostSubject() const		{ return m_frostsubject; }
	const std::string GetFrostMessageID() const		{ return m_frostmessageid; }
	const std::string GetFrostBoard() const			{ return m_frostboard; }
	const std::string GetFrostPublicKey() const		{ return m_frostpublickey; }
	const std::string GetFrostSignature() const		{ return m_frostsignature; }
	const std::string GetFrostSignatureV2() const	{ return m_frostsignaturev2; }
	const std::string GetFrostInReplyTo() const		{ return m_frostinreplyto; }
	const std::vector<frostfileattachment> GetFrostFileAttachments() const	{ return m_frostfileattachments; }
	const std::vector<frostboardattachment> GetFrostBoardAttachments() const	{ return m_frostboardattachments; }

	const std::string GetSignableContentV2() const;

protected:
	long m_frostidlinepos;
	long m_frostidlinelen;
	std::string m_frostdate;
	std::string m_frosttime;
	std::string m_frostauthor;
	std::string m_frostsubject;
	std::string m_frostmessageid;
	std::string m_frostboard;
	std::string m_frostpublickey;
	std::string m_frostsignature;
	std::string m_frostsignaturev2;
	std::string m_frostinreplyto;
	std::vector<frostfileattachment> m_frostfileattachments;
	std::vector<frostboardattachment> m_frostboardattachments;

private:
	const std::string FrostMakeFileName(const std::string &input) const;

};

#endif	// _frost_message_xml_
