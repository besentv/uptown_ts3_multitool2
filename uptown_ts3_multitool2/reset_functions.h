#pragma once

#ifndef RESET_FUNCTIONS_H
#define RESET_FUNCTIONS_H

class UptownResetFunctions
{
public:
	static void moveBack(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, const char* moverName);
	static void reconnect(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, const char* uID, const char* ip, unsigned int port, const char* name, const char* password);

};

#endif