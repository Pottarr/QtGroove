#include "mainwindow.h"
#include "createplaylistdialog.h"
#include "./ui_mainwindow.h"
#include <QStyle>
#include <QFile>
#include <QDir>
#include <QMediaMetaData>

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

    ui->nowPlayingLabel->setText(currentFile.fileName());
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
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:tableName;");
    query.bindValue(":tableName", playlistName);

    if (!query.exec())
    {
        qDebug() << "Result from checking existence: " << query.lastError().text();
        return false;
    }

    return query.next();
}

int MainWindow::checkSongExist(QString path)
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
                                                  "duration TEXT);").arg(playlistName);
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
        QList<QTableWidgetItem*> items = {
            new QTableWidgetItem(query.value(1).toString()),
            new QTableWidgetItem(query.value(2).toString()),
            new QTableWidgetItem(query.value(3).toString()),
            new QTableWidgetItem(query.value(4).toString()),
        };

        int rowCount = ui->songSlot->rowCount();
        ui->songSlot->setRowCount(rowCount+1);
        for (int column = 0; column < items.size(); ++column)
        {
            ui->songSlot->setItem(rowCount, column, items[column]);
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




            player->setSource(list[i].path());
            QString author = player->metaData().value(QMediaMetaData::Author).toString();
            QString dateAdded = player->metaData().value(QMediaMetaData::Date).toString();
            QString duration = getSongWholeDuration();
            QString addSongsCommand = QString("INSERT INTO %1 (song_path,song_name) VALUES ('%2','%3', '%4', '%5', '%6');").arg(currentPlaylist, list[i].path(), list[i].fileName(), author, dateAdded);



            query.exec(addSongsCommand);

            QList<QTableWidgetItem*> items = {
                new QTableWidgetItem(list[i].fileName()),
                new QTableWidgetItem(author),
                new QTableWidgetItem(dateAdded),
                new QTableWidgetItem(duration),
            };

            int rowCount = ui->songSlot->rowCount();
            ui->songSlot->setRowCount(rowCount+1);
            for (int column = 0; column < items.size(); ++column)
            {
                ui->songSlot->setItem(rowCount, column, items[column]);
            }

        }
    }
}


void MainWindow::on_songSlot_itemDoubleClicked(QTableWidgetItem *item)
{
    QString fileName = ui->songSlot->item(item->row(), 0)->text();
    query.exec(QString("SELECT song_path FROM %1 WHERE song_name = '%2'").arg(currentPlaylist, fileName));
    query.next();
    currentFile = query.value(0).toUrl();
    playMusic();
}

