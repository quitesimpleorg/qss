#include <QMouseEvent>
#include "clicklabel.h"

void ClickLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit leftClick();
    }
    return QLabel::mousePressEvent(event);
}
