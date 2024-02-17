#ifndef MYVIDEOTHREAD_H
#define MYVIDEOTHREAD_H

#include <QThread>

#include "../mediadecode.h"
#include "../video_swsscale.h"
#include "src/widget/myopenglwidget.h"
#include <QWaitCondition>
#include <QMutex>

class MyaudioThread;
class MyVideoThread: public QThread
{
    Q_OBJECT
public:
    MyVideoThread();
    ~MyVideoThread();

    void init();
    bool openHaveSws(AVCodecParameters *para,MyOpenGLWidget *openglplay); // 打开，不管成功与否都清理
    bool openNoSws(AVCodecParameters *para,MyOpenGLWidget *openglplay);
    void push(AVPacket *pkt); // 将AVPacket加入到队列中等待解码转换

    void pause();
    void resume();
    void stop();

    void flushBuffers();

private:
    void close();

signals:
    void setProgressTime(qint64);

public:
    // 最大队列
    int _maxList = 100;
    bool _isExit = false;
    bool _isPause = false;

    int _UseYUV=1;

    qint64 pts() const;

    AVRational time_base() const;
    void setTime_base(const AVRational &newTime_base);

    long fps() const;
    void setFps(long newFps);

    double total_frame_delay() const;

    double speed() const;
    void setSpeed(double newSpeed);

private:
    QList <AVPacket *> _packets;
    MediaDecode _decode;
    Video_swsscale *_sws=nullptr;
    MyOpenGLWidget *_videoplay=nullptr;

    qint64 _pts=-1;
    AVRational _time_base;
    double _speed=1;
    long _fps;
    double _extra_delay;
    double _total_frame_delay;

    QMutex _mutex;
    QWaitCondition _condition;

    // QThread interface
protected:
    virtual void run() override;
};

#endif // MYVIDEOTHREAD_H
