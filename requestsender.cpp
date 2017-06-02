#include <windows.h>
#include <QTimer>
#include <QEventLoop>
#include <QSharedPointer>
#include <QDebug>
#include <QDateTime>
#include <QtNetwork>
#include "requestsender.h"

RequestSender::RequestSender(qint64 maxWaitTime /*= 35000*/)
{
    setMaxWaitTime(maxWaitTime);
    _error = NoError;
    file = nullptr;
    m_pnam = new QNetworkAccessManager(this);
    connect(m_pnam, SIGNAL(finished(QNetworkReply*)),
    this, SLOT(slotFinished(QNetworkReply*)));
}

RequestSender::~RequestSender()
{
    if(file!=nullptr)
        delete file;
    qDebug() << "RequestSender destructor";
}

void RequestSender::setProxy(const QNetworkProxy& proxy)
{
    _proxy = proxy;
}

//QByteArray *RequestSender::getFile()
//{
//    return file;
//}

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
//    QTimer timer;
//    timer.setInterval(_maxWaitTime);
//    timer.setSingleShot(true);

    QEventLoop loop;
    QSharedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);

    QNetworkRequest req;

    req = request.request(getRequest);
    req.setRawHeader("User-Agent", "SSStats");
    QNetworkReply* reply = getRequest ?
                    manager->get(req) :
                    manager->post(req, request.data(getRequest));

    reply->ignoreSslErrors();

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    if (getRequest)
//    {
//        QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), &timer, SLOT(start()));
        QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
//        QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
//    }
//    else
//        QObject::connect(reply, SIGNAL(uploadProgress(qint64,qint64)), &timer, SLOT(start()));

//    QObject::connect(&timer, SIGNAL(timeout()), reply, SIGNAL(finished()));

//    timer.start();
    loop.exec();

    QByteArray data;
    if (reply->isFinished())
    {
        int network_error = reply->error();
        if(network_error==QNetworkReply::NoError&&reply->isReadable()){
            _error = NoError;
            data = reply->readAll();
        }
        else{
            _error = NetworkError;
            qDebug() << "\n Network Error:"
                     << network_error << "\n"
                     << reply->errorString() << "\n"
                     << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                     << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()
                     << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        }
    }
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
        if(error()==1)
            qDebug() << "TimeoutError";
        if(error()==2)
            qDebug() << "NetworkError";
        Sleep(2000);
    }

    return answer;
}

void RequestSender::updateProgress(qint64 bytesSent, qint64 bytesTotal)
{
//    qDebug() << (int)(100*bytesSent/bytesTotal;
}

void RequestSender::get(QString url)
{
    Request request;
    request.setAddress(url);
    QNetworkRequest req = request.request();
    req.setRawHeader("User-Agent", "SSStats");
//    qDebug() << req.header(QNetworkRequest::ContentLengthHeader).toString();
//    qDebug() << req.header(QNetworkRequest::ContentTypeHeader).toString();
//    foreach(QByteArray str, req.rawHeaderList())
//        qDebug() << QString::fromUtf8(str.data());
    m_pnam->get(req);
}

void RequestSender::post(QString url, QString name, QString content, QByteArray data)
{
    Request request;
    request.setAddress(url);
    request.setFile(data,name,content);
//    qDebug() << data.size() <<
    QNetworkRequest req = request.request(false);

//    QSslConfiguration config = req;
//    QList<QSslCertificate> certs =
//                          QSslCertificate::fromPath("pistopoiitiko.crt");
//    config.setCaCertificates(certs);
//    request.setSslConfiguration(config);
    req.setRawHeader("User-Agent", "SSStats");
//    qDebug() << req.header(QNetworkRequest::ContentLengthHeader).toString();
//    qDebug() << req.header(QNetworkRequest::ContentTypeHeader).toString();
//    foreach(QByteArray str, req.rawHeaderList())
//        qDebug() << QString::fromUtf8(str.data());
//    qDebug() << QString::fromUtf8(request.data(false).data());
    m_pnam->post(req, request.data(false));
}

void RequestSender::slotFinished(QNetworkReply* pnr)
{
    QVariant redirectionTarget = pnr->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (pnr->error() != QNetworkReply::NoError){
        qDebug() << pnr->error()
                 << pnr->errorString()
                 << pnr->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                 << pnr->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()
                 << pnr->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    }
    else if (!redirectionTarget.isNull()) {
        QUrl newUrl = pnr->url().resolved(redirectionTarget.toUrl());
        emit get(newUrl.toString());
        pnr->deleteLater();
        return;
    }

//    if(file!=nullptr)
//        delete file;
//    if(pnr->isReadable())
//        file = new QByteArray(pnr->readAll());
    QString reply = QString::fromUtf8(pnr->readAll().data());
    if(!reply.isEmpty())
        qDebug() << reply.remove("<br/>");

    pnr->deleteLater();

//    QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
//    statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));

//    foreach(QByteArray str, pnr->rawHeaderList())
//        qDebug() << QString::fromUtf8(str.data());
//    qDebug() << pnr->size() << pnr->url();
//    else
//        emit done(pnr->url(), pnr->readAll());
//    qDebug() << pnr->isReadable() << pnr->isFinished();

//


}
//void HttpWindow::httpFinished()
//{
//    if (httpRequestAborted) {
//        if (file) {
//            file->close();
//            file->remove();
//            delete file;
//            file = 0;
//        }
//        reply->deleteLater();
//        progressDialog->hide();
//        return;
//    }

//    progressDialog->hide();
//    file->flush();
//    file->close();

//    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
//    if (reply->error()) {
//        file->remove();
//        QMessageBox::information(this, tr("HTTP"),
//                                 tr("Download failed: %1.")
//                                 .arg(reply->errorString()));
//        downloadButton->setEnabled(true);
//    } else if (!redirectionTarget.isNull()) {
//        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
//        if (QMessageBox::question(this, tr("HTTP"),
//                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
//                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
//            url = newUrl;
//            reply->deleteLater();
//            file->open(QIODevice::WriteOnly);
//            file->resize(0);
//            startRequest(url);
//            return;
//        }
//    } else {
//        QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
//        statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));
//        downloadButton->setEnabled(true);
//    }

//    reply->deleteLater();
//    reply = 0;
//    delete file;
//    file = 0;
//}


//#ifndef QT_NO_SSL
//void RequestSender::sslErrors(QNetworkReply* reply, const QList<QSslError> &errors)
//{
//    foreach (const QSslError &error, errors) {
//        qDebug() << error.errorString();
//    }

//    reply->ignoreSslErrors();
//}
//#endif

