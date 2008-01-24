#ifndef _uwildmat_
#define _uwildmat_

#include "../pstdint.h"

/*
**  WILDMAT MATCHING
*/
enum uwildmat {
    UWILDMAT_FAIL   = 0,
    UWILDMAT_MATCH  = 1,
    UWILDMAT_POISON
};

bool uwildmat(const char *text, const char *pat);

#endif	// _uwildmat_
