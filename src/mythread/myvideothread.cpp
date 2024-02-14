#include "myvideothread.h"

MyVideoThread::MyVideoThread()
{

}

MyVideoThread::~MyVideoThread()
{
    close();
}

bool MyVideoThread::openHaveSws(AVCodecParameters *para, MyOpenGLWidget *openglplay)
{
    if (!para){
        qWarning()<<"AVCodecParameters has the problem!";
        return false;
    }

    std::unique_lock<std::mutex> guard(_mutex);

    this->UseYUV=0;
    this->_videoplay=openglplay;

    if (!_sws){
        _sws = new Video_swsscale();
        _sws->setBufferSize(*para);
    }


    if (!_decode.setCodec(para)){
        qWarning() << "video Decode open failed!" << Qt::endl;
        return false;
    }
    qDebug() << "VideoThread::Open success!" << Qt::endl;
    return true;
}

bool MyVideoThread::openNoSws(AVCodecParameters *para, MyOpenGLWidget *openglplay)
{
    if (!para){
        qWarning()<<"AVCodecParameters has the problem!";
        return false;
    }

    std::unique_lock<std::mutex> guard(_mutex);

    this->UseYUV=1;
    this->_videoplay=openglplay;

    if (!_decode.setCodec(para)){
        qWarning() << "video Decode open failed!" << Qt::endl;
        return false;
    }
    qDebug() << "VideoThread::Open success!" << Qt::endl;
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
        std::unique_lock<std::mutex> guard(_mutex);

        if (_packets.size() < _maxList){
            _packets.emplaceBack(pkt);
            _mutex.unlock();
            break;
        }

    }
}

void MyVideoThread::run()
{
    while (!_isExit){
        std::unique_lock<std::mutex> guard(_mutex);

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
            AVFrame * frame = _decode.receive();
            if (!frame){
                break;
            }
            if(!UseYUV){
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
            QThread::msleep(40);
        }
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
