#ifndef MEDIADEMUX_H
#define MEDIADEMUX_H

#include <iostream>
#include <mutex>
#include <QString>
#include <QSize>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class MediaDemux
{
public:
    MediaDemux();
    ~MediaDemux();

    bool openUrl(const QString &url);
    AVPacket *read();
    AVCodecParameters *copyVPara();
    AVCodecParameters *copyAPara();
    AVStream *copyVStream();
    AVStream *copyAStream();
    bool isVideo(const AVPacket *pkt);
    bool isAudio(const AVPacket *pkt);
    bool seek(double position);
    bool isEnd();
    void close();
    void clear();
    QSize size() const;

private:
    void initFFmpeg();
    void showError(int err);
    double r2d(AVRational *rational);
    void free();
signals:
    void sendNullPacket(AVPacket *pkt);

public:
    enum MEDIO_TYPE{
        ONLY_VIDEO=1,
        ONLY_AUDIO,
        DOUBLE_AV
    }static type;
    qint64 total_time() const;

    long video_framerate() const;


    static MEDIO_TYPE getType();

private:
    AVFormatContext *_formatcontext=nullptr;

    int _videostream_index=-1;
    int _audiostream_index=-1;
    qint64 _total_time;
    qint64 _total_frames;
    qint64 _video_pts;
    qint64 _audio_pts;
    long _video_framerate;
    long _audio_sample;
    QSize _size;
    char *_error=nullptr;
    bool _is_end=false;
    bool _is_first = true;

    std::mutex _mutex;
};

#endif // MEDIADEMUX_H
