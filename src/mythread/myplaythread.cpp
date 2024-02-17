#include "myplaythread.h"

MyPlayThread::MyPlayThread()
{

}

MyPlayThread::~MyPlayThread()
{
    close();
}

bool MyPlayThread::open(const QString &url,MyOpenGLWidget *opengl,bool UseYUV,MyAudioPlay *audioplay)
{
    if(url.isNull() || url.isEmpty()){
        qWarning()<<"The url is null or empty";
        return false;
    }

    _mutex.lock();

    bool ret=true;

    std::thread t([&,ref(url)](){
        ret=_demux.openUrl(url);
        qDebug() << "demux.Open = " << ret;
        qDebug() << "CopyVPara = " << _demux.copyVPara() << Qt::endl;
        qDebug() << "CopyAPara = " << _demux.copyAPara() << Qt::endl;
        //        qDebug() << "seek=" << demux.seek(0.95) << Qt::endl;
    });
    t.join();
    if(!ret){
        qWarning()<<"The url open false!";
        _mutex.unlock();

        return false;
    }

    emit setTotalTime(_demux.total_time());


    if(_demux.type == _demux.ONLY_AUDIO ||
       _demux.type == _demux.DOUBLE_AV){

        _audioplay.setTime_base(_demux.copyAStream()->time_base);


        ret=_audioplay.open(_demux.copyAStream(),audioplay);
        if(!ret){
            _mutex.unlock();

            return false;
        }
    }


    if(_demux.type == _demux.ONLY_VIDEO ||
       _demux.type == _demux.DOUBLE_AV){

        _videoplay.setTime_base(_demux.copyVStream()->time_base);
        _videoplay.setFps(_demux.video_framerate());

        if(!UseYUV){
            ret=_videoplay.openNoSws(_demux.copyVPara(),opengl);
        }
        else{
            ret=_videoplay.openHaveSws(_demux.copyVPara(),opengl);
        }
        if(!ret){
            _mutex.unlock();

            return false;
        }
    }

    qDebug() << "PlayThread::Open success!" << Qt::endl;

    _mutex.unlock();

    return true;
}

bool MyPlayThread::start()
{
    _mutex.lock();

    if(_demux.type == _demux.ONLY_AUDIO ||
       _demux.type == _demux.DOUBLE_AV){
        if(_demux.type == _demux.ONLY_AUDIO){
            connect(&_audioplay,&MyaudioThread::setProgressTime,this,[&](qint64 time){
                emit setProgressTime(time);
            },Qt::QueuedConnection);
        }

        _audioplay.init();
        _audioplay.start();
        if(!_audioplay.isRunning()){
            return false;
        }
    }

    if(_demux.type == _demux.ONLY_VIDEO ||
       _demux.type == _demux.DOUBLE_AV){
        connect(&_videoplay,&MyVideoThread::setProgressTime,this,[&](qint64 time){
            emit setProgressTime(time);
        },Qt::QueuedConnection);

        _videoplay.init();
        _videoplay.start();
        if(!_videoplay.isRunning()){
            _audioplay.stop();
            _audioplay.wait();  //必须等待线程结束;
            return false;
        }
    }

    _isExit=false;
    _isPause=false;
    QThread::start();

    _mutex.unlock();

    qDebug()<<"Playing start!";
    return true;
}

void MyPlayThread::pause()
{
    std::unique_lock<std::mutex> guard(_mutex1);

    _isPause=true;

    if(_demux.type == _demux.ONLY_AUDIO ||
       _demux.type == _demux.DOUBLE_AV){
        _audioplay.pause();
    }
    if(_demux.type == _demux.ONLY_VIDEO ||
       _demux.type == _demux.DOUBLE_AV){
        _videoplay.pause();
    }

    qDebug()<<"Playing pause!";

}

void MyPlayThread::resume()
{
    _mutex.lock();

    _isPause=false;

    _condition.wakeOne();
    if(_demux.type == _demux.ONLY_AUDIO ||
       _demux.type == _demux.DOUBLE_AV){
        _audioplay.resume();
    }
    if(_demux.type == _demux.ONLY_VIDEO ||
       _demux.type == _demux.DOUBLE_AV){
        _videoplay.resume();
    }

    qDebug()<<"Playing resume!";
    _mutex.unlock();
}

void MyPlayThread::resumeSelf()
{
    _mutex.lock();

    _isPause=false;

    _condition.wakeOne();

    _mutex.unlock();
}

void MyPlayThread::resumeOther()
{
    std::unique_lock<std::mutex> guard(_mutex1);

    if(_demux.type == _demux.ONLY_AUDIO ||
        _demux.type == _demux.DOUBLE_AV){
        _audioplay.resume();
    }
    if(_demux.type == _demux.ONLY_VIDEO ||
        _demux.type == _demux.DOUBLE_AV){
        _videoplay.resume();
    }

}

void MyPlayThread::stop()
{
    _isExit = true;
    _condition.wakeOne(); // 唤醒等待的线程以便它可以退出循环

    if(_demux.type == _demux.ONLY_VIDEO ||
       _demux.type == _demux.DOUBLE_AV){
        _videoplay.stop();
        _videoplay.deleteLater();

        _videoplay.wait();  //必须等待线程结束;
        _videoplay.flushBuffers();
    }

    if(_demux.type == _demux.ONLY_AUDIO ||
       _demux.type == _demux.DOUBLE_AV){
        _audioplay.stop();
        _audioplay.deleteLater();

        _audioplay.wait();  //必须等待线程结束;
    }

}

bool MyPlayThread::seek()
{
    std::unique_lock<std::mutex> guard(_mutex1);

    if(_demux.type == _demux.ONLY_VIDEO ||
        _demux.type == _demux.DOUBLE_AV){
        _videoplay.flushBuffers();
    }
    if(_demux.type == _demux.ONLY_AUDIO ||
        _demux.type == _demux.DOUBLE_AV){
    }
    return _demux.seek(_seek_time) == true ? true : false;
}

void MyPlayThread::run()
{
    while(!_isExit){
        _mutex.lock();

        if(_isPause){
            _condition.wait(&_mutex);
        }

        AVPacket *pkt=_demux.read();
        if (!pkt){
            if(!_demux.isEnd()){
                _mutex.unlock();
                continue;
            }

            // 异步线程退出后，才清空销毁

//            _demux.close();
            _demux.clear();
            if(_demux.type == _demux.ONLY_VIDEO ||
                _demux.type == _demux.DOUBLE_AV){
                _videoplay.stop();
                _videoplay.wait();  //必须等待线程结束;
            }
            if(_demux.type == _demux.ONLY_AUDIO ||
                _demux.type == _demux.DOUBLE_AV){
                _audioplay.stop();
                _audioplay.wait();  //必须等待线程结束;
            }
            _mutex.unlock();

            break;
        }

        if(_demux.isAudio(pkt)){
            _audioplay.push(pkt);
        }
        else{
            _videoplay.push(pkt);
        }

        //尝试精确seek，但效率变低
//        if(abs(pkt->pts/1000-_seek_time)>=1 && _isSeek){
//            _mutex.unlock();
//            continue;
//        }
//        else{
//            if(_demux.isAudio(pkt)){
//                _audioplay.push(pkt);
//            }
//            else{
//                _videoplay.push(pkt);
//            }
//            if(_isSeek){
//                _isSeek=false;
//                qDebug()<<pkt->pts/1000<<_seek_time<<_isSeek;
//                emit seekPreciseFrame();
//            }
//        }

        _mutex.unlock();
    }
}

void MyPlayThread::close()
{

}

double MyPlayThread::speed() const
{
    return _speed;
}

void MyPlayThread::setSpeed(double newSpeed)
{
    _speed = newSpeed;
    _audioplay.setSpeed(newSpeed);
    _videoplay.setSpeed(newSpeed);
}

qint64 MyPlayThread::seek_time() const
{
    return _seek_time;
}

void MyPlayThread::setSeek_time(qint64 newSeek_time)
{
    _seek_time = newSeek_time;
}
