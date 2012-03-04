#ifdef ALTERNATE_CAPTCHA

#include "../../../include/global.h"
#include "../../../include/freenet/captcha/alternatecaptchafonts.h"

#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>

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

	try
	{
		FreeImage::Bitmap bmp;
		Poco::Path path(global::basepath+"fonts");
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
		if(m_fonts.size()==0)
		{
			m_log->fatal("AlternateCaptchaFonts::LoadFonts You have no loadable fonts in the font directory!");
		}
	}
	catch(Poco::Exception &e)
	{
		m_log->error("AlternateCaptchaFonts::LoadFonts caught "+e.displayText());
	}
	catch(...)
	{
		m_log->error("AlternateCaptchaFonts::LoadFonts caught unknown exception");
	}

}

#endif	// ALTERNATE_CAPTCHA
