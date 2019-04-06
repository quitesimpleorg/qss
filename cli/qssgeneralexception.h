#ifndef QSSGENERALEXCEPTION_H
#define QSSGENERALEXCEPTION_H

#include <QException>

class QSSGeneralException : public QException
{
public:
    QString message;
    QSSGeneralException(QString message) { this->message = message; };
    void raise() const override { throw *this; }
    QSSGeneralException *clone() const override { return new QSSGeneralException(*this); }
};

#endif // QSSGENERALEXCEPTION_H
