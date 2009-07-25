#ifndef _alternate_captcha2_
#define _alternate_captcha2_

#ifdef ALTERNATE_CAPTCHA

#include <string>

#include "icaptcha.h"
#include "alternatecaptchafonts.h"

class AlternateCaptcha2:public ICaptcha
{
public:
	AlternateCaptcha2();

	const bool Generate();

	const bool GetPuzzle(std::vector<unsigned char> &puzzle);
	const bool GetSolution(std::vector<unsigned char> &solution);
	const std::string GetMimeType()			{ return "image/bmp"; }
	const std::string GetCaptchaType()		{ return "captcha"; }

private:
	const std::string GenerateRandomString(const int len);

	std::vector<FreeImage::Font> m_fonts;

	std::vector<unsigned char> m_puzzle;
	std::vector<unsigned char> m_solution;

};

#endif	// ALTERNATE_CAPTCHA

#endif	// _alternate_captcha2_
