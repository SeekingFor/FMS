#include "../../../include/freenet/captcha/unlikecaptcha1.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP_Font.h"
#include "../../../include/stringfunctions.h"

#include <vector>
#include <set>

#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>
#include <Poco/TemporaryFile.h>

UnlikeCaptcha1::UnlikeCaptcha1(const std::string &imagedir):m_imagedir(imagedir)
{
	try
	{
		Poco::Path path(imagedir);
		Poco::DirectoryIterator di(path);
		Poco::DirectoryIterator end;

		while(di!=end)
		{
			if(di.name().find("bmp")!=std::string::npos && di.name().find('_')!=std::string::npos)
			{
				std::vector<std::string> fileparts;
				StringFunctions::Split(di.name(),"_",fileparts);
				if(fileparts.size()==2)
				{
					m_sourceimages[fileparts[0]].push_back(fileparts[1]);
				}
			}
			++di;
		}

		std::vector<std::string> erasethese;
		for(std::map<std::string,std::vector<std::string> >::iterator i=m_sourceimages.begin(); i!=m_sourceimages.end(); i++)
		{
			if((*i).second.size()<3)
			{
				erasethese.push_back((*i).first);
			}
		}

		for(std::vector<std::string>::iterator i=erasethese.begin(); i!=erasethese.end(); i++)
		{
			m_sourceimages.erase((*i));
		}

	}
	catch(Poco::PathNotFoundException &e)
	{
		// normal when directory doesn't exist, no logging necessary
	}
	catch(Poco::Exception &e)
	{
		m_log->error("UnlikeCaptcha1 caught "+e.displayText());
	}
	catch(...)
	{
		m_log->error("UnlikeCaptcha1 caught unknown exception");
	}
}

const bool UnlikeCaptcha1::Generate()
{
	std::string tempfilename("");
	std::string sourcenames[4];
	std::string solution("");
	BMP destimage;

	if(m_sourceimages.size()<2)
	{
		return false;
	}

	m_solution.clear();
	m_puzzle.clear();

	// find the 2 sets of images we will use
	int source1num=rand()%m_sourceimages.size();
	int source2num=rand()%m_sourceimages.size();
	while(source2num==source1num)
	{
		source2num=rand()%m_sourceimages.size();
	}
	std::map<std::string,std::vector<std::string> >::const_iterator source1i=m_sourceimages.begin();
	for(int i=0; i<source1num; i++)
	{
		source1i++;
	}
	std::map<std::string,std::vector<std::string> >::iterator source2i=m_sourceimages.begin();
	for(int i=0; i<source2num; i++)
	{
		source2i++;
	}

	// find the images within the sets that we will use
	std::set<int> used;
	int unlikenum=rand()%4;
	StringFunctions::Convert(unlikenum+1,solution);
	solution+=(*source1i).first;
	solution+=(*source2i).first;
	for(int i=0; i<4; i++)
	{
		if(i==unlikenum)
		{
			int num=rand()%(*source1i).second.size();
			sourcenames[i]=(*source1i).first+"_"+(*source1i).second[num];
		}
		else
		{
			int num=rand()%(*source2i).second.size();
			while(used.find(num)!=used.end())
			{
				num=rand()%(*source2i).second.size();
			}
			used.insert(num);
			sourcenames[i]=(*source2i).first+"_"+(*source2i).second[num];
		}
	}
	m_solution.insert(m_solution.end(),solution.begin(),solution.end());

	// load the images and blit on the final bitmap
	destimage.SetSize(200,200);
	Poco::Path path(m_imagedir);
	path.makeDirectory();
	RGBApixel black;
	RGBApixel white;
	RGBApixel pix;

	black.Red=0;
	black.Green=0;
	black.Blue=0;
	black.Alpha=0;
	white.Red=255;
	white.Green=255;
	white.Blue=255;
	white.Alpha=0;

	for(int i=0; i<4; i++)
	{
		BMP sourceimage;
		path.setFileName(sourcenames[i]);
		sourceimage.ReadFromFile(path.toString().c_str());
		RangedPixelToPixelCopy(sourceimage,0,99,99,0,destimage,(i/2)*100,(i%2)*100);
	}

	// create some random noise
	for(int y=0; y<destimage.TellHeight()-1; y++)
	{
		for(int x=0; x<destimage.TellWidth()-1; x++)
		{
			pix=destimage.GetPixel(x,y);
			pix.Red=(std::min)((std::max)((pix.Red+((rand()%21)+10)),0),255);
			pix.Green=(std::min)((std::max)((pix.Green+((rand()%21)+10)),0),255);
			pix.Blue=(std::min)((std::max)((pix.Blue+((rand()%21)+10)),0),255);
			destimage.SetPixel(x,y,pix);
		}
	}

	PrintLetter(destimage,'1',1,1,15,white);
	PrintLetter(destimage,'1',0,0,15,black);
	PrintLetter(destimage,'2',190,1,15,white);
	PrintLetter(destimage,'2',189,0,15,black);
	PrintLetter(destimage,'3',1,199-15,15,white);
	PrintLetter(destimage,'3',0,198-15,15,black);
	PrintLetter(destimage,'4',190,199-15,15,white);
	PrintLetter(destimage,'4',189,198-15,15,black);

	// write the final bitmap and load it as the puzzle
	tempfilename=Poco::TemporaryFile::tempName();
	destimage.WriteToFile(tempfilename.c_str());
	ReadPuzzleData(tempfilename);
	unlink(tempfilename.c_str());
	
	return true;
}

const bool UnlikeCaptcha1::GetPuzzle(std::vector<unsigned char> &puzzle)
{
	puzzle=m_puzzle;
	return true;
}

const bool UnlikeCaptcha1::GetSolution(std::vector<unsigned char> &solution)
{
	solution=m_solution;
	return true;
}

void UnlikeCaptcha1::ReadPuzzleData(const std::string &filename)
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
