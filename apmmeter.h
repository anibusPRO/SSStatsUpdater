#ifndef APMMETER_H
#define APMMETER_H
#include "APMShared\APMTrigger.h"
#include "APMShared\APMLogger.h"
#include "APMShared\APMMeasure.h"
#include <QObject>

class APMMeter : public QObject {
    Q_OBJECT

public:
    APMMeter();
    ~APMMeter();

    void stop();
    int init();
    void mainCycle();
    long getAverageAPM();
    long getCurrentAPM();
    long getMaxAPM();
    long getTime();
    long getTotalActions();

    APMLogger* logger;
    HINSTANCE hinstAPMSharedDll;
    HOOKPROC hprocKeyboard;
    HOOKPROC hprocMouse;
    HHOOK keyboardHook;
    HHOOK mouseHook;
    UINT_PTR timerId;

    APMConfig*  cfg     ;
    APMTrigger* trigger ;
    APMMeasure* measure ;

    bool stop_flag;

private:
    long max;
    bool calc_max;

public slots:
    void start();
};

#endif // APMMETER_H
