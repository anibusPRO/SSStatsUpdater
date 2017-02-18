#ifndef REQUESTSENDER_H
#define REQUESTSENDER_H
//#define NETWORK_SHOW_SEND_REQUESTS

#include <QNetworkProxy>
#include "request.h"

namespace Network
{
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

        RequestSender(qint64 maxWaitTime = 35000);
        ~RequestSender();

        void setProxy(const QNetworkProxy& proxy);

        QByteArray get(Request& request);
        QByteArray post(Request& request);
        QByteArray getWhileSuccess(Request& request, int maxCount = 2);
        QByteArray postWhileSuccess(Request& request, int maxCount = 2);

        void setMaxWaitTime(qint64 max);

        qint64 maxWaitTime() const;
        RequestError error() const;

    private:
        QByteArray sendRequest(Request& request, bool getRequest = true);
        QByteArray sendWhileSuccess(Request& request, int maxCount = 2, bool getRequest = true);

        qint64 _maxWaitTime;
        RequestError _error;
        QNetworkProxy _proxy;
    };
}

#endif // REQUESTSENDER_H
