#include "mainwindow.h"
#include "createplaylistdialog.h"
#include "./ui_mainwindow.h"
#include <QStyle>
#include <QFile>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    createDBFile();

    ui->setupUi(this);
    player->setAudioOutput(audio);
    audio->setVolume(0.2);
    ui->volumeSlider->setValue(20);
    ui->volumeSlider->setSliderPosition(20);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_songProgressBar_valueChanged);
    connect(player, &QMediaPlayer::playingChanged, this, &MainWindow::changePlayButtonIcon);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::LoadedMedia)
        {
            ui->wholeSongDuration->setText(getSongWholeDuration());
        }
    });
}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
}

void MainWindow::createDBFile()
{
    if (!QFile::exists("./db/playlist.db"))
    {
        QDir().mkpath("./db");
    }
    db.setDatabaseName("./db/playlist.db");

    if (db.open())
    {
        qDebug("Opened playlist.db");
    }
    else
    {
        qDebug("Cannot open playlist.db");
    }
}

void MainWindow::on_openFile_clicked()
{
    currentFile = QFileDialog::getOpenFileUrl(this, tr("Open file"), QUrl(""), tr("mp3 (*.mp3)"));
    if (currentFile != QUrl(""))
    {
        player->setSource(currentFile);
        player->play();
        playing = true;

        ui->nowPlayingLabel->setText(QString("Now Playing: %1").arg(currentFile.fileName()));
        ui->nowPlayingLabel->adjustSize();
    }
}

void MainWindow::on_playButton_clicked()
{
    if (playing)
    {
        player->pause();
        playing = false;
    }
    else
    {
        player->play();
        playing = true;
    }
}


void MainWindow::on_stopButton_clicked()
{
    player->stop();
    playing = false;
}


void MainWindow::on_restartButton_clicked()
{
    player->setPosition(0);
    player->play();
    playing = true;
}


void MainWindow::on_previousButton_clicked()
{

}


void MainWindow::on_nextButton_clicked()
{

}


void MainWindow::on_songProgressBar_valueChanged(int value)
{
    if (ui->songProgressBar->maximum() != player->duration())
    {
        ui->songProgressBar->setMaximum(player->duration());
    }
    ui->songProgressBar->setValue(value);
    ui->songProgressBar->setSliderPosition(ui->songProgressBar->value());
    ui->untilEndDuration->setText(getSongLeftDuration(value));
    ui->currentDuration->setText(getSongCurrentDuration(value));
}


void MainWindow::on_songProgressBar_sliderPressed()
{
    player->pause();
}


void MainWindow::on_songProgressBar_sliderReleased()
{
    player->setPosition(ui->songProgressBar->value());
    player->play();
}

void MainWindow::changePlayButtonIcon(bool playing)
{
    playing ? ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause)) : ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

QString MainWindow::getSongWholeDuration()
{
    QString result;
    int wholeDurMS = player->duration();

    int min = wholeDurMS / 60000;
    int sec = (wholeDurMS % 60000) / 1000;

    (sec / 10 == 0) ? result = QString("%1:0%2").arg(QString::number(min), QString::number(sec)) : result = QString("%1:%2").arg(QString::number(min), QString::number(sec));

    return result;
}

QString MainWindow::getSongCurrentDuration(int value)
{
    QString result;
    int currentDurMS = value;

    int min = currentDurMS / 60000;
    int sec = (currentDurMS % 60000) / 1000;

    (sec / 10 == 0) ? result = QString("%1:0%2").arg(QString::number(min), QString::number(sec)) : result = QString("%1:%2").arg(QString::number(min), QString::number(sec));

    return result;
}
QString MainWindow::getSongLeftDuration(int value)
{
    QString result;

    int wholeDurMS = player->duration();
    int currentDurMS = value;
    int leftDurMS = wholeDurMS - currentDurMS;

    int min = leftDurMS / 60000;
    int sec = ((leftDurMS% 60000) / 1000);

    (sec / 10 == 0) ? result = QString("%1:0%2").arg(QString::number(min), QString::number(sec)) : result = QString("%1:%2").arg(QString::number(min), QString::number(sec));

    return result;
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    audio->setVolume(static_cast<float>(value)/100.0);
}




// Refreash some part of the program (NOT THE PLAYER AND CONTROL PANEL) after SQL EXEC
// void MainWindow::refresh()
// {



void MainWindow::on_createPlaylist_clicked()
{
    CreatePlaylistDialog* dialog = new CreatePlaylistDialog(this);

    connect(dialog, &CreatePlaylistDialog::playlistCreated, this, &MainWindow::createPlaylist);

    dialog->exec();

}

bool MainWindow::checkTableExist(QString playlistName)
{
    QSqlQuery query(db);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:tableName;");
    query.bindValue(":tableName", playlistName);

    if (!query.exec())
    {
        qDebug() << "Result from checking existence: " << query.lastError().text();
        return false;
    }

    return query.next();
}

void MainWindow::createPlaylist(const QString& playlistName)
{

    if (!checkTableExist(playlistName))
    {
        QSqlQuery query;
        QString createTableQueryCommand = QString("CREATE TABLE %1 ("
                                                  "song_path TEXT PRIMARY KEY, "
                                                  "song_name TEXT, "
                                                  "author_name TEXT, "
                                                  "date_added DATETIME, "
                                                  "duration INTEGER);").arg(playlistName);
        if (!query.exec(createTableQueryCommand))
        {
            qDebug() << "Error creating a playlist: " << query.lastError().text();
        }
        else
        {
            ClickableLabel* playlistLabel = new ClickableLabel(playlistName);
            playlistLabel->setAutoFillBackground(true);
            playlistLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            playlistLabel->setFixedWidth(120);
            playlistLabel->setFixedHeight(30);
            QFont font("Sans Serif", 10);
            font.setBold(true);
            playlistLabel->setFont(font);
            connect(playlistLabel, &ClickableLabel::clicked, this, &MainWindow::playlistClicked);
            playlistVec.push_back(playlistLabel);
            ui->playlistSlot->addWidget(playlistLabel);

            QString result = QString("Created playlist named: %1").arg(playlistName);
            qDebug() << result;
        }
    }
}

void MainWindow::playlistClicked(QUrl path, QString type)
{
    for (auto label : playlistVec)
    {
        label->setStyleSheet("");
    }
}
