#include "mediademux.h"
#include <QDebug>
#include <QTime>

#define ERROR_LEN 1024  // 异常信息数组长度
#define PRINT_LOG 0

MediaDemux::MediaDemux()
{
    initFFmpeg();
    _error = new char[ERROR_LEN];

}

MediaDemux::~MediaDemux()
{
    close();
}

void MediaDemux::initFFmpeg()
{
    std::unique_lock<std::mutex> guard(_mutex);
    if(_is_first){
        //        av_register_all();         // 已经从源码中删除
        /**
         * 初始化网络库,用于打开网络流媒体，此函数仅用于解决旧GnuTLS或OpenSSL库的线程安全问题。
         * 一旦删除对旧GnuTLS和OpenSSL库的支持，此函数将被弃用，并且此函数不再有任何用途。
         */
        avformat_network_init();
        _is_first=false;
    }
}


bool MediaDemux::openUrl(const QString &url)
{
    if(url.isEmpty()){
        return false;
    }
    //参数设置
    AVDictionary *dict=nullptr;
    // 设置rtsp流使用tcp打开，如果打开失败错误信息为【Error number -135 occurred】
    //可以切换（UDP、tcp、udp_multicast、http），比如vlc推流就需要使用udp打开
    av_dict_set(&dict, "rtsp_transport", "tcp", 0);
    // 设置最大复用或解复用延迟（以微秒为单位）。当通过【UDP】 接收数据时，
    //解复用器尝试重新排序接收到的数据包（因为它们可能无序到达，或者数据包可能完全丢失）。这可以通过将最大解复用延迟设置为零（通过max_delayAVFormatContext 字段）来禁用。
    av_dict_set(&dict, "max_delay", "3", 0);
    // 以微秒为单位设置套接字 TCP I/O 超时，如果等待时间过短，也可能会还没连接就返回了。
    av_dict_set(&dict, "timeout", "1000000", 0);

    const QByteArray path=url.toUtf8().data();
    std::unique_lock<std::mutex> guard(_mutex);
    int ret=avformat_open_input(&_formatcontext,path,nullptr,&dict);
    
    if(dict){
        av_dict_free(&dict);
    }
    if(ret!=0){
        showError(ret);
        free();
        return false;
    }
#if PRINT_LOG
    qDebug()<<"open url success!";
#endif
    
    ret=avformat_find_stream_info(_formatcontext,0);
    if(ret<0){
        showError(ret);
        free();
        return false;
    }
    _total_time=_formatcontext->duration/(AV_TIME_BASE/1000);
#if PRINT_LOG
    qDebug() << QString("视频总时长：%1 ms，[%2]").arg(_total_time).arg(QTime::fromMSecsSinceStartOfDay(int(_total_time)).toString("HH:mm:ss zzz"));
#endif
    av_dump_format(_formatcontext,0,url.toStdString().c_str(),0);






    //此处要更改流的个数判断
    _videostream_index=av_find_best_stream(_formatcontext,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);

    _audiostream_index=av_find_best_stream(_formatcontext,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);

    if(_videostream_index >=0 &&
        _audiostream_index >=0){
        type=DOUBLE_AV;
    }
    else if(_videostream_index <0 &&
            _audiostream_index >=0){
        type=ONLY_AUDIO;
    }
    else if(_videostream_index >=0 &&
               _audiostream_index <0){
        type=ONLY_VIDEO;
    }

    //既无音频流又无视频流
    if(_audiostream_index < 0&&
        _videostream_index < 0){
        showError(_videostream_index+_audiostream_index);
        free();
        return false;
    }

    //video
    if(type == DOUBLE_AV ||type == ONLY_VIDEO){
        AVStream *videostream=_formatcontext->streams[_videostream_index];
        // 获取视频图像分辨率（AVStream中的AVCodecContext在新版本中弃用，改为使用AVCodecParameters）
        _size.setWidth(videostream->codecpar->width);
        _size.setHeight(videostream->codecpar->height);
        _video_framerate = r2d(&videostream->avg_frame_rate);  // 视频帧率
    _total_frames=videostream->nb_frames;
#if PRINT_LOG
        qDebug() << "=======================================================" << Qt::endl;
        qDebug() << "VideoInfo: " << _videostream_index << Qt::endl;
        qDebug() << "codec_id = " << videostream->codecpar->codec_id << Qt::endl;
        qDebug() << "format = " << videostream->codecpar->format << Qt::endl;
        qDebug() << "width=" << videostream->codecpar->width << Qt::endl;
        qDebug() << "height=" << videostream->codecpar->height << Qt::endl;
        // 帧率 fps 分数转换
        qDebug() << "video fps = " << r2d(&videostream->avg_frame_rate) << Qt::endl;
#endif
    }


    //audio
    if(type == DOUBLE_AV || type == ONLY_AUDIO){
        AVStream *audiostream=_formatcontext->streams[_audiostream_index];
        _audio_sample=audiostream->codecpar->sample_rate;
#if PRINT_LOG
    qDebug() << "=======================================================" << Qt::endl;
    qDebug() << "AudioInfo: " << _audiostream_index  << Qt::endl;
    qDebug() << "codec_id = " << audiostream->codecpar->codec_id << Qt::endl;
    qDebug() << "format = " << audiostream->codecpar->format << Qt::endl;
    qDebug() << "sample_rate = " << audiostream->codecpar->sample_rate << Qt::endl;
    // AVSampleFormat;
    qDebug() << "channels = " << audiostream->codecpar->channels << Qt::endl;
    // 一帧数据？？ 单通道样本数
    qDebug() << "frame_size = " << audiostream->codecpar->frame_size << Qt::endl;
#endif
    }


    return true;
}


AVPacket *MediaDemux::read()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if(!_formatcontext){
#if PRINT_LOG
        qDebug()<<"not the AVFormatContext";
#endif
        return nullptr;
    }

    AVPacket *packet=av_packet_alloc();
    int ret=av_read_frame(_formatcontext,packet);
    if(ret<0){
//        emit (sendNullPacket(packet));

        av_packet_free(&packet);
//        free();
        showError(ret);
        return nullptr;
    }

    // pts转换为毫秒
// 计算当前帧时间（毫秒）
#if 1       // 方法一：适用于所有场景，但是存在一定误差
    packet->pts = static_cast<int>(packet->pts*((r2d(&(_formatcontext->streams[packet->stream_index]->time_base)) * 1000)));
    packet->dts = static_cast<int>(packet->dts*((r2d(&(_formatcontext->streams[packet->stream_index]->time_base)) * 1000)));
#else       // 方法二：适用于播放本地视频文件，计算每一帧时间较准，但是由于网络视频流无法获取总帧数，所以无法适用
    m_obtainFrames++;
    packet->pts = qRound64(m_obtainFrames * (qreal(totaltime) / totalframes));
#endif

#if PRINT_LOG
    qDebug()<<"Now reading the packet's pts is:"<<packet->pts;
#endif

    return packet;
}

AVCodecParameters *MediaDemux::copyVPara()
{
    if(type != ONLY_VIDEO && type != DOUBLE_AV){
        return nullptr;
    }

    std::unique_lock<std::mutex> guard(_mutex);
    if (!_formatcontext)
        return nullptr;
    // 拷贝视频参数
    AVCodecParameters *pCodecPara = avcodec_parameters_alloc();
    avcodec_parameters_copy(pCodecPara, _formatcontext->streams[_videostream_index]->codecpar);
    
    return pCodecPara;
}

AVCodecParameters *MediaDemux::copyAPara()
{
    if(type != ONLY_AUDIO && type != DOUBLE_AV){
        return nullptr;
    }

    std::unique_lock<std::mutex> guard(_mutex);
    if (!_formatcontext)
        return nullptr;
    // 拷贝音频参数
    AVCodecParameters *pCodecPara = avcodec_parameters_alloc();
    avcodec_parameters_copy(pCodecPara, _formatcontext->streams[_audiostream_index]->codecpar);
    
    return pCodecPara;
}

AVStream *MediaDemux::copyVStream()
{
    if(type != ONLY_VIDEO && type != DOUBLE_AV){
        return nullptr;
    }

    std::unique_lock<std::mutex> guard(_mutex);
    if(!_formatcontext){
#if PRINT_LOG
        qDebug()<<"not the AVFormatContext";
#endif
        return nullptr;
    }
    AVStream *stream=_formatcontext->streams[_videostream_index];

    return stream;
}

AVStream *MediaDemux::copyAStream()
{
    if(type != ONLY_AUDIO && type != DOUBLE_AV){
        return nullptr;
    }

    std::unique_lock<std::mutex> guard(_mutex);
    if(!_formatcontext){
#if PRINT_LOG
        qDebug()<<"not the AVFormatContext";
#endif
        return nullptr;
    }
    AVStream *stream=_formatcontext->streams[_audiostream_index];

    return stream;
}

bool MediaDemux::isVideo(const AVPacket *pkt)
{
    if (!pkt) return false;
    if (pkt->stream_index == _audiostream_index)
        return false;
    
    return true;
}

bool MediaDemux::isAudio(const AVPacket *pkt)
{
    if (!pkt) return false;
    if (pkt->stream_index == _videostream_index)
        return false;
    
    return true;
}

bool MediaDemux::seek(double position)
{
    std::unique_lock<std::mutex> guard(_mutex);
    if(!_formatcontext){
#if PRINT_LOG
        qDebug()<<"not the AVFormatContext";
#endif
        return false;
    }
    // 清理先前未滑动时解码到的视频帧
    avformat_flush(_formatcontext);

    long long seek_position  = static_cast<long long>(_formatcontext->streams[_videostream_index]->duration * position); // 计算要移动到的位置
    int ret = av_seek_frame(_formatcontext, _videostream_index, seek_position, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    if (ret < 0){
//        free();
        return false;
    }

    return true;
}

bool MediaDemux::isEnd()
{
    return this->_is_end;
}

void MediaDemux::close()
{
    clear();
    free();
    
    _total_time     = 0;
    _videostream_index    = 0;
    _audiostream_index    = 0;
    _total_frames   = 0;
    _audio_sample  = 0;
    _video_pts           = 0;
    _audio_pts     = 0;
    _video_framerate  =0;
    _size          = QSize(0, 0);
}


void MediaDemux::showError(int err)
{
#if PRINT_LOG
    memset(_error, 0, ERROR_LEN);        // 将数组置零
    av_strerror(err, _error, ERROR_LEN);
    qWarning() << "DecodeVideo Error：" << _error;
#else
    Q_UNUSED(err)
#endif
}

double MediaDemux::r2d(AVRational *r)
{
    return r->den == 0 ? 0 : (double)r->num / (double)r->den;
}

void MediaDemux::clear()
{
    // 因为avformat_flush不会刷新AVIOContext (s->pb)。如果有必要，在调用此函数之前调用avio_flush(s->pb)。
    if(_formatcontext && _formatcontext->pb)
    {
        avio_flush(_formatcontext->pb);
    }
    if(_formatcontext)
    {
        avformat_flush(_formatcontext);   // 清理读取缓冲
    }
}

void MediaDemux::free()
{
    // 关闭失败m_formatContext，并将指针置为null
    if(_formatcontext)
    {
        avformat_close_input(&_formatcontext);
        _formatcontext=nullptr;
    }
//    if(m_buffer)
//    {
//        delete [] m_buffer;
//        m_buffer = nullptr;
    //    }
}

QSize MediaDemux::size() const
{
    return _size;
}
