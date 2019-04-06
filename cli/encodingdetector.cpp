#include <QDataStream>
#include "encodingdetector.h"
#include <qssgeneralexception.h>
EncodingDetector::EncodingDetector()
{

}


QString EncodingDetector::detectEncoding(const QByteArray &data) const
{
    uchardet_t detector = uchardet_new();
    if(uchardet_handle_data(detector, data.data(), data.size()) != 0 )
    {
      uchardet_delete(detector);
      throw QSSGeneralException("Decoder failed");
    }
    uchardet_data_end(detector);
    QString encoding = uchardet_get_charset(detector);
    uchardet_delete(detector);
    return encoding;

}
QString EncodingDetector::detectEncoding(QDataStream &s) const
{
    uchardet_t detector = uchardet_new();

    char buffer[4096];
    int n;
    while((n = s.readRawData(buffer, sizeof(buffer))) > 0)
    {
        if(uchardet_handle_data(detector, buffer, n) != 0)
        {
            uchardet_delete(detector);

            throw QSSGeneralException("Decoder failed");
        }
    }
    if(n == -1)
    {
        uchardet_delete(detector);
        throw QSSGeneralException("Read failed");
    }
    uchardet_data_end(detector);
    QString encoding = uchardet_get_charset(detector);
    uchardet_delete(detector);
    return encoding;
}


