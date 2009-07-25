#ifndef _unlike_captcha1_
#define _unlike_captcha1_

#include "icaptcha.h"
#include "../../ilogger.h"

#include <map>
#include <string>

class UnlikeCaptcha1:public ICaptcha,public ILogger
{
public:
	UnlikeCaptcha1(const std::string &imagedir);

	const bool Generate();

	const bool GetPuzzle(std::vector<unsigned char> &puzzle);
	const bool GetSolution(std::vector<unsigned char> &solution);
	const std::string GetMimeType()			{ return "image/bmp"; }
	const std::string GetCaptchaType()		{ return "unlikecaptcha1"; }

private:
	void ReadPuzzleData(const std::string &filename);

	std::string m_imagedir;

	std::vector<unsigned char> m_puzzle;
	std::vector<unsigned char> m_solution;

	std::map<std::string,std::vector<std::string> > m_sourceimages;

};

#endif	// _unlike_captcha1_
