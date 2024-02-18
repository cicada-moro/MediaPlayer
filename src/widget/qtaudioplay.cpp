#include "qtaudioplay.h"

QtAudioplay::QtAudioplay()
{

}

QtAudioplay::~QtAudioplay()
{
    QtAudioplay::close();
}

bool QtAudioplay::write(const uchar *data, int datasize)
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

int QtAudioplay::getFree()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if (!_m_sink)
    {
        return 0;
    }
    int free = _m_sink->bytesFree();
    return free;
}

bool QtAudioplay::open()
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

void QtAudioplay::close()
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

void QtAudioplay::setVolume(qreal value)
{
    if(!_m_sink){
        return;
    }
    std::unique_lock<std::mutex> guard(_mutex);

    _m_sink->setVolume(value);
}

void QtAudioplay::flushBuffer()
{
    std::unique_lock<std::mutex> guard(_mutex);
    if(_m_sink){
        _m_sink->reset();
        _io->reset();
    }
}
