#ifndef _httpdefs_
#define _httpdefs_

struct httpstate
{
	char *m_indata;
	long m_indatalen;
	long m_indatapos;
	char *m_outdata;
	long m_outdatalen;
	long m_outdatapos;
};

#endif	// _httpdefs_
