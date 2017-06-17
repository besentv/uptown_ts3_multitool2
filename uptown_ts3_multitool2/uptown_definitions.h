#pragma once
#ifndef UPTOWN_DEFINITIONS
#define UPTOWN_DEFINITIONS

#define UPTOWN_DATABASE_ENTRY_NOT_EXISTING -100
#define UPTOWN_DATABASE_ENTRY_EXISTS 100

enum MoverStatus
{
	MOVERSTATUS_NEVER_ALLOWED = 1,
	MOVERSTATUS_ALWAYS_ALLOWED,
	MOVERSTATUS_ENUM_END
};

const char *uptown_getMoverStatusAsString(int status);
const char *uptown_getChanneldenyStatusAsString(int status);
extern char *UPTOWN_HOTKEYSTRING_CHANNEL_MOVE;
extern char *UPTOWN_HOTKEYSTRING_CHANNEL_KICK;
extern char *UPTOWN_HOTKEYSTRING_SERVER_KICK;
extern char *UPTOWN_HOTKEYSTRING_CHANNELDENY;
#endif