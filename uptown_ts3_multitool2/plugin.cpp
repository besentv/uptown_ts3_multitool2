#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <string>
#include <sstream>


#include "plugin_definitions.h"
#include "teamlog\logtypes.h"
#include "teamspeak\clientlib_publicdefinitions.h"
#include "teamspeak\public_definitions.h"
#include "teamspeak\public_errors.h"
#include "teamspeak\public_errors_rare.h"
#include "teamspeak\public_rare_definitions.h"
#include "ts3_functions.h"

#include "plugin.h"
#include "reset_functions.h"
#include "uptown_database.h"
#include "uptown_definitions.h"

#define PLUGIN_API_VERSION 21
#define PLUGIN_VERSION "Beta 1.7.2"

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

UptownDatabase *uptowndatabase;

struct TS3Functions ts3Functions;
static char* pluginID = NULL;

char* password = NULL;

uint64 scHandler = 0;
uint64 lastscH = 0;
uint64 lastChannelID = 0;

unsigned int error = 0;

bool reconnecting = false;
int isAntiChannelKick = 1;
int isAntiChannelMove = 0;
int isAntiServerKick = 1;
int channelDenyState = 0;

char pluginPath[1024];

const char* ts3plugin_name() {
	return "Uptown - TS3 Multitool";
}

const char* ts3plugin_version() {
	return PLUGIN_VERSION;
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
	return "besentv.xyz";
}

const char* ts3plugin_description() {
	return "Plugin to save your rights on any TeamSpeak server. ;)";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

int ts3plugin_init() {
	printf("Uptown: init\n");
	ts3Functions.getPluginPath(pluginPath, 1024, pluginID);
	uptowndatabase = new UptownDatabase(pluginPath);

	return 0;
}

void ts3plugin_shutdown() {
	printf("Uptown: shutdown\n");
	delete uptowndatabase;
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

int ts3plugin_offersConfigure() {
	return PLUGIN_OFFERS_NO_CONFIGURE;
}

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

const char* ts3plugin_commandKeyword() {
	return "uptown";
}

const char* ts3plugin_infoTitle() {
	return "Uptown";
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}

int ts3plugin_requestAutoload() {
	return 0;
}

void switchHotkeyStatus(int *hotkey) {
	if (*hotkey == 0) {
		*hotkey = 1;

	}
	else if (*hotkey == 1) {
		*hotkey = 0;
	}
	else
	{
		*hotkey = 0;
	}
}

void switchHotkeyStatusChanneldeny(int *hotkey) {
	switch (*hotkey) {
	case CHANNELDENY_DISABLED:
		*hotkey = CHANNELDENY_MOVE;
		break;
	case CHANNELDENY_MOVE:
		*hotkey = CHANNELDENY_KICK;
		break;
	case CHANNELDENY_KICK:
		*hotkey = CHANNELDENY_DISABLED;
		break;
	default:
		*hotkey = CHANNELDENY_DISABLED;
	}
}

void getHotkeySettings() {
	if (uptowndatabase->database_initialized) {
		isAntiChannelKick = uptowndatabase->hotkeysettings_getHotkeyState(UPTOWN_HOTKEYSTRING_CHANNEL_KICK);
		isAntiChannelMove = uptowndatabase->hotkeysettings_getHotkeyState(UPTOWN_HOTKEYSTRING_CHANNEL_MOVE);
		isAntiServerKick = uptowndatabase->hotkeysettings_getHotkeyState(UPTOWN_HOTKEYSTRING_SERVER_KICK);
		channelDenyState = uptowndatabase->hotkeysettings_getHotkeyState(UPTOWN_HOTKEYSTRING_CHANNELDENY);
	}
}

void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	char* str;
	char* UID;
	char* name = "";
	int permission;
	int channeldeny;
	switch (type) {
	case PLUGIN_CHANNEL:
		if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_FLAG_DEFAULT, &str) != ERROR_ok) {
			//printf("Error getting channel name\n");
			return;
		}
		*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));
		snprintf(*data, INFODATA_BUFSIZE, "Channel id is: %llu Channel flag is default %s", id, str);
		free(str);
		break;
	case PLUGIN_CLIENT: {
		if (uptowndatabase->database_initialized) {
			if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &name) != ERROR_ok) {
				name = "error";
			}
			if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNIQUE_IDENTIFIER, &UID) == ERROR_ok) {
				permission = uptowndatabase->allowlist_getMovePermissionState(UID);
				channeldeny = uptowndatabase->channeldeny_existsEntry(UID);
				*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));
				snprintf(*data, INFODATA_BUFSIZE, "%s's move permission state: \"%s\" - channeldeny state: \"%s\"", name, uptown_getMoverStatusAsString(permission), uptown_getChanneldenyStatusAsString(channeldeny));
			}
			free(UID);
			free(name);
		}
		break;
	}
	default:
		data = NULL;  /* Ignore */
		return;
	}

	/* Must be allocated in the plugin! */
	 /* bbCode is supported. HTML is not supported */
	//ts3Functions.freeMemory(str);
}

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);

/*
* Menu IDs for this plugin. Pass these IDs when creating a menuitem to the TS3 client. When the menu item is triggered,
* ts3plugin_onMenuItemEvent will be called passing the menu ID of the triggered menu item.
* These IDs are freely choosable by the plugin author. It's not really needed to use an enum, it just looks prettier.
*/
enum {
	MENU_ID_CLIENT_1 = 1,
	MENU_ID_CLIENT_2,
	MENU_ID_CHANNEL_1,
	MENU_ID_CHANNEL_2,
	MENU_ID_CHANNEL_3,
	MENU_ID_GLOBAL_1,
	MENU_ID_GLOBAL_2
};

/*
* Initialize plugin menus.
* This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
* Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
* If plugin menus are not used by a plugin, do not implement this function or return NULL.
*/
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
	/*
	* Create the menus
	* There are three types of menu items:
	* - PLUGIN_MENU_TYPE_CLIENT:  Client context menu
	* - PLUGIN_MENU_TYPE_CHANNEL: Channel context menu
	* - PLUGIN_MENU_TYPE_GLOBAL:  "Plugins" menu in menu bar of main window
	*
	* Menu IDs are used to identify the menu item when ts3plugin_onMenuItemEvent is called
	*
	* The menu text is required, max length is 128 characters
	*
	* The icon is optional, max length is 128 characters. When not using icons, just pass an empty string.
	* Icons are loaded from a subdirectory in the TeamSpeak client plugins folder. The subdirectory must be named like the
	* plugin filename, without dll/so/dylib suffix
	* e.g. for "test_plugin.dll", icon "1.png" is loaded from <TeamSpeak 3 Client install dir>\plugins\test_plugin\1.png
	*/

	BEGIN_CREATE_MENUS(1);  /* IMPORTANT: Number of menu items must be correct! */
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CLIENT, MENU_ID_CLIENT_1, "Change move/kick permission state", "1.png");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CLIENT, MENU_ID_CLIENT_2, "Add/remove this client from channeldeny", "");
	//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_1, "Channel item 1", "1.png");
	//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_2, "Channel item 2", "2.png");
	//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_3, "Channel item 3", "3.png");
	//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "Global item 1", "1.png");
	//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_2, "Global item 2", "2.png");
	END_CREATE_MENUS;  /* Includes an assert checking if the number of menu items matched */

					   /*
					   * Specify an optional icon for the plugin. This icon is used for the plugins submenu within context and main menus
					   * If unused, set menuIcon to NULL
					   */
	*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
	_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "t.png");



	/*
	* Menus can be enabled or disabled with: ts3Functions.setPluginMenuEnabled(pluginID, menuID, 0|1);
	* Test it with plugin command: /test enablemenu <menuID> <0|1>
	* Menus are enabled by default. Please note that shown menus will not automatically enable or disable when calling this function to
	* ensure Qt menus are not modified by any thread other the UI thread. The enabled or disable state will change the next time a
	* menu is displayed.
	*/
	/* For example, this would disable MENU_ID_GLOBAL_2: */
	/* ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 0); */

	/* All memory allocated in this function will be automatically released by the TeamSpeak client later by calling ts3plugin_freeMemory */
}


/* Helper function to create a hotkey */
static struct PluginHotkey* createHotkey(const char* keyword, const char* description) {
	struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
	_strcpy(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
	_strcpy(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
	return hotkey;
}

/* Some makros to make the code to create hotkeys a bit more readable */
#define BEGIN_CREATE_HOTKEYS(x) const size_t sz = x + 1; size_t n = 0; *hotkeys = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS (*hotkeys)[n++] = NULL; assert(n == sz);

/*
* Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
* Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
* This function is automatically called by the client after ts3plugin_init.
*/
void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys) {
	/* Register hotkeys giving a keyword and a description.
	* The keyword will be later passed to ts3plugin_onHotkeyEvent to identify which hotkey was triggered.
	* The description is shown in the clients hotkey dialog. */
	BEGIN_CREATE_HOTKEYS(4);  /* Create 3 hotkeys. Size must be correct for allocating memory. */
	CREATE_HOTKEY(UPTOWN_HOTKEYSTRING_CHANNEL_MOVE, "Anti channel move");
	CREATE_HOTKEY(UPTOWN_HOTKEYSTRING_CHANNEL_KICK, "Anti channel kick");
	CREATE_HOTKEY(UPTOWN_HOTKEYSTRING_SERVER_KICK, "Anti server kick");
	CREATE_HOTKEY(UPTOWN_HOTKEYSTRING_CHANNELDENY, "Channeldeny");
	END_CREATE_HOTKEYS;

	/* The client will call ts3plugin_freeMemory to release all allocated memory */
}
void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	anyID ownID = 0;
	char *UID;
	uint64 ownChannelID = 0;
	if (ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {
		if (ts3Functions.getChannelOfClient(serverConnectionHandlerID, ownID, &ownChannelID) == ERROR_ok) {
			if (newChannelID == ownChannelID && ownID != clientID) {
				if (channelDenyState) {
					if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &UID) == ERROR_ok) {
						if (uptowndatabase->channeldeny_existsEntry(UID) == UPTOWN_DATABASE_ENTRY_EXISTS) {
							switch (channelDenyState)
							{
							case CHANNELDENY_KICK: {
								ts3Functions.requestClientKickFromChannel(serverConnectionHandlerID, clientID, "Automated kick from channel by uptown_ts3_multitool2", "");
								break;
							}
							case CHANNELDENY_MOVE: {
								ts3Functions.requestClientMove(serverConnectionHandlerID, clientID, oldChannelID, "", "");
								break;
							}
							default:
								break;
							}
						}
					}
				}
			}
		}
	}
}


void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
	int moverStatus = uptowndatabase->allowlist_getMovePermissionState(moverUniqueIdentifier);
	char *UID;
	ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_UNIQUE_IDENTIFIER, &UID);
	printf("UID in client move %s\n", UID);

	if (moverStatus == MOVERSTATUS_ALWAYS_ALLOWED) {
		return;
	}
	if (moverStatus == MOVERSTATUS_NEVER_ALLOWED || isAntiChannelMove) {
		UptownResetFunctions::moveBack(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, moverName);
	}
}
void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	int moverStatus = uptowndatabase->allowlist_getMovePermissionState(kickerUniqueIdentifier);

	if (moverStatus == MOVERSTATUS_ALWAYS_ALLOWED) {
		return;
	}
	if (moverStatus == MOVERSTATUS_NEVER_ALLOWED || isAntiChannelMove) {
		UptownResetFunctions::moveBack(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, kickerName);
	}
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {

	char pw[INFODATA_BUFSIZE] = { '\0' };
	char cHost[INFODATA_BUFSIZE] = { '\0' };
	unsigned short port = 0;
	char *string = NULL;
	char *ownName = NULL;
	anyID ownID;

	if ((error = ts3Functions.getServerConnectInfo(serverConnectionHandlerID, cHost, &port, pw, INFODATA_BUFSIZE)) == ERROR_ok) {
	}

	if (isAntiServerKick) {

		if (error = ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_UNIQUE_IDENTIFIER, &string) == ERROR_ok) {

			if (error = ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_NICKNAME, &ownName) == ERROR_ok) {
				printf("UID in client server kick %s\n", string);
				if (error = ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {

					if (ownID == 0) {
						printf("Kicked from -> Host: %s - Port: %u - Password: %s\n", cHost, port, pw);
						UptownResetFunctions::reconnect(serverConnectionHandlerID, clientID, oldChannelID, string, cHost, port, ownName, pw);
					}
				}
			}

		}
	}
	free(ownName);
	free(string);
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	if (newStatus == STATUS_CONNECTION_ESTABLISHED) {
		char pw[INFODATA_BUFSIZE] = { '\0' };
		char cHost[INFODATA_BUFSIZE] = { '\0' };
		char cIDs[INFODATA_BUFSIZE] = { '\0' };
		unsigned short port = 0;
		anyID ownID = 0;

		if ((error = ts3Functions.getServerConnectInfo(serverConnectionHandlerID, cHost, &port, pw, INFODATA_BUFSIZE)) == ERROR_ok) {
			printf("Connected to -> Host: %s - Port: %u - Password: %s\n", cHost, port, pw);
		}
		if (reconnecting == true) {

			if (error = ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {

				if (error = ts3Functions.requestClientMove(serverConnectionHandlerID, ownID, lastChannelID, "", "") == ERROR_ok) {
					printf("Reconncecting complete.");
				}
			}

			reconnecting = false;
		}

	}

}

//void ts3plugin_onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* path) {
//
//	anyID ownID = 0;
//
//	ts3Functions.getClientID(serverConnectionHandlerID, &ownID);
//	if (ownID != clientID) {
//		return;
//	}
//
//	if (path != NULL) {
//		
//	}
//	else {
//		printf("onAvatarUpdated: %llu %d - deleted\n", (long long unsigned int)serverConnectionHandlerID, clientID);
//		ts3Functions.playWaveFile(serverConnectionHandlerID, "avatar_removed.wav");
//	}
//}

void ts3plugin_onHotkeyEvent(const char* keyword) {

	printf("Uptown: HotkeyEvent:%s\n", keyword);

	if (strcmp(keyword, UPTOWN_HOTKEYSTRING_CHANNEL_MOVE) == 0)
	{
		switchHotkeyStatus(&isAntiChannelMove);
		uptowndatabase->hotkeysettings_changeHotkeySavedState(UPTOWN_HOTKEYSTRING_CHANNEL_MOVE, isAntiChannelMove);
		if (isAntiChannelMove) {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" enabled.");
		}
		else {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" disabled.");
		}
	}
	if (strcmp(keyword, UPTOWN_HOTKEYSTRING_CHANNEL_KICK) == 0)
	{
		switchHotkeyStatus(&isAntiChannelKick);
		uptowndatabase->hotkeysettings_changeHotkeySavedState(UPTOWN_HOTKEYSTRING_CHANNEL_KICK, isAntiChannelKick);
		if (isAntiChannelKick) {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" enabled.");
		}
		else {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" disabled.");
		}
	}
	if (strcmp(keyword, UPTOWN_HOTKEYSTRING_SERVER_KICK) == 0)
	{
		switchHotkeyStatus(&isAntiServerKick);
		uptowndatabase->hotkeysettings_changeHotkeySavedState(UPTOWN_HOTKEYSTRING_SERVER_KICK, isAntiServerKick);
		if (isAntiServerKick) {
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" enabled.");
		}
		else {
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" disabled.");
		}
	}
	if (strcmp(keyword, UPTOWN_HOTKEYSTRING_CHANNELDENY) == 0)
	{
		switchHotkeyStatusChanneldeny(&channelDenyState);
		uptowndatabase->hotkeysettings_changeHotkeySavedState(UPTOWN_HOTKEYSTRING_CHANNELDENY, channelDenyState);
		switch (channelDenyState) {
		case CHANNELDENY_DISABLED:{
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" disabled.");
			break;
			}
		case CHANNELDENY_MOVE: {
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" mode set to move");
			break;
			}
		case CHANNELDENY_KICK: {
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" mode set to kick");
			break;
			}
		default:
			break;
		}
	}
}

/*
* Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
*
* Parameters:
* - serverConnectionHandlerID: ID of the current server tab
* - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
* - menuItemID: Id used when creating the menu item
* - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
*/
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	char *UID = 0;
	int permissionState = 0;
	char ts3PrintMessage[128];
	switch (type) {
	case PLUGIN_MENU_TYPE_GLOBAL:
		/* Global menu item was triggered. selectedItemID is unused and set to zero. */
		switch (menuItemID) {
		case MENU_ID_GLOBAL_1:
			/* Menu global 1 was triggered */
			break;
		case MENU_ID_GLOBAL_2:
			/* Menu global 2 was triggered */
			break;
		default:
			break;
		}
		break;
	case PLUGIN_MENU_TYPE_CHANNEL:
		/* Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel */
		switch (menuItemID) {
		case MENU_ID_CHANNEL_1:
			/* Menu channel 1 was triggered */
			break;
		case MENU_ID_CHANNEL_2:
			/* Menu channel 2 was triggered */
			break;
		case MENU_ID_CHANNEL_3:
			/* Menu channel 3 was triggered */
			break;
		default:
			break;
		}
		break;
	case PLUGIN_MENU_TYPE_CLIENT:
		/* Client contextmenu item was triggered. selectedItemID is the clientID of the selected client */
		switch (menuItemID) {
		case MENU_ID_CLIENT_1:
			if (uptowndatabase->database_initialized) {
				ts3Functions.getClientVariableAsString(serverConnectionHandlerID, selectedItemID, CLIENT_UNIQUE_IDENTIFIER, &UID);
				permissionState = uptowndatabase->allowlist_getMovePermissionState(UID);
				if (permissionState == UPTOWN_DATABASE_ENTRY_NOT_EXISTING) {
					uptowndatabase->allowlist_addEntry(UID, MOVERSTATUS_NEVER_ALLOWED);
					snprintf(ts3PrintMessage, sizeof ts3PrintMessage, "Uptown: Changed move permission state of UID: '%s' from '%s' to '%s' .", UID, uptown_getMoverStatusAsString(UPTOWN_DATABASE_ENTRY_NOT_EXISTING), uptown_getMoverStatusAsString(MOVERSTATUS_NEVER_ALLOWED));
					puts(ts3PrintMessage);
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage);
				}
				else if ((permissionState + 1) < MOVERSTATUS_ENUM_END && permissionState >= MOVERSTATUS_NEVER_ALLOWED) {
					uptowndatabase->allowlist_changeMovePermissionState(UID, (permissionState + 1));
					snprintf(ts3PrintMessage, sizeof ts3PrintMessage, "Uptown: Changed move permission state of UID: '%s' from '%s' to '%s' .", UID, uptown_getMoverStatusAsString(permissionState), uptown_getMoverStatusAsString((permissionState + 1)));
					puts(ts3PrintMessage);
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage);
				}
				else if ((permissionState + 1) == MOVERSTATUS_ENUM_END)
				{
					uptowndatabase->allowlist_removeEntry(UID);
					snprintf(ts3PrintMessage, sizeof ts3PrintMessage, "Uptown: Removed permission state of UID: '%s' .", UID);
					puts(ts3PrintMessage);
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage);
				}
			}
			break;
		case MENU_ID_CLIENT_2:
			if (uptowndatabase->database_initialized) {
				ts3Functions.getClientVariableAsString(serverConnectionHandlerID, selectedItemID, CLIENT_UNIQUE_IDENTIFIER, &UID);
				int entryState = uptowndatabase->channeldeny_existsEntry(UID);
				switch (entryState)
				{
				case UPTOWN_DATABASE_ENTRY_EXISTS: {
					uptowndatabase->channeldeny_removeEntry(UID);
					break;
				}
				case UPTOWN_DATABASE_ENTRY_NOT_EXISTING: {
					uptowndatabase->channeldeny_addEntry(UID);
					break;
				}
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	free(UID);
}

