#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "sqlite\sqlite3.h"
#include "allowlist_database.h"
#include "allowlist_definitions.h"

#define UPTOWN_DATABASE_FILENAME "uptown_database.db"

#ifdef _WIN32


#endif // _WIN32

sqlite3 *uptownDatabase;

bool database_initialized = false;

int allowlist_empty_callback(void *NotUsed, int argc, char **argv, char **azColName) {
	return 0;
}

void uptown_initDatabase() {
	char *zErrMsg = 0;
	std::stringstream ALLOWLIST_PATH;
	std::stringstream sqlcommand2;
	
	std::stringstream sqlcommand;
	sqlcommand << "CREATE TABLE IF NOT EXISTS uptown_allowlist (uid TEXT PRIMARY KEY NOT NULL,state INTEGER NOT NULL);";
	sqlcommand << "CREATE TABLE IF NOT EXISTS uptown_channeldenylist(uid TEXT PRIMARY KEY NOT NULL);";


	ALLOWLIST_PATH << getenv("APPDATA") << "\\TS3Client\\plugins\\" << UPTOWN_DATABASE_FILENAME;

	if (sqlite3_open(ALLOWLIST_PATH.str().c_str(), &uptownDatabase) != SQLITE_OK) {
		printf("Uptown: SQL Error 0:Can't open database: %s\n", sqlite3_errmsg(uptownDatabase));
	}
	else {
		printf("Uptown: Opened database successfully\n");
		database_initialized = true;
	}
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), allowlist_empty_callback, 0, &zErrMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 1:%s\n", zErrMsg);
	}
}

void uptown_closeDatabase() {
	sqlite3_close(uptownDatabase);
	database_initialized = false;
}

void allowlist_addEntry(const char *UID, int moverStatus){
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "INSERT INTO uptown_allowlist (uid, state) VALUES ('" << UID << "'," << moverStatus << ");";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), allowlist_empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 2:%s\n", errMsg);
	}
	free(errMsg);
}

void allowlist_removeEntry(const char* UID) {
	char *errMsg = 0;
	std::stringstream sqlcommand;
	sqlcommand << "DELETE FROM uptown_allowlist WHERE uid='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlcommand.str().c_str(), allowlist_empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 3:%s\n", errMsg);
	}
}

void allowlist_changeMovePermissionState(const char* UID, int moverStatus) {
	char *errMsg = 0;
	std::stringstream sqlCommand;
	sqlCommand << "UPDATE uptown_allowlist SET state =" << moverStatus << " WHERE uid ='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), allowlist_empty_callback, 0, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 5: %s", *errMsg);
	}
	free(errMsg);
}

int allowlist_getMovePermissionState_callback(void *ret, int argc, char **argv, char **columnName) {
	int i;
	for (i = 0; i<argc; i++) {
		//printf("%s = %s\n", columnName[i], argv[i] ? argv[i] : "NULL");
		if (strcmp(columnName[i], "state") == 0) {
			*((int*)ret) = atoi(argv[i]);
		}
	//printf("\n");
	}
	return 0;
}


/*Retruns permission state as they are listed in allowlist_definitions*/
int allowlist_getMovePermissionState(const char *UID) {
	char *errMsg = 0;
	int *ret = (int *)malloc(sizeof(int));
	*ret = UPTOWN_DATABASE_ENTRY_NOT_EXISTING;
	std::stringstream sqlCommand;
	sqlCommand << "SELECT * FROM uptown_allowlist WHERE uid ='" << UID << "';";
	if (sqlite3_exec(uptownDatabase, sqlCommand.str().c_str(), allowlist_getMovePermissionState_callback, (void*)ret, &errMsg) != SQLITE_OK) {
		printf("Uptown: SQL Error 4: %s", *errMsg);
		return -1;
	}
	int i = *ret;
	free(errMsg);
	free(ret);

	return i;
}

void channeldeny_addEntry(const char *UID) {


}

void channeldeny_removeEntry(const char *UID) {


}

//TODO CHECKSTATE, 