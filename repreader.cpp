#include "repreader.h"
#include <QVector>
#include <QDebug>
#include "repreader.h"
#include <QFile>
#include <QStringList>

RepReader::RepReader()
{
    this->BinReader = 0;
}

RepReader::~RepReader()
{
    if(BinReader) delete BinReader;
    if(replay)
    {
        for(int i=0;i<this->replay->Players.size();++i)
            delete this->replay->Players[i];
        for(int i=0;i<this->replay->Actions.size();++i)
            delete this->replay->Actions[i];
        delete replay;
    }
}

Replay *RepReader::getReplay() const
{
    return replay;
}

void RepReader::setReplay(Replay *value)
{
    replay = value;
}


bool RepReader::ReadReplayFully(QDataStream *stream, QString fileName)
{
    qDebug() << "Read Header";
    if (this->ReadHeader(stream, fileName))
    {
        qDebug() << "Read Players";
        for (int players = 0; players < this->replay->Slots; players++)
        {
            this->ReadPlayer();
            this->replay->Players.append(this->player);
        }

//        this->replay->ActionStart = BinReader->device()->pos();
//        qDebug() << "Read Actions";
//        this->ReadActionDetail();
        return true;
    }
    return false;
}

QString RepReader::remove_zeros(char *bytes, int len)
{
    QString temp_str="";
    for(int i=0; i<len; i+=2)
    {
        temp_str += bytes[i];
    }
    return temp_str;
}

int RepReader::GetAverageAPM(int id)
{
    return replay->GetPlayerMidApm(id);
}

// был случай, когда игрок находился в обсах, а другой игрок, с таким же ником был в слоте игрока
// и тогда игрок из обса отправлял статистику, так как программа считала, что обс был в игре
// поэтому решено проверять находился ли игрок с таким же ником в обсах
bool RepReader::playerIsObserver(QString name)
{
    for(int i=0; i<this->replay->PlayerCount; ++i)
        if(this->replay->Players.at(i)->Name==name&&this->replay->Players.at(i)->Type==4)
            return true;
    return false;
}

bool RepReader::ReadHeader(QDataStream *stream, QString fullFileName)
{
    this->replay = new Replay(fullFileName);

    if (this->OpenFile(stream))
    {
        // " версия"
        this->replay->Version = BinReader->ReadInt32();
        // пропускает количество симоволов и имя движка

        auto charactersBuffer = BinReader->ReadChars(31);
        this->replay->MOD = QString(charactersBuffer);
//        qDebug() << this->replay->MOD;

        BinReader->ReadChars(45);

        auto POSTGAMEINFO = (BinReader->ReadChars(12));

        BinReader->ReadChar();
        auto DATADATA = BinReader->ReadChars(8);
//        qDebug() << DATADATA;
        int *intyu = new int[3];
        BinReader->ReadInt32Array(intyu, 3);
        delete[] intyu;

        //"длительность реплея в тиках системных часов"
        this->replay->TotalTicks = BinReader->ReadInt32();
        this->replay->Duration = this->replay->TotalTicks/8; // [c]

        BinReader->ReadChars(12);
        BinReader->ReadInt32();
        BinReader->ReadInt32();
        BinReader->ReadInt32();

        auto FOLDINFO = BinReader->ReadChars(8);
//        qDebug() <<FOLDINFO;
        BinReader->ReadInt32();

        this->replay->BeginFOLDINFO = this->BinReader->device()->pos();
        this->replay->LengthFOLDINFO = BinReader->ReadInt32();

        BinReader->ReadInt32();

        auto GAMEINFO =  BinReader->ReadChars(8);
//        qDebug() <<GAMEINFO;
        BinReader->ReadChar();

        auto FOLDWMAN = BinReader->ReadChars(8);
//        qDebug() <<FOLDWMAN;
        BinReader->ReadInt32();

        this->replay->BeginFOLDWMAN = this->BinReader->device()->pos();
        this->replay->LengthFOLDWMAN = BinReader->ReadInt32();

        int *intsf = new int[8];
        BinReader->ReadInt32Array(intsf, 8);
        delete[] intsf;

        this->replay->PlayerCount = BinReader->ReadInt32();
        this->replay->MapSize = BinReader->ReadInt32();


        // пропускает количество симоволов и имя карты в файле локали
        // "Мод"
        auto tempValue = BinReader->ReadInt32();
        BinReader->ReadChars(tempValue);


        tempValue = BinReader->ReadInt32();
//        char *locale = new char[tempValue*2];
//        BinReader->ReadBytes(locale, tempValue*2);
//        QString mapLocale = remove_zeros(locale, tempValue*2);
//        this->replay->MapLocale = mapLocale;
//        delete[] locale;

        this->replay->MapLocale = QString::fromUtf16((ushort*)BinReader->ReadBytesArray(tempValue*2).data()).left(tempValue);
//        qDebug() << tempValue << this->replay->MapLocale;

        // размер называния карты "название карты"
        tempValue = BinReader->ReadInt32();

        auto mapNameBuffer = BinReader->ReadChars(tempValue);
//        qDebug() << mapNameBuffer;
        QString map_name(mapNameBuffer);
        this->replay->Map = map_name.right(map_name.size() - 18);
        qDebug() << this->replay->Map;
        BinReader->ReadChars(16);
        auto DATABASE = BinReader->ReadChars(8);
        qDebug() <<DATABASE;
        // "Версия chunk"
        BinReader->ReadInt32();

        this->replay->BeginDATABASE = BinReader->device()->pos();
        // "Размер chunk"
        this->replay->LengthDATABASE = BinReader->ReadInt32();

        BinReader->ReadChars(4);

        qint16 *array = new qint16[6];
        BinReader->ReadInt16Array(array, 6);
        delete[] array;
        // "Размер слотов в игре. Всегда 8"
        this->replay->Slots = BinReader->ReadInt32();

//        this->replay->BeginDATABASE = BinReader->device()->pos();
//        this->replay->LengthDATABASE = BinReader->ReadInt32("Длина базы данных");

        // сложность ИИ
        this->replay->settings->AIDiff = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // начальные ресуры
        this->replay->settings->StartRes = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // фиксированные команды
        this->replay->settings->LockTeams = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // читы
        this->replay->settings->CheatsON = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // начальные позиции
        this->replay->settings->StartLocation = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // скорость игры
        this->replay->settings->GameSpeed = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // шаринг ресурсов
        this->replay->settings->ResShare = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        // скорость прироста ресурсов
        this->replay->settings->ResRate = BinReader->ReadInt32();
        /*qDebug() <<*/ BinReader->ReadChars(4);

        BinReader->ReadChar();
        this->replay->BeginNAME = (int)BinReader->device()->pos();
        // "Имя в списке реплеев"
        int nameLength = BinReader->ReadInt32();
//        qDebug() << "nameLength" << nameLength;
        if (nameLength <= 0 || nameLength > 48)
        {
            qDebug() << "Could not read replay name";
            BinReader->ReadInt32();
        }
        else
        {
            replay->Name = QString::fromUtf16((ushort*)BinReader->ReadBytesArray(nameLength*2+4).data())/*.left(nameLength)*/;
//            qDebug() << nameLength << replay->Name;
//            char *cName = new char[nameLength];
//            BinReader->ReadBytes(cName, nameLength*2);
//            replay->Name = remove_zeros(cName, nameLength*2);
//            delete[] cName;
//            for(int i=0; i<nameLength*2;++i)
//            {
//                if(i%2==0)
//                    this->player->Name+=BinReader->ReadChar();
//                else
//                    BinReader->ReadChar();
//            }
//            qDebug() <<replay->Name;
        }

//         пропустим 4 байта до условий победы
//        BinReader->ReadInt32();
        // "Условия победы"
        auto count = BinReader->ReadInt32();

        int *temp_array = new int[count];
        BinReader->ReadInt32Array(temp_array, count);

        QVector<int> win_conditions;
        for(int i=0; i<count; ++i)
            win_conditions.append(temp_array[i]);

        this->replay->conditions->hasAnnihilate = win_conditions.contains((int)WinConditions::Annihilate );
        this->replay->conditions->hasSuddenDeath = win_conditions.contains((int)WinConditions::SuddenDeath);
        this->replay->conditions->hasAssassinate = win_conditions.contains((int)WinConditions::Assassinate);
        this->replay->conditions->hasEconomicVictory = win_conditions.contains((int)WinConditions::EconomicVictory);
        this->replay->conditions->hasControlArea = win_conditions.contains((int)WinConditions::ControlArea);
        this->replay->conditions->hasDestroyHQ = win_conditions.contains((int)WinConditions::DestroyHQ);
        this->replay->conditions->hasTakeAndHold = win_conditions.contains((int)WinConditions::TakeAndHold);
        this->replay->conditions->hasGameTimer = win_conditions.contains((int)WinConditions::GameTimer);

//        if(this->replay->conditions->hasAnnihilate     ) qDebug() << "Annihilate";
//        if(this->replay->conditions->hasSuddenDeath    ) qDebug() << "SuddenDeath";
//        if(this->replay->conditions->hasAssassinate    ) qDebug() << "Assassinate";
//        if(this->replay->conditions->hasEconomicVictory) qDebug() << "EconomicVictory";
//        if(this->replay->conditions->hasControlArea    ) qDebug() << "ControlArea";
//        if(this->replay->conditions->hasDestroyHQ      ) qDebug() << "DestroyHQ";
//        if(this->replay->conditions->hasTakeAndHold    ) qDebug() << "TakeAndHold";
//        if(this->replay->conditions->hasGameTimer      ) qDebug() << "GameTimer";

        BinReader->ReadChars(5);

        this->replay->PlayerStart = BinReader->device()->pos();
        delete[] temp_array;
        return true;
    }
    else
    {
        return false;
    }
}

QByteArray RepReader::add_zeros(QString str)
{
    QByteArray ar;
    ar.resize(str.size()*2+4);

    QDataStream stream(&ar, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    // запишем размер строки
    stream << str.size();
    // запишем символы строки разделяя нулями
    for(int i=0; i<str.size()*2; ++i)
    {
        if(i%2==0)
            ar[i+4] = str[i/2].toAscii();
        else
            ar[i+4] = 0x00;
    }
    return ar;
}

QString RepReader::RenameReplay()
{
    QString ingame_rep_name="";
    QStringList clan_tags;
    int p_count = replay->PlayerCount;
    clan_tags <<"Cg|"<<"AZ|"<<"sF|"<<"HG|"<<"RuW|"<<"SWS}{"<<"Sugar_"<<"[TOXiC]"<<"{KILLA}"<<"[SWS]";
    if(p_count==2)
    {
        for(int i=0; i<p_count; ++i)
        {
            foreach (QString tag, clan_tags) {
                if(replay->Players.at(i)->Name.contains(tag))
                    replay->Players[i]->Name = replay->Players.at(i)->Name.remove(0,tag.size());
            }
        }
        ingame_rep_name += replay->Players.at(0)->getVeryShortRaceName();
        ingame_rep_name += "v"+replay->Players.at(1)->getVeryShortRaceName() + "|";
        ingame_rep_name += replay->getShortMapName() + "|";
        ingame_rep_name += replay->Players.at(0)->Name.left(8);
        ingame_rep_name += "V"+replay->Players.at(1)->Name.left(8);
    }
    else
    {
        for(int i=0; i<p_count; ++i)
            ingame_rep_name += replay->Players.at(i)->getVeryShortRaceName() + "/";
        ingame_rep_name += replay->getShortMapName();
    }

    // найдем строку за которой следует имя реплейя в файле
    BinReader->device()->seek(replay->BeginNAME);

    // получим длину названия реплея в игре
    int rep_name_length = BinReader->ReadInt32();

    // обновим размеры блоков данных в файле
    // посчитаем разницу между старым и новым называнием
    int bytesLengthDifference = (ingame_rep_name.size() - rep_name_length)*2;

    this->BinReader->device()->seek(replay->BeginFOLDINFO);
    this->BinReader->WriteInt32(replay->LengthFOLDINFO+bytesLengthDifference);

    this->BinReader->device()->seek(replay->BeginDATABASE);
    this->BinReader->WriteInt32(replay->LengthDATABASE+bytesLengthDifference);

    this->BinReader->device()->seek(replay->BeginNAME+4+rep_name_length*2);
    QByteArray temp_buffer;
    temp_buffer = this->BinReader->device()->readAll();

    this->BinReader->device()->seek(replay->BeginNAME);

    QByteArray tr = add_zeros(ingame_rep_name);
    BinReader->device()->write(tr);
    BinReader->device()->write(temp_buffer);

    QString rep_filename="",races="",players="";

    for(int i=0; i<p_count; ++i)
    {
        races += replay->Players.at(i)->getVeryShortRaceName();
        players += "#"+replay->Players.at(i)->Name;
    }
    rep_filename += QString::number(p_count)+races+"#"+this->replay->getShortMapName()+players;

    return rep_filename;
}

QString RepReader::RenameReplay(Replay *rep)
{
//    QString rep_filename="";
    QString ingame_rep_name="";

    if(rep->PlayerCount==2)
    {
        QFile file(rep->FullFileName);

        if(file.open(QIODevice::ReadWrite))
        {
            QDataStream in(&file);
            OpenFile(&in);
            for(int i=0; i<rep->PlayerCount; ++i)
            {
                if(rep->Players.at(i)->Name.contains("Cg|")||rep->Players.at(i)->Name.contains("AZ|")
                        ||rep->Players.at(i)->Name.contains("sF|")||rep->Players.at(i)->Name.contains("HG|"))
                    rep->Players[i]->Name = rep->Players.at(i)->Name.remove(0, 3);
//                qDebug() <<"Cg|"<<"AZ|"<<"sF|"<<"HG|";
                if(rep->Players.at(i)->Name.contains("RuW|"))
                    rep->Players[i]->Name = rep->Players.at(i)->Name.remove(0, 4);
                if(rep->Players.at(i)->Name.contains("SWS}{")||rep->Players.at(i)->Name.contains("AZ | "))
                    rep->Players[i]->Name = rep->Players.at(i)->Name.remove(0, 5);
                if(rep->Players.at(i)->Name.contains("Sugar_"))
                    rep->Players[i]->Name = rep->Players.at(i)->Name.remove(0, 6);
                if(rep->Players.at(i)->Name.contains("[TOXiC]")||rep->Players.at(i)->Name.contains("{KILLA}"))
                    rep->Players[i]->Name = rep->Players.at(i)->Name.remove(0, 7);
            }

            ingame_rep_name += rep->Players.at(0)->Name.left(7);
            ingame_rep_name += "V"+rep->Players.at(1)->Name.left(7) + "|";
            ingame_rep_name += rep->Players.at(0)->getShortRaceName();
            ingame_rep_name += "/"+rep->Players.at(1)->getShortRaceName() + "|";
            ingame_rep_name += rep->getShortMapName();

            // найдем строку за которой следует имя реплейя в файле
            BinReader->device()->seek(rep->BeginNAME);

            // получим длину названия реплея в игре
            int rep_name_length = BinReader->ReadInt32();

            // обновим размеры блоков данных в файле
            // посчитаем разницу между старым и новым называнием
            int bytesLengthDifference = (ingame_rep_name.size() - rep_name_length)*2;

            this->BinReader->device()->seek(rep->BeginFOLDINFO);
            this->BinReader->WriteInt32(rep->LengthFOLDINFO+bytesLengthDifference);

            this->BinReader->device()->seek(rep->BeginDATABASE);
            this->BinReader->WriteInt32(rep->LengthDATABASE+bytesLengthDifference);


            this->BinReader->device()->seek(rep->BeginNAME+4+rep_name_length*2);
            QByteArray temp_buffer;
            temp_buffer = this->BinReader->device()->readAll();

            this->BinReader->device()->seek(rep->BeginNAME);

            QByteArray tr = add_zeros(ingame_rep_name);
            BinReader->device()->write(tr);
            BinReader->device()->write(temp_buffer);
            file.close();
        }
    }
    // получим данные о текущей дате
//    QString date = QDateTime::currentDateTime().toString(".yyyy-MM-dd.hh-mm-ss'.png'");

//    rep_filename += QString::number(rep->PlayerCount) + "P";
//    for(int i=0; i<rep->PlayerCount; ++i)
//        rep_filename += "_"+rep->Players.at(i)->Name;
//    for(int i=0; i<rep->PlayerCount; ++i)
//        rep_filename += "_"+rep->Players.at(i)->getShortRaceName();
//    rep_filename += "_"+rep->getShortMapName();

    return QString();
}


bool RepReader::OpenFile(QDataStream *stream)
{
    if(!this->BinReader) this->BinReader = new ExtendedBinReader(stream->device());
    else this->BinReader->setDevice(stream->device());
    return true;
}

bool RepReader::isStandart(int game_type)
{
    if(!replay->conditions->isStandart(game_type))
    {
        qDebug() << "Conditions is not standart";
        replay->conditions->debug();
        return false;
    }
    if(!replay->settings->isStandart(game_type)  )
    {
        qDebug() << "Settings is not standart";
        return false;
    }
    return true;
}

int RepReader::FindString(QString str, int max_offset)
{
    QString temp_str("");
    bool flag=true;
    int j = max_offset;
    while(j)
    {   char temp_char = this->BinReader->ReadChar();
        if(str.contains(temp_char))
        {
            temp_str +=temp_char;
            for(int i = 0; i<str.size()-1; ++i)
            {
                temp_char = this->BinReader->ReadChar();
                if(str.contains(temp_char))
                {
                    temp_str +=temp_char;
                    flag=true;
                }
                else
                {
                    temp_str.clear();
                    flag=false;
                    break;
                }
            }
        }
        else temp_str.clear();
        if(temp_str==str)
            break;
        --j;
    }
    if(flag)
        return this->BinReader->device()->pos()-str.size();
    else
        return 0;
}

void RepReader::ReadPlayer()
{

    this->player = new Player();

    auto charactersBuffer = BinReader->ReadChars(8);
    QString readerString(charactersBuffer);
    if (readerString == "FOLDGPLY")
    {
        // "Версия chunk'а"
        BinReader->ReadInt32();

        this->replay->BeginFOLDGPLYz.append(BinReader->device()->pos());

        this->player->Temp = BinReader->ReadInt32();

        this->player->Bytes = BinReader->ReadChars(16);

        this->replay->BeginPlayersChunkDataSizes.insert(BinReader->device()->pos(), BinReader->ReadInt32());

         BinReader->ReadInt32();

        // "Ниже идет имя игрока"
        auto nameLength = BinReader->ReadInt32();
        player->Name = QString::fromUtf16((ushort*)BinReader->ReadBytesArray(nameLength*2).data()).left(nameLength);
//        qDebug() << nameLength << player->Name;

        // "Тип игрока 0 Host/2 player/4 specc/7 empty/1,3,11 computer"
        this->player->Type = BinReader->ReadInt32();
        // "Номер команды игрока"
        this->player->Team = BinReader->ReadInt32() + 1;

        auto tempValue = BinReader->ReadInt32();

        this->player->Race = BinReader->ReadChars(tempValue);

        BinReader->ReadInt32();
        BinReader->ReadChars(45);

        this->replay->BeginPlayerDiffirences.append(BinReader->device()->pos());

        BinReader->ReadChars(12);
        // если игрок не обозреватель
        if(player->Type!=7)
        {
            charactersBuffer = BinReader->ReadChars(8);
            QString readerString(charactersBuffer);
            if (readerString == "FOLDTCUC")
            {
                auto empB = BinReader->ReadChars(12);

                auto DATALCIN = BinReader->ReadChars(8);

                BinReader->ReadInt32();
                tempValue = BinReader->ReadInt32();
                charactersBuffer = BinReader->ReadChars(tempValue);

                BinReader->ReadInt32();

                auto DATAUNCU = BinReader->ReadChars(8);
//                qDebug() << DATAUNCU;
                BinReader->ReadInt32();
                BinReader->ReadInt32();
                BinReader->ReadInt32();

                tempValue = BinReader->ReadInt32();
                BinReader->ReadChars(tempValue * 2);
                BinReader->ReadChars(20);

                for (int bannerbadge = 0; bannerbadge < 2; bannerbadge++)
                {
                    charactersBuffer = BinReader->ReadChars(8);
                    readerString = charactersBuffer;

                    if (readerString == "FOLDTCBD")
                    {
//                        qDebug() << "FOLDTCBD";
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();


                        charactersBuffer = BinReader->ReadChars(8);
                        readerString = charactersBuffer;

                        BinReader->ReadInt32();
                        BinReader->ReadInt32();

                        tempValue = BinReader->ReadInt32();
                        charactersBuffer = BinReader->ReadChars(tempValue);
                        readerString = charactersBuffer;

                        charactersBuffer = BinReader->ReadChars(8);
                        readerString = charactersBuffer;

                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        // "Высота бэйджа"
                        int height = BinReader->ReadInt32();
                        // "Ширина бэйджа"
                        int width = BinReader->ReadInt32();
                        BinReader->ReadInt32();

                        charactersBuffer = BinReader->ReadChars(8);
                        readerString = charactersBuffer;

                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();

                        for (int y = 0; y < height; y++)
                            for (int x = 0; x < width; x++)
                                BinReader->ReadChars(4);
                    }
                    else if (readerString == "FOLDTCBN")
                    {
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt64();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();

                        tempValue = BinReader->ReadInt32();

                        charactersBuffer = BinReader->ReadChars(tempValue);
                        readerString = charactersBuffer;

                        BinReader->ReadInt64();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        // "Ширина баннера"
                        int width = BinReader->ReadInt32();
                        // "Высота баннера"
                        int height = BinReader->ReadInt32();
                        BinReader->ReadInt32();

                        BinReader->ReadInt64();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();
                        BinReader->ReadInt32();

                        for (int y = 0; y < height; y++)
                            for (int x = 0; x < width; x++)
                                BinReader->ReadChars(4);
                    }
                    else
                    {
                        bannerbadge = 2;
                        BinReader->device()->seek(BinReader->device()->pos()-8);
                    }
                }
            }
            else
            {
                BinReader->device()->seek(BinReader->device()->pos()-8);
            }
        }
    }
    else
    {
        BinReader->device()->seek(BinReader->device()->pos()-8);
    }
}

void RepReader::ReadActionDetail()
{
    while (BinReader->device()->pos() < BinReader->device()->size())
    {
        int action1 = BinReader->ReadInt32();
        int action1Len = BinReader->ReadInt32();

        if (action1==1)
        {
//            qDebug() << "MsgPos:" << BinReader->device()->pos();
            int action2 = BinReader->ReadByte();
            if (action2==1)
            {
//                qDebug() << "Chat Message Type 1:" << BinReader->device()->pos();
                BinReader->ReadBytesArray(3);
                BinReader->ReadInt32();
                BinReader->ReadByte();

                auto tempValue = BinReader->ReadInt32();
                qDebug() << QString::fromUtf16((ushort*)BinReader->ReadBytesArray(tempValue*2).data()).left(tempValue);


                BinReader->ReadInt32();
                BinReader->ReadInt32();
                BinReader->ReadInt32();

                auto mesLength = BinReader->ReadInt32();
                qDebug() << QString::fromUtf16((ushort*)BinReader->ReadBytesArray(mesLength*2).data()).left(mesLength);

            }
            else if (action2==action1Len)
            {
//                qDebug() << "Chat Message Type 2:" << BinReader->device()->pos();
                auto tempValue = BinReader->ReadInt32();
                BinReader->device()->seek(tempValue * 4+16);
            }
            else
            {
//                qDebug() << "Chat Message Type 3:" << BinReader->device()->pos();
                BinReader->ReadChars(3);
                auto tempValue = BinReader->ReadInt32();
                BinReader->ReadByte();
                BinReader->ReadChars(tempValue);
            }
        }
        else if (action1 == 0 && action1Len > 17)
        {
            auto startActionPosition = BinReader->device()->pos();
//            qDebug() << "Read Action" << startActionPosition;
            if (action1Len < 55)
            {
//                qDebug() << "action1Len < 55";
//                /*currentAction->AdditionalInfo = */BinReader->ReadChars(action1Len);
//                currentAction->IsForced = true;
//                this->replay->Actions.append(currentAction);
//                continue;
            }

            // всегда 80
            BinReader->ReadByte();

            _lastTick = BinReader->ReadInt32();

//            qDebug() << 0;
            // number of actions performed in total.
            this->replay->ActionCount = BinReader->ReadInt32();
            // 'random' number generated during the game to calculate whether projectiles hit etc.
            BinReader->ReadInt32();

            auto action_count = BinReader->ReadInt32();

//            qDebug() << 1 << action_count;
            while((--action_count+1)!=0)
            {
                BinReader->ReadInt32();
                BinReader->ReadInt32();
//                qDebug() << 1.1;
                int temp_len = BinReader->ReadInt32();
                BinReader->ReadByte();
//                qDebug() << 1.2 << temp_len;
                int i = 0;
                while( i<temp_len)
                {
//                    qDebug() << 2;
                    int j = 0 ;
                    GameAction *currentAction = new GameAction();
                    currentAction->Tick = _lastTick;
                    auto actionPos = BinReader->device()->pos();

                    auto sub_action_len = BinReader->ReadByte();
                    BinReader->ReadByte();
                    BinReader->ReadInt32();
                    j += 6;
//                    qDebug() << 3 << sub_action_len;
                    currentAction->Kind = BinReader->ReadInt32();
                    currentAction->KindValue = BinReader->ReadInt32();

                    BinReader->ReadByte();
                    j += 9;
                    qint16 pNum1 = BinReader->ReadInt16();
//                    qDebug() << 4 << currentAction->Kind << pNum1;
                    currentAction->PlayerNumber = pNum1 - (qint16)1000;
                    qint16 pNum2 = BinReader->ReadInt16();
//                    qDebug() << "Player Number" << currentAction->PlayerNumber  << pNum2;
//                    if(currentAction->PlayerNumber==0)
//                    {
//                        qDebug() << "SubActionPos:" << BinReader->device()->pos();
//                        qDebug() << "Action:" << currentAction->Kind << currentAction->KindValue;
//                    }

                    currentAction->PlayerNumber2 = pNum2 - (qint16)1000;
                    qint16 pActionNum =  BinReader->ReadInt16();
//                    qDebug() << "Player Number2" << currentAction->PlayerNumber2 << pActionNum;


                    currentAction->PlayerActionNumber = pActionNum;

//                    qDebug() << "ActionData2" << 5;
                    j += 6 ;
                    currentAction->ActionData2 = BinReader->ReadBytesArray(sub_action_len - j);

                    i += sub_action_len;

                    this->replay->Actions.append(currentAction);
                }
            }
        }
        else
            BinReader->ReadBytesArray(action1Len);
    }
}
