#include "audio_resample.h"
#include <QDebug>

Audio_resample::Audio_resample()
{

}

Audio_resample::~Audio_resample()
{
    clear();
}

bool Audio_resample::setSwrContext(int outformat, AVFrame *frame)
{
    if (!frame){
        qWarning()<<"frame is have the exception";
        return false;
    }

    // 音频重采样 上下文初始化
    _m_swrContext = swr_alloc_set_opts(_m_swrContext,
                              av_get_default_channel_layout(2),	// 输出格式
                              (AVSampleFormat)outformat,			// 输出样本格式 1 AV_SAMPLE_FMT_S16
                              frame->sample_rate,					// 输出采样率
                              av_get_default_channel_layout(frame->channels), // 输入格式
                              (AVSampleFormat)frame->format,
                              frame->sample_rate,
                              0, 0
                              );
    int ret = swr_init(_m_swrContext);
    if (ret != 0)
    {
        qDebug() << "swr_init  failed!";
        return false;
    }
    //unsigned char *pcm = NULL;
    return true;
}

int Audio_resample::getResample(int outformat, AVFrame *frame,uchar* pcm)
{
    if (!pcm)
    {
        av_frame_free(&frame);
        return 0;
    }

    std::unique_lock<std::mutex> guard(_mutex);

    if(!setSwrContext(outformat,frame)){
        return 0;
    }

    uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
    data[0] = pcm;
    int ret = swr_convert(_m_swrContext,
                           data, frame->nb_samples,		// 输出
                           (const uint8_t**)frame->data, frame->nb_samples	// 输入
                           );
    if (ret <= 0)
    {
        qWarning()<<"swr_convert false!";
        return ret;
    }
    int outSize = ret * frame->channels * av_get_bytes_per_sample((AVSampleFormat)outformat);
//    int outSize = av_samples_get_buffer_size(nullptr, frame->channels,
//                                             frame->nb_samples,
//                                             (AVSampleFormat)outformat, 1);
    return outSize;
}

void Audio_resample::clear()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if(_m_swrContext){
        swr_free(&_m_swrContext);
        _m_swrContext=nullptr;
    }
}
