#pragma once

#include <windows.h>
#include "..\APMKeyHook\APMKeyHook.h"
#include "APMConfig.h"

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

class APMMeasure {
private:
	HANDLE hSharedMemory;
	LPLONG lpSharedMemory;

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

	long getTotalActions();
	void setTotalActions(long n);
public:
	APMMeasure(APMConfig* n_cfg);
	~APMMeasure();
	void moveCurrentAPM();
	long getAverageAPM();
	long getCurrentAPM();
    long getTime();
    void resetAllAPM();
	APMLoggableSnapshot getSnapshot();
};
