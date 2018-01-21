#include "ownradio.h"

OwnRadio::OwnRadio()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data/client");
    db.open();

    /*
    QSqlQuery query;
    query.exec("SELECT recname, deviceid, artist FROM track");

    while(query.next())
    {
        QString recname = query.value(0).toString();
        QString deviceid = query.value(1).toString();
        QString artist = query.value(2).toString();

        qDebug() << recname;
        qDebug() << deviceid;
        qDebug() << artist;
    }*/
}

QByteArray OwnRadio::getNextTrack(QUuid deviceId)
{
    QSqlQuery query;
    query.prepare("SELECT recid, recname, artist, length FROM track");
    query.exec();

    int numberOfRows = 0;
    if(query.last())
    {
        numberOfRows =  query.at() + 1;
        query.first();
        query.previous();
    }

    qDebug() << "SIZE = " << numberOfRows;

    int num = qrand() % numberOfRows;
    query.seek(num);

    QString recid = query.value(0).toString();
    QString recname = query.value(1).toString();
    QString artist = query.value(2).toString();
    QString length = query.value(3).toString();

    qDebug() << recid;
    qDebug() << recname;

    qDebug() << "Device id = " << deviceId.toString();

    QJsonObject track;

    track["id"] = recid;
    track["length"] = length;
    track["name"] = recname;
    track["artist"] = artist;
    track["methodid"] = "1";

    return QJsonDocument(track).toJson(QJsonDocument::Compact);
}

QByteArray OwnRadio::getTrack(QUuid trackId)
{
    QString str = QString("SELECT * FROM track WHERE recid = '%1'")
            .arg(trackId.toString().replace("{", "").replace("}", ""));

    qDebug() << "query = " << str;

    QSqlQuery query;
    query.exec(str);

    QString path;
    while(query.next())
    {
        path = query.value(0).toString() + ".mp3";
        qDebug() << "path = " << path;
    }

    QFile file("cache/" + path);

    file.open(QIODevice::ReadOnly);

    return file.readAll();
}

bool OwnRadio::saveHistory(QUuid deviceId, QUuid trackId, QJsonDocument history)
{
    qDebug() << "deviceId = " << deviceId.toString();
    qDebug() << "trackId = " << trackId.toString();

    QJsonObject entry = history.object();

    qDebug() << "lastListen = " << entry["lastListen"].toString();
    qDebug() << "isListen = " << entry["isListen"].toString();
    qDebug() << "methodid = " << entry["methodid"].toString();

    // TODO: save history to database
    /*
    QSqlQuery query;
    query.prepare("INSERT INTO track(recid, recname, datetimelastlisten, path, artist, length) values(:recid, :recname, :datetimelastlisten, :path, :artist, :length)");

    QString recname = QString::fromLatin1(ref.tag()->title().toCString());
    QString artist = QString::fromLatin1(ref.tag()->artist().toCString());

    qDebug() << recname;
    qDebug() << artist;

    query.bindValue(":recid", guid);
    query.bindValue(":recname", recname);
    query.bindValue(":datetimelastlisten", "00:00:00");
    query.bindValue(":path", "cache/" + guid + ".mp3");
    query.bindValue(":artist", artist);
    query.bindValue(":length", ref.audioProperties()->length());

    qDebug() << query.exec();*/

    return false;
}

bool OwnRadio::loadTrack()
{
    // TODO: release loading

    return false;
}


