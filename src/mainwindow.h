#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMediaPlayer>
#include <QDebug>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QFileDialog>

#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "ownradio.h"
#include "ownradioclient.h"

using namespace qhttp::server;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbConnect_clicked(bool checked);
    void on_pbStartListening_clicked(bool checked);
    void on_pbSend_clicked();
    void on_pbNext_clicked();
    void newConnection();
    void readyRead();
    void replyFinished(QNetworkReply* reply);

    // Test
    void on_pbInit_clicked();

    void on_pbGetNextTrack_clicked();

    void on_pbGetNextTrackClient_clicked();

    void on_pbPlayClient_clicked();

    void on_pbUpload_clicked();

    void on_pbCache_clicked();

    void on_pbPlayFromCache_clicked();

    void on_pbGetDirectory_clicked();

private:
    void log(const QString&);
    void start();
    void init();

private:
    Ui::MainWindow *ui;

    QTcpServer *tcpServer;
    QTcpSocket *clientConnection;
    QTcpSocket *connection;
    QMediaPlayer *player;
    QHttpServer *server;
    OwnRadio *ownRadio;
    OwnRadioClient *client;
};

#endif // MAINWINDOW_H
