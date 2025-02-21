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

    void on_pauseButton_clicked();

    void on_stopButton_clicked();

    void on_restartButton_clicked();

    void on_previousButton_clicked();

    void on_nextButton_clicked();

private:
    Ui::MainWindow *ui;
    void detectMusic(int value);
    QMediaPlayer* player = new QMediaPlayer;
    QAudioOutput* audio = new QAudioOutput;
    QUrl currentFile;
    bool playing;
};
#endif // MAINWINDOW_H
