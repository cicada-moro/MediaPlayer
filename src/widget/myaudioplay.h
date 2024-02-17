#ifndef MYAUDIOPLAY_H
#define MYAUDIOPLAY_H

#include <mutex>
#include <QIODevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSink>
#include <QThread>

class MyAudioPlay:public QThread
{
public:
    MyAudioPlay();
    ~MyAudioPlay();

    //打开音频播放
    bool open();
    void close();
    //播放音频
    bool write(const uchar *data, int datasize);
    int getFree();
    void setVolume(qreal);
    void flushBuffer();

    int sampleRate = 44100;
    int sampleSize = 16;
    int channels = 2;

private:
    QIODevice *_io = nullptr;
    QAudioSink *_m_sink=nullptr;
    double _volume;
    std::mutex _mutex;
};

#endif // MYAUDIOPLAY_H
