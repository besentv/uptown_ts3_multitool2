#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "sqlite\sqlite3.h"
#include "uptown_database.h"
#include "uptown_definitions.h"


UptownDatabase::UptownDatabase(char *filepath)
{
	initDatabase(filepath);
}

UptownDatabase::~UptownDatabase()
{
	closeDatabase();
}

int UptownDatabase::empty_callback(void *NotUsed, int argc, char **argv, char **azColName) {
	return 0;
}

void UptownDatabase::initDatabase(char *filepath) {
	char *zErrMsg = 0;
	std::stringstream ALLOWLIST_PATH;
	std::stringstream sqlcommand2;

	std::stringstream sqlcommand;
	sqlcommand << "CREATE TABLE IF NOT EXISTS uptown_allowlist (uid TEXT PRIMARY KEY NOT NULL,state INTEGER NOT NULL);";
	sqlcommand << "CREATE TABLE IF NOT EXISTS uptown_channeldenylist(uid TEXT PRIMARY KEY NOT NULL);";
	sqlcommand << "CREATE TABLE IF NOT EXISTS uptown_hotkeysettings(hotkey TEXT PRIMARY KEY NOT NULL,enabled INTEGER NOT NULL);";
	sqlcommand << "CREATE TABLE IF NOT EXISTS uptown_permadescription (uid TEXT PRIMARY KEY NOT NULL,description TEXT NOT NULL);";

	ALLOWLIST_PATH << filepath << UPTOWN_DATABASE_FILENAME;

	if (sqlite3_open(ALLOWLIST_PATH.str().c_str(), &uptownDatabase) != SQLITE_OK) {
		printf("Uptown: SQL Error 0:Can't open database: %s\n", sqlite3_errmsg(uptownDatabase));
	}
	else {
		printf("Uptown: Opened database successfully\n");
		database_initialized = true;
	}
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &zErrMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 1:%s\n", zErrMsg);
	}
	if (database_initialized) {
		if (sqlite3_exec(uptownDatabase, sqlcommand2.str().c_str(), empty_callback, 0, &zErrMsg) != SQLITE_OK) {
			printf("Uptown: SQL Error 2:%s\n", zErrMsg);
		}
		hotkeysettings_addEntry(UPTOWN_HOTKEYSTRING_CHANNEL_KICK, 1);
		hotkeysettings_addEntry(UPTOWN_HOTKEYSTRING_CHANNEL_MOVE, 0);
		hotkeysettings_addEntry(UPTOWN_HOTKEYSTRING_SERVER_KICK, 1);
		hotkeysettings_addEntry(UPTOWN_HOTKEYSTRING_CHANNELDENY, 0);
	}
}

void UptownDatabase::closeDatabase() {
	sqlite3_close(uptownDatabase);
	database_initialized = false;
}

void UptownDatabase::allowlist_addEntry(const char *UID, int moverStatus) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "INSERT INTO uptown_allowlist (uid, state) VALUES ('" << UID << "'," << moverStatus << ");";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 3:%s\n", errMsg);
	}
	free(errMsg);
}

void UptownDatabase::allowlist_removeEntry(const char* UID) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "DELETE FROM uptown_allowlist WHERE uid='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 4:%s\n", errMsg);
	}
	free(errMsg);
}

void UptownDatabase::allowlist_changeMovePermissionState(const char* UID, int moverStatus) {
	char *errMsg = 0;
	std::stringstream sqlCommand;
	sqlCommand << "UPDATE uptown_allowlist SET state =" << moverStatus << " WHERE uid ='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 5: %s", errMsg);
	}
	free(errMsg);
}

int UptownDatabase::allowlist_getMovePermissionState_callback(void *ret, int argc, char **argv, char **columnName) {
	int i;
	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", columnName[i], argv[i] ? argv[i] : "NULL");
		if (strcmp(columnName[i], "state") == 0) {
			*((int*)ret) = atoi(argv[i]);
		}
		//printf("\n");
	}
	return 0;
}


/*Retruns permission state as they are listed in allowlist_definitions*/
int UptownDatabase::allowlist_getMovePermissionState(const char *UID) {
	char *errMsg = 0;
	int *ret = (int *)malloc(sizeof(int));
	*ret = UPTOWN_DATABASE_ENTRY_NOT_EXISTING;
	std::stringstream sqlCommand;
	sqlCommand << "SELECT * FROM uptown_allowlist WHERE uid ='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), allowlist_getMovePermissionState_callback, (void*)ret, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 6: %s", errMsg);
		return -1;
	}
	int i = *ret;
	free(errMsg);
	free(ret);

	return i;
}

void UptownDatabase::channeldeny_addEntry(const char *UID) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "INSERT INTO uptown_channeldenylist (uid) VALUES ('" << UID << "')";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 7:%s\n", errMsg);
	}
	free(errMsg);
}

void UptownDatabase::channeldeny_removeEntry(const char *UID) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "DELETE FROM uptown_channeldenylist WHERE uid='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 8:%s\n", errMsg);
	}
	free(errMsg);
}

int UptownDatabase::channeldeny_existsEntry_callback(void *ret, int argc, char** argv, char **zArgv) {
	*(int*)ret = UPTOWN_DATABASE_ENTRY_EXISTS;
	return 0;
}

int UptownDatabase::channeldeny_existsEntry(const char* UID) {
	char *errMsg = 0;
	int *ret = (int *)malloc(sizeof(int));
	*ret = UPTOWN_DATABASE_ENTRY_NOT_EXISTING;
	std::stringstream sqlCommand;
	sqlCommand << "SELECT * FROM uptown_channeldenylist WHERE uid ='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), channeldeny_existsEntry_callback, (void*)ret, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 9: %s\n", errMsg);
		*ret = -1;
	}
	int i = *ret;
	free(errMsg);
	free(ret);

	return i;
}

void UptownDatabase::hotkeysettings_changeHotkeySavedState(char *hotkey, int state) {
	char* errMsg = 0;
	std::stringstream sqlCommand;
	sqlCommand << "UPDATE uptown_hotkeysettings SET enabled = " << state << " WHERE hotkey = '" << hotkey << "'; ";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 10: %s\n", errMsg);
	}
	free(errMsg);
}

void UptownDatabase::hotkeysettings_addEntry(const char *hotkey, int state) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "INSERT INTO uptown_hotkeysettings (hotkey,enabled) VALUES ('" << hotkey << "'," << state << ");";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 11:%s\n", errMsg);
	}
	free(errMsg);
}

int UptownDatabase::hotkeysettings_getHotkeyState_callback(void *ret, int argc, char **argv, char **columnName) {
	int i;
	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", columnName[i], argv[i] ? argv[i] : "NULL");
		if (strcmp(columnName[i], "enabled") == 0) {
			if (argv[i]) {
				*((int*)ret) = atoi(argv[i]);
			}
			else {
				*(int*)ret = 0;
			}
		}
		//printf("\n");
	}
	return 0;
}

int UptownDatabase::hotkeysettings_getHotkeyState(char *hotkey) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	int *ret = (int *)malloc(sizeof(int));
	sqlcommand << "SELECT * FROM uptown_hotkeysettings WHERE hotkey ='" << hotkey << "';";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), hotkeysettings_getHotkeyState_callback, (void*)ret, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 12: %s", errMsg);
		return -1;
	}
	int i = *ret;
	free(errMsg);
	free(ret);

	return i;
}

void UptownDatabase::permaDescription_addEntry(char * UID, char * desc)
{
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "INSERT INTO uptown_permadescription (uid, soundfileuri) VALUES ('" << UID << "','" << desc << "');";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 13:%s\n", errMsg);
	}
	free(errMsg);
}

void UptownDatabase::permaDescription_removeEntry(char * UID)
{
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "DELETE FROM uptown_permadescription WHERE uid='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 14:%s\n", errMsg);
	}
	free(errMsg);
}

int UptownDatabase::permadescription_getDescription_callback(void * ret, int argc, char ** argv, char ** columnName)
{
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", columnName[i], argv[i] ? argv[i] : "NULL");
		if (strcmp(columnName[i], "description") == 0) {
			char **res_str = (char**)ret;
			*res_str = (char*)realloc(*res_str, sizeof(*res_str));
			strcpy_s(*res_str, sizeof(char) * 512, argv[i]);
			//strcpy(*res_str, argv[i]);
		}
		printf("\n");
	}
	return 0;
}

char* UptownDatabase::permaDescription_getEntry(char * UID)
{
	char *errMsg = 0;
	char *ret = (char*) malloc(sizeof(char)*512);
	std::stringstream sqlCommand;
	sqlCommand << "SELECT * FROM uptown_permadescription WHERE uid ='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), permadescription_getDescription_callback, &ret, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 15: %s", errMsg);
		return ret;
	}
	free(errMsg);

	return ret;
}



