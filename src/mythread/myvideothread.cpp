#include "myvideothread.h"
#include "./myaudiothread.h"
#include "../mediademux.h"

MyVideoThread::MyVideoThread()
{

}

MyVideoThread::~MyVideoThread()
{
    close();
}

void MyVideoThread::init()
{
    _isExit=false;
    _isPause=false;
}

bool MyVideoThread::openHaveSws(AVCodecParameters *para, MyOpenGLWidget *openglplay)
{
    if (!para){
        qWarning()<<"AVCodecParameters has the problem!";
        return false;
    }

    _mutex.lock();

    this->_UseYUV=0;
    this->_videoplay=openglplay;

    if (!_sws){
        _sws = new Video_swsscale();
        _sws->setBufferSize(*para);
    }


    if (!_decode.setCodec(para)){
        qWarning() << "video Decode open failed!" << Qt::endl;
        _mutex.unlock();

        return false;
    }
    qDebug() << "VideoThread::Open success!" << Qt::endl;
    _mutex.unlock();

    return true;
}

bool MyVideoThread::openNoSws(AVCodecParameters *para, MyOpenGLWidget *openglplay)
{
    if (!para){
        qWarning()<<"AVCodecParameters has the problem!";
        return false;
    }

    _mutex.lock();

    this->_UseYUV=1;
    this->_videoplay=openglplay;

    if (!_decode.setCodec(para)){
        qWarning() << "video Decode open failed!" << Qt::endl;
        _mutex.unlock();

        return false;
    }
    qDebug() << "VideoThread::Open success!" << Qt::endl;
    _mutex.unlock();

    return true;
}

void MyVideoThread::push(AVPacket *pkt)
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

void MyVideoThread::pause()
{
    _mutex.lock();
    _isPause=true;
    _mutex.unlock();
}

void MyVideoThread::resume()
{
    _mutex.lock();
    _condition.wakeOne();
    _isPause=false;
    _mutex.unlock();


}

void MyVideoThread::stop()
{
    _mutex.lock();

    _isExit = true;
    _condition.wakeOne(); // 唤醒等待的线程以便它可以退出循环

    _mutex.unlock();
}

void MyVideoThread::flushBuffers()
{
    avcodec_flush_buffers(_decode.codeccontext());
    if(!_packets.isEmpty()){
        _packets.clear();
    }
}

void MyVideoThread::run()
{
    while (!_isExit){
        _mutex.lock();

        if(_isPause){
            _condition.wait(&_mutex);
        }

        //没有数据
        if (_packets.empty()){
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

            _extra_delay=frame->repeat_pict/(_fps*2);
            _total_frame_delay=(1.0/_fps)+_extra_delay;

            _pts=_decode.pts();
//            _pts=frame->best_effort_timestamp*av_q2d(_time_base);
            emit setProgressTime(_pts/1000);




            if(!_UseYUV){
                QImage image=_sws->getScaleImage(_videoplay->width(),_videoplay->height(),frame);
//                    w.ui->video->setPixmap(QPixmap::fromImage(image));
                if(image.format()==QImage::Format_Invalid){
                    continue;
                }
                QMetaObject::invokeMethod(_videoplay,[&]{

                    _videoplay->updateImage(image);
                });
            }
            else{
                QMetaObject::invokeMethod(_videoplay,[&]{

                    _videoplay->updateImage(frame);
                });
            }
//            QThread::msleep(_total_frame_delay*1000);
            //音视频同步
            if(MediaDemux::getType() == MediaDemux::DOUBLE_AV &&
                MyaudioThread::pts()!=-1 &&_pts!=0){
                    double audio_pts=(MyaudioThread::pts()/1000.0);
                    double dely=audio_pts-_pts/1000.0;
//                    qDebug()<<"audio_pts:"<<audio_pts<<"- videoo_pts:"<<_pts/1000.0<<"= dely:"<<dely;
                    if(dely>0){
                    QThread::msleep((_total_frame_delay*1000+dely)/_speed);
                    }
                    else if(dely<0){
                        if(fabs(dely)>0.05){
                            msleep(10);
                            continue;
                        }
                        else{
                            QThread::msleep(_total_frame_delay*1000/_speed);
                        }
                    }
            }
            else{

                QThread::msleep(_total_frame_delay*1000/_speed);

            }
        }
        _mutex.unlock();

    }
}

void MyVideoThread::close()
{
    if(_sws){
        delete _sws;
        _sws=nullptr;
    }
    if(_videoplay){
        _videoplay=nullptr;
    }
}

qint64 MyVideoThread::pts() const
{
    return _pts;
}

AVRational MyVideoThread::time_base() const
{
    return _time_base;
}

void MyVideoThread::setTime_base(const AVRational &newTime_base)
{
    _time_base = newTime_base;
}

long MyVideoThread::fps() const
{
    return _fps;
}

void MyVideoThread::setFps(long newFps)
{
    _fps = newFps;
}

double MyVideoThread::total_frame_delay() const
{
    return _total_frame_delay;
}

double MyVideoThread::speed() const
{
    return _speed;
}

void MyVideoThread::setSpeed(double newSpeed)
{
    _speed = newSpeed;
}
