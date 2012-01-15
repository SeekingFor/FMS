#ifndef _emoticon_replacer_
#define _emoticon_replacer_

#include <vector>
#include <string>

class EmoticonReplacer
{
public:
	EmoticonReplacer();
	EmoticonReplacer(const std::string &imagepath);

	void Initialize(const std::string &imagepath);

	const std::string Replace(const std::string &message) const;

private:
	std::string m_imagepath;
	std::vector<std::pair<std::string,std::string> > m_emoticons;	// emoticon,file name
};

#endif	// _emoticon_replacer_
