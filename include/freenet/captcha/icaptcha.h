#ifndef _icaptcha_
#define _icaptcha_

#include <vector>
#include <string>

class ICaptcha
{
public:
	ICaptcha()				{};
	virtual ~ICaptcha()		{};

	virtual const bool Generate()=0;
	
	virtual const bool GetPuzzle(std::vector<unsigned char> &puzzle)=0;
	virtual const bool GetSolution(std::vector<unsigned char> &solution)=0;
	virtual const std::string GetMimeType()=0;
	virtual const std::string GetCaptchaType()=0;
	
};

#endif	// _icaptcha_
