#ifndef MYVIDEOTHREAD_H
#define MYVIDEOTHREAD_H

#include <QThread>

#include "../mediadecode.h"
#include "../video_swsscale.h"
#include "src/widget/myopenglwidget.h"


class MyVideoThread: public QThread
{
public:
    MyVideoThread();
    ~MyVideoThread();

    bool openHaveSws(AVCodecParameters *para,MyOpenGLWidget *openglplay); // 打开，不管成功与否都清理
    bool openNoSws(AVCodecParameters *para,MyOpenGLWidget *openglplay);
    void push(AVPacket *pkt); // 将AVPacket加入到队列中等待解码转换
private:
    void close();

public:
    // 最大队列
    int _maxList = 100;
    bool _isExit = false;
    int UseYUV=1;

private:
    QList <AVPacket *> _packets;
    std::mutex _mutex;
    MediaDecode _decode;
    Video_swsscale *_sws=nullptr;
    MyOpenGLWidget *_videoplay=nullptr;

    // QThread interface
protected:
    virtual void run() override;
};

#endif // MYVIDEOTHREAD_H
