#ifndef MYAUDIOTHREAD_H
#define MYAUDIOTHREAD_H

#include <QThread>

#include "../mediadecode.h"
#include "../audio_resample.h"
#include "src/widget/myaudioplay.h"
#include <QWaitCondition>
#include <QMutex>


#ifdef __cplusplus
extern "C" {

#include "src/sonic/sonic.h"

#endif
#ifdef __cplusplus
}
#endif


class MyaudioThread: public QThread
{
    Q_OBJECT
public:
    MyaudioThread();
    ~MyaudioThread();

    void init();
    bool open(AVStream *stream,MyAudioPlay *audioplay); // 打开，不管成功与否都清理
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

    bool _usesdl;

    qint64 static pts();

    AVRational getTime_base() const;
    void setTime_base(const AVRational &newTime_base);

    double speed() const;
    void setSpeed(double newSpeed);

    bool usesdl() const;
    void setUsesdl(bool newUsesdl);

private:
    QList <AVPacket *> _packets;

    MediaDecode _decode;
    Audio_resample *_res=nullptr;
    MyAudioPlay *_audioplay=nullptr;
    sonicStream _sonicstream;

    static qint64  _pts;
    AVRational _time_base;
    double _speed=1;

    QMutex _mutex;
    QWaitCondition _condition;

    // QThread interface
protected:
    virtual void run() override;
};

#endif // MYAUDIOTHREAD_H
