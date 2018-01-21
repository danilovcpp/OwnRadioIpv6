#ifndef OWNRADIOCLIENT_H
#define OWNRADIOCLIENT_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMediaPlayer>
#include <QDateTime>
#include <QFile>
#include <QUuid>
#include <QDir>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <fileref.h>
#include <tag.h>

class OwnRadioClient : public QObject
{
    Q_OBJECT

public:
    OwnRadioClient(const QString& host, const QString& deviceId);
    void getNextTrack();
    void getTrack(const QString& trackId);
    void saveHistory(const QString& trackId);
    void uploadTrack(const QString& path);
    void cacheTrack();
    void cacheTrack(const QString& path);
    void playFromCache();

    void play();
    void pause();

    void setHost(const QString& host);

signals:
    void getNextTrackFinished();
    void uploadTrackFinished();

private slots:
    void getNextTrackSlot();
    void getTrackSlot();
    void saveHistorySlot();
    void uploadTrackSlot();

    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QNetworkAccessManager *manager;
    QMediaPlayer *player;
    QString host;
    QString deviceId;

    QString trackId;

    QNetworkReply *getNextTrackReply;
    QNetworkReply *getTrackReply;
    QNetworkReply *saveHistoryReply;
    QNetworkReply *uploadTrackReply;

    QSqlDatabase db;
};

#endif // OWNRADIOCLIENT_H
