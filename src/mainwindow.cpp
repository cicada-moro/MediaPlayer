#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mediademux.h"
#include "mediadecode.h"
#include <thread>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_open_triggered()
{
    //=================1、解封装测试====================
    QString url = "C:\\Users\\moro\\Videos\\input.mp4";
    MediaDemux demux; // 测试XDemux
    std::thread t([&demux,&url](){
        qDebug() << "demux.Open = " << demux.openUrl(url);
        qDebug() << "CopyVPara = " << demux.copyVPara() << Qt::endl;
        qDebug() << "CopyAPara = " << demux.copyAPara() << Qt::endl;
        //        qDebug() << "seek=" << demux.seek(0.95) << Qt::endl;
    });
    t.join();


    int size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, demux.size().width(), demux.size().height(), 4);
    /**
     * 【注意：】这里可以多分配一些，否则如果只是安装size分配，大部分视频图像数据拷贝没有问题，
     *         但是少部分视频图像在使用sws_scale()拷贝时会超出数组长度，在使用使用msvc debug模式时delete[] m_buffer会报错（HEAP CORRUPTION DETECTED: after Normal block(#32215) at 0x000001AC442830370.CRT delected that the application wrote to memory after end of heap buffer）
     *         特别是这个视频流http://vfx.mtime.cn/Video/2019/02/04/mp4/190204084208765161.mp4
     */
    uchar* m_buffer = new uchar[size + 1000];    // 这里多分配1000个字节就基本不会出现拷贝超出的情况了，反正不缺这点内存

    //=================2、解码测试====================
    MediaDecode decode; // 测试XDecode
    qDebug() << "vdecode.Open() = " << decode.setCodec(demux.copyVStream()) << Qt::endl;
    MediaDecode adecode;
    qDebug() << "adecode.Open() = " << adecode.setCodec(demux.copyAStream()) << Qt::endl;
    SwsContext *m_swsContext=nullptr;


    auto futureLambda = std::thread([&](){
        while(1)
        {
            AVPacket* pkt = demux.read();

            if (demux.isAudio(pkt))
            {
                adecode.send(pkt);
                AVFrame* frame = adecode.receive();
#if POINT_INFO
                qDebug() << "Audio:" << frame << Qt::endl;
#endif
            }
            else
            {
                decode.send(pkt);
                AVFrame* frame = decode.receive();
#if POINT_INFO
                qDebug() << "Video:" << frame << Qt::endl;
#endif

                if(frame){
                    if(!m_swsContext)
                    {
                        // 获取缓存的图像转换上下文。首先校验参数是否一致，如果校验不通过就释放资源；然后判断上下文是否存在，如果存在直接复用，如不存在进行分配、初始化操作
                        m_swsContext = sws_getCachedContext(m_swsContext,
                                                            frame->width,                     // 输入图像的宽度
                                                            frame->height,                    // 输入图像的高度
                                                            (AVPixelFormat)frame->format,     // 输入图像的像素格式
                                                            ui->video->width(),                     // 输出图像的宽度
                                                            ui->video->height(),                    // 输出图像的高度
                                                            AV_PIX_FMT_RGBA,                    // 输出图像的像素格式
                                                            SWS_BILINEAR,                       // 选择缩放算法(只有当输入输出图像大小不同时有效),一般选择SWS_FAST_BILINEAR
                                                            nullptr,                            // 输入图像的滤波器信息, 若不需要传NULL
                                                            nullptr,                            // 输出图像的滤波器信息, 若不需要传NULL
                                                            nullptr);                          // 特定缩放算法需要的参数(?)，默认为NULL
                    }

                    // AVFrame转QImage
                    uchar* data[]  = {m_buffer};
                    int    lines[AV_NUM_DATA_POINTERS];
                    av_image_fill_linesizes(lines, AV_PIX_FMT_RGBA, ui->video->width());  // 使用像素格式pix_fmt和宽度填充图像的平面线条大小。
                    int ret = sws_scale(m_swsContext,              // 缩放上下文
                                        frame->data,            // 原图像数组
                                        frame->linesize,        // 包含源图像每个平面步幅的数组
                                        0,                        // 开始位置
                                        frame->height,          // 行数
                                        data,                     // 目标图像数组
                                        lines);                   // 包含目标图像每个平面的步幅的数组
                    QImage image(data[0], ui->video->width(), ui->video->height(), QImage::Format_RGBA8888);

                    //                    w.ui->video->setPixmap(QPixmap::fromImage(image));
                    QMetaObject::invokeMethod(this,[&]{

                        ui->video->updateImage(image.copy());
                    });
                    QThread::msleep(40);
                }

            }
            if (!pkt){
                // 异步线程退出后，才清空销毁
                demux.close();
                decode.close();
                adecode.close();
                sws_freeContext(m_swsContext);
                break;
            }

        }
    });
}

