#pragma once

#ifndef RESETFUNCTIONS_H
#define RESETFUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif


	void moveBack(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, const char* moverName);
	void reconnect(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, const char* uID, const char* ip, unsigned int port, const char* name, const char* password);
	

#ifdef __cplusplus
}
#endif


#endif