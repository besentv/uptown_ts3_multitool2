#pragma once
#ifndef MOVE_ALLOWLIST_HANDLER_H
#define MOVE_ALLOWLIST_HANDLER_H

extern bool database_initialized;

void uptown_initDatabase();
void uptown_closeDatabase();
void allowlist_addEntry(const char *UID, int moverStatus);
void allowlist_removeEntry(const char* UID);
void allowlist_changeMovePermissionState(const char* UID, int moverStatus);
int allowlist_getMovePermissionState(const char *UID);


#endif