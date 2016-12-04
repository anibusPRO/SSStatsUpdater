#ifndef EXTENDEDBINREADER_H
#define EXTENDEDBINREADER_H

#include <QString>
#include <QDataStream>

class ExtendedBinReader : public QDataStream
{
public:

    ExtendedBinReader(QIODevice *parent = 0);
    ~ExtendedBinReader();


    qint16 ReadInt16();
    qint32 ReadInt32();
    qint64 ReadInt64();
    void WriteInt32(int num);

    char ReadChar();
    QString ReadChars(int count);
    quint8 ReadByte();
    quint8 ReadByte(QString comment);
    QByteArray ReadBytesArray(int count);

    void ReadInt32Array(int *ar, int count);
    void ReadInt16Array(qint16 *ar, int counter);
    void ReadBytes(char *array, int count);
};

#endif // EXTENDEDBINREADER_H
