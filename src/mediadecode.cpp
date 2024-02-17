#include "mediadecode.h"
#include <QDebug>
#include <winsock.h>

#define ERROR_LEN 1024  // 异常信息数组长度
#define PRINT_LOG 0

MediaDecode::MediaDecode()
{
    _error = new char[ERROR_LEN];
}

MediaDecode::~MediaDecode()
{
    close();
}


bool MediaDecode::setCodec(AVStream *stream)
{
    close();

    std::unique_lock<std::mutex> guard(_mutex);

    const AVCodec *codec=avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec){
        qDebug() << "can't find the codec id " << stream->codecpar->codec_id << Qt::endl;
        return false;
    }

    this->_stream_index=stream->index;
    _codeccontext=avcodec_alloc_context3(codec);
    if(!_codeccontext){
#if PRINT_LOG
        qWarning() << "创建视频解码器上下文失败！";
#endif
        free();
        return false;
    }

    int ret=avcodec_parameters_to_context(_codeccontext,stream->codecpar);
    if(ret<0){
        showError(ret);
        free();
        return false;
    }
    _codeccontext->flags2 |= AV_CODEC_FLAG2_FAST;    // 允许不符合规范的加速技巧。
    _codeccontext->thread_count = 8;                 // 使用8线程解码
    _codeccontext->pkt_timebase = stream->time_base; //解决音频Could not update timestamps for skipped samples

    ret=avcodec_open2(_codeccontext,nullptr,nullptr);
    if(ret<0){
        showError(ret);
        free();
        return false;
    }

#if PRINT_LOG
    qDebug()<<"avcodec_open success!";
#endif
    return true;
}

bool MediaDecode::setCodec(AVCodecParameters *para)
{
    close();

    std::unique_lock<std::mutex> guard(_mutex);

    const AVCodec *codec=avcodec_find_decoder(para->codec_id);
    if (!codec){
        qDebug() << "can't find the codec id " << para->codec_id << Qt::endl;
        return false;
    }

    _codeccontext=avcodec_alloc_context3(codec);
    if(!_codeccontext){
#if PRINT_LOG
        qWarning() << "创建视频解码器上下文失败！";
#endif
        free();
        return false;
    }

    int ret=avcodec_parameters_to_context(_codeccontext,para);
    if(ret<0){
        showError(ret);
        free();
        return false;
    }
    _codeccontext->flags2 |= AV_CODEC_FLAG2_FAST;    // 允许不符合规范的加速技巧。
    _codeccontext->thread_count = 8;                 // 使用8线程解码

    ret=avcodec_open2(_codeccontext,nullptr,nullptr);
    if(ret<0){
        showError(ret);
        free();
        return false;
    }

#if PRINT_LOG
    qDebug()<<"avcodec_open success!";
#endif
    return true;
}

bool MediaDecode::send(AVPacket *pkt)
{
    if (!pkt || pkt->size <= 0 || !pkt->data){
#if PRINT_LOG
    qWarning() << "AVPacket is error";
#endif
        return false;
    }

    std::unique_lock<std::mutex> guard(_mutex);

    if(!_codeccontext){
#if PRINT_LOG
        qWarning() << "No the Video_AVCodecContext";
#endif
        return false;
    }
    if (_codeccontext->codec_id == AV_CODEC_ID_H264){



    }
    int ret=avcodec_send_packet(_codeccontext,pkt);
    av_packet_free(&pkt);
    if (ret != 0){
#if PRINT_LOG
        showError(ret);
#endif
        return false;
    }



    return true;
}

AVFrame *MediaDecode::receive()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if(_avframe){
        av_frame_free(&this->_avframe);
    }
    this->_avframe=av_frame_alloc();


    if(!_codeccontext){
#if PRINT_LOG
        qWarning() << "No the Video_AVCodecContext";
#endif
        free();
        return nullptr;
    }

    int ret=avcodec_receive_frame(_codeccontext,this->_avframe);
//        while(ret==0){
//            ret=avcodec_receive_frame(_vcodeccontext,this->_avframe);
//        }
    if (ret != 0){
        av_frame_free(&this->_avframe);
        return nullptr;
    }



    _pts=this->_avframe->pts;

#if PRINT_LOG
    qDebug() << "this frame's pts:"<<_pts;
#endif

    return this->_avframe;
}

void MediaDecode::showError(int err)
{
#if PRINT_LOG
    memset(_error, 0, ERROR_LEN);        // 将数组置零
    av_strerror(err, _error, ERROR_LEN);
    qWarning() << "DecodeVideo Error：" << _error;
#else
    Q_UNUSED(err)
#endif
}

double MediaDecode::r2d(AVRational *r)
{
    return r->den == 0 ? 0 : (double)r->num / (double)r->den;
}

void MediaDecode::close()
{
    free();

//    _total_time     = 0;
//    _videostream_index    = 0;
//    _audiostream_index    = 0;
//    _total_frames   = 0;
//    _audio_sample  = 0;
//    _video_pts           = 0;
//    _audio_pts     = 0;
//    _video_framerate  =0;
//    _size          = QSize(0, 0);
}

void MediaDecode::free()
{
//    // 释放上下文swsContext。
//    if(_swscontext)
//    {
//        sws_freeContext(_swscontext);
//        _swscontext = nullptr;             // sws_freeContext不会把上下文置NULL
//    }
    // 释放编解码器上下文和与之相关的所有内容，并将NULL写入提供的指针
    if(_codeccontext)
    {
        avcodec_flush_buffers(_codeccontext); // 清理解码器申请内存
        avcodec_close(_codeccontext);
        avcodec_free_context(&_codeccontext);
        _codeccontext=nullptr;
    }
//    if(_acodeccontext)
//    {
//        avcodec_flush_buffers(_acodeccontext); // 清理解码器申请内存
//        avcodec_close(_acodeccontext);
//        avcodec_free_context(&_acodeccontext);
//        _acodeccontext=nullptr;
//    }

    if(_avpacket)
    {
        av_packet_free(&_avpacket);
        _avpacket=nullptr;
    }
    if(_avframe)
    {
        av_frame_free(&_avframe);
        _avframe=nullptr;
    }
}
//读取完成后向解码器中传如空AVPacket，否则无法读取出最后几帧
void MediaDecode::receiveNullPacket(AVPacket *pkt)
{

    avcodec_send_packet(_codeccontext, pkt);
}

AVCodecContext *MediaDecode::codeccontext() const
{
    return _codeccontext;
}

qint64 MediaDecode::pts() const
{
    return _pts;
}

