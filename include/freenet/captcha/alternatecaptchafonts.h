#ifndef _alternatecaptcha_fonts_
#define _alternatecaptcha_fonts_

#ifdef ALTERNATE_CAPTCHA

#include "../../ilogger.h"
#include "freeimage/bitmap.h"
#include "freeimage/font.h"
#include <vector>

class AlternateCaptchaFonts:public ILogger
{
public:
	AlternateCaptchaFonts();
	
	const std::vector<FreeImage::Font> &Fonts() const		{ return m_fonts; }

private:
	void LoadFonts();

	static bool m_fontsloaded;
	static std::vector<FreeImage::Font> m_fonts;

};
	
#endif	// ALTERNATE_CAPTCHA

#endif	// _alternatecaptcha_fonts_
