#include "../../../include/freenet/captcha/simplecaptcha.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP_Font.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP_Geometry.h"

#include <Poco/TemporaryFile.h>

#include <cstdlib>

#ifdef XMEM
	#include <xmem.h>
#endif

void SimpleCaptcha::Generate()
{
	BMP bmp;
	int bmpwidth=110;
	int bmpheight=50;
	RGBApixel lettercols[5];
	std::string puzzlestring;
	std::string tempfilename="";
	
	tempfilename=Poco::TemporaryFile::tempName();

	puzzlestring=GenerateRandomString(5);
	m_solution.clear();
	m_solution.insert(m_solution.begin(),puzzlestring.begin(),puzzlestring.end());

	bmp.SetSize(bmpwidth,bmpheight);

	// first draw some random lines around the bitmap
	int numlines=(rand()%10)+10;
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

	// draw some random arcs
	int numarcs=(rand()%10)+10;
	for(int i=0; i<numarcs; i++)
	{
		RGBApixel pixel;
		int x1=rand()%bmpwidth;
		int y1=rand()%bmpwidth;
		int radius=rand()%(bmpheight/2);
		double startangle=(double)(rand()%360)*(3.14159/180);
		double endangle=(double)(rand()%360)*(3.14159/180);
		pixel.Red=100+(rand()%150);
		pixel.Green=100+(rand()%150);
		pixel.Blue=100+(rand()%150);
		DrawArc(bmp,x1,y1,radius,startangle,endangle,pixel);
	}

	for(int i=0; i<5; i++)
	{
		// keep the colors dark
		lettercols[i].Red=(rand()%150);
		lettercols[i].Green=(rand()%150);
		lettercols[i].Blue=(rand()%150);
		// draw a line with the letter color
		DrawAALine(bmp,rand()%bmpwidth,rand()%bmpheight,rand()%bmpwidth,rand()%bmpheight,lettercols[i]);
	}

	// print puzzle onto bitmap
	for(int i=0; i<5; i++)
	{
		PrintLetter(bmp,puzzlestring[i],5+(i*20)+(rand()%11)-5,10+(rand()%10),20,lettercols[i]);
	}

	/* TOO dificult to solve with this
	// skew image left/right
	double skew=0;
	for(int yy=0; yy<bmpheight; yy++)
	{
		RGBApixel pixel;
		skew=skew+(double)((rand()%11)-5)/20.0;
		int xx;
		for(xx=0; xx<skew; xx++)
		{
			pixel.Red=255;
			pixel.Green=255;
			pixel.Blue=255;
			bmp.SetPixel(xx,yy,pixel);
		}
		if(skew<0)
		{
			for(xx=0; xx<bmpwidth+skew; xx++)
			{
				pixel=bmp.GetPixel(xx-skew,yy);
				bmp.SetPixel(xx,yy,pixel);
			}
		}
		else
		{
			for(xx=bmpwidth-1; xx>skew; xx--)
			{
				pixel=bmp.GetPixel(xx-skew,yy);
				bmp.SetPixel(xx,yy,pixel);
			}
		}
		for(xx=bmpwidth+skew; xx<bmpwidth; xx++)
		{
			pixel.Red=255;
			pixel.Green=255;
			pixel.Blue=255;
			bmp.SetPixel(xx,yy,pixel);
		}
	}
	*/

	// generate noise for each pixel
	for(int yy=0; yy<bmpheight; yy++)
	{
		for(int xx=0; xx<bmpwidth; xx++)
		{
			RGBApixel pixel=bmp.GetPixel(xx,yy);
			int tempred=pixel.Red+(rand()%21)-10;
			int tempgreen=pixel.Green+(rand()%21)-10;
			int tempblue=pixel.Blue+(rand()%21)-10;

			tempred < 0 ? tempred=0 : false;
			tempred > 255 ? tempred=255 : false;
			tempgreen < 0 ? tempgreen=0 : false;
			tempgreen > 255 ? tempgreen=255 : false;
			tempblue < 0 ? tempblue=0 : false;
			tempblue > 255 ? tempblue=255 : false;

			pixel.Red=tempred;
			pixel.Green=tempgreen;
			pixel.Blue=tempblue;

			bmp.SetPixel(xx,yy,pixel);
		}
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
