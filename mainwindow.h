#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileDialog>
#include <QUrl>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openFile_clicked();

    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_restartButton_clicked();

    void on_previousButton_clicked();

    void on_nextButton_clicked();

    void on_songProgressBar_valueChanged(int value);

    void on_songProgressBar_sliderPressed();

    void on_songProgressBar_sliderReleased();

private:
    Ui::MainWindow *ui;
    QMediaPlayer* player = new QMediaPlayer;
    QAudioOutput* audio = new QAudioOutput;
    QUrl currentFile;
    bool playing;
    int previousSecond = 0;

    void changePlayButtonIcon(bool);
};
#endif // MAINWINDOW_H
