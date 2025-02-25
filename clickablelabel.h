#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QObject>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    ClickableLabel(const QString& text = "", QWidget* parent = nullptr, QUrl path = QUrl(""), QString type = "playlist");
    ~ClickableLabel();
    QUrl getPath();
    QString getType();
signals:
    void clicked(QUrl path, QString type);
    void doubleClicked(QUrl path, QString type);
private:
    QUrl path;
    QString type;
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
};

#endif // CLICKABLELABEL_H
