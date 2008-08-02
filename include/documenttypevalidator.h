#ifndef _documenttypevalidator_
#define _documenttypevalidator_

#include <vector>

class DocumentTypeValidator
{
public:
	DocumentTypeValidator()				{}
	virtual ~DocumentTypeValidator()	{}
	virtual const bool Validate(const std::vector<unsigned char> &data)=0;	
};

#endif	// _documenttypevalidator_
