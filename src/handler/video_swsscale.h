#ifndef VIDEO_SWSSCALE_H
#define VIDEO_SWSSCALE_H

#include <QObject>
#include "mediademux.h"
#include <QImage>


class Video_swsscale
{
public:
    Video_swsscale();
    ~Video_swsscale();
private:
    bool setSwsContext(double des_width,
                       double des_height,
                       AVFrame *frame);
    void clear();
public:
    void setBufferSize(MediaDemux &demux);
    void setBufferSize(AVCodecParameters &para);
    QImage getScaleImage(double des_width,
                         double des_height,
                         AVFrame *frame);
private:
    uchar *_m_buffer=nullptr;
    SwsContext * _m_swsContext=nullptr;
    std::mutex _mutex; // 互斥锁
};

#endif // VIDEO_SWSSCALE_H
