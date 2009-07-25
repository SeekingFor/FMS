#include "../../../include/freenet/captcha/alternatecaptcha2.h"

#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>

#include <cmath>
#include <vector>
#include <cstdlib>

#ifdef ALTERNATE_CAPTCHA

AlternateCaptcha2::AlternateCaptcha2()
{
	AlternateCaptchaFonts fonts;
	m_fonts=fonts.Fonts();
}

const bool AlternateCaptcha2::Generate()
{
	if(m_fonts.size()==0)
	{
		return false;
	}

	std::string puzzlestring=GenerateRandomString(6);
	std::vector<int> fontnums(puzzlestring.size(),0);
	
	m_solution.clear();
	m_puzzle.clear();
	
	FreeImage::Bitmap bigbmp(300,300,32);
	bigbmp.SetTransparent();

	// get total width of text
	int textwidth=0;
	for(int i=0; i<puzzlestring.size(); i++)
	{
		fontnums[i]=rand()%m_fonts.size();
		textwidth+=m_fonts[fontnums[i]].Char(puzzlestring[i]).Width();
	}

	RGBQUAD white;
	white.rgbRed=255;
	white.rgbGreen=255;
	white.rgbBlue=255;
	white.rgbReserved=255;
	int numlines=(rand()%3)+2;
	for(int i=0; i<numlines; i++)
	{
		int x1=rand()%150;
		int y1=(rand()%80)+110;
		int x2=(rand()%150)+150;
		int y2=(rand()%80)+110;
		bigbmp.Line(x1,y1,x2,y2,white);
		bigbmp.Line(x1+1,y1,x2+1,y2,white);
	}
	
	// draw the text on the bigbmp centered
	int currentx=150-(textwidth/2);
	for(int i=0; i<puzzlestring.size(); i++)
	{
		FreeImage::Bitmap charbmp=m_fonts[fontnums[i]].Char(puzzlestring[i]);
		bigbmp.BlitTrans(charbmp,currentx,150-(charbmp.Height()/2),0,0,charbmp.Width(),charbmp.Height());
		currentx+=m_fonts[fontnums[i]].Char(puzzlestring[i]).Width();//+1;
	}
	
	// rotate and skew the big bitmap a few times
	int lastrot=0;
	int thisrot=0;
	int numrots=(rand()%3)+4;
	for(int i=0; i<numrots; i++)
	{
		thisrot=((rand()%180)-90);
		bigbmp.Rotate(-lastrot+thisrot,0,0,150,150);
		int offset=rand()%10000;
		float freq=5.0+(float(rand()%7000)/1000.0);
		float amp=1.0+(float(rand()%500)/500.0);
		for(int y=0; y<bigbmp.Height(); y++)
		{
			double shift=sin((double)(y+offset)/freq)*amp;
			bigbmp.HorizontalOffset(y,shift);
		}
		lastrot=thisrot;
	}

	// rotate the big bitmap back to (almost) horizontal
	bigbmp.Rotate(-lastrot+((rand()%20)-10),0,0,150,150);

	// grab the center of the big bitmap as the final bitmap
	FreeImage::Bitmap bmp(110,50,32);
	bmp.SetTransparent();
	bmp.Blit(bigbmp,0,0,95,125,110,50,255);

	m_solution.insert(m_solution.end(),puzzlestring.begin(),puzzlestring.end());
	bmp.SaveToMemory("bmp",m_puzzle);

	return true;
}

const std::string AlternateCaptcha2::GenerateRandomString(const int len)
{
	// no l,1 O,0 because they look too much alike
	static std::string chars="abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ23456789@#";
	std::string temp="";
	for(int i=0; i<len; i++)
	{
		temp+=chars[rand()%chars.size()];
	}
	return temp;
}

const bool AlternateCaptcha2::GetPuzzle(std::vector<unsigned char> &puzzle)
{
	puzzle=m_puzzle;
	return true;
}

const bool AlternateCaptcha2::GetSolution(std::vector<unsigned char> &solution)
{
	solution=m_solution;
	return true;
}

#endif	// ALTERNATE_CAPTCHA
