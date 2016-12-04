#ifndef REPREADER_H
#define REPREADER_H
#include "replay.h"
#include "player.h"
#include "extendedbinreader.h"
#include <QDataStream>

class RepReader
{
public:
    RepReader();
    ~RepReader();

    Replay *replay;

    bool ReadReplayFully(QDataStream *stream, QString fileName);
    bool ReadHeader(QDataStream *stream, QString fullFileName);

    QString RenameReplay();
    QString RenameReplay(Replay *rep);
    bool isStandart();
    int GetAverageAPM(int id);

    Replay *getReplay() const;
    void setReplay(Replay *value);


private:

    bool OpenFile(QDataStream *stream);
    void ReadPlayer();
    void ReadActionDetail();

    int FindString(QString str, int max_offset);
    QByteArray add_zeros(QString str);
    QString remove_zeros(char *bytes, int len);

    int _lastTick;
    Player *player;
    WinConditions conditions;
    ExtendedBinReader *BinReader;
};
#endif //REPREADER_H
