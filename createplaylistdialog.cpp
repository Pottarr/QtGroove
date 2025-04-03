#include "createplaylistdialog.h"
#include "ui_createplaylistdialog.h"

CreatePlaylistDialog::CreatePlaylistDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreatePlaylistDialog)
{

    ui->setupUi(this);
    setWindowTitle("QtGroove: Create Playlist");
    setWindowIcon(QIcon(":/icons/QtGroove.ico"));
    ui->playlistNameInput->setFocus();
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


void CreatePlaylistDialog::on_cancelButton_clicked()
{
    this->close();
}

