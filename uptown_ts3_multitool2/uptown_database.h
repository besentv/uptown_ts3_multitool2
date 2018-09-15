#pragma once
#ifndef UPTOWN_DATABASE
#define UPTOWN_DATABASE

class UptownDatabase
{
public:
	bool database_initialized;
	void closeDatabase();
	void allowlist_addEntry(const char *UID, int moverStatus);
	void allowlist_removeEntry(const char* UID);
	void allowlist_changeMovePermissionState(const char* UID, int moverStatus);
	int allowlist_getMovePermissionState(const char *UID);
	void channeldeny_addEntry(const char *UID);
	void channeldeny_removeEntry(const char *UID);
	int channeldeny_existsEntry(const char* UID);
	void hotkeysettings_changeHotkeySavedState(char *hotkey, int state);
	void hotkeysettings_addEntry(const char *hotkey, int state);
	int hotkeysettings_getHotkeyState(char *hotkey);
	void permaDescription_addEntry(char *UID, char* desc);
	void permaDescription_removeEntry(char *UID);
	char* permaDescription_getEntry(char *UID);

	UptownDatabase(std::string filepath);
	~UptownDatabase();

private:
	struct sqlite3 *uptownDatabase;
	void initDatabase(std::string filepath);
	static int hotkeysettings_getHotkeyState_callback(void *ret, int argc, char **argv, char **columnName);
	static int empty_callback(void *NotUsed, int argc, char **argv, char **azColName);
	static int allowlist_getMovePermissionState_callback(void *ret, int argc, char **argv, char **columnName);
	static int channeldeny_existsEntry_callback(void *ret, int argc, char** argv, char **zArgv);
	static int permadescription_getDescription_callback(void *ret, int argc, char **argv, char **columnName);

};


#endif