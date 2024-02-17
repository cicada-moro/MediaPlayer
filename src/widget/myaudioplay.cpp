#include "myaudioplay.h"

MyAudioPlay::MyAudioPlay()
{

}

MyAudioPlay::~MyAudioPlay()
{
    close();
}

bool MyAudioPlay::open()
{
    close();
    std::unique_lock<std::mutex> guard(_mutex);


    //初始化音频输出设备
    QMediaDevices *m_devices=new  QMediaDevices();
    QAudioDevice m_device=m_devices->defaultAudioOutput();
    QAudioFormat fmt=m_device.preferredFormat();
    _m_sink=new QAudioSink(m_device,fmt);
    _m_sink->setVolume(0.3);


    //IODevice获取本机扬声器
    _io=_m_sink->start();
    _io->open(QIODevice::ReadWrite);

    if(_io)
        return true;
    return false;
}

void MyAudioPlay::close()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if (_io)
    {
        _io->close();
        _io=nullptr;
    }
    if (_m_sink)
    {
        _m_sink->stop();
        delete _m_sink;
        _m_sink=nullptr;
    }
}

bool MyAudioPlay::write(const uchar *data, int datasize)
{
    if (!data || datasize <= 0){
        return false;
    }
    std::unique_lock<std::mutex> guard(_mutex);

    if (!_m_sink || !_io)
    {
        return false;
    }
    int size = _io->write((char *)data, datasize);
    if (datasize != size)
        return false;
    return true;
}

int MyAudioPlay::getFree()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if (!_m_sink)
    {
        return 0;
    }
    int free = _m_sink->bytesFree();
    return free;
}

void MyAudioPlay::setVolume(qreal value)
{
    if(!_m_sink){
        return;
    }
    std::unique_lock<std::mutex> guard(_mutex);

    _m_sink->setVolume(value);
}

void MyAudioPlay::flushBuffer()
{
    std::unique_lock<std::mutex> guard(_mutex);
    if(_m_sink){
        _m_sink->reset();
        _io->reset();
    }
}


