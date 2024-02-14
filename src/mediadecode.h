#ifndef MEDIADECODE_H
#define MEDIADECODE_H

#include <QString>
#include <mutex>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class MediaDecode
{
public:
    MediaDecode();
    ~MediaDecode();

    bool setCodec(AVStream *stream);
    bool setCodec(AVCodecParameters *para);
    bool send(AVPacket *packet);
    AVFrame *receive();

    void close();
private:

    void showError(int err);
    double r2d(AVRational *rational);
    void free();
public slots:
    void receiveNullPacket(AVPacket *pkt);
private:

    AVCodecContext *_codeccontext=nullptr;
//    AVCodecContext *_vcodeccontext=nullptr;
//    AVCodecContext *_acodeccontext=nullptr;
    AVPacket *_avpacket=nullptr;
    AVFrame *_avframe=nullptr;

    int _stream_index=-1;
    qint64 _pts;
    char *_error=nullptr;
    bool _is_end=false;

    std::mutex _mutex;
};

#endif // MEDIADECODE_H
