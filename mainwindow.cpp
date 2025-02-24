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
    ui->horizontalSlider->setValue(20);
    ui->horizontalSlider->setSliderPosition(20);
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

    // connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
    //     ui->songProgressBar->setValue(position);
    //     ui->currentDuration->setText(getSongCurrentDuration());
    // });

    // connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
    //     ui->untilEndDuration->setText(getSongLeftDuration());
    // });
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
