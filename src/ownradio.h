#ifndef OWNRADIOSERVER_H
#define OWNRADIOSERVER_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>

class OwnRadio
{
public:
    OwnRadio();
    QByteArray getNextTrack(QUuid deviceId);
    QByteArray getTrack(QUuid trackId);
    bool saveHistory(QUuid deviceId, QUuid trackId, QJsonDocument history);
    bool loadTrack();
private:
    QSqlDatabase db;
};

#endif // OWNRADIOSERVER_H
