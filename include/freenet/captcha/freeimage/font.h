#ifndef _freeimage_font_
#define _freeimage_font_

#include "bitmap.h"

namespace FreeImage
{

class Font
{
public:
	Font();
	Font(const Bitmap &bmp);
	~Font();
	
	const bool Load(const Bitmap &bmp);

	const Bitmap &Char(const int num);

	const int FontWidth() const		{ return m_fontwidth; }
	const int FontHeight() const	{ return m_fontheight; }

private:
	int m_fontwidth;
	int m_fontheight;
	Bitmap m_blank;
	std::vector<Bitmap> m_chars;

};

}

#endif	// _freeimage_font_
