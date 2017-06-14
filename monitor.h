#ifndef MONITOR_H
#define MONITOR_H

#include <windows.h>
#include <QList>
#include <QString>
#include <QMap>
#include <QObject>

class Monitor : public QObject {
    Q_OBJECT
public:
    Monitor();
    ~Monitor();
    const int PROCESS_WM_READ = 0x0010;
    QMap<QString, QString> PlayersInfo;
    bool finished;

    BYTE steamHeader[18] =  { 0x18, 0x0, 0x0, 0x0, 0x2F, 0x0, 0x73, 0x0, 0x74, 0x0, 0x65, 0x0, 0x61, 0x0, 0x6D, 0x0, 0x2F, 0x0 };

public slots:
    int GetSteamPlayersInfo();

signals:
    void OnNewSteamPairFound(long, QString);

};

#endif // MONITOR_H
