#ifndef EDITSONGMETADATADIALOG_H
#define EDITSONGMETADATADIALOG_H

#include <QDialog>

namespace Ui {
class EditSongMetaDataDialog;
}

class EditSongMetaDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditSongMetaDataDialog(QWidget *parent = nullptr);
    ~EditSongMetaDataDialog();

signals:
    void songMetaDataEdited(QString songName, QString artist, QString dateAdded);

private slots:
    void on_submitButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::EditSongMetaDataDialog *ui;
    QString newSongName;
    QString newArtist;
    QString newDateAdded;
};

#endif // EDITSONGMETADATADIALOG_H
