#pragma once
#ifndef UPTOWN
#define UPTOWN

class Uptown
{
public:

	void onHotkeyEvent(std::string hotkeystring);
	void onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
	void onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage);
	void onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage);
	void onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage);
	void onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);
	void onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
	void infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data);

	Uptown(struct TS3Functions ts3Functions,std::string pluginID);
	~Uptown();

private:
	void getHotkeySettings();
	void moveToPrevousChannel(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, const char* moverName);
	void moveAllToCurrentChannelFrom(uint64 serverConnectionHandlerID, uint64 channelID);

	struct TS3Functions ts3Functions;

	UptownDatabase *database;
	std::string pluginID;
	std::string pluginPath;

	int antiKickHotkeyState;
	int antiMoveHotkeyState;
	int antiServerKickHotkeyState;
	int channeldenyHotkeyState;

	bool isReconnecting;
	uint64 lastChannelID;
};

#endif
