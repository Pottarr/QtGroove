#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QAudioOutput>
#include <QFileDialog>
#include <QUrl>
#include <QSqlDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include "clickablelabel.h"

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
    void createPlaylist(const QString&);


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

    void on_volumeSlider_valueChanged(int value);

    void on_createPlaylist_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audio = new QAudioOutput;
    QUrl currentFile;
    QVBoxLayout* playlistSlot;
    std::vector<ClickableLabel*> playlistVec;
    bool playing = false;

    void changePlayButtonIcon(bool);

    QString getSongWholeDuration();

    QString getSongCurrentDuration(int);

    QString getSongLeftDuration(int);

    void createDBFile();

    void refresh();

    void playlistClicked(QUrl path, QString type);
};
#endif // MAINWINDOW_H
