#include "myaudiothread.h"
#include <QDebug>

MyaudioThread::MyaudioThread()
{

}

MyaudioThread::~MyaudioThread()
{
    close();
}

bool MyaudioThread::open(AVStream *stream)
{
    if (!stream){
        qWarning()<<"AVStream has the problem!";
        return false;
    }

    std::unique_lock<std::mutex> guard(_mutex);

    if (!_res){
        _res = new Audio_resample();
    }

//    if (!_audioplay.open())
//    {
//        qWarning() << "MyAudioPlay open failed!" << Qt::endl;
//        return false;
//    }
//    _audioplay.moveToThread(this);

    if (!_decode.setCodec(stream)){
        qWarning() << "audio Decode open failed!" << Qt::endl;
        return false;
    }
    qDebug() << "AudioThread::Open success!" << Qt::endl;
    return true;
}

void MyaudioThread::push(AVPacket *pkt)
{
    if (!pkt){
        qWarning()<<"AVPacket has the problem!";
        return;
    }
    // 阻塞
    while (!_isExit){
        std::unique_lock<std::mutex> guard(_mutex);

        if (_packets.size() < _maxList){
            _packets.emplaceBack(pkt);
            _mutex.unlock();
            break;
        }
    }
}

void MyaudioThread::run()
{
    if (!_audioplay.open()){
        qWarning() << "MyAudioPlay open failed!" << Qt::endl;
        return;
    }

    uchar* pcm = new uchar[1024 * 1024];
    while (!_isExit){
        std::unique_lock<std::mutex> guard(_mutex);

        //没有数据
        if (_packets.empty() || !_res){
            _mutex.unlock();
            continue;
        }

        AVPacket *pkt = _packets.front();
        _packets.pop_front();
        bool ret = _decode.send(pkt);
        if (!ret){
            _mutex.unlock();
            continue;
        }
        //一次send 多次receive
        while (!_isExit){
            AVFrame * frame = _decode.receive();
            if (!frame){
                break;
            }
            //重采样
            int len = _res->getResample(AV_SAMPLE_FMT_S16,frame, pcm);
            //播放音频
            while (!_isExit){
                while(_audioplay.getFree()<len){
                    QThread::msleep(10);
                }
                _audioplay.write(pcm, len);
                break;
            }
        }

    }
    delete []pcm;
    pcm=nullptr;
}

void MyaudioThread::close()
{
    if(_res){
        delete _res;
        _res=nullptr;
    }
}
