#include <QTimer>
#include <QEventLoop>
#include <QSharedPointer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include "requestsender.h"
#include <windows.h>

namespace Network
{
    RequestSender::RequestSender(qint64 maxWaitTime /*= 35000*/)
    {
        setMaxWaitTime(maxWaitTime);
        _error = NoError;
    }

    RequestSender::~RequestSender()
    {

    }

    void RequestSender::setProxy(const QNetworkProxy& proxy)
    {
        _proxy = proxy;
    }

    QByteArray RequestSender::get(Request& request)
    {
        return sendRequest(request, true);
    }

    QByteArray RequestSender::post(Request& request)
    {
        return sendRequest(request, false);
    }

    QByteArray RequestSender::getWhileSuccess(Request& request, int maxCount /*= 2*/)
    {
        return sendWhileSuccess(request, maxCount, true);
    }

    QByteArray RequestSender::postWhileSuccess(Request& request, int maxCount /*= 2*/)
    {
        return sendWhileSuccess(request, maxCount, false);
    }

    void RequestSender::setMaxWaitTime(qint64 max)
    {
        _maxWaitTime = max;
    }

    qint64 RequestSender::maxWaitTime() const
    {
        return _maxWaitTime;
    }

    RequestSender::RequestError RequestSender::error() const
    {
        return _error;
    }

    QByteArray RequestSender::sendRequest(Request& request, bool getRequest /*= true*/)
    {
//        qDebug() << "sendRequest function";
        QTimer timer;
        timer.setInterval(_maxWaitTime);
        timer.setSingleShot(true);

        QEventLoop loop;
        QSharedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
        manager->setProxy(_proxy);
        QNetworkRequest req;

        if(getRequest)
            req = request.request();
        else
            req = request.request(getRequest);
//        qDebug() << "prepare to send complite";
//        qDebug() << "send replay:" << getRequest;
        QNetworkReply* reply = getRequest ? manager->get(req) :
                                            manager->post(req, request.data(getRequest));
//        qDebug() << "send finished";
#if defined(NETWORK_SHOW_SEND_REQUESTS)
        if (getRequest)
            qDebug() << "[GET] " <<  request.request().url().toString();
        else
            qDebug() << "[POST]" << request.request(false).url().toString() << request.data();
#endif

//        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//        QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);

        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        if (getRequest)
            QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), &timer, SLOT(start()));
        else
        {
            QObject::connect(reply, SIGNAL(uploadProgress(qint64,qint64)), &timer, SLOT(start()));
            QObject::connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(progress(qint64,qint64)));
        }

        QObject::connect(&timer, SIGNAL(timeout()), reply, SIGNAL(finished()));

        timer.start();
        loop.exec();

        QByteArray data;

        if (reply->isFinished() && reply->error() == QNetworkReply::NoError)
        {
            data = reply->readAll();
            _error = RequestSender::NoError;
        }
        else
        {
            _error = RequestSender::TimeoutError;
        }
        if(_error!=NoError)
            qDebug() << "TimeoutError";

        reply->deleteLater();

#if defined(NETWORK_SHOW_SEND_REQUESTS)
        qDebug() << "[ANSWER]" << data;
#endif

        return data;
    }

    QByteArray RequestSender::sendWhileSuccess(Request& request, int maxCount /*= 2*/, bool getRequest /*= true*/)
    {
//        if (maxCount < 0)
//            throw QString(__LINE__ + " " __FILE__);

        int c = 0;
        QByteArray answer;

        while (c < maxCount)
        {
            c++;
            answer = getRequest ? get(request) : post(request);

            if (error() == NoError)
                break;

            qDebug() << "Ошибка при отправке запроса. Код ошибки - " << error() << ". Повторная отправка запроса через 2 секунды\n";
//            QThread::currentThread()->msleep(2000);
            Sleep(2000);
        }

        return answer;
    }

    void RequestSender::progress(qint64 bytesSent, qint64 bytesTotal)
    {
        qDebug() << "uploading progress:" << (double)bytesSent*100/(double)bytesTotal <<"%";
    }

}
