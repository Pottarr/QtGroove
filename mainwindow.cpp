#include "mainwindow.h"
#include "createplaylistdialog.h"
#include "./ui_mainwindow.h"
#include <QStyle>
#include <QFile>
#include <QDir>
#include <QMediaMetaData>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    createDBFile();

    ui->setupUi(this);
    initializePlaylist();
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
        if (status == QMediaPlayer::EndOfMedia)
        {
            on_nextButton_clicked();
        }
    });
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

void MainWindow::initializePlaylist()
{
    query.exec("SELECT * FROM playlists;");
    while (query.next())
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(query.value(0).toString());
        QFont font("Sans Serif", 10);
        font.setBold(true);
        item->setFont(font);
        item->setTextAlignment(Qt::AlignHCenter);
        ui->playlistSlot->addItem(item);
    }
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
        query.exec("CREATE TABLE IF NOT EXISTS playlists (name TEXT PRIMARY KEY NOT NULL);");
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
        playMusic();
    }
}

void MainWindow::playMusic()
{
    player->setSource(currentFile);
    player->play();
    playing = true;

    ui->nowPlayingLabel->setText(ui->songSlot->item(songQueue.at(currentQueuePosition), 0)->text());
    ui->nowPlayingLabel->adjustSize();
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
    if (!songQueue.empty() && currentQueuePosition != 0)
    {
        --currentQueuePosition;
        currentRow = songQueue.at(currentQueuePosition);
        currentFile = ui->songSlot->item(currentRow, 0)->data(5).toUrl();
        playMusic();
    }
}


void MainWindow::on_nextButton_clicked()
{
    if (!songQueue.empty())
    {
        if (currentQueuePosition != songQueue.size()-1)
        {
            ++currentQueuePosition;
            currentRow = songQueue.at(currentQueuePosition);
        }
        else
        {
            if (shuffleMode)
            {

            }
            else
            {
                if (currentRow != ui->songSlot->rowCount()-1)
                {
                    ++currentRow;
                    ++currentQueuePosition;
                    songQueue.push_back(currentRow);
                }
            }
        }
        ui->songSlot->selectRow(currentRow);
        currentFile = ui->songSlot->item(currentRow, 0)->data(5).toUrl();
        playMusic();
    }
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
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:tableName;");
    query.bindValue(":tableName", playlistName);

    if (!query.exec())
    {
        qDebug() << "Result from checking existence: " << query.lastError().text();
        return false;
    }

    return query.next();
}

bool MainWindow::checkSongExist(QString path)
{
    query.exec(QString("SELECT count(*) FROM %1 WHERE song_path = '%2';").arg(currentPlaylist, path));
    query.next();
    return query.value(0).toInt();
}

void MainWindow::createPlaylist(const QString& playlistName)
{

    if (!checkTableExist(playlistName) && playlistName != QString(""))
    {
        QString createTableQueryCommand = QString("CREATE TABLE %1 ("
                                                  "song_path TEXT PRIMARY KEY, "
                                                  "song_name TEXT, "
                                                  "author_name TEXT, "
                                                  "date_added TEXT, "
                                                  "duration INTEGER);").arg(playlistName);
        if (!query.exec(createTableQueryCommand))
        {
            qDebug() << "Error creating a playlist: " << query.lastError().text();
        }
        else
        {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(playlistName);
            QFont font("Sans Serif", 10);
            font.setBold(true);
            item->setFont(font);
            item->setTextAlignment(Qt::AlignHCenter);
            ui->playlistSlot->addItem(item);

            QString result = QString("Created playlist named: %1").arg(playlistName);
            qDebug() << result;
        }

        query.prepare("INSERT INTO playlists VALUES (?);");
        query.addBindValue(playlistName);
        query.exec();
    }
}

void MainWindow::on_playlistSlot_itemDoubleClicked(QListWidgetItem *item)
{
    ui->addSongs->setEnabled(true);
    currentPlaylist = item->text();
    ui->songSlot->clearContents();
    ui->songSlot->setRowCount(0);
    query.exec(QString("SELECT * FROM %1").arg(currentPlaylist));
    while (query.next())
    {
        QString durationText = changeDurationToText(query.value(4).toInt());

        QList<QTableWidgetItem*> items = {
            new QTableWidgetItem(query.value(1).toString()),
            new QTableWidgetItem(query.value(2).toString()),
            new QTableWidgetItem(query.value(3).toString()),
            new QTableWidgetItem(durationText),
        };

        int rowCount = ui->songSlot->rowCount();
        ui->songSlot->setRowCount(rowCount+1);
        for (int column = 0; column < items.size(); ++column)
        {
            ui->songSlot->setItem(rowCount, column, items[column]);
            items[column]->setData(5, query.value(0));
        }

    }
}

void MainWindow::on_addSongs_clicked()
{
    QList<QUrl> list = QFileDialog::getOpenFileUrls(this, tr("Add music"), QUrl(""), tr("mp3 (*.mp3)"));
    for (int i = 0; i < list.size(); ++i)
    {
        if (!checkSongExist(list[i].path()))
        {
            currentFile = list[i].toLocalFile();
            QMediaPlayer tempPlayer;
            QAudioOutput tempAudio;
            tempPlayer.setSource(currentFile);
            tempPlayer.setAudioOutput(&tempAudio);
            QThread::msleep(200);
            tempPlayer.pause();

            QString name =  currentFile.fileName();
            QString author = "Not Found";
            QDateTime dateTimeAdded = QDateTime::currentDateTime();
            QString dateTimeAddedString = dateTimeAdded.toString("dd/MM/yyyy hh:mm:ss");
            int duration = 1000;

            qDebug() << "Current playlist: " << currentPlaylist;

            qDebug() << "Successfully retrieved the path: " << currentFile;
            qDebug() << "Successfully retrieved the name: " << currentFile.fileName();

            QElapsedTimer timer;
            timer.start();
            while (timer.elapsed() < 2000) {
                QCoreApplication::processEvents();
                if (!tempPlayer.metaData().value(QMediaMetaData::Duration).isNull()) {
                    break;
                }
            }

            if (!tempPlayer.metaData().value(QMediaMetaData::Title).isNull()) {
                name = tempPlayer.metaData().value(QMediaMetaData::Title).toString();
            }
            if (!tempPlayer.metaData().value(QMediaMetaData::Duration).isNull()) {
                duration = tempPlayer.metaData().value(QMediaMetaData::Duration).toInt();
            }
            if (!tempPlayer.metaData().value(QMediaMetaData::ContributingArtist).isNull()) {
                author = tempPlayer.metaData().value(QMediaMetaData::ContributingArtist).toString();
            }
            // if (!tempPlayer.metaData().value(QMediaMetaData::Date).isNull()) {
                // dateAdded = tempPlayer.metaData().value(QMediaMetaData::Date).toString();
            // }

            QString addSongsCommand = QString("INSERT INTO %1 (song_path, song_name, author_name, date_added, duration) VALUES ('%2', '%3', '%4', '%5', %6);").arg(currentPlaylist, currentFile.toString(), name, author, dateTimeAddedString).arg(duration);

            QSqlQuery query;

            if (!query.exec(addSongsCommand))
            {
                qDebug() << "Error from SQL" << query.lastError().text();
            }
            else
            {

                QString durationText = changeDurationToText(duration);

                QList<QTableWidgetItem*> items = {
                    new QTableWidgetItem(name),
                    new QTableWidgetItem(author),
                    new QTableWidgetItem(dateTimeAddedString),
                    new QTableWidgetItem(durationText),
                };

                int rowCount = ui->songSlot->rowCount();
                ui->songSlot->setRowCount(rowCount+1);
                for (int column = 0; column < items.size(); ++column)
                {
                    ui->songSlot->setItem(rowCount, column, items[column]);
                    items[column]->setData(5, currentFile);
                }
            }
        }
    }
}


void MainWindow::on_songSlot_itemDoubleClicked(QTableWidgetItem *item)
{
    currentFile = item->data(5).toUrl();
    songQueue.clear();
    currentRow = item->row();
    songQueue.push_back(currentRow);
    currentQueuePosition = 0;
    playMusic();
}

QString MainWindow::changeDurationToText(int durationMS)
{

    QString result;
    int min = durationMS / 60000;
    int sec = (durationMS % 60000) / 1000;

    (sec / 10 == 0) ? result = QString("%1:0%2").arg(QString::number(min), QString::number(sec)) : result = QString("%1:%2").arg(QString::number(min), QString::number(sec));

    return result;
}

void MainWindow::on_shuffleButton_clicked()
{
    if (!shuffleMode)
    {
        shuffleMode = true;
    }
    else
    {
        shuffleMode = false;
    }
}

