#pragma once

#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef WIN32
#define PLUGINS_EXPORTDLL __declspec(dllexport)
#else
#define PLUGINS_EXPORTDLL __attribute__ ((visibility("default")))
#endif

#define ADRESSES_ARRAY_MAX_SIZE 6
#define INFODATA_BUFSIZE 512

#ifdef __cplusplus
extern "C" {
#endif

	extern struct TS3Functions ts3Functions;
	extern unsigned int error;
	extern bool reconnecting;
	extern uint64 lastscH;
	extern uint64 lastChannelID;

	void switchHotkeyStatus(bool *hotkey);

	PLUGINS_EXPORTDLL const char* ts3plugin_name();
	PLUGINS_EXPORTDLL const char* ts3plugin_version();
	PLUGINS_EXPORTDLL int ts3plugin_apiVersion();
	PLUGINS_EXPORTDLL const char* ts3plugin_author();
	PLUGINS_EXPORTDLL const char* ts3plugin_description();
	PLUGINS_EXPORTDLL void ts3plugin_setFunctionPointers(const struct TS3Functions funcs);
	PLUGINS_EXPORTDLL int ts3plugin_init();
	PLUGINS_EXPORTDLL void ts3plugin_shutdown();
	PLUGINS_EXPORTDLL int ts3plugin_offersConfigure();
	PLUGINS_EXPORTDLL void ts3plugin_registerPluginID(const char* id);
	PLUGINS_EXPORTDLL const char* ts3plugin_commandKeyword();
	PLUGINS_EXPORTDLL const char* ts3plugin_infoTitle();
	PLUGINS_EXPORTDLL void ts3plugin_freeMemory(void* data);
	PLUGINS_EXPORTDLL int ts3plugin_requestAutoload();
	PLUGINS_EXPORTDLL void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys);


	PLUGINS_EXPORTDLL void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage);
	PLUGINS_EXPORTDLL void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage);
	PLUGINS_EXPORTDLL void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage);
	PLUGINS_EXPORTDLL void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
	PLUGINS_EXPORTDLL void ts3plugin_onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* path);
	PLUGINS_EXPORTDLL void ts3plugin_onHotkeyEvent(const char* keyword);

#ifdef __cplusplus
}
#endif

#endif
