#ifndef AUDIO_RESAMPLE_H
#define AUDIO_RESAMPLE_H

#include <mutex>
#include <QObject>

extern "C" {
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
}

class Audio_resample
{
public:
    Audio_resample();
    ~Audio_resample();
private:
    bool setSwrContext(int outformat,
                       AVFrame *frame);
    void clear();
public:
    int getResample(int outformat,
                    AVFrame *frame,
                    uchar* pcm);

private:
    SwrContext  *_m_swrContext=nullptr;

    std::mutex _mutex; // 互斥锁
};

#endif // AUDIO_RESAMPLE_H
