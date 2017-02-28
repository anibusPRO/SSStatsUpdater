#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

//# define NETWORK_SHOW_SEND_REQUESTS

#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include "apmmeter.h"
#include <QCoreApplication>

class StatsCollector : public QObject
{
    Q_OBJECT
public:
    StatsCollector(QObject* pobj=0);
    ~StatsCollector();
    void start();

    // для хранения текущего активного профиля
    // имя папки профиля можно узнать так же из файла Local.ini
//    QString _cur_profile;
    // флаг опредляющий выполнение основного цикла работы программы
    bool stop=false;

private:
    QString get_soulstorm_installlocation();
    QString calcMD5(QString fileName);
    QString calcMD5(QByteArray data);
    bool init_player();
    bool send_stats(QString path_to_profile);
    bool send_logfile();
    int updateUpdater();
    QMap<QString, QString> accounts;
    QString server_addr;
    QString version;
    QCoreApplication *app;
    RequestSender* sender;
    GameInfoReader* reader;
    APMMeter *apm_meter;
    Logger log;

signals:
    void start_meter();
    void sendfile(QString url,
                  QString name,
                  QString content,
                  QByteArray data);
    void get(QString url);

private slots:
//    void slotError ( );
    void slotDone (const QUrl&url, const QByteArray&btr);
};

#endif // STATSCOLLECTOR_H
