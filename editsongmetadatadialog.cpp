#include "editsongmetadatadialog.h"
#include "ui_editsongmetadatadialog.h"

EditSongMetaDataDialog::EditSongMetaDataDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditSongMetaDataDialog)
{
    ui->setupUi(this);
}

EditSongMetaDataDialog::~EditSongMetaDataDialog()
{
    delete ui;
}


void EditSongMetaDataDialog::on_submitButton_clicked()
{
    newSongName = this->ui->newSongNameInput->text();
    newArtist = this->ui->newArtistInput->text();
    newDateAdded = this->ui->newDateAddedInput->text();
    emit songMetaDataEdited(newSongName, newArtist, newDateAdded);
    this->close();
}

void EditSongMetaDataDialog::on_cancelButton_clicked()
{
    this->close();
}
