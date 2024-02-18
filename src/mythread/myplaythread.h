#ifndef MYPLAYTHREAD_H
#define MYPLAYTHREAD_H

#include "src/mythread/myaudiothread.h"
#include "src/mythread/myvideothread.h"
#include <QWaitCondition>
#include <QMutex>

class MyPlayThread: public QThread
{
    Q_OBJECT
public:
    MyPlayThread();
    ~MyPlayThread();

    bool open(const QString &url,MyOpenGLWidget *opengl,bool UseYUV,
              MyAudioPlay *audioplay,bool UseSDL);
    bool start();
    void pause();
    void resume();
    void resumeSelf();
    void resumeOther();
    void stop();

    bool seek();

    bool _isExit = false;
    bool _isPause = false;
    bool _isSeek = false;

    qint64 seek_time() const;
    void setSeek_time(qint64 newSeek_time);

    double speed() const;
    void setSpeed(double newSpeed);

private:
    void close();

signals:
    void setTotalTime(qint64);
    void setProgressTime(qint64);
    void seekPreciseFrame();

private:
    MediaDemux _demux;

    MyaudioThread _audioplay;
    MyVideoThread _videoplay;

    QMutex _mutex;
    std::mutex _mutex1;
    QWaitCondition _condition;

    qint64 _seek_time=-1;
    double _speed=1;

    // QThread interface
protected:
    virtual void run() override;
};

#endif // MYPLAYTHREAD_H
