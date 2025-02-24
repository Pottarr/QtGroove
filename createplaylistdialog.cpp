#include "createplaylistdialog.h"
#include "ui_createplaylistdialog.h"
#include "mainwindow.h"

CreatePlaylistDialog::CreatePlaylistDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreatePlaylistDialog)
{
    ui->setupUi(this);
    // connect(this, &CreatePlaylistDialog::on_pushButton_clicked, parent, [this, parent]()
    // {
        // parent->createPlaylist(this->playlistName);
    // });
}

CreatePlaylistDialog::~CreatePlaylistDialog()
{
    delete ui;
}

void CreatePlaylistDialog::on_submitButton_clicked()
{
    this->playlistName = ui->playlistNameInput->text();
    emit playlistCreated(playlistName);
    this->close();

}

