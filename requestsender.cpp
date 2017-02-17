#include <QTimer>
#include <QEventLoop>
#include <QSharedPointer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

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
        QTimer timer;
        timer.setInterval(_maxWaitTime);
        timer.setSingleShot(true);

        QEventLoop loop;
        QSharedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);

        QNetworkRequest req;

        if(getRequest)
            req = request.request();
        else
            req = request.request(getRequest);
        req.setRawHeader("User-Agent", "SSStats");
        QNetworkReply* reply = getRequest ? manager->get(req) :
                                            manager->post(req, request.data(getRequest));
        reply->ignoreSslErrors();

        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

        if (getRequest)
            QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), &timer, SLOT(start()));
        else
            QObject::connect(reply, SIGNAL(uploadProgress(qint64,qint64)), &timer, SLOT(start()));

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
            _error = RequestSender::TimeoutError;
        if(_error!=NoError)
            qDebug() << "Reply Error:"
                     << reply->error() << "\n"
                     << reply->errorString() << "\n"
                     << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                     << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();;

        reply->deleteLater();

        return data;
    }

    QByteArray RequestSender::sendWhileSuccess(Request& request, int maxCount /*= 2*/, bool getRequest /*= true*/)
    {
        int c = 0;
        QByteArray answer;

        while (c < maxCount)
        {
            c++;
            answer = getRequest ? get(request) : post(request);

            if (error() == NoError)
                break;

            qDebug() << "Ошибка при отправке запроса. Код ошибки - " << error() << ". Повторная отправка запроса через 2 секунды\n";
            Sleep(2000);
        }

        return answer;
    }

    bool RequestSender::waitForConnect(int nTimeOutms, QNetworkAccessManager *manager)
    {
//        QTimer *timer = NULL;
//        QEventLoop eventLoop;
//        bool bReadTimeOut = false;

//        m_bReadTimeOut = false;

//        if (nTimeOutms > 0)
//        {
//            timer = new QTimer(this);

//            connect(timer, SIGNAL(timeout()), this, SLOT(slotWaitTimeout()));
//            timer->setSingleShot(true);
//            timer->start(nTimeOutms);

//            connect(this, SIGNAL(signalReadTimeout()), &eventLoop, SLOT(quit()));
//        }

//        // Wait on QNetworkManager reply here
//        connect(manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));

//        if (m_pReply != NULL)
//        {
//            // Preferrably we wait for the first reply which comes faster than the finished signal
//            connect(m_pReply, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
//        }
//        eventLoop.exec();

//        if (timer != NULL)
//        {
//            timer->stop();
//            delete timer;
//            timer = NULL;
//        }

//        bReadTimeOut = m_bReadTimeOut;
//        m_bReadTimeOut = false;

//        return !bReadTimeOut;
    }
}

