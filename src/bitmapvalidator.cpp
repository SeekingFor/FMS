#include "../include/bitmapvalidator.h"
#include "../include/freenet/captcha/easybmp/EasyBMP.h"

#include <Poco/TemporaryFile.h>

#include <sstream>
#include <cstdlib>

BitmapValidator::BitmapValidator():m_maxwidth(-1),m_maxheight(-1)
{
	
}

BitmapValidator::~BitmapValidator()
{
	
}

const bool BitmapValidator::Validate(const std::vector<unsigned char> &data)
{
	bool validated=false;
	std::string tempname=Poco::TemporaryFile::tempName();

	if(data.size()==0)
	{
		return false;
	}

	FILE *outfile=fopen(tempname.c_str(),"w+b");
	if(outfile)
	{
		fwrite(&data[0],1,data.size(),outfile);
		fclose(outfile);
		
		BMP temp;
		if(temp.ReadFromFile(tempname.c_str()))
		{
			validated=true;
			if(m_maxwidth!=-1 && temp.TellWidth()>m_maxwidth)
			{
				validated=false;
			}
			if(m_maxheight!=-1 && temp.TellHeight()>m_maxheight)
			{
				validated=false;
			}
		}

		unlink(tempname.c_str());
		
	}
	
	return validated;
}
