#include "request.h"
#include <QVariant>
#include <QNetworkRequest>
#include <QStringList>
#include <QDebug>
#include <QBuffer>
#include <QUrlQuery>

Request::Request(QString address /*= QString()*/)
{
    setAddress(address);
}

Request::~Request()
{

}

QString Request::address() const
{
    return _address;
}

void Request::setAddress(QString address)
{
    QUrl u(address, QUrl::StrictMode);
    //QUrlQuery u(address);
    //for (QPair<QString, QString> value : u.queryItems())
        //addParam(value.first, value.second);
    for (QPair<QString, QString> value : QUrlQuery(QUrl(address)).queryItems())
        addParam(value.first, value.second);

    _address = address;
}

bool Request::setFile(QByteArray data, QString name, QString content)
{
    if(data.isEmpty())
        return false;
//    qDebug() << type << name << content;
//    paramFileType=type.toUtf8();
    paramFileName=name.toUtf8();
    paramContentType=content.toUtf8();
    paramData=data;

    return true;
}

void Request::addParam(QString name, QVariant value)
{
    _params[name] = value.toString();
}

bool Request::removeParam(QString name)
{
    if (false == _params.contains(name))
        return false;
    _params.remove(name);
    return true;
}

QStringList Request::paramsNames() const
{
//        QList<QString> list;
//        foreach (QVariant var, _params.keys()) {
//            list.append(var.toString());
//        }
//        QStringList s_list(list);
    return _params.keys();
}

QMap<QString, QString> Request::params() const
{
    return _params;
}

QUrl Request::url(bool forGetRequest /*= true*/) const
{
    QUrl url(address(), QUrl::StrictMode);
    //QUrl url(address());
    if (forGetRequest)
        //url.setEncodedQuery(data(forGetRequest));
        url.setQuery(data(forGetRequest));

    return url;
}

QNetworkRequest Request::request(bool forGetRequest /*= true*/)/* const*/
{
    QNetworkRequest r(url(forGetRequest));

    if (!forGetRequest)
    {

        //задаем разделитель
        QByteArray postData, boundary="1BEF0A57BE110FD467A";
        //параметр 2 - файл
        postData.append("--"+boundary+"\r\n");//разделитель
        //имя параметра
        postData.append("Content-Disposition: form-data; name=\"");
        postData.append("ReplayCRC32");
        postData.append("\"\r\n\r\n");
        //значение параметра
        postData.append(CRC32fromByteArray(paramData));
        postData.append("\r\n");

        //параметр 2 - файл
        postData.append("--"+boundary+"\r\n");//разделитель
        //имя параметра
        postData.append("Content-Disposition: form-data; name=\"file\";");
//        postData.append(paramFileType);
//        //имя файла
        postData.append("filename=\"");
        postData.append(paramFileName);
        postData.append("\"\r\n");
        //тип содержимого файла
        postData.append("Content-Type: "+paramContentType+"\r\n");
        //передаем в base64
        postData.append("Content-Transfer-Encoding: binary\r\n\r\n");
        //данные
        postData.append(paramData);
        postData.append("\r\n");
        //"хвост" запроса
        postData.append("--"+boundary+"--\r\n");

        r.setHeader(QNetworkRequest::ContentTypeHeader,"multipart/form-data; boundary="+boundary);
        r.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(postData.length()));
        dataToSend = postData;
    }

    return r;
}

QByteArray Request::data(bool forGetRequest /*= true*/) const
{
    if(forGetRequest)
    {
        auto b = _params.begin();
        auto e = _params.end();

        QByteArray byteArrayData;

        while (b != e)
        {
            byteArrayData.append(b.key());
            byteArrayData.append('=');
            byteArrayData.append(b.value());
            byteArrayData.append('&');

            b++;
        }

        byteArrayData.chop(1);
        return byteArrayData;
    }
    else
        return dataToSend;
}

QString Request::CRC32fromByteArray( const QByteArray & array )
{
    quint32 crc32 = 0xffffffff;
    QByteArray buf(256, 0);
    qint64 n;
    QBuffer buffer;
    buffer.setData( array );
    if ( !buffer.open( QIODevice::ReadOnly ) )
        return 0;
    while( ( n = buffer.read( buf.data(), 256) ) > 0 )
        for ( qint64 i = 0; i < n; i++ )
            crc32 = ( crc32 >> 8 ) ^ CRC32Table[(crc32 ^ buf.at(i)) & 0xff ];
    buffer.close();
    crc32 ^= 0xffffffff;
    return QString("%1").arg(crc32, 8, 16, QChar('0'));
}
