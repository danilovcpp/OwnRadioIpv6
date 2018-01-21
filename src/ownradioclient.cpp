#include "ownradioclient.h"

OwnRadioClient::OwnRadioClient(const QString& host, const QString& deviceId) :
    host(host),
    deviceId(deviceId)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data/client");
    db.open();

    QSqlQuery query;
    query.exec("SELECT recid, recname, artist FROM track");

    while(query.next())
    {
        QString recid = query.value(0).toString();
        QString recname = query.value(1).toString();
        QString artist = query.value(2).toString();

        qDebug() << recid;
        qDebug() << recname;
        qDebug() << artist;
    }

    manager = new QNetworkAccessManager(this);
    player = new QMediaPlayer(this);
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

    this->cacheTrack();
}

void OwnRadioClient::getNextTrack()
{
    qDebug() << "getNextTrack()";

    getNextTrackReply = manager->get(QNetworkRequest(QUrl(host + "v3/tracks/" + deviceId + "/next")));

    connect(getNextTrackReply, SIGNAL(finished()), this, SLOT(getNextTrackSlot()));
}

void OwnRadioClient::getTrack(const QString& trackId)
{
    qDebug() << "getTrack()";

    getTrackReply = manager->get(QNetworkRequest(QUrl(host + "v3/tracks/" + trackId)));

    connect(getTrackReply, SIGNAL(finished()), this, SLOT(getTrackSlot()));
}

void OwnRadioClient::saveHistory(const QString& trackId)
{
    qDebug() << "saveHistory()";

    QJsonObject data;
    data["lastListen"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    data["isListen"] = 1; // TODO
    data["methodid"] = 4; // TODO

    QJsonDocument doc(data);
    QString json(doc.toJson(QJsonDocument::Compact));

    qDebug() << json;

    QUrl url(host + "v3/histories/" + deviceId + "/" + trackId);
    QNetworkRequest request(url);

    request.setRawHeader("User-Agent", "ownRadio Qt client");
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Content-Length", QByteArray::number(json.size()));

    saveHistoryReply = manager->post(request, doc.toJson(QJsonDocument::Compact));
    connect(saveHistoryReply, SIGNAL(finished()), this, SLOT(saveHistorySlot()));
}

void OwnRadioClient::uploadTrack(const QString& path)
{
    qDebug() << "uploadTrack()";

    QString fileGuid = QUuid::createUuid().toString().remove("{").remove("}");
    qDebug() << fileGuid;

    QHttpMultiPart *data = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart guidPart;
    guidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"fileGuid\""));
    guidPart.setBody(fileGuid.toUtf8());

    QHttpPart pathPart;
    pathPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"filePath\""));
    pathPart.setBody(path.toUtf8());

    QHttpPart devicePart;
    devicePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"deviceId\""));
    devicePart.setBody(this->deviceId.toUtf8());

    QHttpPart musicPart;
    musicPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"musicFile\"; filename=\""+ fileGuid + "\""));

    QFile *file = new QFile(path);
    file->open(QIODevice::ReadOnly);
    musicPart.setBodyDevice(file);

    data->append(guidPart);
    data->append(pathPart);
    data->append(devicePart);
    data->append(musicPart);

    QUrl url(host + "v3/tracks");
    QNetworkRequest request(url);

    uploadTrackReply = manager->post(request, data);
    connect(uploadTrackReply, SIGNAL(finished()), this, SLOT(uploadTrackSlot()));
}

void OwnRadioClient::cacheTrack()
{
    this->getNextTrack();
}

void OwnRadioClient::cacheTrack(const QString &path)
{
    qDebug() << "cache << " << path;

    QString guid = QUuid::createUuid().toString().remove("{").remove("}");

    qDebug() << "GUID: " << guid;

    QFile::copy(path, "cache/" + guid + ".mp3");

    TagLib::FileRef ref(path.toStdString().c_str());

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

    qDebug() << query.exec();
}

void OwnRadioClient::playFromCache()
{
    QDir dir("cache");
    QStringList filter("*.mp3");
    QStringList files = dir.entryList(filter);

    qDebug() << files;

    int rand = qrand() % files.count();

    QString path("cache/" + files[rand]);
    qDebug() << path;

    player->stop();
    player->setMedia(QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()));
    player->play();

    this->cacheTrack();
}

void OwnRadioClient::play()
{
    qDebug() << "play()";

    player->play();
}

void OwnRadioClient::pause()
{
    qDebug() << "pause()";

    player->pause();
}

void OwnRadioClient::setHost(const QString &host)
{
    this->host = host;
}

void OwnRadioClient::getNextTrackSlot()
{
    qDebug() << "getNextTrack finished!";
    QByteArray data = getNextTrackReply->readAll();
    qDebug() << data;

    emit getNextTrackFinished();

    if(!trackId.isEmpty())
        this->saveHistory(trackId);

    QJsonObject track = QJsonDocument::fromJson(data).object();
    trackId = track["id"].toString();


    QSqlQuery query;
    query.prepare("INSERT INTO track(recid, recname, datetimelastlisten, path, artist, length) values(:recid, :recname, :datetimelastlisten, :path, :artist, :length)");

    query.bindValue(":recid", track["id"].toString());
    query.bindValue(":recname", track["name"].toString());
    query.bindValue(":datetimelastlisten", "00:00:00");
    query.bindValue(":path", "cache/" + track["id"].toString() + ".mp3");
    query.bindValue(":artist", track["artist"].toString());
    query.bindValue(":length", track["length"].toString());

    qDebug() << query.exec();

    this->getTrack(trackId);

    getNextTrackReply->deleteLater();
}

void OwnRadioClient::getTrackSlot()
{
    qDebug() << "getTrackSlot finished!";
    QByteArray data = getTrackReply->readAll();

    QFile file("cache/" + trackId + ".mp3");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    getTrackReply->deleteLater();
}

void OwnRadioClient::saveHistorySlot()
{
    qDebug() << "saveHistory finished!";

    QVariant status = saveHistoryReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(!status.isValid()) {
        qDebug() << saveHistoryReply->errorString();
    }

    saveHistoryReply->deleteLater();
}

void OwnRadioClient::uploadTrackSlot()
{
    qDebug() << "uploadTrackSlot finished!";

    QVariant status = uploadTrackReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(!status.isValid()) {
        qDebug() << uploadTrackReply->errorString();
    }

    uploadTrackReply->deleteLater();
}

void OwnRadioClient::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "mediaStatusChanged: " << status;

    if(status == QMediaPlayer::EndOfMedia) {
        this->getNextTrack();
    }
}
