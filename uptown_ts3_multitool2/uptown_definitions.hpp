#pragma once
#ifndef UPTOWN_DEFINITIONS
#define UPTOWN_DEFINITIONS

#define UPTOWN_DATABASE_ENTRY_NOT_EXISTING -100
#define UPTOWN_DATABASE_ENTRY_EXISTS 100
#define UPTOWN_DATABASE_FILENAME "uptown_database.db"
#define UPTOWN_INFODATA_BUFSIZE 512

#define PLUGIN_API_VERSION 21
#define PLUGIN_VERSION "Beta 1.8"

namespace UptownDefinitions {
	enum MoverStatus
	{
		MOVERSTATUS_NEVER_ALLOWED = 1,
		MOVERSTATUS_ALWAYS_ALLOWED,
		MOVERSTATUS_ENUM_END
	};

	enum ChanneldenyStatus
	{
		CHANNELDENY_DISABLED = 0,
		CHANNELDENY_MOVE,
		CHANNELDENY_KICK
	};

	enum MenuIDs {
		MENU_ID_CLIENT_1 = 1,
		MENU_ID_CLIENT_2,
		MENU_ID_CHANNEL_1,
		MENU_ID_CHANNEL_2,
		MENU_ID_CHANNEL_3,
		MENU_ID_GLOBAL_1,
		MENU_ID_GLOBAL_2
	};

	const char *getMoverStatusAsString(int status);
	const char *getChanneldenyStatusAsString(int status);
	extern char *HOTKEYSTRING_CHANNEL_MOVE;
	extern char *HOTKEYSTRING_CHANNEL_KICK;
	extern char *HOTKEYSTRING_SERVER_KICK;
	extern char *HOTKEYSTRING_CHANNELDENY;
}
#endif