#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcpServer = new QTcpServer(this);
    init();
    start();

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    player = new QMediaPlayer;

    QProcess process;
    process.start("../maintenancetool --checkupdates");

    // Wait until the update tool is finished
    process.waitForFinished();

    if(process.error() != QProcess::UnknownError)
    {
        log("Error checking for updates");
        return;
    }

    // Read the output
    QByteArray data = process.readAllStandardOutput();

    // No output means no updates available
    // Note that the exit code will also be 1, but we don't use that
    // Also note that we should parse the output instead of just checking if it is empty if we want specific update info
    if(data.isEmpty()) {
        log("No updates available");
        return;
    } else {
        log(QString(data));
    }

    // Call the maintenance tool binary
    // Note: we start it detached because this application need to close for the update
    QStringList args("--updater");
    bool success = QProcess::startDetached("../maintenancetool", args);

    // Close the application
    qApp->closeAllWindows();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbStartListening_clicked(bool checked)
{
    log(checked ? "started" : "closed");

    int port = ui->leServerPort->text().toInt();

    if(checked) {
        if (!tcpServer->listen(QHostAddress::Any, port)) {
            qDebug() <<  QObject::tr("Unable to start the server: %1.")
                         .arg(tcpServer->errorString());
        } else {
            qDebug() << "Server successfully started!";
        }
    } else {
        tcpServer->close();
        qDebug() << "Server closed!";
    }
}

void MainWindow::newConnection()
{
    log("New connection!");

    clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void MainWindow::readyRead()
{
    log("Receive:");
    log(clientConnection->readAll());
}

void MainWindow::on_pbConnect_clicked(bool checked)
{
    log(checked ? "connected" : "disconnected");

    if(checked) {
        qDebug() << "connected!";
        int port = ui->lePort->text().toInt();
        QString host = ui->leHost->text();

        connection = new QTcpSocket(this);
        connection->connectToHost(host, port);

        if(connection->isOpen()) {
            ui->pbSend->setEnabled(true);
            ui->ptMessage->setEnabled(true);
        } else {
            ui->pbConnect->setChecked(false);
        }

    } else {
        qDebug() << "disconnected!";
        connection->close();

        ui->pbSend->setEnabled(false);
        ui->ptMessage->setEnabled(false);
    }
}

void MainWindow::on_pbSend_clicked()
{
    QByteArray data;
    data.append(ui->ptMessage->toPlainText());

    log("Send: ");
    log(data.data());

    connection->write(data, data.length());
}

void MainWindow::log(const QString &message)
{
    ui->textBrowser->append(message);
}

void MainWindow::start()
{
    qDebug("start");

    server = new QHttpServer();

    server->listen(QHostAddress::Any, 8080,
        [&](QHttpRequest* req, QHttpResponse* res) {

            qDebug() << req->url();

            QRegExp exp("/v3/tracks/[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}/next", Qt::CaseInsensitive);

            if(exp.exactMatch(req->url().toString()))
            {
                res->setStatusCode(qhttp::ESTATUS_OK);

                QRegExp rx("[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}", Qt::CaseInsensitive);
                rx.indexIn(req->url().toString());

                QUuid deviceId(rx.cap());
                QByteArray track = ownRadio->getNextTrack(deviceId);

                res->end(track);
                return;
            } else
            {
                qDebug() << "!!! -- track -- !!!";
                res->setStatusCode(qhttp::ESTATUS_OK);

                QRegExp rx("[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}", Qt::CaseInsensitive);
                rx.indexIn(req->url().toString());

                QUuid deviceId(rx.cap());

                QByteArray track = ownRadio->getTrack(deviceId);
                res->addHeader("Content-Type", "audio/mpeg");
                res->end(track);
                return;
            }

            res->setStatusCode(qhttp::ESTATUS_OK);

            res->end("Hello World!\n");
    });

    if(!server->isListening()) {
        qDebug("failed to listen");
    }
}

void MainWindow::init()
{
    QSettings settings("config.ini", QSettings::IniFormat);

    if(!settings.childGroups().contains("ownRadio")) {
        QString deviceId = QUuid::createUuid().toString().remove("{").remove("}");
        settings.setValue("ownRadio/deviceId", deviceId);
        settings.setValue("ownRadio/host", "http://api.ownradio.ru/");
    };

    QString host = settings.value("ownRadio/host").toString();
    QString deviceId = settings.value("ownRadio/deviceId").toString();

    client = new OwnRadioClient(host, deviceId);
}

// TODO: ownradioclient
void MainWindow::on_pbNext_clicked()
{
    qDebug() << "get next track";

    client->getNextTrack();

    /*
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://api.ownradio.ru/v3/tracks/16f1d9a7-5325-4167-b73e-3360e55b087d/next")));
    */
}

void MainWindow::replyFinished(QNetworkReply* reply)
{
    /*
    qDebug() << "replyFinished";
    QByteArray data = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QString trackInfo = QString("%1 - %2")
            .arg(obj["artist"].toString())
            .arg(obj["name"].toString());

    QString guid = obj["id"].toString();

    //QMediaPlayer* //(this, QMediaPlayer::StreamPlayback);
    player->stop();
    player->setMedia(QUrl("http://api.ownradio.ru/v3/tracks/" + guid));
    player->setVolume(100);
    player->play();

    ui->lbTrack->setText(trackInfo);

    qDebug() << data;*/
}


void MainWindow::on_pbInit_clicked()
{
    ownRadio = new OwnRadio();
}

void MainWindow::on_pbGetNextTrack_clicked()
{
    QUuid uuid;

    QByteArray track = ownRadio->getNextTrack(uuid.createUuid());
    log(track);
}

void MainWindow::on_pbGetNextTrackClient_clicked()
{
    client->getNextTrack();
}

void MainWindow::on_pbPlayClient_clicked()
{
    client->play();
}

void MainWindow::on_pbUpload_clicked()
{
    client->uploadTrack("/home/developer/thunder.mp3");
}

void MainWindow::on_pbCache_clicked()
{
    client->cacheTrack();
}

void MainWindow::on_pbPlayFromCache_clicked()
{
    client->playFromCache();
}

void MainWindow::on_pbGetDirectory_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 QDir::homePath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    qDebug() << dir;

    QDir myDir(dir);
    QStringList filter("*.mp3");
    QStringList filesList = myDir.entryList(filter);

    for(int i = 0; i < filesList.count(); i++)
    {
        client->cacheTrack(dir + "/" + filesList.at(i));
    }
}
