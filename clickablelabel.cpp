#include "clickablelabel.h"

ClickableLabel::ClickableLabel(const QString& text, QWidget* parent, QUrl path, QString type)
    : QLabel(parent), path(path), type(type)
{
    this->setText(text);
}
ClickableLabel::~ClickableLabel(){}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    emit clicked(path, type);
    this->setStyleSheet("background-color: rgb(0,0,255); color: rgb(255,255,255);");
}

void ClickableLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit doubleClicked(path, type);
    this->setStyleSheet("background-color: rgb(0,0,255); color: rgb(255,255,255);");
}

QUrl ClickableLabel::getPath()
{
    return path;
}
QString ClickableLabel::getType()
{
    return type;
}

