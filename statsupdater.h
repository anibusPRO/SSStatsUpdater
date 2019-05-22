#ifndef SSSTATSUPDATER_H
#define SSSTATSUPDATER_H

#include "..\SSStats\defines.h"
#include "..\SSStats\logger.h"
#include "..\SSStats\systemwin32.h"
#include "..\SSStats\requestsender.h"

#include <QCoreApplication>
#include <QStringList>
#include <QtXml>

class StatsUpdater: public QObject
{
    Q_OBJECT
public:
    StatsUpdater();
    ~StatsUpdater();

    void start();
private:

    Logger log;
    void send_logfile(int version);
    static QString calcMD5(QString fileName);
    static QString calcMD5(QByteArray data);
    static QString CRC32fromIODevice( QIODevice * device );
    static QString CRC32fromByteArray( const QByteArray & array );
    bool updateExecutable(QString name, bool start_after=false);
    void traverseNode(const QDomNode& node, QString tagName, QDomText domText);
    void download_map(QString map_name, QString path);
    QString get_soulstorm_installlocation();
    QString get_steam_id();
    QString server_addr;
    QStringList steam_id64;
    QString sender_steamID;
    QString ss_path;

signals:
    void sendfile(QString url,
                  QString name,
                  QString content,
                  QByteArray data);
    void get(QString url);
};

#endif // SSSTATSUPDATER_H
