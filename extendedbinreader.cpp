#include "extendedbinreader.h"
#include <QDebug>
#include <typeinfo>

ExtendedBinReader::ExtendedBinReader(QIODevice *parent)
    : QDataStream(parent)
{

}

ExtendedBinReader::~ExtendedBinReader()
{

}

qint16 ExtendedBinReader::ReadInt16()
{
    QByteArray buffer;
    int count=2;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);
    delete[] temp;
    return (qint16)(((quint8)buffer[0]&0xFF) | ((quint8)buffer[1] << 8));
}

qint32 ExtendedBinReader::ReadInt32()
{
    QByteArray buffer;
    int count=4;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);
    delete[] temp;
    return (int)(((quint8)buffer[0]&0xFF) | ((quint8)buffer[1]<<8) | ((quint8)buffer[2]<<16) | ((quint8)buffer[3]<<24));
}

qint64 ExtendedBinReader::ReadInt64()
{
    QByteArray buffer;
    int count=8;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);

    uint lo = (uint)((quint8)buffer[0] | ((quint8)buffer[1]) << 8 |
                     ((quint8)buffer[2]) << 16 | (quint8)buffer[3] << 24);

    uint hi = (uint)((quint8)buffer[4] | ((quint8)buffer[5]) << 8 |
                     ((quint8)buffer[6]) << 16 | (quint8)buffer[7] << 24);
    delete[] temp;
    return (long) ((ulong)hi) << 32 | lo;
}

char ExtendedBinReader::ReadChar()
{
    QByteArray buffer;
    int count=1;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);
    delete[] temp;
    return (char)buffer[0];
}

quint8 ExtendedBinReader::ReadByte()
{
    QByteArray buffer;
    int count=1;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);
    delete[] temp;
    return (quint8)buffer[0];
}



void ExtendedBinReader::ReadInt32Array(int *ar, int count)
{
    for (int i = 0; i < count; i++)
    {
        QByteArray buffer;

        int n=4;
        char *temp = new char[n];
        this->readRawData(temp, n);

        buffer.append(temp, n);

        delete [] temp;
        ar[i] = (int)(((quint8)buffer[0]&0xFF) | ((quint8)buffer.at(1)<<8) | ((quint8)buffer[2]<<16) | ((quint8)buffer[3]<<24));
    }
}

void ExtendedBinReader::ReadInt16Array(qint16* ar, int counter)
{
    for (int i = 0; i < counter; i++)
    {
        QByteArray buffer;
        int count=2;
        char *temp = new char[count];
        this->readRawData(temp, count);
        buffer.append(temp, count);
        delete[] temp;
        ar[i] = (short)((quint8)(buffer[0] & 0xFF) | (quint8)buffer[1] << 8);
    }
}

QString ExtendedBinReader::ReadChars(int count)
{
    QByteArray buffer;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);
    delete[] temp;
    return QString(buffer.data());
}
// читает массив char и возвращает указатель на него
void ExtendedBinReader::ReadBytes(char *array, int count)
{
    this->readRawData(array, count);
}
// читает массив байт размером count из бинарного файла и возвращает его
QByteArray ExtendedBinReader::ReadBytesArray(int count)
{
    QByteArray buffer;
    char *temp = new char[count];
    this->readRawData(temp, count);
    buffer.append(temp, count);
    delete[] temp;
    return buffer;
}

void ExtendedBinReader::WriteInt32(int num)
{
    QByteArray ar;
    QDataStream stream(&ar, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << num;
    this->device()->write(ar);
}

