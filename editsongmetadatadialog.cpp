#include "editsongmetadatadialog.h"
#include "ui_editsongmetadatadialog.h"

EditSongMetaDataDialog::EditSongMetaDataDialog(QWidget *parent, const QString &songPath, const QString &songName, const QString &authorName, const QString &dateAdded)
    : QDialog(parent)
    , ui(new Ui::EditSongMetaDataDialog)
{
    ui->setupUi(this);
    sameSongPath = songPath;
    this->ui->newSongNameInput->setText(songName);
    this->ui->newAuthorNameInput->setText(authorName);
    this->ui->newDateAddedInput->setText(dateAdded);
}

EditSongMetaDataDialog::~EditSongMetaDataDialog()
{
    delete ui;
}


void EditSongMetaDataDialog::on_submitButton_clicked()
{
    newSongName = this->ui->newSongNameInput->text();
    newAuthorName = this->ui->newAuthorNameInput->text();
    newDateAdded = this->ui->newDateAddedInput->text();
    emit songMetaDataEdited(sameSongPath, newSongName, newAuthorName, newDateAdded);
    this->close();
}

void EditSongMetaDataDialog::on_cancelButton_clicked()
{
    this->close();
}
