#ifndef _freeimage_bitmap_
#define _freeimage_bitmap_

#include <FreeImage.h>

#include <string>
#include <vector>

namespace FreeImage
{

class Bitmap
{
public:
	Bitmap();
	Bitmap(const Bitmap &bmp);
	Bitmap(const std::string &typestr, const std::string &filename);
	Bitmap(FIBITMAP *bmp);
	Bitmap(const int width, const int height, const int bpp);
	~Bitmap();

	const bool Create(const int width, const int height, const int bpp);

	inline const unsigned int Height() const	{ return m_bmp ? FreeImage_GetHeight(m_bmp) : 0 ; }
	inline const unsigned int Width() const		{ return m_bmp ? FreeImage_GetWidth(m_bmp) : 0 ; }
	inline const unsigned int BPP() const		{ return m_bmp ? FreeImage_GetBPP(m_bmp) : 0 ; }
	void ConvertBPP(const int bpp);

	void Clear(const RGBQUAD color);
	void ClearTransparent();

	const bool Save(const std::string &filename) const;
	const bool SaveToMemory(const std::string &typestr, std::vector<unsigned char> &data) const;
	const bool Load(const std::string &typestr, const std::string &filename);
	const bool LoadFromMemory(const std::string &typestr, std::vector<unsigned char> &data);

	void SetTransparent();
	
	void PutPixel(const int x, const int y, RGBQUAD color);
	RGBQUAD GetPixel(const int x, const int y) const;
	const int GetPixelIndex(const int x, const int y) const;
	void Line(const int x1, const int y1, const int x2, const int y2, const RGBQUAD color);
	void FastLine(const int x1, const int y1, const int x2, const int y2, const RGBQUAD color);
	//void Arc(const int cx, const int cy, const int r, const float start, const float end, const RGBQUAD color);
	void Rect(const int x1, const int y1, const int w, const int h, const bool filled, const RGBQUAD color);
	void Rotate(const double angle, const int shiftx, const int shifty, const int originx, const int originy);
	void HorizontalOffset(const int y, const double shift);
	void VerticalOffset(const int x, const double shift);

	void Blit(const Bitmap &bmp, const int destx, const int desty, const int sourcex, const int sourcey, const int w, const int h, const int alpha);
	void BlitTrans(const Bitmap &bmp, const int destx, const int desty, const int sourcex, const int sourcey, const int w, const int h);

	Bitmap &operator=(const Bitmap &rhs);

private:
	void Destroy();
	void AALine(const int x1, const int y1, const int x2, const int y2, const RGBQUAD color);
	double LineFunction(const double slopex, const double slopey, const int startx, const int starty, const double testx, const double texty);

	FIBITMAP *m_bmp;

};

}	// namespace

#endif	// _freeimage_bitmap_
