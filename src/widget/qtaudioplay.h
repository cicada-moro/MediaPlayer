#ifndef QTAUDIOPLAY_H
#define QTAUDIOPLAY_H

#include "myaudioplay.h"
#include <mutex>
#include <QIODevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSink>

class QtAudioplay : public MyAudioPlay
{
public:
    QtAudioplay();
    ~QtAudioplay();


    // MyAudioPlay interface
public:
    virtual bool open() override;
    virtual void close() override;
    virtual void setVolume(qreal) override;
    virtual void flushBuffer() override;
    virtual bool write(const uchar *data, int datasize) override;
    virtual int getFree() override;

private:
    QIODevice *_io = nullptr;
    QAudioSink *_m_sink=nullptr;

    // MyAudioPlay interface

};


#endif // QTAUDIOPLAY_H
