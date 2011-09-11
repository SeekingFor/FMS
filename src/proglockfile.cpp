#include "../include/proglockfile.h"

#include <cstdio>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

ProgLockFile::ProgLockFile(const std::string &filename):m_filename(filename),m_fd(0),m_unlink(false)
{

}

ProgLockFile::~ProgLockFile()
{
	if(m_fd!=0)
	{
#ifdef _WIN32
		_close(m_fd);
		if(m_unlink)
		{
			_unlink(m_filename.c_str());
		}
#else
		close(m_fd);
		if(m_unlink)
		{
			unlink(m_filename.c_str());
		}
#endif
	}
}

const bool ProgLockFile::TryLock()
{
#ifdef _WIN32
	if((m_fd=_open(m_filename.c_str(),O_WRONLY|O_CREAT,_S_IREAD|_S_IWRITE))==-1)
	{
		return false;
	}
	
	HANDLE fhandle=(HANDLE)_get_osfhandle(m_fd);
	OVERLAPPED ov;
	ov.hEvent=0;
	ov.Offset=0;
	BOOL res=LockFileEx(fhandle,LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,0,1,0,&ov);
	if(res==0)
	{
		return false;
	}

	m_unlink=true;
	return true;

#else
	//http://www.linuxquestions.org/questions/programming-9/restricting-multiple-instance-of-a-program-242069/
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;

	if((m_fd = open(m_filename.c_str(), O_WRONLY|O_CREAT, 0666)) == -1)
	{
		return false;
	}

	if(fcntl(m_fd, F_SETLK, &fl) == -1)
	{
		return false;
	}

	m_unlink=true;
	return true;
#endif
}
