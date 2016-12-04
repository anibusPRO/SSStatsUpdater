#ifndef REQUEST_H
#define REQUEST_H
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QFile>

namespace Network
{
    class Request
    {
    public:
        Request(QString address = QString());
        ~Request();

        QString address() const;
        void setAddress(QString address);
        void setFile(QByteArray ar);
        void addParam(QString name, QVariant value);
        bool removeParam(QString name);

        QStringList paramsNames() const;
        QMap<QString, QString> params() const;

        QUrl url(bool withParams = true) const;
        QNetworkRequest request(bool withParams = true)/* const*/;
        QByteArray data(bool forGetRequest = true) const;

    private:
        QByteArray playback;
        QByteArray dataToSend; // byte array to be sent in POST
        QString _address;
        QMap<QString, QString> _params;

    };
}

#endif // REQUEST_H
