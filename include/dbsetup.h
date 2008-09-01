#ifndef _dbsetup_
#define _dbsetup_

// opens database and creates tables and initial inserts if necessary
void SetupDB();
// verifies DB isn't corrupt
const bool VerifyDB();

#endif	// _dbsetup_
