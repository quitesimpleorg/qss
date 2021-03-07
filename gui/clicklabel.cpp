#include <QMouseEvent>
#include "clicklabel.h"

void ClickLabel::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		emit leftClick();
	}
	if(event->button() == Qt::RightButton)
	{
		emit rightClick();
	}
	return QLabel::mousePressEvent(event);
}
