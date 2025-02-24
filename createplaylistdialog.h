#ifndef CREATEPLAYLISTDIALOG_H
#define CREATEPLAYLISTDIALOG_H

#include <QDialog>

namespace Ui {
class CreatePlaylistDialog;
}

class CreatePlaylistDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatePlaylistDialog(QWidget *parent);
    ~CreatePlaylistDialog();

signals:
    void playlistCreated(const QString &playlistName);

private slots:
    void on_submitButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::CreatePlaylistDialog *ui;
    QString playlistName;
};

#endif // CREATEPLAYLISTDIALOG_H
