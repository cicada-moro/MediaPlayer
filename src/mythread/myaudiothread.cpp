#include "myaudiothread.h"
#include "SDL/SDL_audio.h"
#include "src/widget/sdlaudioplay.h"
#include <QDebug>

//定义音频数据格式
#define AUDIO_FORMAT AV_SAMPLE_FMT_S16

MyaudioThread::MyaudioThread()
{

}

MyaudioThread::~MyaudioThread()
{
    close();
}

void MyaudioThread::init()
{
    _isExit=false;
    _isPause=false;
}

bool MyaudioThread::open(AVStream *stream,MyAudioPlay *audioplay)
{
    if (!stream){
        qWarning()<<"AVStream has the problem!";
        return false;
    }

    _mutex.lock();

    _sonicstream=sonicCreateStream(stream->codecpar->sample_rate,stream->codecpar->channels);

    audioplay->setAudioPara(stream->codecpar->sample_rate,stream->codecpar->frame_size,
                            AUDIO_FORMAT,stream->codecpar->channels);
    this->_audioplay=audioplay;

    if (!_res){
        _res = new Audio_resample();
    }

//    if (!_audioplay->open())
//    {
//        qWarning() << "MyAudioPlay open failed!" << Qt::endl;
//        return false;
//    }
//    _audioplay.moveToThread(this);

    if (!_decode.setCodec(stream)){
        qWarning() << "audio Decode open failed!" << Qt::endl;
        _mutex.unlock();

        return false;
    }
    qDebug() << "AudioThread::Open success!" << Qt::endl;
    _mutex.unlock();

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
        _mutex.lock();

        if (_packets.size() < _maxList){
            _packets.emplaceBack(pkt);
            _mutex.unlock();
            break;
        }
        _mutex.unlock();
    }
}

void MyaudioThread::pause()
{
    _mutex.lock();
    _isPause=true;
    _mutex.unlock();

}

void MyaudioThread::resume()
{
    _mutex.lock();
    _condition.wakeOne();
    _isPause=false;
    _mutex.unlock();

}

void MyaudioThread::stop()
{
    _mutex.lock();

    _isExit = true;
    _condition.wakeOne(); // 唤醒等待的线程以便它可以退出循环

    _mutex.unlock();
}

void MyaudioThread::flushBuffers()
{
    avcodec_flush_buffers(_decode.codeccontext());
    if(!_packets.isEmpty()){
        _packets.clear();
    }
    _audioplay->flushBuffer();
}

void MyaudioThread::run()
{
    //音频播放器需要在子线程中打开，不然会出现Timers cannot be started from another thread
    //且音频输出速度会收到影响
    if (!_audioplay->open())
    {
        qWarning() << "MyAudioPlay open failed!" << Qt::endl;
        return;
    }
    uchar* pcm = new uchar[1024 * 1024];
    while (!_isExit){
        _mutex.lock();

        if(_isPause){
            _condition.wait(&_mutex);
        }

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
            if(_isPause){
                _condition.wait(&_mutex);
            }

            AVFrame * frame = _decode.receive();
            if (!frame){
                break;
            }
            //重采样
            int len = _res->getResample(AUDIO_FORMAT,frame, pcm);
            //播放音频

            if(_speed!=1){
                short * pcm_sonic=(short*)pcm ;
                if(len ==0 ){
                    sonicFlushStream(_sonicstream);
                }else{
//                    由于sonic库处理的数据为short类型或者float类型，但是ffmpeg重采样后的数据和
//                    SDL使用的数据都是uint8类型，所以这里数据长度就需要重新计算：
//                    原始的数据长度/(音频通道数 * S16类型数据所占字节数)
                    len=len/frame->channels/ av_get_bytes_per_sample(AUDIO_FORMAT);
                    int ret = sonicWriteShortToStream(_sonicstream, pcm_sonic, len);
                    if(!ret){
                        qDebug()<<"sonic false!!";
                    }
                    else{
                        int sample = sonicSamplesAvailable(_sonicstream);
                        len = sonicReadShortFromStream(_sonicstream, pcm_sonic, sample);
                        len=len*frame->channels*av_get_bytes_per_sample(AUDIO_FORMAT);
                        pcm=(uchar*)pcm_sonic;
                    }
                }
            }

            _pts=_decode.pts();
//            _pts=frame->pts*av_q2d(_time_base);
            emit setProgressTime(_pts/1000);

            while (!_isExit){
                if(!_usesdl){
                    while(_audioplay->getFree()<len){
                        QThread::msleep(1);
                    }
                    _audioplay->write(pcm, len);
                    break;
                }
                else{
//                    static_cast<SDLAudioPlay*>(_audioplay)->_sdlAudioSpec.freq/=2;
                    _audioplay->write(pcm,len);
                    break;
                }
            }
        }
        _mutex.unlock();

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
    if(_audioplay){
        _audioplay=nullptr;
    }
}

bool MyaudioThread::usesdl() const
{
    return _usesdl;
}

void MyaudioThread::setUsesdl(bool newUsesdl)
{
    _usesdl = newUsesdl;
}

qint64 MyaudioThread::_pts=-1;
qint64 MyaudioThread::pts()
{
    return _pts;
}

AVRational MyaudioThread::getTime_base() const
{
    return _time_base;
}

void MyaudioThread::setTime_base(const AVRational &newTime_base)
{
    _time_base = newTime_base;
}

double MyaudioThread::speed() const
{
    return _speed;
}

void MyaudioThread::setSpeed(double newSpeed)
{
    sonicSetSpeed(_sonicstream,newSpeed);
    sonicSetPitch(_sonicstream, 1.0);
    sonicSetRate(_sonicstream, 1.0);
    _speed = newSpeed;
}
