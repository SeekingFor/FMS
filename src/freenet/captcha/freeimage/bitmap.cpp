#include "../../../../include/freenet/captcha/freeimage/bitmap.h"

#include <cmath>
#include <cstdlib>

namespace FreeImage
{

Bitmap::Bitmap()
{
	m_bmp=0;
}

Bitmap::Bitmap(const Bitmap &bmp)
{
	m_bmp=0;
	*this=bmp;
}

Bitmap::Bitmap(const std::string &typestr, const std::string &filename)
{
	m_bmp=0;
	Load(typestr,filename);
}

Bitmap::Bitmap(FIBITMAP *bmp):m_bmp(bmp)
{

}

Bitmap::Bitmap(const int width, const int height, const int bpp)
{
	m_bmp=FreeImage_Allocate(width,height,bpp,0,0,0);
}

Bitmap::~Bitmap()
{
	Destroy();
}

void Bitmap::AALine(const int x1, const int y1, const int x2, const int y2, const RGBQUAD color)
{
	RGBQUAD tempcol;
	int height=Height();
	int width=Width();
	int fromx=x1;
	int fromy=y1;
	int tox=x2;
	int toy=y2;
	double sx=x2-x1;
	double sy=y2-y1;

	if(fabs(sx)<=0.8 && fabs(sy)<=0.8)
	{
		return;
	}

	double hypot=sqrt((sx*sx)+(sy*sy));
	sx/=hypot;
	sy/=hypot;

	fromx<0 ? fromx=0 : false;
	fromx>=width ? fromx=width-1 : false;
	tox<0 ? tox=0 : false;
	tox>=width ? tox=width-1 : false;
	fromy<0 ? fromy=0 : false;
	fromy>=height ? fromy=height-1 : false;
	toy<0 ? toy=0 : false;
	toy>=height ? toy=height-1 : false;

	int lx=fromx;
	int rx=tox;
	if(rx<lx)
	{
		std::swap(rx,lx);
	}
	int ty=fromy;
	int by=toy;
	if(by<ty)
	{
		std::swap(ty,by);
	}

	for(int i=lx; i<=rx; i++)
	{
		for(int j=ty; j<=by; j++)
		{
			double ii=0;
			double jj=0;
			double dx=0.25;
			double dy=0.25;
			double x=i-1.5*dx;
			double y=j-1.5*dy;
			double temp=0;
			
			for(ii=-2; ii<=1; ii++)
			{
				for(jj=-2; jj<=1; jj++)
				{
					x=i+ii*dx+0.5*dx;
					y=j+jj*dy+0.5*dy;
					double temp1 = LineFunction( sx, sy , fromx, fromy, x,y );
					temp1<=0.5 ? temp1=1.0 : temp1=0.0;
					temp+=temp1;
				}
			}

			temp/=16.0;
			double minval=0.03125;

			if(temp>minval)
			{
				FreeImage_GetPixelColor(m_bmp,i,(height-1)-j,&tempcol);
				tempcol.rgbRed=((1.0-temp)*((double)tempcol.rgbRed)+temp*((double)color.rgbRed));
				tempcol.rgbGreen=((1.0-temp)*((double)tempcol.rgbGreen)+temp*((double)color.rgbGreen));
				tempcol.rgbBlue=((1.0-temp)*((double)tempcol.rgbBlue)+temp*((double)color.rgbBlue));
				FreeImage_SetPixelColor(m_bmp,i,(height-1)-j,&tempcol);
			}

		}
	}

}

void Bitmap::Blit(const Bitmap &bmp, const int destx, const int desty, const int sourcex, const int sourcey, const int w, const int h, const int alpha)
{
	if(m_bmp && bmp.m_bmp)
	{
		int width=w;
		int height=h;
		if(destx+width-1>=Width())
		{
			width=Width()-destx+1;
		}
		if(desty+height-1>=Height())
		{
			height=Height()-desty+1;
		}
		FIBITMAP *temp=FreeImage_Copy(bmp.m_bmp,sourcex,sourcey,sourcex+width,sourcey+height);
		FreeImage_Paste(m_bmp,temp,destx,desty,alpha);
		FreeImage_Unload(temp);
	}
}

void Bitmap::BlitTrans(const Bitmap &bmp, const int destx, const int desty, const int sourcex, const int sourcey, const int w, const int h)
{
	if(m_bmp && bmp.m_bmp)
	{
		RGBQUAD sourcecol;
		RGBQUAD destcol;
		int sourcew=bmp.Width();
		int sourceh=bmp.Height();
		int destw=Width();
		int desth=Height();
		int dx=destx;
		int dy=desty;
		double alpha=0.0;
		for(int y=sourcey; y<sourcey+h; y++, dy++)
		{
			dx=destx;
			for(int x=sourcex; x<sourcex+w; x++, dx++)
			{
				if(y>=0 && y<sourceh && x>=0 && x<sourcew && dx>=0 && dx<destw && dy>=0 && dy<desth)
				{
					FreeImage_GetPixelColor(bmp.m_bmp,x,(sourceh-1)-y,&sourcecol);
					if(sourcecol.rgbReserved==255)		// opaque
					{
						FreeImage_SetPixelColor(m_bmp,dx,(desth-1)-dy,&sourcecol);
					}
					else if(sourcecol.rgbReserved>0)	// some translucency
					{
						FreeImage_GetPixelColor(m_bmp,dx,(desth-1)-dy,&destcol);
						alpha=(double)sourcecol.rgbReserved/255.0;
						sourcecol.rgbRed=(sourcecol.rgbRed*alpha)+(destcol.rgbRed*(1.0-alpha));
						sourcecol.rgbGreen=(sourcecol.rgbGreen*alpha)+(destcol.rgbGreen*(1.0-alpha));
						sourcecol.rgbBlue=(sourcecol.rgbBlue*alpha)+(destcol.rgbBlue*(1.0-alpha));
						sourcecol.rgbReserved<destcol.rgbReserved ? sourcecol.rgbReserved=destcol.rgbReserved : false;
						FreeImage_SetPixelColor(m_bmp,dx,(desth-1)-dy,&sourcecol);
					}
				}
			}
		}
	}
}

void Bitmap::Clear(RGBQUAD color)
{
	if(m_bmp)
	{
		int h=FreeImage_GetHeight(m_bmp);
		int w=FreeImage_GetWidth(m_bmp);
		for(int y=0; y<h; y++)
		{
			for(int x=0; x<w; x++)
			{
				FreeImage_SetPixelColor(m_bmp,x,y,&color);
			}
		}
	}
}

void Bitmap::ClearTransparent()
{
	if(m_bmp)
	{
		RGBQUAD color;
		color.rgbRed=0;
		color.rgbGreen=0;
		color.rgbBlue=0;
		color.rgbReserved=0;
		int h=Height();
		int w=Width();
		for(int y=0; y<h; y++)
		{
			for(int x=0; x<w; x++)
			{
				FreeImage_SetPixelColor(m_bmp,x,y,&color);
			}
		}
	}
}

void Bitmap::ConvertBPP(const int bpp)
{
	if(m_bmp)
	{
		FIBITMAP *temp;
		switch(bpp)
		{
		case 2:
			temp=FreeImage_ConvertToGreyscale(m_bmp);
			Destroy();
			m_bmp=temp;
			break;
		case 4:
			temp=FreeImage_ConvertTo4Bits(m_bmp);
			Destroy();
			m_bmp=temp;
			break;
		case 8:
			temp=FreeImage_ConvertTo8Bits(m_bmp);
			Destroy();
			m_bmp=temp;
			break;
		case 16:
			temp=FreeImage_ConvertTo16Bits565(m_bmp);
			Destroy();
			m_bmp=temp;
			break;
		case 24:
			temp=FreeImage_ConvertTo24Bits(m_bmp);
			Destroy();
			m_bmp=temp;
			break;
		case 32:
		default:
			temp=FreeImage_ConvertTo32Bits(m_bmp);
			Destroy();
			m_bmp=temp;
			break;
		}
	}
}

const bool Bitmap::Create(const int width, const int height, const int bpp)
{
	Destroy();
	m_bmp=FreeImage_Allocate(width,height,bpp,0,0,0);
	if(m_bmp)
	{
		return true;
	}
	return false;
}

void Bitmap::Destroy()
{
	if(m_bmp)
	{
		FreeImage_Unload(m_bmp);
		m_bmp=0;
	}
}

void Bitmap::FastLine(const int x1, const int y1, const int x2, const int y2, RGBQUAD color)
{
	if(m_bmp)
	{
		int width=Width();
		int height=Height();
		int fromx=x1;
		int fromy=y1;
		int tox=x2;
		int toy=y2;

		fromx<0 ? fromx=0 : false;
		fromx>=width ? fromx=width-1 : false;
		tox<0 ? tox=0 : false;
		tox>=width ? tox=width-1 : false;
		fromy<0 ? fromy=0 : false;
		fromy>=height ? fromy=height-1 : false;
		toy<0 ? toy=0 : false;
		toy>=height ? toy=height-1 : false;

		int dx=tox-fromx;
		int dy=toy-fromy;

		if(dx==0 && dy==0)
		{
			return;
		}

		int xinc1=1;
		if(dx<0)
		{
			xinc1=-1;
			dx=-dx;
		}
		int yinc1=1;
		if(dy<0)
		{
			yinc1=-1;
			dy=-dy;
		}

		int x=fromx;
		int y=fromy;
		int xinc2=xinc1;
		int yinc2=yinc1;

		double den;
		double num;
		int numadd;
		int numpixels;

		if(dx>=dy)
		{
			xinc1=0;
			yinc2=0;
			den=dx+0.0;
			num=0.5*dx;
			numadd=dy;
			numpixels=dx;
		}
		else
		{
			xinc2=0;
			yinc1=0;
			den=dy+0.0;
			num=0.5*dy;
			numadd=dx;
			numpixels=dy;
		}

		int cpixel;
		for(cpixel=0; cpixel<=numpixels; cpixel++)
		{
			FreeImage_SetPixelColor(m_bmp,x,(height-1)-y,&color);
			num+=numadd;
			if(num>=den)
			{
				num=-den;
				x+=xinc1;
				y+=yinc1;
			}
			x+=xinc2;
			y+=yinc2;
		}
	}
}

RGBQUAD Bitmap::GetPixel(const int x, const int y) const
{
	RGBQUAD color;
	color.rgbRed=0;
	color.rgbGreen=0;
	color.rgbBlue=0;
	color.rgbReserved=0;

	if(m_bmp && x>=0 && x<Width() && y>=0 && y<Height())
	{
		FreeImage_GetPixelColor(m_bmp,x,Height()-1-y,&color);
	}

	return color;

}

const int Bitmap::GetPixelIndex(const int x, const int y) const
{
	if(m_bmp)
	{
		BYTE index;
		FreeImage_GetPixelIndex(m_bmp,x,Height()-1-y,&index);
		return index;
	}
	else
	{
		return -1;
	}
}

void Bitmap::HorizontalOffset(const int y, const double shift)
{
	if(m_bmp)
	{
		int width=Width();
		int height=Height();
		int startx=width-1;
		int endx=ceil(shift);
		int dx=-1;
		int offset1=-(floor(shift));
		int offset2=-(ceil(shift));
		RGBQUAD color1;
		RGBQUAD color2;
		RGBQUAD newcolor;
		double part2=shift-floor(shift);
		double part1=1.0-part2;

		if(shift<0)
		{
			startx=0;
			endx=width-1-ceil(abs(shift));
			dx=1;
			offset1=-ceil(shift);
			offset2=-floor(shift);
			part2=abs(shift-ceil(shift));
			part1=1.0-part2;
		}

		FreeImage_GetPixelColor(m_bmp,startx+offset1,(height-1)-y,&color1);
		for(int x=startx+dx; x!=endx; x+=dx)
		{
			FreeImage_GetPixelColor(m_bmp,x+offset2,(height-1)-y,&color2);
			
			newcolor.rgbRed=(color1.rgbRed*part1)+(color2.rgbRed*part2);
			newcolor.rgbGreen=(color1.rgbGreen*part1)+(color2.rgbGreen*part2);
			newcolor.rgbBlue=(color1.rgbBlue*part1)+(color2.rgbBlue*part2);

			FreeImage_SetPixelColor(m_bmp,x,(height-1)-y,&newcolor);

			color1=color2;
		}
	}
}

void Bitmap::Line(const int x1, const int y1, const int x2, const int y2, const RGBQUAD color)
{
	if(m_bmp)
	{
		if(x1!=x2 && y1!=y2)
		{
			AALine(x1,y1,x2,y2,color);
		}
		else
		{
			FastLine(x1,y1,x2,y2,color);
		}
	}
}

double Bitmap::LineFunction(const double slopex, const double slopey, const int startx, const int starty, const double testx, const double testy)
{
	return fabs(slopex*(testy-starty)-slopey*(testx-startx));
}

const bool Bitmap::Load(const std::string &typestr, const std::string &filename)
{
	Destroy();
	FREE_IMAGE_FORMAT type=FIF_BMP;
	if(typestr.find("png")!=std::string::npos)
	{
		type=FIF_PNG;
	}
	m_bmp=FreeImage_Load(type,filename.c_str(),0);
	if(m_bmp)
	{
		return true;
	}
	return false;
}

const bool Bitmap::LoadFromMemory(const std::string &typestr, std::vector<unsigned char> &data)
{
	bool loaded=false;
	Destroy();
	if(data.size()>0)
	{
		FREE_IMAGE_FORMAT type=FIF_BMP;
		if(typestr.find("png")!=std::string::npos)
		{
			type=FIF_PNG;
		}

		FIMEMORY *mem=FreeImage_OpenMemory((BYTE *)&data[0],data.size());
		m_bmp=FreeImage_LoadFromMemory(type,mem,0);
		if(m_bmp)
		{
			loaded=true;
		}
		FreeImage_CloseMemory(mem);
	}
	return loaded;
}

Bitmap &Bitmap::operator=(const Bitmap &rhs)
{
	if(this!=&rhs)
	{
		Destroy();
		if(rhs.Width()>0 && rhs.Height()>0)
		{
			m_bmp=FreeImage_Allocate(rhs.Width(),rhs.Height(),rhs.BPP(),0,0,0);
			//FIBITMAP *temp=FreeImage_Copy(rhs.m_bmp,0,0,rhs.Width()-1,rhs.Height()-1);
			//FreeImage_Paste(m_bmp,rhs.m_bmp,0,0,-1);
			//FreeImage_Unload(temp);
			Blit(rhs,0,0,0,0,rhs.Width(),rhs.Height(),-1);
		}
	}
	return *this;
}

void Bitmap::PutPixel(const int x, const int y, RGBQUAD color)
{
	if(m_bmp && x>=0 && x<FreeImage_GetWidth(m_bmp) && y>=0 && y<FreeImage_GetHeight(m_bmp))
	{
		FreeImage_SetPixelColor(m_bmp,x,(Height()-1)-y,&color);
	}
}

void Bitmap::Rect(const int x1, const int y1, const int w, const int h, const bool filled, RGBQUAD color)
{
	if(m_bmp)
	{
		int height=Height();
		int width=Width();
		for(int y=y1; y<y1+h; y++)
		{
			if(y>=0 && y<height)
			{
				for(int x=x1; x<x1+w; x++)
				{
					if(x>=0 && x<width)
					{
						if(x==x1 || x==x1+w-1 || y==y1 || y==y1+h-1 || filled)
						{
							FreeImage_SetPixelColor(m_bmp,x,(height-1)-y,&color);
						}
					}
				}
			}
		}
	}
}

void Bitmap::Rotate(const double angle, const int shiftx, const int shifty, const int originx, const int originy)
{
	if(m_bmp)
	{
		int height=Height();
		FIBITMAP *old=m_bmp;
		m_bmp=FreeImage_RotateEx(m_bmp,angle,shiftx,-shifty,originx,(height-1)-originy,true);
		FreeImage_Unload(old);
	}
}

const bool Bitmap::Save(const std::string &filename) const
{
	if(m_bmp)
	{
		FREE_IMAGE_FORMAT type=FIF_BMP;

		if(filename.find(".png")!=std::string::npos)
		{
			type=FIF_PNG;
		}

		if(FreeImage_Save(type,m_bmp,filename.c_str(),0))
		{
			return true;
		}
	}
	return false;
}

const bool Bitmap::SaveToMemory(const std::string &typestr, std::vector<unsigned char> &data) const
{
	bool saved=false;
	if(m_bmp)
	{
		FREE_IMAGE_FORMAT type=FIF_BMP;
		if(typestr.find("png")!=std::string::npos)
		{
			type=FIF_PNG;
		}
		FIMEMORY *mem=FreeImage_OpenMemory(0,0);
		if(FreeImage_SaveToMemory(type,m_bmp,mem,0))
		{
			FreeImage_SeekMemory(mem,0,SEEK_END);
			data.resize(FreeImage_TellMemory(mem));
			FreeImage_SeekMemory(mem,0,SEEK_SET);
			FreeImage_ReadMemory(&data[0],1,data.size(),mem);
			saved=true;
		}
		FreeImage_CloseMemory(mem);
	}
	return saved;
}

void Bitmap::SetTransparent()
{
	if(m_bmp)
	{
		FreeImage_SetTransparent(m_bmp,true);
	}
}

void Bitmap::VerticalOffset(const int x, const double shift)
{
	if(m_bmp)
	{
		int width=Width();
		int height=Height();
		int starty=width-1;
		int endy=ceil(shift);
		int dy=-1;
		int offset1=-(floor(shift));
		int offset2=-(ceil(shift));
		RGBQUAD color1;
		RGBQUAD color2;
		RGBQUAD newcolor;
		double part2=shift-floor(shift);
		double part1=1.0-part2;

		if(shift<0)
		{
			starty=0;
			endy=width-1-ceil(abs(shift));
			dy=1;
			offset1=-ceil(shift);
			offset2=-floor(shift);
			part2=abs(shift-ceil(shift));
			part1=1.0-part2;
		}

		FreeImage_GetPixelColor(m_bmp,x,(height-1)-(starty+offset1),&color1);
		for(int y=starty+dy; y!=endy; y+=dy)
		{
			FreeImage_GetPixelColor(m_bmp,x,(height-1)-(y+offset2),&color2);
			
			newcolor.rgbRed=(color1.rgbRed*part1)+(color2.rgbRed*part2);
			newcolor.rgbGreen=(color1.rgbGreen*part1)+(color2.rgbGreen*part2);
			newcolor.rgbBlue=(color1.rgbBlue*part1)+(color2.rgbBlue*part2);

			FreeImage_SetPixelColor(m_bmp,x,(height-1)-y,&newcolor);

			color1=color2;
		}
	}
}

}	// namespace FreeImage
