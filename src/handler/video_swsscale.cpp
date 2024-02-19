#include "video_swsscale.h"
#include <QDebug>

Video_swsscale::Video_swsscale()
{

}

Video_swsscale::~Video_swsscale()
{
    clear();
}

void Video_swsscale::setBufferSize(MediaDemux &demux)
{
    std::unique_lock<std::mutex> guard(_mutex);

    int size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, demux.size().width(), demux.size().height(), 4);
    /**
     * 【注意：】这里可以多分配一些，否则如果只是安装size分配，大部分视频图像数据拷贝没有问题，
     *         但是少部分视频图像在使用sws_scale()拷贝时会超出数组长度，在使用使用msvc debug模式时delete[] m_buffer会报错（HEAP CORRUPTION DETECTED: after Normal block(#32215) at 0x000001AC442830370.CRT delected that the application wrote to memory after end of heap buffer）
     *         特别是这个视频流http://vfx.mtime.cn/Video/2019/02/04/mp4/190204084208765161.mp4
     */
    delete this->_m_buffer;
    this->_m_buffer=nullptr;
    this->_m_buffer = new uchar[size + 1000];    // 这里多分配1000个字节就基本不会出现拷贝超出的情况了，反正不缺这点内存

}

void Video_swsscale::setBufferSize(AVCodecParameters &para)
{
    std::unique_lock<std::mutex> guard(_mutex);

    int size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, para.width, para.height, 4);
    /**
     * 【注意：】这里可以多分配一些，否则如果只是安装size分配，大部分视频图像数据拷贝没有问题，
     *         但是少部分视频图像在使用sws_scale()拷贝时会超出数组长度，在使用使用msvc debug模式时delete[] m_buffer会报错（HEAP CORRUPTION DETECTED: after Normal block(#32215) at 0x000001AC442830370.CRT delected that the application wrote to memory after end of heap buffer）
     *         特别是这个视频流http://vfx.mtime.cn/Video/2019/02/04/mp4/190204084208765161.mp4
     */
    delete this->_m_buffer;
    this->_m_buffer=nullptr;
    this->_m_buffer = new uchar[size + 1000];    // 这里多分配1000个字节就基本不会出现拷贝超出的情况了，反正不缺这点内存
}

bool Video_swsscale::setSwsContext(double des_width,
                                   double des_height,
                                   AVFrame *frame)
{
    if(des_width<0||des_height<0){
        qWarning()<<"target image resolution is not right";
        return false;
    }
    if(!frame){
        qWarning()<<"frame is have the exception";
        return false;
    }
    // 获取缓存的图像转换上下文。首先校验参数是否一致，如果校验不通过就释放资源；然后判断上下文是否存在，如果存在直接复用，如不存在进行分配、初始化操作
    _m_swsContext = sws_getCachedContext(_m_swsContext,
                                        frame->width,                     // 输入图像的宽度
                                        frame->height,                    // 输入图像的高度
                                        (AVPixelFormat)frame->format,     // 输入图像的像素格式
                                        des_width,                     // 输出图像的宽度
                                        des_height,                    // 输出图像的高度
                                        AV_PIX_FMT_RGBA,                    // 输出图像的像素格式
                                        SWS_BILINEAR,                       // 选择缩放算法(只有当输入输出图像大小不同时有效),一般选择SWS_FAST_BILINEAR
                                        nullptr,                            // 输入图像的滤波器信息, 若不需要传NULL
                                        nullptr,                            // 输出图像的滤波器信息, 若不需要传NULL
                                        nullptr);                          // 特定缩放算法需要的参数(?)，默认为NULL
    return true;
}

QImage Video_swsscale::getScaleImage(double des_width,
                                     double des_height,
                                     AVFrame *frame)
{
    std::unique_lock<std::mutex> guard(_mutex);

    if(!setSwsContext(des_width,
                      des_height,
                      frame)
        ){
        return QImage(1, 1, QImage::Format_Invalid);
    }

    // AVFrame转QImage
    uchar* data[]  = {_m_buffer};
    int    lines[AV_NUM_DATA_POINTERS];
    av_image_fill_linesizes(lines, AV_PIX_FMT_RGBA, des_width);  // 使用像素格式pix_fmt和宽度填充图像的平面线条大小。
    int ret = sws_scale(_m_swsContext,              // 缩放上下文
                        frame->data,            // 原图像数组
                        frame->linesize,        // 包含源图像每个平面步幅的数组
                        0,                        // 开始位置
                        frame->height,          // 行数
                        data,                     // 目标图像数组
                        lines);                   // 包含目标图像每个平面的步幅的数组
    if(ret==0){
        qWarning()<<"sws_scale false!";
    }
    av_frame_unref(frame);
    QImage image(data[0], des_width,des_height, QImage::Format_RGBA8888);
    return image;
}

void Video_swsscale::clear()
{
    std::unique_lock<std::mutex> guard(_mutex);

    if(_m_buffer){
        delete _m_buffer;
        _m_buffer=nullptr;
    }
    if(_m_swsContext){
        sws_freeContext(_m_swsContext);
        _m_swsContext=nullptr;
    }
}
