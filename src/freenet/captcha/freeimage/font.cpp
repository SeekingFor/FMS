#include "../../../../include/freenet/captcha/freeimage/font.h"

namespace FreeImage
{

Font::Font()
{
	m_fontwidth=0;
	m_fontheight=0;
}

Font::Font(const Bitmap &bmp)
{
	m_fontwidth=0;
	m_fontheight=0;
	Load(bmp);
}

Font::~Font()
{

}

const Bitmap &Font::Char(const int num)
{
	if(num>=0 && num<256 && m_chars.size()>num)
	{
		return m_chars[num];
	}
	else
	{
		return m_blank;
	}
}

const bool Font::Load(const Bitmap &bmp)
{
	m_fontwidth=bmp.Width()/16;
	m_fontheight=bmp.Height()/14;

	m_chars.clear();
	m_chars.resize(256);

	int charnum=32;
	for(int yy=0; yy<bmp.Height(); yy+=m_fontheight)
	{
		for(int xx=0; xx<bmp.Width(); xx+=m_fontwidth)
		{
			bool found=false;
			int width=m_fontwidth;
			int idx=bmp.GetPixelIndex(xx+m_fontwidth-1,yy);
			for(int x=xx+m_fontwidth-1; x>xx && found==false; x--)
			{
				for(int y=yy; y<yy+m_fontheight && found==false; y++)
				{
					if(bmp.GetPixelIndex(x,y)!=idx)
					{
						found=true;
					}
				}
				if(found==false)
				{
					width--;
				}
			}
			width==0 ? width=1 : width=width;
			width++;
			width>m_fontwidth ? width=m_fontwidth : false;
			m_chars[charnum].Create(width,m_fontheight,32);
			m_chars[charnum].Blit(bmp,0,0,xx,yy,width,m_fontheight,-1);
			m_chars[charnum].SetTransparent();
			RGBQUAD col=m_chars[charnum].GetPixel(m_chars[charnum].Width()-1,0);
			for(int y=0; y<m_chars[charnum].Height(); y++)
			{
				for(int x=0; x<m_chars[charnum].Width(); x++)
				{
					RGBQUAD col2=m_chars[charnum].GetPixel(x,y);
					if(col.rgbRed==col2.rgbRed && col.rgbGreen==col2.rgbGreen && col.rgbBlue==col2.rgbBlue)
					{
						col2.rgbReserved=0;
						m_chars[charnum].PutPixel(x,y,col2);
					}
				}
			}
			charnum++;
		}
	}

	return true;
}

}	// namespace FreeImage
