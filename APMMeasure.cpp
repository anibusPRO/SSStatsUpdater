#include "APMMeasure.h"
#include <stdio.h>
#include <QDebug>

APMMeasure::APMMeasure() {
    reset_pending = TRUE;


//    HANDLE CreateFileMapping(
//      HANDLE hFile,           // идентификатор отображаемого файла
//      LPSECURITY_ATTRIBUTES lpFileMappingAttributes, // дескриптор
//                                                     // защиты
//      DWORD flProtect,         // защита для отображаемого файла
//      DWORD dwMaximumSizeHigh, // размер файла (старшее слово)
//      DWORD dwMaximumSizeLow,  // размер файла (младшее слово)
//      LPCTSTR lpName);         // имя отображенного файла
    hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(long), L"APMHook-Shared-Memory");
    lpSharedMemory = (LPLONG)MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);

	resetAllAPM();
}

APMMeasure::~APMMeasure() {
    UnmapViewOfFile(lpSharedMemory);
	CloseHandle(hSharedMemory);
}

long APMMeasure::getTotalActions() {
//    LONG InterlockedExchange( PLONG plTarget, LONG IValue);
//монопольно заменят текущее значение переменной типа LONG,
//адрес которой передается в первом параметре, на значение,
//передаваемое во втором параметре, возвращает значение переменной до замены

	total_actions += InterlockedExchange(lpSharedMemory, 0);

	if(reset_pending && total_actions > 0) {
		current_starttick = absolute_starttick = GetTickCount();
		reset_pending = FALSE;
	}
	return total_actions;
}

void APMMeasure::setTotalActions(long n) {
	total_actions = n;
    InterlockedExchange(lpSharedMemory, 0);
}

// очищает массив действий
void APMMeasure::resetAllAPM() {
	setTotalActions(0);
	absolute_starttick = GetTickCount();

	current_actions_offset = 0;
	current_starttick = absolute_starttick;
	ring_pos = 0;
	for(int i=0;i<RING_SIZE;i++) {
		ring_buffer[i].actions = 0;
		ring_buffer[i].time = MEASURE_CYCLE_LENGTH;
	}
}

// заносит текущее действие в массив действий
void APMMeasure::moveCurrentAPM() {
    // смещаемся к следующему действию
	ring_pos++;
    // если смещение больше размера массива
	if(ring_pos >= RING_SIZE)
        // то обнуляем смещение
		ring_pos = 0;

	long actions = getTotalActions();
	DWORD now = GetTickCount();

	ring_buffer[ring_pos].actions = actions-current_actions_offset;
	ring_buffer[ring_pos].time = now-current_starttick;

	current_actions_offset = actions;
	current_starttick = now;
}

// вычисляет значение APM и возвращает его в виде long
long APMMeasure::computeAPM(long actions, DWORD raw_span) {
    // делим вермя на 1000
	double span = (double)(raw_span)/1000.0;
    // затем делим количество действий на вермя, получаем количествой действий
    // в секунду, умножаем на 60 и получаем количество действий в минуту
    if(span!=0)
        return (long)(((double)actions/span)*60);
    else
        return 0;
}

// получить текущий APM
long APMMeasure::getCurrentAPM() {
    // сумма действий
	long action_sum = 0;
    // продолжительность
	DWORD duration_sum = 0;
	for(int i=0;i<RING_SIZE;i++) {
		action_sum += ring_buffer[i].actions;
		duration_sum += ring_buffer[i].time;
	}
	return computeAPM(action_sum, duration_sum);
}

// получаем средний апм, передав в качестве параметров для функции вычислений APM
// общее количество действий и общее время за которое эти действия были совершены
long APMMeasure::getAverageAPM() {
	return computeAPM(getTotalActions(), GetTickCount()-absolute_starttick);
}

DWORD APMMeasure::getTime()
{
    return GetTickCount()-absolute_starttick;
}

APMLoggableSnapshot APMMeasure::getSnapshot() {
	APMLoggableSnapshot snap;
	snap.valid = !reset_pending;

	snap.snap.time = GetTickCount()-absolute_starttick;
	snap.snap.actions = getTotalActions();
	snap.snap.apm = getCurrentAPM();

	return snap;
}
