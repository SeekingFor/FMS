#ifndef _dbsetup_
#define _dbsetup_

#include <string>

// opens database and creates tables and initial inserts if necessary
void SetupDB();
// verifies DB isn't corrupt
const bool VerifyDB();

// returns result of PRAGMA integrity_check
const std::string TestDBIntegrity();

#endif	// _dbsetup_
