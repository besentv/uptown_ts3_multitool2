#include "uptown_definitions.h"

char *UPTOWN_HOTKEYSTRING_CHANNEL_MOVE = "channel_move";
char *UPTOWN_HOTKEYSTRING_CHANNEL_KICK = "channel_kick";
char *UPTOWN_HOTKEYSTRING_SERVER_KICK = "server_kick";
char *UPTOWN_HOTKEYSTRING_CHANNELDENY = "channeldeny";

const char * uptown_getMoverStatusAsString(int status)
{
	switch (status)
	{
	case UPTOWN_DATABASE_ENTRY_NOT_EXISTING:
		return "not in database";
	case MOVERSTATUS_NEVER_ALLOWED:
		return "never allowed";
	case MOVERSTATUS_ALWAYS_ALLOWED:
		return "always allowed";
	default:
		return "error";
		break;
	}
}

const char *uptown_getChanneldenyStatusAsString(int status) {
	if (status == UPTOWN_DATABASE_ENTRY_EXISTS) {
		return "DENIED";
	}
	else if(status == UPTOWN_DATABASE_ENTRY_NOT_EXISTING) {
		return "allowed";
	}
}
