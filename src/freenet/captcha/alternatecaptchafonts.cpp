#ifdef ALTERNATE_CAPTCHA

#include "../../../include/freenet/captcha/alternatecaptchafonts.h"

#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>

bool AlternateCaptchaFonts::m_fontsloaded(false);
std::vector<FreeImage::Font> AlternateCaptchaFonts::m_fonts;

AlternateCaptchaFonts::AlternateCaptchaFonts()
{
	if(m_fontsloaded==false)
	{
		FreeImage_Initialise(true);
		LoadFonts();
		m_fontsloaded=true;
	}
}

void AlternateCaptchaFonts::LoadFonts()
{

	FreeImage::Bitmap bmp;
	Poco::Path path("fonts");
	Poco::DirectoryIterator di(path);
	Poco::DirectoryIterator end;

	while(di!=end)
	{
		if(di.name().find("bmp")!=std::string::npos)
		{
			bmp.Load("bmp",di.path().toString());
			m_fonts.push_back(FreeImage::Font(bmp));
		}
		++di;
	}

}

#endif	// ALTERNATE_CAPTCHA
