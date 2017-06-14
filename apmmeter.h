#ifndef APMMETER_H
#define APMMETER_H
#include "APMMeasure.h"
#include <QObject>



class APMMeter : public QObject {
    Q_OBJECT

public:
    APMMeter();
    ~APMMeter();
    int init();

    long getAverageAPM();
    long getCurrentAPM();
    long getMaxAPM();
    long getTime();
    long getTotalActions();

    bool stopped;

private:
    long max;
    bool calc_max;
    bool initialized;

    HINSTANCE hinstAPMSharedDll;
    HOOKPROC hprocKeyboard;
    HOOKPROC hprocMouse;
    HHOOK keyboardHook;
    HHOOK mouseHook;
    UINT_PTR timerId;

    APMMeasure* measure ;

public slots:
    void start();
    void stop();
};

#endif // APMMETER_H
