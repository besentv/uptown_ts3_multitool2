#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>


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
#include "allowlist_database.h"
#include "allowlist_definitions.h"

#define PLUGIN_API_VERSION 21
#define PLUGIN_VERSION "Beta 1.6.0"

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

struct TS3Functions ts3Functions;

static char* pluginID = NULL;
char* hotkeyStrings[] = { "channel_move", "channel_kick", "server_kick" };
char* password = NULL;

uint64 scHandler = 0;
uint64 lastscH = 0;
uint64 lastChannelID = 0;

unsigned int error = 0;

bool reconnecting = false;
bool isAntiChannelKick = true;
bool isAntiChannelMove = false;
bool isAntiServerKick = true;


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
	uptown_initDatabase();

	return 0;
}

void ts3plugin_shutdown() {
	printf("Uptown: shutdown\n");
	uptown_closeDatabase();
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
	return "Uptown to save your rights on TeamSpeak 3 Servers.";
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}

int ts3plugin_requestAutoload() {
	return 0;
}

void switchHotkeyStatus(bool *hotkey) {

	*hotkey = (!(*hotkey));

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
	//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CLIENT, MENU_ID_CLIENT_2, "Client item 2", "");
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
	BEGIN_CREATE_HOTKEYS(3);  /* Create 3 hotkeys. Size must be correct for allocating memory. */
	CREATE_HOTKEY(hotkeyStrings[0], "Anti channel move");
	CREATE_HOTKEY(hotkeyStrings[1], "Anti channel kick");
	CREATE_HOTKEY(hotkeyStrings[2], "Anti server kick");
	END_CREATE_HOTKEYS;

	/* The client will call ts3plugin_freeMemory to release all allocated memory */
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
	int moverStatus = allowlist_getMovePermissionState(moverUniqueIdentifier);

	if (moverStatus == MOVERSTATUS_ALWAYS_ALLOWED) {
		return;
	}
	if (moverStatus == MOVERSTATUS_NEVER_ALLOWED || isAntiChannelMove) {
		moveBack(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, moverName);
	}
}
void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	int moverStatus = allowlist_getMovePermissionState(kickerUniqueIdentifier);

	if (moverStatus == MOVERSTATUS_ALWAYS_ALLOWED) {
		return;
	}
	if (moverStatus == MOVERSTATUS_NEVER_ALLOWED || isAntiChannelMove) {
		moveBack(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, kickerName);
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

				if (error = ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {

					if (ownID == 0) {
						printf("Kicked from -> Host: %s - Port: %u - Password: %s\n", cHost, port, pw);
						reconnect(serverConnectionHandlerID, clientID, oldChannelID, string, cHost, port, ownName, pw);
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

	if (strcmp(keyword, hotkeyStrings[0]) == 0)
	{
		switchHotkeyStatus(&isAntiChannelMove);
		if (isAntiChannelMove) {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" enabled.");
		}
		else {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" disabled.");
		}
	}
	if (strcmp(keyword, hotkeyStrings[1]) == 0)
	{
		switchHotkeyStatus(&isAntiChannelKick);
		if (isAntiChannelKick) {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" enabled.");
		}
		else {
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" disabled.");
		}
	}
	if (strcmp(keyword, hotkeyStrings[2]) == 0)
	{
		switchHotkeyStatus(&isAntiServerKick);
		if (isAntiServerKick) {
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" enabled.");
		}
		else {
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" disabled.");
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
			if (database_initialized) {
				ts3Functions.getClientVariableAsString(serverConnectionHandlerID, selectedItemID, CLIENT_UNIQUE_IDENTIFIER, &UID);
				permissionState = allowlist_getMovePermissionState(UID);
				if (permissionState == UPTOWN_DATABASE_ENTRY_NOT_EXISTING) {
					allowlist_addEntry(UID, MOVERSTATUS_NEVER_ALLOWED);
					snprintf(ts3PrintMessage, sizeof ts3PrintMessage, "Uptown: Changed move permission state of UID: '%s' from %d to %d .", UID, permissionState, MOVERSTATUS_NEVER_ALLOWED);
					puts(ts3PrintMessage);
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage);
				}
				else if ((permissionState + 1) < MOVERSTATUS_ENUM_END && permissionState >= MOVERSTATUS_NEVER_ALLOWED) {
					allowlist_changeMovePermissionState(UID, (permissionState + 1));
					snprintf(ts3PrintMessage, sizeof ts3PrintMessage, "Uptown: Changed move permission state of UID: '%s' from %d to %d .", UID, permissionState, (permissionState + 1));
					puts(ts3PrintMessage);
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage);
				}
				else if ((permissionState + 1) == MOVERSTATUS_ENUM_END)
				{
					allowlist_removeEntry(UID);
					snprintf(ts3PrintMessage, sizeof ts3PrintMessage, "Uptown: Removed permission state of UID: '%s' .", UID);
					puts(ts3PrintMessage);
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage);
				}
			}
			break;
		case MENU_ID_CLIENT_2:
			/* Menu client 2 was triggered */
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

