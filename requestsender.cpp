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
    _maxWaitTime = maxWaitTime;
    _error = NoError;
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(map(QNetworkReply*)));
    connect(&m_mapper, SIGNAL(mapped(QString)), SLOT(onFeedRetrieved(QString)));

}

void RequestSender::onFeedRetrieved(const QString &fileName)
{
    auto *pnr = qobject_cast<QNetworkReply*>(m_mapper.mapping(fileName));

    QVariant redirectionTarget = pnr->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (pnr->error() != QNetworkReply::NoError){
        qDebug() << pnr->error()
                 << pnr->errorString()
                 << pnr->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                 << pnr->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    }else if (!redirectionTarget.isNull()) {
        QUrl newUrl = pnr->url().resolved(redirectionTarget.toUrl());
        qDebug() << "Redirection to:" << newUrl.toString();
        QNetworkAccessManager *manager = pnr->manager();
        QNetworkRequest redirection(newUrl);
        QNetworkReply *newReply = manager->get(redirection);
        m_mapper.setMapping(newReply, fileName);
        pnr->deleteLater();
        return;
    }else if(fileName.contains("stats")){
        qDebug() << "Reply from server on stats:";
        QByteArray replyQBAR = pnr->readAll();
        emit send_reply(replyQBAR);
        QString reply = QString::fromUtf8(replyQBAR);
        qDebug() << reply.replace("<br>", "\n");
    }else if(fileName=="logs"){
    }else if(!fileName.isEmpty()){
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
    }else{
        QByteArray replyQBAR = pnr->readAll();
        emit send_reply(replyQBAR);
        QString str_reply = QString::fromUtf8(replyQBAR.data());
        int statusCode = pnr->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << statusCode << str_reply.replace("<br>", "\n");
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

QByteArray RequestSender::get(QString url)
{
    Request request(url);
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
void RequestSender::setUserAgent(QString agent)
{
    _userAgent = agent;
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

    QEventLoop loop;
    QSharedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);

    QNetworkRequest req;

    req = request.request(getRequest);
    req.setRawHeader("User-Agent", _userAgent.toUtf8());
    QNetworkReply* reply = getRequest ?
                    manager->get(req) :
                    manager->post(req, request.data(getRequest));
//    reply->ignoreSslErrors();

    QTimer timout;
    timout.setSingleShot(true);
    connect(&timout, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timout.start(_maxWaitTime);
    loop.exec();

    QByteArray data;
    if (reply->isFinished())
    {
        timout.stop();
        int network_error = reply->error();
        if(reply->error()==QNetworkReply::NoError&&reply->isReadable()){
            _error = NoError;
            data = reply->readAll();
        }
        else{
            _error = NetworkError;
            qDebug() << network_error
                     << reply->errorString()
                     << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                     << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        }
    }
    else{
        reply->abort();
        _error = TimeoutError;
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
    req.setRawHeader("User-Agent", _userAgent.toUtf8());

    auto reply = m_manager->get(req);

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));

    // Ensure a unique mapping
//    Q_ASSERT(m_mapper.mapping(fileName) == nullptr);

//    qDebug() << "setMapping" << reply << fileName;
    m_mapper.setMapping(reply, fileName);
}

void RequestSender::POST_REQUEST(QString url, QString name, QString content, QByteArray data, QString mapping)
{
    Request request;
    request.setAddress(url);
    request.setFile(data,name,content);
    QNetworkRequest req = request.request(false);
    req.setRawHeader("User-Agent", _userAgent.toUtf8());

    auto reply = m_manager->post(req, request.data(false));
    m_mapper.setMapping(reply, mapping);
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

bool RequestSender::compress(const QString& zip, const QString& dir, const QString& pwd)
{
    QFileInfo fi(dir);
    if (!fi.isDir()) {
        qDebug() << "Directory does not exist.";
        return false;
    }

    Zip::ErrorCode ec;
    Zip uz;

    ec = uz.createArchive(zip);
    if (ec != Zip::Ok) {
        qDebug() << "Unable to create archive: " << uz.formatError(ec).toLatin1().data();
        return false;
    }

    uz.setPassword(pwd);
    ec = uz.addDirectoryContents(dir);
    if (ec != Zip::Ok) {
        qDebug() << "Unable to add directory: " << uz.formatError(ec).toLatin1().data();
    }

    uz.setArchiveComment("This archive has been created using OSDaB Zip (http://osdab.42cows.org/).");

    if (uz.closeArchive() != Zip::Ok) {
        qDebug() << "Unable to close the archive: " << uz.formatError(ec).toLatin1().data();
    }

    return ec == Zip::Ok;
}

