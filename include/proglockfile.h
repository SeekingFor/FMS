#ifndef _proglockfile_
#define _proglockfile_

#include <string>

class ProgLockFile
{
public:
	ProgLockFile(const std::string &filename);
	~ProgLockFile();

	const bool TryLock();

private:

	std::string m_filename;
	int m_fd;

};

#endif	// _proglockfile_
