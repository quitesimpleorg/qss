#include "clicklabel.h"

void ClickLabel::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
