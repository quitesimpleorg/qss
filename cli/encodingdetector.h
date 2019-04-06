#ifndef ENCODINGDETECTOR_H
#define ENCODINGDETECTOR_H
#include <QString>
#include <uchardet/uchardet.h>
class EncodingDetector
{

public:
    EncodingDetector();
    QString detectEncoding(const QByteArray &data) const;
    QString detectEncoding(QDataStream &s) const;
};

#endif // ENCODINGDETECTOR_H
