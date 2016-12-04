#include "request.h"
#include <QVariant>
#include <QNetworkRequest>
#include <QStringList>
#include <QDebug>

namespace Network
{

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
        QUrl u(address);
        for (QPair<QString, QString> value : u.queryItems())
            addParam(value.first, value.second);
//        for (QPair<QString, QString> value : QUrlQuery(QUrl(address)).queryItems())
//            addParam(value.first, value.second);
        _address = address;
    }

    void Request::setFile(QByteArray ar)
    {
        playback = ar;
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
        QUrl url(address());
        if (forGetRequest)
            url.setEncodedQuery(data(forGetRequest));
//            url.setQuery(data());
        return url;
    }

    QNetworkRequest Request::request(bool forGetRequest /*= true*/)/* const*/
    {
        QNetworkRequest r(url(forGetRequest));

        if (!forGetRequest)
        {
            qDebug() << "file size:" << playback.size();
            QByteArray param1Name="param1" ,param1Value="value1";
            QByteArray param2Name="replay", param2FileName="temp.rec",
                param2ContentType="application/octet-stream", param2Data=playback;

            //задаем разделитель
            QByteArray postData, boundary="1BEF0A57BE110FD467A";
            //первый параметр
            postData.append("--"+boundary+"\r\n");//разделитель
            //имя параметра
            postData.append("Content-Disposition: form-data; name=\"");
            postData.append(param1Name);
            postData.append("\"\r\n\r\n");
            //значение параметра
            postData.append(param1Value);
            postData.append("\r\n");

            //параметр 2 - файл
            postData.append("--"+boundary+"\r\n");//разделитель
            //имя параметра
            postData.append("Content-Disposition: form-data; name=\"");
            postData.append(param2Name);
            //имя файла
            postData.append("\"; filename=\"");
            postData.append(param2FileName);
            postData.append("\"\r\n");
            //тип содержимого файла
            postData.append("Content-Type: "+param2ContentType+"\r\n");
            //передаем в base64
            postData.append("Content-Transfer-Encoding: binary\r\n\r\n");
            //данные
            postData.append(param2Data);
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
        qDebug() << "forGetRequest" << forGetRequest;
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
        else return dataToSend;

    }

}
