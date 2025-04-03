#include "mainwindow.h"
#include "createplaylistdialog.h"
#include "editsongmetadatadialog.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    createDBFile();

    ui->setupUi(this);
    setWindowTitle("QtGroove");
    setWindowIcon(QIcon(":/icons/QtGroove.ico"));
    initializePlaylist();
    player->setAudioOutput(audio);
    audio->setVolume(0.2);
    ui->volumeSlider->setValue(20);
    ui->volumeSlider->setSliderPosition(20);
    ui->playButton->setIcon(QIcon::fromTheme("media-playback-start"));
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_songProgressBar_valueChanged);
    connect(player, &QMediaPlayer::playingChanged, this, &MainWindow::changePlayButtonIcon);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::EndOfMedia)
        {
            if (loopMode == 1)
            {
                player->play();
            }
            else
            {
                on_nextButton_clicked();
            }
        }
    });
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::LoadedMedia)
        {
            ui->wholeSongDuration->setText(getSongWholeDuration());

            if (singleFileMode)
            {
            player->metaData().value(QMediaMetaData::Title).isNull() ?
                ui->nowPlayingLabel->setText(currentFile.fileName()) :
                ui->nowPlayingLabel->setText(player->metaData().value(QMediaMetaData::Title).toString());
            }
            else
            {

                if (!query.exec(QString("SELECT song_name FROM '%1' WHERE song_path = '%2'").arg(currentPlaylist, currentFile.toString()))) {
                    qDebug() << "Query failed:" << query.lastError().text();
                } else if (query.next()) {  // Moves to the first row
                    QString displaySongName = query.value(0).toString();  // Get the first column value
                    qDebug() << "Song Name:" << displaySongName;
                    displaySongName.isEmpty() ?
                        ui->nowPlayingLabel->setText(player->metaData().value(QMediaMetaData::Title).toString()) :
                        ui->nowPlayingLabel->setText(displaySongName);
                } else {
                    qDebug() << "No matching record found.";
                    player->metaData().value(QMediaMetaData::Title).isNull() ?
                        ui->nowPlayingLabel->setText(currentFile.fileName()) :
                        ui->nowPlayingLabel->setText(player->metaData().value(QMediaMetaData::Title).toString());
            }
            }

            ui->nowPlayingLabel->adjustSize();
        }
    });
    ui->songSlot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->playlistSlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->songSlot, &QTableWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(ui->playlistSlot, &QTableWidget::customContextMenuRequested, this, &MainWindow::showContextMenuPlaylist);

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
        query.exec("CREATE TABLE IF NOT EXISTS playlists (name TEXT PRIMARY KEY NOT NULL);"); }
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
        singleFileMode = true;
    }
}

void MainWindow::playMusic()
{
    player->setSource(currentFile);
    player->play();
    playing = true;
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
    if (!songQueue.empty())
    {
        if (currentQueuePosition != 0) // not first item
        {
            --currentQueuePosition;
            currentRow = songQueue.at(currentQueuePosition);
        }
        else // first item
        {
            if (loopMode == 2)
            {
                currentQueuePosition = songQueue.size() - 1;
                currentRow = songQueue.at(currentQueuePosition);
            }
        }
        currentFile = ui->songSlot->item(currentRow, 0)->data(songPathRole).toUrl();
        ui->songSlot->selectRow(currentRow);
        playMusic();
        qDebug() << "Current index: " << currentQueuePosition << ", aka row: " << currentRow;
    }
}

void MainWindow::on_nextButton_clicked()
{
    bool lastSongNoLoop = false;
    if (!songQueue.empty() && !singleFileMode)
    {
        if (currentQueuePosition < songQueue.size()-1) // when not in the last position of queue list
        {
            ++currentQueuePosition;
            currentRow = songQueue.at(currentQueuePosition);
        }
        else // when in the last position of the queue list
        {
            if (shuffleMode) // shuffle
            {
                if (currentQueuePosition < songQueue.size()-1) // not the last item in the playlist
                {
                    ++currentRow;
                    ++currentQueuePosition;
                }
                else // last item in playlist
                {
                    if (loopMode == 2)
                    {
                        std::shuffle(songQueue.begin(), songQueue.end(), *QRandomGenerator::global()); // reshuffle the playlist
                        currentQueuePosition = 0;
                        currentRow = songQueue.at(currentQueuePosition);
                        qDebug() << "Current songQueue: " << songQueue;
                    } else // no loop or loop track
                    {
                        lastSongNoLoop = true;
                    }
                }
            }
            else // no shuffle
            {
                if (currentRow < ui->songSlot->rowCount()-1) // not the last item in the playlist
                {
                    ++currentRow;
                    ++currentQueuePosition;
                }
                else // last item in playlist
                {
                    if (loopMode == 2) // loop playlist
                    {
                        currentQueuePosition = 0;
                        currentRow = songQueue.at(currentQueuePosition);
                    }
                    else // no loop or loop track
                    {
                        lastSongNoLoop = true;
                    }
                }
            }
        }
        if (currentQueuePosition < songQueue.size() && !lastSongNoLoop)
        {
            ui->songSlot->selectRow(currentRow);
            currentFile = ui->songSlot->item(currentRow, 0)->data(songPathRole).toUrl();
            playMusic();
            qDebug() << "Current index: " << currentQueuePosition << ", aka row: " << currentRow;
        }
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
    playing ?  ui->playButton->setIcon(QIcon::fromTheme("media-playback-pause")) : ui->playButton->setIcon(QIcon::fromTheme("media-playback-start"));
}

QString MainWindow::getSongWholeDuration()
{
    QString result;
    int wholeDurMS = player->duration();

    result = changeDurationToText(wholeDurMS);

    return result;
}

QString MainWindow::getSongCurrentDuration(int value)
{
    QString result;
    int currentDurMS = value;

    result = changeDurationToText(currentDurMS);

    return result;
}
QString MainWindow::getSongLeftDuration(int value)
{
    QString result;

    int wholeDurMS = player->duration();
    int currentDurMS = value;
    int leftDurMS = wholeDurMS - currentDurMS;

    result = changeDurationToText(leftDurMS);

    return result;
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    audio->setVolume(static_cast<float>(value)/100.0);
}

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

            items[column]->setData(songPathRole, query.value(0));
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
                    items[column]->setData(songPathRole, currentFile);
                }
            }
        }
    }
}


void MainWindow::on_songSlot_itemDoubleClicked(QTableWidgetItem *item)
{
    currentFile = item->data(songPathRole).toUrl();
    songQueue.clear();
    currentRow = item->row();
    for (int i = 0; i < ui->songSlot->rowCount(); i++)
    {
        songQueue.push_back(i);
    }
    if (shuffleMode)
    {
        std::shuffle(songQueue.begin(), songQueue.end(), *QRandomGenerator::global());

        qDebug() << "Current songQueue: " << songQueue;
    }
    currentQueuePosition = currentRow;
    playMusic();
    qDebug() << "Current songQueue: " << songQueue;
    qDebug() << "Current index: " << currentQueuePosition << ", aka row: " << currentRow;
    singleFileMode = false;
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = ui->songSlot->itemAt(pos);
    if (!item) return;
    QString itemPath = item->data(songPathRole).toString();


    int row = item->row();


    QMenu contextMenu(tr("Song Slot Context Menu"), this);
    contextMenu.addAction("Edit song metadata", this, [this, row]()
    {
        QString songPath = this->ui->songSlot->item(row, 0)->data(songPathRole).toString();
        QString songName = this->ui->songSlot->item(row, 0)->text();  // Get song name
        QString authorName = this->ui->songSlot->item(row, 1)->text();  // Get author name
        QString dateAdded = this->ui->songSlot->item(row, 2)->text();  // Get author name
        EditSongMetaDataDialog* dialog = new EditSongMetaDataDialog(this, songPath, songName, authorName, dateAdded);

        connect(dialog, &EditSongMetaDataDialog::songMetaDataEdited, this, &MainWindow::editSongMetaData);

        dialog->exec();

    });

    contextMenu.addAction("Remove from playlist", this, [&]()
    {
        ui->songSlot->removeRow(row);
        query.exec(QString("DELETE FROM %1 WHERE song_path = '%2';").arg(currentPlaylist, itemPath));
        int i = 0;
        while (i < songQueue.size())
        {
            if (songQueue[i] == row)
            {
                if (songQueue[currentQueuePosition] > row)
                {
                    --currentQueuePosition;
                }
                songQueue.removeAt(i);

            }
            else if (songQueue[i] > row)
            {
                --songQueue[i];
                ++i;
            }
            else
            {
                ++i;
            }
        }
        qDebug() << "Current songQueue: " << songQueue;
    });
    contextMenu.exec(ui->songSlot->viewport()->mapToGlobal(pos));
}

void MainWindow::showContextMenuPlaylist(const QPoint& pos)
{
    QListWidgetItem* item = ui->playlistSlot->itemAt(pos);
    if (!item) return;

    QString playlistName = item->text();
    QMenu contextMenu;
    contextMenu.addAction("Delete playlist", this, [&]()
    {
        query.exec(QString("DROP TABLE %1;").arg(playlistName));
        query.exec(QString("DELETE FROM playlists WHERE name = '%1';").arg(playlistName));
        ui->playlistSlot->takeItem(ui->playlistSlot->row(item));
        ui->addSongs->setEnabled(false);
        songQueue.clear();
        ui->songSlot->clearContents();
        ui->songSlot->setRowCount(0);
    });

    contextMenu.exec(ui->playlistSlot->viewport()->mapToGlobal(pos));

}

void MainWindow::editSongMetaData(QString &songPath, QString &songName, QString &authorName, QString &dateAdded)
{
    if (!query.exec(QString("UPDATE '%1' SET song_name = '%2', author_name = '%3', date_added = '%4' WHERE song_path = '%5'").arg(currentPlaylist, songName, authorName, dateAdded, songPath)))
    {
        qDebug() << "Failed: " << query.lastError().text();
    }

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

            items[column]->setData(songPathRole, query.value(0));
        }

    }

}

QString MainWindow::changeDurationToText(int durationMS)
{

    QString result;
    int min = durationMS / 60000;
    int sec = (durationMS % 60000) / 1000;

    QString minText;
    QString secText;

    (min / 10 == 0) ? minText = QString("0%1").arg(QString::number(min)) : minText = QString::number(min);
    (sec / 10 == 0) ? secText = QString("0%1").arg(QString::number(sec)) : secText = QString::number(sec);

    result = QString("%1:%2").arg(minText, secText);

    return result;
}

void MainWindow::on_shuffleButton_clicked()
{
    if (!shuffleMode)
    {
        shuffleMode = true;
        ui->shuffleButton->setText("Shuffle Mode: on");

        std::shuffle(songQueue.begin(), songQueue.end(), *QRandomGenerator::global());

        qDebug() << "Current songQueue: " << songQueue;
    }
    else
    {
        shuffleMode = false;
        ui->shuffleButton->setText("Shuffle Mode: off");
        songQueue.clear();
        for (int i = 0; i < ui->songSlot->rowCount(); i++)
        {
            songQueue.push_back(i);
        }
        currentQueuePosition = currentRow;
        qDebug() << "Current songQueue: " << songQueue;
    }
}

void MainWindow::on_loopComboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == QString("No Loop"))
    {
        loopMode = 0;
    }
    else if (arg1 == QString("Loop Track"))
    {
        loopMode = 1;
    }
    else if (arg1 == QString("Loop Playlist"))
    {
        loopMode = 2;
    }
    qDebug() << loopMode;
}

void MainWindow::on_playbackSpeedComboBox_currentTextChanged(const QString &arg1)
{
    QString a = arg1;
    float speed = a.removeLast().toFloat();
    player->setPlaybackRate(speed);
}

