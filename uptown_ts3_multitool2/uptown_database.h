#pragma once
#ifndef UPTOWN_DATABASE
#define UPTOWN_DATABASE

extern bool database_initialized;

void uptown_initDatabase(char *filepath);
void uptown_closeDatabase();
void allowlist_addEntry(const char *UID, int moverStatus);
void allowlist_removeEntry(const char* UID);
void allowlist_changeMovePermissionState(const char* UID, int moverStatus);
int allowlist_getMovePermissionState(const char *UID);
void channeldeny_addEntry(const char *UID);
void channeldeny_removeEntry(const char *UID);
int channeldeny_existsEntry(const char* UID);
void uptown_hotkeysettings_changeHotkeySavedState(char *hotkey, int state);
void uptown_hotkeysettings_addEntry(const char *hotkey, int state);
int uptown_hotkeysettings_getHotkeyState(char *hotkey);


#endif