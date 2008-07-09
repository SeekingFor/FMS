#ifndef _bitmapvalidator_
#define _bitmapvalidator_

#include "documenttypevalidator.h"

class BitmapValidator:public DocumentTypeValidator
{
public:
	const bool Validate(const std::vector<unsigned char> &data);
};

#endif	// _bitmapvalidator_
