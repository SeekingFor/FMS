#ifndef _bitmapvalidator_
#define _bitmapvalidator_

#include "documenttypevalidator.h"

class BitmapValidator:public DocumentTypeValidator
{
public:
	BitmapValidator();
	~BitmapValidator();

	const bool Validate(const std::vector<unsigned char> &data);
	void SetMax(const int maxw, const int maxh)	{ m_maxwidth=maxw; m_maxheight=maxh; }

private:
	int m_maxwidth;
	int m_maxheight;
};

#endif	// _bitmapvalidator_
