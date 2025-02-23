#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player->setAudioOutput(audio);
    audio->setVolume(0.2);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_songProgressBar_valueChanged);
    connect(player, &QMediaPlayer::playingChanged, this, &MainWindow::changePlayButtonIcon);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openFile_clicked()
{
    currentFile = QFileDialog::getOpenFileUrl(this, tr("Open file"), QUrl(""), tr("mp3 (*.mp3)"));
    if (currentFile != QUrl(""))
    {
        player->setSource(currentFile);
        player->play();
        playing = true;

        connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status)
        {
            if (status == QMediaPlayer::LoadedMedia)
            {
                ui->wholeSongDuration->setText(getSongWholeDuration());
            }
        });

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
    ui->songProgressBar->setSliderPosition(ui->songProgressBar->value());

    connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        ui->songProgressBar->setValue(position);
        ui->currentDuration->setText(getSongCurrentDuration());
    });

    connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        ui->untilEndDuration->setText(getSongLeftDuration());
    });
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
    // int wholeDurMS = player->metaData().value(QMediaMetaData::Duration).toInt();
    int wholeDurMS = player->duration();

    int min = wholeDurMS / 60000;
    int sec = (wholeDurMS % 60000) / 1000;

    if (sec / 10 == 0)
    {
        result = QString("%1:0%2").arg(QString::number(min), QString::number(sec));
    }
    else
    {
        result = QString("%1:%2").arg(QString::number(min), QString::number(sec));
    }

    return result;
}

QString MainWindow::getSongCurrentDuration()
{
    QString result;
    int currentDurMS = player->position();

    int min = currentDurMS / 60000;
    int sec = (currentDurMS % 60000) / 1000;

    if (sec / 10 == 0)
    {
        result = QString("%1:0%2").arg(QString::number(min), QString::number(sec));
    }
    else
    {
        result = QString("%1:%2").arg(QString::number(min), QString::number(sec));
    }

    return result;
}
QString MainWindow::getSongLeftDuration()
{
    QString result;

    int wholeDurMS = player->duration();
    int currentDurMS = player->position();
    int leftDurMS = wholeDurMS - currentDurMS;

    int min = leftDurMS / 60000;
    int sec = ((leftDurMS% 60000) / 1000);

    if (sec / 10 == 0)
    {
        result = QString("%1:0%2").arg(QString::number(min), QString::number(sec));
    }
    else
    {
        result = QString("%1:%2").arg(QString::number(min), QString::number(sec));
    }

    return result;
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{

}
