#ifndef MYAUDIOPLAY_H
#define MYAUDIOPLAY_H

#include <mutex>
#include <QIODevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSink>

class MyAudioPlay
{
public:
    MyAudioPlay();
    virtual ~MyAudioPlay();

    //打开音频播放
    virtual bool open()=0;
    virtual void close()=0;
    virtual void setVolume(qreal)=0;
    virtual void flushBuffer()=0;

    virtual bool write(const uchar *data, int datasize);
    virtual int getFree();

    void setAudioPara(int sampleRate,
                      int sampleSize,
                      int format,
                      int channels);

    int _sampleRate = 44100;
    int _sampleSize = 16;
    int _format=1;
    int _channels = 2;

public:
    static double _volume;

protected:
    std::mutex _mutex;
};

#endif // MYAUDIOPLAY_H
