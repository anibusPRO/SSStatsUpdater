#ifndef REQUESTSENDER_H
#define REQUESTSENDER_H
//#define NETWORK_SHOW_SEND_REQUESTS

#include <QNetworkProxy>
#include <QSignalMapper>
//#include <QStringListModel>
#include <QUrl>
#include "OSDaB-Zip/unzip.h"
#include "OSDaB-Zip/zip.h"
#include "request.h"


class RequestSender : public QObject
{
    Q_OBJECT
public:

    enum RequestError
    {
        NoError,
        TimeoutError,
        NetworkError
    };

    RequestSender(qint64 maxWaitTime = 35000, QObject * parent = nullptr);
    ~RequestSender();

    void setProxy(const QNetworkProxy& proxy);

    QByteArray get(QString url);
    QByteArray get(Request& request);
    QByteArray post(Request& request);
    QByteArray getWhileSuccess(Request& request, int maxCount = 2);
    QByteArray postWhileSuccess(Request& request, int maxCount = 2);
    static bool compress(const QString& zip, const QString& dir, const QString& pwd);
    static bool decompress(const QString& file, const QString& out, const QString& pwd);
    void setMaxWaitTime(qint64 max);
    qint64 maxWaitTime() const;
    RequestError error() const;
    void setUserAgent(QString agent);
private:
    QByteArray sendRequest(Request& request, bool getRequest = true);
    QByteArray sendWhileSuccess(Request& request, int maxCount = 2, bool getRequest = true);
    qint64 _maxWaitTime;
    RequestError _error;
    QNetworkProxy _proxy;
    QNetworkAccessManager* m_manager;
    QSignalMapper m_mapper;
    QString _userAgent;

public slots:
    void GET_REQUEST(QString url, QString fileName);
    void POST_REQUEST(QString url,
              QString name,
              QString content,
              QByteArray data,
              QString mapping);
private slots:
    void slotFinished(QNetworkReply*);
    void map(QNetworkReply* reply);
    void onFeedRetrieved(const QString &fileName);

signals:
    void send_reply(QByteArray);
    void downloadProgress(qint64,qint64);
};

#endif // REQUESTSENDER_H
