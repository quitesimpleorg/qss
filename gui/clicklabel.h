#ifndef CLICKLABEL_H
#define CLICKLABEL_H
#include <QLabel>
#include <QMenu>

class ClickLabel : public QLabel
{
	Q_OBJECT
public:
	using QLabel::QLabel;
signals:
	void leftClick();
	void rightClick();
protected:
	void mousePressEvent(QMouseEvent *event);
};

#endif // CLICKLABEL_H
