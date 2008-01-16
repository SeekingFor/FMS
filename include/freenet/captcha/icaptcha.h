#ifndef _icaptcha_
#define _icaptcha_

#include <vector>

class ICaptcha
{
public:

	virtual void Generate()=0;
	
	virtual const bool GetPuzzle(std::vector<unsigned char> &puzzle)=0;
	virtual const bool GetSolution(std::vector<unsigned char> &solution)=0;
	
};

#endif	// _icaptcha_
