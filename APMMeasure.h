#pragma once

#include <windows.h>

#include "types.h"
#include <QtCore/QtGlobal>
#include <QCoreApplication>

#define RING_SIZE 20
#define MEASURE_CYCLE_LENGTH 500

typedef struct {
	long	actions;
	DWORD	time;
} APMFrame;

typedef struct {
	long actions;
	DWORD time;
	long apm;
} APMSnapshot;

typedef struct {
	BOOL valid;
	APMSnapshot snap;
} APMLoggableSnapshot;

class LobbyPlayers : public QObject {
public:
  int race;
  int gamesCount;
  int winsCount;
  int winRate;
  int mmr;
  int mmr1v1;
  int apm;
  char name[];
};

class PGameInfo {
public:
    PGameInfo();
    ~PGameInfo();
    bool fontsInited = false;
    LobbyPlayers* lobbyPlayers[8];
    char* players[8][2];
    int playersNumber;
    int downloadProgress;
    char mapName[1];
    PVOID sidsAddr[10];
    long CurrentAPM;
    long AverageAPM;
    long MaxAPM;
    LONG total_actions;
    bool enableDXHook;
    bool showMenu;
    bool showRaces;
    bool showAPM;
    bool sidsAddrLock;
    DWORD statsThrId;
    int version;

};

class APMMeasure {
public:
    APMMeasure();
    ~APMMeasure();
    void moveCurrentAPM();
    long getAverageAPM();
    long getCurrentAPM();
    DWORD getTime();
    long getTotalActions();
    void resetAllAPM();
    APMLoggableSnapshot getSnapshot();

private:
	HANDLE hSharedMemory;
    PGameInfo lpSharedMemory;

	DWORD absolute_starttick;

	long total_actions;
	long current_actions_offset;
	DWORD current_starttick;

    // массив из 20 действий и времени их выполнения
	APMFrame ring_buffer[RING_SIZE];
    // позиция в массив действий
	int ring_pos;

	BOOL reset_pending;

	static long computeAPM(long actions, DWORD starttick);
	void setTotalActions(long n);
};
