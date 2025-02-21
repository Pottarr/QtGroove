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
    ui->pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::detectMusic);
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
    }
}

void MainWindow::detectMusic(int value)
{
    qDebug() << value;
}


void MainWindow::on_pauseButton_clicked()
{
    if (playing)
    {
        ui->pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        player->pause();

        playing = false;
    }
    else
    {
        ui->pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
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
    return;
}


void MainWindow::on_nextButton_clicked()
{
    return;
}

