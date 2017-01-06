#include <stdio.h>
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
#include "ResetFunctions.h"

#define PLUGIN_API_VERSION 21
#define PLUGIN_VERSION "Beta 1.5.2"

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

	return 0;
}

void ts3plugin_shutdown() {
	printf("Uptown: shutdown\n");
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
	if (isAntiChannelMove) {
		moveBack(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, moverName);
	}
}
void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	if (isAntiChannelKick) {
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

	printf("HotkeyEvent:%s\n", keyword);

	if(strcmp(keyword, hotkeyStrings[0]) == 0)
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
