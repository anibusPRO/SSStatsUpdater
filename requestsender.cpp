#include <windows.h>
#include <QTimer>
#include <QEventLoop>
#include <QSharedPointer>
#include <QDebug>
#include <QDateTime>
#include <QtNetwork>
#include "requestsender.h"


void RequestSender::map(QNetworkReply *reply)
{
     m_mapper.map(reply);
}

RequestSender::RequestSender(qint64 maxWaitTime /*= 35000*/, QObject *parent) : QObject{parent}
{
    setMaxWaitTime(maxWaitTime);
    _error = NoError;
    m_manager = new QNetworkAccessManager(this);
//    connect(m_manager, SIGNAL(finished(QNetworkReply*)),
//    this, SLOT(slotFinished(QNetworkReply*)));
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(map(QNetworkReply*)));
    connect(&m_mapper, SIGNAL(mapped(QString)), SLOT(onFeedRetrieved(QString)));

}

void RequestSender::onFeedRetrieved(const QString &fileName)
{
//   qDebug() << "onFeedRetrieved" << QThread::currentThreadId() << fileName;
   auto *pnr = qobject_cast<QNetworkReply*>(m_mapper.mapping(fileName));

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
       QNetworkAccessManager *manager = pnr->manager();
       QNetworkRequest redirection(newUrl);
       QNetworkReply *newReply = manager->get(redirection);
       m_mapper.setMapping(newReply, fileName);
       pnr->deleteLater();
       return;
   }
   else if(!fileName.isEmpty()){
       qDebug() << "Downloading" << fileName << pnr->header(QNetworkRequest::ContentLengthHeader).toInt();
       QByteArray btar = pnr->readAll();
       if(!btar.isEmpty())
       {
           QFile cur_file(fileName);
           if(cur_file.open(QIODevice::WriteOnly)){
               cur_file.write(btar);
               cur_file.close();
               qDebug() << fileName << "downloaded successfully";
               if(fileName.section(".",1,1)=="zip"&&decompress(cur_file.fileName(),QFileInfo(fileName).absolutePath(),""))
               {
                   qDebug() << fileName << "unpacked successfully";
                   QFile::remove(fileName);
               }
           }
           else
               qDebug() << "Could not open" << fileName;
       }
       else
           qDebug() << "Reply is empty";
   }
   else
   {
       QString reply = QString::fromUtf8(pnr->readAll().data());
       if(!reply.isEmpty())
           qDebug() << reply.remove("<br>");
   }
}

RequestSender::~RequestSender()
{
    delete m_manager;
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

//    if (getRequest)
//    {
//        QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), &timer, SLOT(start()));
//        QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
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

void RequestSender::GET_REQUEST(QString url, QString fileName)
{
    Request request;
    request.setAddress(url);
    QNetworkRequest req = request.request();
    req.setRawHeader("User-Agent", "SSStats");

    auto reply = m_manager->get(req);
    // Ensure a unique mapping
//    Q_ASSERT(m_mapper.mapping(dataModel) == nullptr);

//    qDebug() << "setMapping" << reply << fileName;
    m_mapper.setMapping(reply, fileName);
}

void RequestSender::POST_REQUEST(QString url, QString name, QString content, QByteArray data)
{
    Request request;
    request.setAddress(url);
    request.setFile(data,name,content);
    QNetworkRequest req = request.request(false);
    req.setRawHeader("User-Agent", "SSStats");

    m_manager->post(req, request.data(false));
}

void RequestSender::slotFinished(QNetworkReply* pnr)
{
    qDebug() << "slotFinished" << QThread::currentThreadId ();
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
        QNetworkAccessManager *manager = pnr->manager();
        QNetworkRequest redirection(newUrl);
        QNetworkReply *newReply = manager->get(redirection);
        pnr->deleteLater();
        return;
    }
    else
    {
        QString reply = QString::fromUtf8(pnr->readAll().data());
        if(!reply.isEmpty())
            qDebug() << reply.remove("<br>");
    }

    pnr->deleteLater();
}

bool RequestSender::decompress(const QString& file, const QString& out, const QString& pwd)
{
    if (!QFile::exists(file))
    {
        qDebug() << "File does not exist.";
        return false;
    }

    UnZip::ErrorCode ec;
    UnZip uz;

    if (!pwd.isEmpty())
        uz.setPassword(pwd);

    ec = uz.openArchive(file);
    if (ec != UnZip::Ok)
    {
        qDebug() << "Failed to open archive: " << uz.formatError(ec).toLatin1().data();
        return false;
    }

    ec = uz.extractAll(out);
    if (ec != UnZip::Ok)
    {
        qDebug() << "Extraction failed: " << uz.formatError(ec).toLatin1().data();
        uz.closeArchive();
        return false;
    }

    return true;
}

//#ifndef QT_NO_SSL
//void RequestSender::sslErrors(QNetworkReply* reply, const QList<QSslError> &errors)
//{
//    foreach (const QSslError &error, errors) {
//        qDebug() << error.errorString();
//    }

//    reply->ignoreSslErrors();
//}
//#endif

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

