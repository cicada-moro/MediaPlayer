#include "mainwindow.h"

#include <QApplication>
#include "ui_mainwindow.h"

int main1(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);

    MainWindow w;
    w.show();
    w.ui->video->updateImage(QImage("C:\\Users\\moro\\Pictures\\Camera Roll\\9f001542788d14d61c1615952cf08d42400984833.jpg"));

    return a.exec();
}








#define POINT_INFO 0


#include "mediademux.h"
#include "mediadecode.h"
#include "video_swsscale.h"
#include "ui_mainwindow.h"
#include "audio_resample.h"
#include "src/widget/myaudioplay.h"
#include <thread>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>



int main2(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    MainWindow w;
    w.show();

    //=================1、解封装测试====================
    QString url ="C:\\Users\\moro\\Videos\\input.mp4";
    QString url1 = "C:\\Users\\moro\\Music\\花に亡霊（电影《想哭的我戴上了猫的面具》主题曲）-ヨルシカ.128.mp3";
    MediaDemux demux; // 测试Demux
    std::thread t([&demux,&url](){
        qDebug() << "demux.Open = " << demux.openUrl(url);
        qDebug() << "CopyVPara = " << demux.copyVPara() << Qt::endl;
        qDebug() << "CopyAPara = " << demux.copyAPara() << Qt::endl;
//        qDebug() << "seek=" << demux.seek(0.95) << Qt::endl;
    });
    t.join();



    //=================2、解码测试====================
    MediaDecode decode; // 测试Decode
    qDebug() << "vdecode.Open() = " << decode.setCodec(demux.copyVStream()) << Qt::endl;
    MediaDecode adecode;
    qDebug() << "adecode.Open() = " << adecode.setCodec(demux.copyAStream()) << Qt::endl;
    Video_swsscale *v_sws=nullptr;
    Audio_resample *a_res=nullptr;
    uchar* pcm = new uchar[1024 * 1024];



    QFuture<void> thread =QtConcurrent::run([&](){
        MyAudioPlay audioplay;
        audioplay.open();
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
                if(frame){
                    a_res=new Audio_resample();
                    int len = a_res->getResample(AV_SAMPLE_FMT_S16,frame, pcm);
                    while(audioplay.getFree()<len){
                        QThread::msleep(10);
                    }
                    audioplay.write(pcm, len);
//                    while (len > 0)
//                    {
//                        if (audioplay.getFree() >= len)
//                        {
//                            audioplay.write(pcm, len);
//                            break;
//                        }
//                    }
                }
            }
            else
            {
                decode.send(pkt);
                AVFrame* frame = decode.receive();
#if POINT_INFO
                qDebug() << "Video:" << frame << Qt::endl;
#endif
                v_sws=new Video_swsscale();
                v_sws->setBufferSize(demux);
                if(frame){
//                    QImage image=v_sws->getScaleImage(w.ui->video->width(),w.ui->video->height(),frame);
////                    w.ui->video->setPixmap(QPixmap::fromImage(image));
//                    if(image.format()==QImage::Format_Invalid){
//                        continue;
//                    }
                    QMetaObject::invokeMethod(&w,[&]{

                        w.ui->video->updateImage(frame);
                    });
                    QThread::msleep(40);
                }

            }
            if (!pkt){
                // 异步线程退出后，才清空销毁
                demux.close();
                decode.close();
                adecode.close();
                delete v_sws;
                v_sws=nullptr;
                delete a_res;
                a_res=nullptr;
                delete []pcm;
                pcm=nullptr;
                break;
            }

        }
    });

    return a.exec();
}








#include "src/mythread/myaudiothread.h"
#include "src/mythread/myvideothread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    MainWindow w;
    w.show();

    //=================1、解封装测试====================
    QString url ="C:\\Users\\moro\\Videos\\outputname.mp4";
    QString url1 = "C:\\Users\\moro\\Music\\花に亡霊（电影《想哭的我戴上了猫的面具》主题曲）-ヨルシカ.128.mp3";
    MediaDemux demux; // 测试Demux
    std::thread t([&demux,&url](){
        qDebug() << "demux.Open = " << demux.openUrl(url);
        qDebug() << "CopyVPara = " << demux.copyVPara() << Qt::endl;
        qDebug() << "CopyAPara = " << demux.copyAPara() << Qt::endl;
        //        qDebug() << "seek=" << demux.seek(0.95) << Qt::endl;
    });
    t.join();

    MyaudioThread audioplay;
    MyVideoThread videoplay;

    QFuture<void> thread =QtConcurrent::run([&](){

            audioplay.open(demux.copyAStream());
            videoplay.openNoSws(demux.copyVPara(),w.ui->video);
            audioplay.start();
            videoplay.start();

            while(1){
                AVPacket *pkt=demux.read();
                if(demux.isAudio(pkt)){
                    audioplay.push(pkt);
                    continue;
                }
                else{
                    videoplay.push(pkt);
                }
                if (!pkt){
                    // 异步线程退出后，才清空销毁
                    demux.close();
                    audioplay.quit();
                    videoplay.quit();
                    break;
                }
            }
    });

    return a.exec();
}

