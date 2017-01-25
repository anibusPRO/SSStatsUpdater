#pragma once

#include <windows.h>
#include "APMMeasure.h"

#define MAPNAME_LENGTH 32
#define PLAYERNAME_LENGTH 32
#define MAX_PLAYERS 5

typedef struct {
	char file_id[3];
	char version;
	DWORD header_size;

	WORD year;
	char month;
	char day;
	char hour;
	char minute;

	char is_win;
	char map[MAPNAME_LENGTH];
	char players[PLAYERNAME_LENGTH][MAX_PLAYERS];
} APMLogHeader;

class APMLogger {
private:
	HANDLE hFile;
public:
        APMLogger(char filename);
	~APMLogger();

	void addEntry(APMLoggableSnapshot snap);
};
