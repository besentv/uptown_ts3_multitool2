#include <string.h>
#include <string>
#include <sstream>
#include <iostream>

#include "plugin_definitions.h"
#include "teamlog/logtypes.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_rare_definitions.h"
#include "ts3_functions.h"

#include "uptown_definitions.h"
#include "uptown_database.h"
#include "uptown.h"

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

void Uptown::getHotkeySettings()
{
	if (database->database_initialized) {
		antiKickHotkeyState = database->hotkeysettings_getHotkeyState(UptownDefinitions::HOTKEYSTRING_CHANNEL_KICK);
		antiMoveHotkeyState = database->hotkeysettings_getHotkeyState(UptownDefinitions::HOTKEYSTRING_CHANNEL_MOVE);
		antiServerKickHotkeyState = database->hotkeysettings_getHotkeyState(UptownDefinitions::HOTKEYSTRING_SERVER_KICK);
		channeldenyHotkeyState = database->hotkeysettings_getHotkeyState(UptownDefinitions::HOTKEYSTRING_CHANNELDENY);
	}
}

void Uptown::moveToPrevousChannel(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, const char * moverName)
{
	int result = 0;
	anyID ownID = 0;	
	ts3Functions.getClientID(serverConnectionHandlerID, &ownID);
	if (ownID != clientID) {
		return;
	}
	std::cout << "Uptown: Moved by \"" << moverName << "\" Trying to move back... " << std::endl;
	if (ts3Functions.getChannelVariableAsInt(serverConnectionHandlerID, oldChannelID, CHANNEL_FLAG_PASSWORD, &result) == ERROR_ok) {
		if (result == 1) {
			std::cout << "insuccessful. Channel is protected by password!" << std::endl;
			return;
		}
		else if (result == 0) {
			if (ts3Functions.requestClientMove(serverConnectionHandlerID, clientID, oldChannelID, "", "") == ERROR_ok) {
				std::cout << "successful." << std::endl;
				return;
			}
			else {
				std::cout << "insuccessful." << std::endl;
				return;
			}
		}
	}
	else
	{
		std::cout << "insuccessful." << std::endl;
	}
}

void Uptown::moveAllToCurrentChannelFrom(uint64 serverConnectionHandlerID, uint64 channelID)
{
	anyID ownID;
	if (ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {
		uint64 currentChannelID;
		if (ts3Functions.getChannelOfClient(serverConnectionHandlerID, ownID, &currentChannelID)== ERROR_ok) {
			anyID *channelList;
			if (ts3Functions.getChannelClientList(serverConnectionHandlerID, channelID ,&channelList) == ERROR_ok) {
				int clientType;
				for (int i = 0; channelList[i]; i++) {
					if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, channelList[i], CLIENT_TYPE, &clientType) == ERROR_ok) {
						if (clientType == 1) {
							continue;
						}
						ts3Functions.requestClientMove(serverConnectionHandlerID, channelList[i], currentChannelID, "", "");
					}
				}
			}
		}
	}

}

Uptown::Uptown(struct TS3Functions ts3Functions, std::string pluginID)
{
	this->ts3Functions = ts3Functions;
	this->pluginID = pluginID;

	char pluginPath[1024];
	this->ts3Functions.getPluginPath(pluginPath, 1024, pluginID.c_str());
	this->pluginPath = pluginPath;
	this->database = new UptownDatabase(this->pluginPath.c_str());
	this->getHotkeySettings();

}

Uptown::~Uptown()
{
	delete database;
}

void Uptown::onHotkeyEvent(std::string hotkeystring)
{
	std::cout << "Uptown: HotkeyEvent:" << hotkeystring << std::endl;

	if (hotkeystring.compare(UptownDefinitions::HOTKEYSTRING_CHANNEL_MOVE) == 0)
	{
		switch (antiMoveHotkeyState) {
		case 0:
			antiMoveHotkeyState = 1;
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" enabled.");
			break;
		case 1:
			antiMoveHotkeyState = 0;
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" disabled.");
			break;
		default:
			antiMoveHotkeyState = 0;
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Move\" disabled.");
		}
		database->hotkeysettings_changeHotkeySavedState(UptownDefinitions::HOTKEYSTRING_CHANNEL_MOVE, antiMoveHotkeyState);
	}
	else if (hotkeystring.compare(UptownDefinitions::HOTKEYSTRING_CHANNEL_KICK) == 0)
	{
		switch (antiKickHotkeyState) {
		case 0:
			antiKickHotkeyState = 1;
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" enabled.");
			break;
		case 1:
			antiKickHotkeyState = 0;
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" disabled.");
			break;
		default:
			antiKickHotkeyState = 0;
			ts3Functions.printMessageToCurrentTab("\"Anti Channel Kick\" disabled.");
		}
		database->hotkeysettings_changeHotkeySavedState(UptownDefinitions::HOTKEYSTRING_CHANNEL_KICK, antiKickHotkeyState);
	}
	else if (hotkeystring.compare(UptownDefinitions::HOTKEYSTRING_SERVER_KICK) == 0)
	{
		switch (antiServerKickHotkeyState) {
		case 0:
			antiServerKickHotkeyState = 1;
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" enabled.");
			break;
		case 1:
			antiServerKickHotkeyState = 0;
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" disabled.");
			break;
		default:
			antiServerKickHotkeyState = 0;
			ts3Functions.printMessageToCurrentTab("\"Anti Server Kick\" disabled.");
		}
		database->hotkeysettings_changeHotkeySavedState(UptownDefinitions::HOTKEYSTRING_SERVER_KICK, antiServerKickHotkeyState);
	}
	else if (hotkeystring.compare(UptownDefinitions::HOTKEYSTRING_CHANNELDENY) == 0)
	{
		switch (channeldenyHotkeyState) {
		case UptownDefinitions::CHANNELDENY_DISABLED:
			channeldenyHotkeyState = UptownDefinitions::CHANNELDENY_MOVE;
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" mode set to move");
			break;
		case UptownDefinitions::CHANNELDENY_MOVE:
			channeldenyHotkeyState = UptownDefinitions::CHANNELDENY_KICK;
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" mode set to kick");
			break;
		case UptownDefinitions::CHANNELDENY_KICK:
			channeldenyHotkeyState = UptownDefinitions::CHANNELDENY_DISABLED;
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" disabled.");
			break;
		default:
			channeldenyHotkeyState = UptownDefinitions::CHANNELDENY_DISABLED;
			ts3Functions.printMessageToCurrentTab("\"Channeldeny\" disabled.");
		}
		database->hotkeysettings_changeHotkeySavedState(UptownDefinitions::HOTKEYSTRING_CHANNELDENY, channeldenyHotkeyState);
	}
}

void Uptown::onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char * moveMessage)
{
	anyID ownID = 0;
	char *UID = 0;
	uint64 ownChannelID = 0;
	if (channeldenyHotkeyState) {
		if (ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {
			if (ts3Functions.getChannelOfClient(serverConnectionHandlerID, ownID, &ownChannelID) == ERROR_ok) {
				if (newChannelID == ownChannelID && ownID != clientID) {
					if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &UID) == ERROR_ok) {
						if (database->channeldeny_existsEntry(UID) == UPTOWN_DATABASE_ENTRY_EXISTS) {
							switch (channeldenyHotkeyState)
							{
							case UptownDefinitions::CHANNELDENY_KICK: {
								ts3Functions.requestClientKickFromChannel(serverConnectionHandlerID, clientID, "Automated kick from channel by uptown_ts3_multitool2", "");
								break;
							}
							case UptownDefinitions::CHANNELDENY_MOVE: {
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

void Uptown::onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char * moverName, const char * moverUniqueIdentifier, const char * moveMessage)
{
	int moverStatus = database->allowlist_getMovePermissionState(moverUniqueIdentifier);
	char *UID;
	if (ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_UNIQUE_IDENTIFIER, &UID) == ERROR_ok) {

		if (moverStatus == UptownDefinitions::MOVERSTATUS_ALWAYS_ALLOWED) {
			return;
		}
		else if (moverStatus == UptownDefinitions::MOVERSTATUS_NEVER_ALLOWED || antiMoveHotkeyState) {
			this->moveToPrevousChannel(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, moverName);
		}
	}
}

void Uptown::onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char * kickerName, const char * kickerUniqueIdentifier, const char * kickMessage)
{
	int moverStatus = database->allowlist_getMovePermissionState(kickerUniqueIdentifier);

	if (moverStatus == UptownDefinitions::MOVERSTATUS_ALWAYS_ALLOWED) {
		return;
	}
	else if (moverStatus == UptownDefinitions::MOVERSTATUS_NEVER_ALLOWED || antiKickHotkeyState) {
		this->moveToPrevousChannel(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, kickerName);
	}
}

void Uptown::onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char * kickerName, const char * kickerUniqueIdentifier, const char * kickMessage)
{
	char password[UPTOWN_INFODATA_BUFSIZE] = { '\0' };
	char ip[UPTOWN_INFODATA_BUFSIZE] = { '\0' };
	unsigned short port = 0;
	char *uID = 0;
	char *ownName = 0;
	anyID ownID;

	if (antiServerKickHotkeyState) {
		if ((ts3Functions.getServerConnectInfo(serverConnectionHandlerID, ip, &port, password, 512)) == ERROR_ok) {
			if (ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_UNIQUE_IDENTIFIER, &uID) == ERROR_ok) {
				if (ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_NICKNAME, &ownName) == ERROR_ok) {
					if (ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {
						if (ownID == 0) {
							std::cout << "Kicked from -> Host:" << ip << " - Port:" << port << " - Password:" << password << std::endl;
							std::stringstream wholeIp;
							wholeIp << ip << ":" << std::to_string(port);
							if (ts3Functions.guiConnect(PLUGIN_CONNECT_TAB_CURRENT, wholeIp.str().c_str(), wholeIp.str().c_str(), password, ownName, NULL, NULL, "uptown_multitool", "uptown_multitool", "uptown_multitool", "default", uID, "", "", &serverConnectionHandlerID) == ERROR_ok) {
								isReconnecting = true;
								lastChannelID = oldChannelID;
							}
						}
					}
				}
			}
		}
	}
	free(ownName);
	free(uID);
}

void Uptown::onMenuItemEvent(uint64 serverConnectionHandlerID, PluginMenuType type, int menuItemID, uint64 selectedItemID)
{
	char *UID = 0;
	int entryState = 0;
	std::stringstream ts3PrintMessage;

	switch (type) {
	case PLUGIN_MENU_TYPE_CHANNEL:
		/* Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel */
		switch (menuItemID) {
		case UptownDefinitions::MENU_ID_CHANNEL_1:
			/* Menu channel 1 was triggered */
			this->moveAllToCurrentChannelFrom(serverConnectionHandlerID, selectedItemID);
			break;
		default:
			break;
		}
		break;
	case PLUGIN_MENU_TYPE_CLIENT:
		/* Client contextmenu item was triggered. selectedItemID is the clientID of the selected client */
		switch (menuItemID) {
		case UptownDefinitions::MENU_ID_CLIENT_1:
			if (database->database_initialized) {
				ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID) selectedItemID, CLIENT_UNIQUE_IDENTIFIER, &UID);
				entryState = database->allowlist_getMovePermissionState(UID);
				if (entryState == UPTOWN_DATABASE_ENTRY_NOT_EXISTING) {
					database->allowlist_addEntry(UID, UptownDefinitions::MOVERSTATUS_NEVER_ALLOWED);
					ts3PrintMessage << "[b]Uptown: Changed move permission state of UID:" << UID << " from '" << UptownDefinitions::getMoverStatusAsString(UPTOWN_DATABASE_ENTRY_NOT_EXISTING) << "' to '" << UptownDefinitions::getMoverStatusAsString(UptownDefinitions::MOVERSTATUS_NEVER_ALLOWED) << "' .[/b]" << std::endl;
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage.str().c_str());
				}
				else if ((entryState + 1) < UptownDefinitions::MOVERSTATUS_ENUM_END && entryState >= UptownDefinitions::MOVERSTATUS_NEVER_ALLOWED) {
					database->allowlist_changeMovePermissionState(UID, (entryState + 1));
					ts3PrintMessage << "[b]Uptown: Changed move permission state of UID:" << UID << " from '" << UptownDefinitions::getMoverStatusAsString(entryState) << "' to '" << UptownDefinitions::getMoverStatusAsString((entryState + 1)) << "' .[/b]" << std::endl;
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage.str().c_str());
				}
				else if ((entryState + 1) == UptownDefinitions::MOVERSTATUS_ENUM_END)
				{
					database->allowlist_removeEntry(UID);
					ts3PrintMessage << "[b]Removed move permission state of UID:" << UID << " .[/b]" << std::endl;
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage.str().c_str());
				}
			}
			break;
		case UptownDefinitions::MENU_ID_CLIENT_2:
			if (database->database_initialized) {
				ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID) selectedItemID, CLIENT_UNIQUE_IDENTIFIER, &UID);
				entryState = database->channeldeny_existsEntry(UID);
				switch (entryState)
				{
				case UPTOWN_DATABASE_ENTRY_EXISTS: {
					database->channeldeny_removeEntry(UID);
					ts3PrintMessage << "[b]Removed UID:" << UID << " from Channeldeny.[/b]" << std::endl;
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage.str().c_str());
					break;
				}
				case UPTOWN_DATABASE_ENTRY_NOT_EXISTING: {
					database->channeldeny_addEntry(UID);
					ts3PrintMessage << "[b]Added UID:" << UID << " to Channeldeny.[/b]" << std::endl;
					ts3Functions.printMessageToCurrentTab(ts3PrintMessage.str().c_str());
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

void Uptown::onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
	if (newStatus == STATUS_CONNECTION_ESTABLISHED) {
		char password[UPTOWN_INFODATA_BUFSIZE] = { '\0' };
		char ip[UPTOWN_INFODATA_BUFSIZE] = { '\0' };
		char cIDs[UPTOWN_INFODATA_BUFSIZE] = { '\0' };
		unsigned short port = 0;
		anyID ownID = 0;

		if (ts3Functions.getServerConnectInfo(serverConnectionHandlerID, ip, &port, password, UPTOWN_INFODATA_BUFSIZE) == ERROR_ok) {
			std::cout << "Connected to -> Host:" << ip << " - Port:" << port << " - Password:" << password << std::endl;
		}
		if (isReconnecting) {
			if (ts3Functions.getClientID(serverConnectionHandlerID, &ownID) == ERROR_ok) {
				if (ts3Functions.requestClientMove(serverConnectionHandlerID, ownID, lastChannelID, "", "") == ERROR_ok) {
					std::cout << "Reconnecting complete." << std::endl;
				}
			}
			isReconnecting = false;
		}
	}
}

void Uptown::infoData(uint64 serverConnectionHandlerID, uint64 id, PluginItemType type, char ** data)
{
	char* str = NULL;
	char* UID = NULL;
	char* name = NULL;
	int permission;
	int channeldeny;
	std::stringstream infoDataMessage;
	switch (type) {
	case PLUGIN_CHANNEL:
		if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_FLAG_DEFAULT, &str) != ERROR_ok) {
			return;
		}
		infoDataMessage << "Channel id is:" << id << " Channel flag default:" << str << std::endl;
		*data = (char*)malloc(UPTOWN_INFODATA_BUFSIZE * sizeof(char));
		_strcpy(*data, UPTOWN_INFODATA_BUFSIZE, infoDataMessage.str().c_str());
		free(str);
		break;
	case PLUGIN_CLIENT: {
		if (database->database_initialized) {
			if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &name) != ERROR_ok) {
				name = "error";
			}
			if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNIQUE_IDENTIFIER, &UID) == ERROR_ok) {
				permission = database->allowlist_getMovePermissionState(UID);
				channeldeny = database->channeldeny_existsEntry(UID);
				*data = (char*)malloc(UPTOWN_INFODATA_BUFSIZE * sizeof(char));
				infoDataMessage << name << "'s move permission state: \"" << UptownDefinitions::getMoverStatusAsString(permission) << "\" - channeldeny state: \"" << UptownDefinitions::getChanneldenyStatusAsString(channeldeny) << "\"";
				_strcpy(*data, UPTOWN_INFODATA_BUFSIZE, infoDataMessage.str().c_str());
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
