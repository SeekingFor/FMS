#include "../../../include/freenet/captcha/simplecaptcha.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP_Font.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP_Geometry.h"

#ifdef XMEM
	#include <xmem.h>
#endif

void SimpleCaptcha::Generate()
{
	BMP bmp;
	int bmpwidth=100;
	int bmpheight=50;
	std::string puzzlestring;
	std::string tempfilename=GenerateRandomString(10);
	tempfilename+=".bmp";

	puzzlestring=GenerateRandomString(5);
	m_solution.clear();
	m_solution.insert(m_solution.begin(),puzzlestring.begin(),puzzlestring.end());

	bmp.SetSize(bmpwidth,bmpheight);

	// first draw some random lines around the bitmap
	int numlines=(rand()%20)+10;
	for(int i=0; i<numlines; i++)
	{
		RGBApixel pixel;
		int x1=rand()%bmpwidth;
		int y1=rand()%bmpheight;
		int x2=rand()%bmpwidth;
		int y2=rand()%bmpwidth;
		// keep the colors light
		pixel.Red=100+(rand()%150);
		pixel.Green=100+(rand()%150);
		pixel.Blue=100+(rand()%150);
		DrawAALine(bmp,x1,y1,x2,y2,pixel);
	}

	// print puzzle onto bitmap
	for(int i=0; i<5; i++)
	{
		// keep the colors dark
		RGBApixel pixel;
		pixel.Red=(rand()%200);
		pixel.Green=(rand()%200);
		pixel.Blue=(rand()%200);
		PrintLetter(bmp,puzzlestring[i],5+(i*20),10+(rand()%10),20,pixel);
	}

	bmp.WriteToFile(tempfilename.c_str());
	ReadPuzzleData(tempfilename);
	unlink(tempfilename.c_str());
}

const std::string SimpleCaptcha::GenerateRandomString(const int len)
{
	// no l,1 O,0 because they look too much alike
	static std::string chars="abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ23456789";
	std::string temp="";
	for(int i=0; i<len; i++)
	{
		temp+=chars[rand()%chars.size()];
	}
	return temp;
}

const bool SimpleCaptcha::GetPuzzle(std::vector<unsigned char> &puzzle)
{
	puzzle=m_puzzle;
	return true;
}

const bool SimpleCaptcha::GetSolution(std::vector<unsigned char> &solution)
{
	solution=m_solution;
	return true;
}

void SimpleCaptcha::ReadPuzzleData(const std::string &filename)
{
	long filelen;
	FILE *infile=fopen(filename.c_str(),"rb");
	if(infile)
	{
		fseek(infile,0,SEEK_END);
		filelen=ftell(infile);
		fseek(infile,0,SEEK_SET);
		m_puzzle.resize(filelen);
		fread(&m_puzzle[0],1,filelen,infile);
		fclose(infile);
	}
}
