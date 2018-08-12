#ifndef CLICKLABEL_H
#define CLICKLABEL_H
#include <QLabel>

class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel;
signals:
    void clicked();
protected:
        void mousePressEvent(QMouseEvent *event);

};

#endif // CLICKLABEL_H
