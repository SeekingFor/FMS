#include "../include/bitmapvalidator.h"
#include "../include/freenet/captcha/easybmp/EasyBMP.h"

#include <Poco/TemporaryFile.h>

#include <sstream>
#include <cstdlib>

const bool BitmapValidator::Validate(const std::vector<unsigned char> &data)
{
	bool validated=false;
	std::string tempname=Poco::TemporaryFile::tempName();

	FILE *outfile=fopen(tempname.c_str(),"w+b");
	if(outfile)
	{
		fwrite(&data[0],1,data.size(),outfile);
		fclose(outfile);
		
		BMP temp;
		if(temp.ReadFromFile(tempname.c_str()))
		{
			validated=true;	
		}

		unlink(tempname.c_str());
		
	}
	
	return validated;
}
