#include "../../../include/freenet/captcha/alternatecaptcha1.h"

#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>

#include <cmath>

#ifdef ALTERNATE_CAPTCHA

bool AlternateCaptcha1::m_fontsloaded(false);
std::vector<FreeImage::Font> AlternateCaptcha1::m_fonts;

AlternateCaptcha1::AlternateCaptcha1()
{
	if(m_fontsloaded==false)
	{
		FreeImage_Initialise(true);
		LoadFonts();
		m_fontsloaded=true;
	}
}

void AlternateCaptcha1::Generate()
{
	std::string puzzlestring=GenerateRandomString(5);
	FreeImage::Bitmap tempchar(50,50,32);
	tempchar.SetTransparent();
	FreeImage::Bitmap bmp(110,50,32);
	bmp.SetTransparent();

	m_solution.clear();
	m_puzzle.clear();

	// draw the text
	if(m_fonts.size()>0)
	{
		for(int i=0; i<puzzlestring.size(); i++)
		{
			RGBQUAD black;
			black.rgbRed=0;
			black.rgbGreen=0;
			black.rgbBlue=0;
			int xoffset=(i*20)-(5+(rand()%5));
			int yoffset=(rand()%20)-10;
			int fontnum=rand()%m_fonts.size();
			FreeImage::Bitmap charbmp=m_fonts[fontnum].Char(puzzlestring[i]);

			tempchar.SetTransparent();
			tempchar.ClearTransparent();
			tempchar.BlitTrans(charbmp,25-(charbmp.Width()/2),25-(charbmp.Height()/2),0,0,charbmp.Width(),charbmp.Height());
			tempchar.Rotate((rand()%60)-30,0,0,25,25);

			bmp.BlitTrans(tempchar,xoffset,yoffset,0,0,50,50);
		}
	}

	//place some random lines
	RGBQUAD white;
	white.rgbRed=255;
	white.rgbGreen=255;
	white.rgbBlue=255;
	white.rgbReserved=255;
	int numlines=(rand()%5)+10;
	for(int i=0; i<numlines; i++)
	{
		// draw 4 short lines very close to each other
		int x1[4];
		int y1[4];
		int x2[4];
		int y2[4];

		x1[0]=rand()%bmp.Width();
		y1[0]=rand()%bmp.Height();
		x2[0]=x1[0]+(rand()%40)-20;
		y2[0]=y1[0]+(rand()%40)-20;

		for(int i=1; i<4; i++)
		{
			x1[i]=x1[i-1]+rand()%3-1;
			y1[i]=y1[i-1]+rand()%3-1;
			x2[i]=x2[i-1]+rand()%3-1;
			y2[i]=y2[i-1]+rand()%3-1;
		}

		for(int i=0; i<4; i++)
		{
			bmp.Line(x1[i],y1[i],x2[i],y2[i],white);
		}
	}

	int offset=rand()%10000;
	for(int y=0; y<bmp.Height(); y++)
	{
		double shift=sin((double)(y+offset)/3.0);
		bmp.HorizontalOffset(y,shift);
	}

	m_solution.clear();
	m_solution.insert(m_solution.end(),puzzlestring.begin(),puzzlestring.end());
	m_puzzle.clear();
	bmp.SaveToMemory("bmp",m_puzzle);

}

const std::string AlternateCaptcha1::GenerateRandomString(const int len)
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

const bool AlternateCaptcha1::GetPuzzle(std::vector<unsigned char> &puzzle)
{
	puzzle=m_puzzle;
	return true;
}

const bool AlternateCaptcha1::GetSolution(std::vector<unsigned char> &solution)
{
	solution=m_solution;
	return true;
}

void AlternateCaptcha1::LoadFonts()
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
