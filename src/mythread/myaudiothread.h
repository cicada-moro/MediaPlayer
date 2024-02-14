#ifndef MYAUDIOTHREAD_H
#define MYAUDIOTHREAD_H

#include <QThread>

#include "../mediadecode.h"
#include "../audio_resample.h"
#include "src/widget/myaudioplay.h"

class MyaudioThread: public QThread
{
public:
    MyaudioThread();
    ~MyaudioThread();

    bool open(AVStream *stream); // 打开，不管成功与否都清理
    void push(AVPacket *pkt); // 将AVPacket加入到队列中等待解码转换
private:
    void close();

public:
    // 最大队列
    int _maxList = 1000;
    bool _isExit = false;

private:
    QList <AVPacket *> _packets;
    std::mutex _mutex;
    MediaDecode _decode;
    Audio_resample *_res=nullptr;
    MyAudioPlay _audioplay;

    // QThread interface
protected:
    virtual void run() override;
};

#endif // MYAUDIOTHREAD_H
