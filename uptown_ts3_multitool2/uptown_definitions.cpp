#include "uptown_definitions.h"

namespace UptownDefinitions {
	char *HOTKEYSTRING_CHANNEL_MOVE = "channel_move";
	char *HOTKEYSTRING_CHANNEL_KICK = "channel_kick";
	char *HOTKEYSTRING_SERVER_KICK = "server_kick";
	char *HOTKEYSTRING_CHANNELDENY = "channeldeny";

	const char * getMoverStatusAsString(int status)
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

	const char *getChanneldenyStatusAsString(int status) {
		if (status == UPTOWN_DATABASE_ENTRY_EXISTS) {
			return "DENIED";
		}
		else if (status == UPTOWN_DATABASE_ENTRY_NOT_EXISTING) {
			return "allowed";
		}
	}
}
