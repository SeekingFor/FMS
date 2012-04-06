#ifndef _global_
#define _global_

#include <string>
#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>
#include <Poco/SingletonHolder.h>
#include "stringtranslation.h"
#include "db/sqlite3db.h"

#define VERSION_MAJOR		"0"
#define VERSION_MINOR		"3"
#define VERSION_RELEASE		"67"
#define FMS_VERSION			VERSION_MAJOR"."VERSION_MINOR"."VERSION_RELEASE
#define FMS_VERSION_LONG	3067
#define FMS_FREESITE_USK	"USK@0npnMrqZNKRCRoGojZV93UNHCMN-6UU3rRSAmP6jNLE,~BG-edFtdCC1cSH4O3BWdeIYa8Sw5DfyrSV-TKdO5ec,AQACAAE/fms/133/"
#define FMS_VERSION_EDITION	"72"

#define MAX_IDENTITY_NAME_LENGTH			40
#define MAX_IDENTITY_NAME_LENGTH_STR		"40"
#define MAX_BOARD_NAME_LENGTH				40
#define MAX_BOARD_NAME_LENGTH_STR			"40"
#define MAX_BOARD_DESCRIPTION_LENGTH		50
#define MAX_BOARD_DESCRIPTION_LENGTH_STR	"50"
#define MAX_TRUST_COMMENT_LENGTH			50
#define MAX_TRUST_COMMENT_LENGTH_STR		"50"
#define MAX_SIGNATURE_LENGTH				500

typedef Poco::ScopedLock<Poco::FastMutex> Guard;
extern Poco::SingletonHolder<StringTranslation> Translation;

std::string CreateShortIdentityName(const std::string &name, const std::string &publickey);
void UpdateMissingAuthorID(SQLite3DB::DB *db, const int identityid, const std::string &name, const std::string &publickey);

namespace global
{
	extern std::string basepath;
	extern bool volatile shutdown;
	extern long volatile httplistenport;
}

#endif	// _global_
