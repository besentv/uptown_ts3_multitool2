#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sstream>
#include <string>

#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin_definitions.h"

#include "plugin.h"
#include "reset_functions.h"

void UptownResetFunctions::moveBack(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, const char* moverName) {

	int result = 0;
	anyID ownID = 0;

	ts3Functions.getClientID(serverConnectionHandlerID, &ownID);
	if (ownID != clientID) {
		return;
	}
	printf("Uptown: Moved by %s .\n Trying to move back... ", moverName);
	if (error = ts3Functions.getChannelVariableAsInt(serverConnectionHandlerID, oldChannelID, CHANNEL_FLAG_PASSWORD, &result) == ERROR_ok) {

		if (result == 1) {
			printf("insuccessful. Channel is protected by password!\n");
			return;
		}
		else if (result == 0) {

			if (error = ts3Functions.requestClientMove(serverConnectionHandlerID, clientID, oldChannelID, "", "") == ERROR_ok) {
				printf("successful.\n");
				return;
			}
			else {
				printf("insucessful.\n");
				return;
			}

		}

	}
	else
	{
		printf("insucessful.\n");
	}

}
void UptownResetFunctions::reconnect(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, const char* uID, const char* ip, unsigned int port, const char* name, const char* password) {
	std::stringstream wholeIp;
	wholeIp << ip << ":" << std::to_string(port);
	printf("UID in client server kick %s\n", uID);
	if (error = ts3Functions.guiConnect(PLUGIN_CONNECT_TAB_CURRENT, wholeIp.str().c_str(), wholeIp.str().c_str(), password, name, NULL, NULL, "uptown_multitool", "uptown_multitool", "uptown_multitool", "default", uID, "", "", &serverConnectionHandlerID) == ERROR_ok) {
			reconnecting = true;
			lastChannelID = oldChannelID;
	}

}