#include "mainwindow.h"
#include "src/mythread/myplaythread.h"
#include "src/widget/qtaudioplay.h"
#include "src/widget/sdlaudioplay.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QMovie>

#define USE_SDL 1

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _videofmt("视频文件(*.mp4 *.mov *.avi *.mkv *.wmv *.flv *.webm *.mpeg *.mpg *.3gp *.m4v *.rmvb *.vob *.ts *.mts *.m2ts *.f4v *.divx *.xvid);;")
    , _audiofmt("音频文件(*.mp3 *.wma *.wav *.aac *.flac);;")
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint);
    _border = new MyBorderContainer(this);


    _video_list=new QStringListModel(_list);
    ui->listView->setModel(_video_list);
    ui->listView->setWordWrap(true);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setMovement(QListView::Free);

    QPixmap volume_pix(":/image/image/volume.png");
    volume_pix.scaled(ui->volume_icon->size(),Qt::KeepAspectRatio);
    ui->volume_icon->setFixedSize(36,34);
    ui->volume_icon->setPixmap(volume_pix);
    ui->volume_icon->setScaledContents(true);

    ui->volume_slider->setRange(0,100);

#ifdef USE_SDL
    _useSDL=true;
#else
    _useSDL=false;
#endif

    if(!_useSDL){
        _audioplay=new QtAudioplay();
    }
    else{
        _audioplay=new SDLAudioPlay();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    clearMember();
}

QString MainWindow::getShowTime(qint64 time)
{
    // 传秒数ss=1，传毫秒数ss=1000
    qint64 ss = 1;
    qint64 mi = ss * 60;
    qint64 hh = mi * 60;
    qint64 dd = hh * 24;

    qint64 day = time / dd;
    qint64 hour = (time - day * dd) / hh;
    qint64 minute = (time - day * dd - hour * hh) / mi;
    qint64 second = (time - day * dd - hour * hh - minute * mi) / ss;

    QString hou = QString::number(hour, 10);
    QString min = QString::number(minute, 10);
    QString sec = QString::number(second, 10);

    hou = hou.length() == 1 ? QString("0%1").arg(hou) : hou;
    min = min.length() == 1 ? QString("0%1").arg(min) : min;
    sec = sec.length() == 1 ? QString("0%1").arg(sec) : sec;

    if(hou=="00"){
        return min+":"+sec;
    }
    return hou + ":" + min + ":" + sec;
}


void MainWindow::setProgressTime(qint64 time)
{
    if(time<0){
        return;
    }
//    qDebug()<<time<<getShowTime(time);
//    if(time<currenttime){
//        return;
//    }
    _currenttime=time;

    if(t->_isSeek){
        set_loadingimage(true);

        t->_isSeek=false;
    }

    if(!_is_start && !t->_isPause){

        ui->start->setEnabled(true);
        ui->restart->setEnabled(true);
        ui->multiple->setEnabled(true);
        ui->volume_slider->setEnabled(true);
        ui->video_slider->setEnabled(true);
        ui->start->setIcon(QIcon(":/image/image/stop.png"));
        _is_start=true;
        _is_finish=false;
        set_loadingimage(true);
    }
    ui->video_progress->setText(QString("%1/%2").arg(getShowTime(time)).arg(getShowTime(_totaltime)));
    ui->video_slider->setValue(time);
}


void MainWindow::on_window_closed_clicked()
{
    clearMember();
    this->close();
}


void MainWindow::on_window_expanded_clicked()
{
    QScreen *screen=QGuiApplication::primaryScreen();
    QRect deskrect=screen->availableGeometry();
    QIcon icon=ui->window_expanded->icon();
    if(this->width()<deskrect.width()){
        QIcon icon1(":/image/image/small_size.png");
        this->setGeometry(deskrect);
        ui->window_expanded->setIcon(icon1);
    }
    else if(this->width()>=deskrect.width()){
        this->setGeometry(deskrect.width()/2-958/2,deskrect.height()/2-625/2,958,625);
        QIcon icon1(":/image/image/window_enlarged.png");
        ui->window_expanded->setIcon(icon1);
    }

}

void MainWindow::on_window_smalled_clicked()
{
    this->showMinimized();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        //获得鼠标的初始位置
        _mouseStartPoint = event->globalPos();
        //        mouseStartPoint = event->pos();//会抖动
        //获得窗口的初始位置
        _windowTopLeftPoint = this->frameGeometry().topLeft();
        QPoint begin(ui->titlebar->x()+this->x(),ui->titlebar->y()+this->y());
        for(int i=0;i<ui->titlebar->width();i++){
            for(int j=0;j<ui->titlebar->height();j++){
                QPoint p=begin+QPoint(i,j);
                if(_mouseStartPoint==p){
                    _m_bDrag=true;
                    return;
                }
            }
        }
    }

}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        _m_bDrag = false;
    }

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(_m_bDrag)
    {
        //获得鼠标移动的距离
        QPoint distance = event->globalPos() - _mouseStartPoint;
        //        QPoint distance = event->pos() - mouseStartPoint;
        //改变窗口的位置
        this->move(_windowTopLeftPoint + distance);
    }

}


void MainWindow::on_open_triggered()
{
//    QString url = "C:\\Users\\moro\\Videos\\input.mp4";
//    QString url2 = "C:\\Users\\moro\\Music\\花に亡霊（电影《想哭的我戴上了猫的面具》主题曲）-ヨルシカ.128.mp3";
//    QString url1="C:\\Users\\moro\\Videos\\20230823_145252.mp4";



    //获取文件路径
    QString dir=QCoreApplication::applicationDirPath();
    if(t && t->isRunning()){
        t->pause();
        ui->start->setIcon(QIcon(":/image/image/start.png"));
        _is_start=false;
    }

    QString url=QFileDialog::getOpenFileName(this,"打开一个文件",dir,_videofmt+_audiofmt);
    qDebug()<<url;
    if(url.isEmpty()){
        if(t){
            t->resume();
        }
        return;
    }
    else{
        set_loadingimage(true);

        //清空video
        if(t && t->isRunning()){
            _is_open=false;
            _is_start=false;
//            delete ui->video;
//            ui->video = new MyOpenGLWidget(ui->player);
//            ui->video->setObjectName(QString::fromUtf8("video"));
//            ui->video->setEnabled(true);
            t->disconnect();
            t->stop();
            t->deleteLater();
            t->wait();  //必须等待线程结束;
            delete t;
            t=nullptr;
        }
        ui->multiple->setCurrentText("倍数:1");
        ui->volume_slider->setValue(30);
        QPixmap volume_pix(":/image/image/volume.png");
        ui->volume_icon->setPixmap(volume_pix);

        ui->start->setEnabled(false);
        ui->restart->setEnabled(false);
        ui->multiple->setEnabled(false);
        ui->volume_slider->setEnabled(false);
        ui->video_slider->setEnabled(false);

    }
    QFile file(url);
    QFileInfo fileinfo(url);



    //文件加入视频列表
    if(!_list.contains(fileinfo.fileName())){
        _list.append(fileinfo.fileName());
    }
    _fileinfo_list[fileinfo.fileName()]=url;//设置列表元素名
    _video_list->setStringList(_list);//更新view
    //选中当前行
    QModelIndex index=_video_list->index(_list.indexOf(fileinfo.fileName()));
    ui->listView->setCurrentIndex(index);

    //设置标题
    ui->title->setText(fileinfo.fileName());


    t=new MyPlayThread();

    connect(t,&MyPlayThread::setTotalTime,this,[&](qint64 totaltime){
        ui->video_slider->setTracking(true);//启用滑块追踪
        ui->video_slider->setRange(0,totaltime);
        qDebug()<<"总时长："<<totaltime<<Qt::endl;
        this->_totaltime=totaltime;
        //        totaltime=getShowTime(totalMs);
        ui->video_progress->setText(QString("%1/%2").arg("00:00").arg(getShowTime(totaltime)));
        ui->video_slider->setValue(0);
    });

    connect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));

    connect(t,&MyPlayThread::finished,this,[&](){
        ui->start->setIcon(QIcon(":/image/image/start.png"));
        ui->video_slider->setValue(_totaltime);

        qDebug()<<"finished";
        _is_start=false;
        _is_finish=true;
//        _is_open=false;
//        if(t){
//            t->deleteLater();
//            t->wait();  //必须等待线程结束;
//            delete t;
//            t=nullptr;
//        }
    });

    set_loadingimage(false);
    t->open(url,this->ui->video,false,_audioplay,_useSDL);
    _is_open=true;

    bool is_start=t->start();
    if(!t->isRunning() || !is_start){
        QMessageBox::warning(this,"error","视频启动失败");
        return;
    }

//    ui->start->setEnabled(true);
//    ui->restart->setEnabled(true);
//    ui->multiple->setEnabled(true);
//    ui->volume_slider->setEnabled(true);
//    ui->video_slider->setEnabled(true);
//    ui->start->setIcon(QIcon(":/image/image/stop.png"));
//    _is_start=true;
//    _is_finish=false;
}



void MainWindow::on_start_clicked()
{
    if(_is_finish){
        on_restart_clicked();
    }
    else{
        if(!_is_start){
            //        if(getShowTime(ui->video_slider->value())==totaltime){
            //            ui->video_slider->setValue(0);
            //        }
            ui->start->setIcon(QIcon(":/image/image/stop.png"));
            _is_start=true;

            connect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
            t->resume();
            return;
        }
        else{
            ui->start->setIcon(QIcon(":/image/image/start.png"));
            _is_start=false;
            qDebug()<<"set  Icon";

            disconnect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
            t->pause();

            return;
        }
    }
}



void MainWindow::on_restart_clicked()
{
    if(_is_finish){
        t->start();
        t->pause();
    }
    if(_is_start){
        t->pause();
    }   

    ui->multiple->setCurrentText("倍数:1");
    t->setSeek_time(0);
    t->seek();
    t->resume();
    if(!_is_start){
        ui->start->setIcon(QIcon(":/image/image/stop.png"));
        _is_start=true;
    }
    _is_finish=false;
}

void MainWindow::on_before_clicked()
{
    QModelIndex curindex=ui->listView->currentIndex();
    if(curindex.row()<=0){
        QMessageBox::information(this,"提示","无上一内容");
        return;
    }
    QModelIndex lastindex=_video_list->index(curindex.row()-1);
    on_listView_doubleClicked(lastindex);
    ui->listView->setCurrentIndex(lastindex);
}


void MainWindow::on_next_clicked()
{
    QModelIndex curindex=ui->listView->currentIndex();
    if(curindex.row()>=_list.size()-1){
        QMessageBox::information(this,"提示","无下一内容");
        return;
    }
    QModelIndex nextindex=_video_list->index(curindex.row()+1);
    on_listView_doubleClicked(nextindex);
    ui->listView->setCurrentIndex(nextindex);
}


void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    //清空video
    if(t && t->isRunning()){
        _is_open=false;
        _is_start=false;
        //            delete ui->video;
        //            ui->video = new MyOpenGLWidget(ui->player);
        //            ui->video->setObjectName(QString::fromUtf8("video"));
        //            ui->video->setEnabled(true);
        t->disconnect();
        t->stop();
        t->deleteLater();
        t->wait();  //必须等待线程结束;
        delete t;
        t=nullptr;
    }
    ui->multiple->setCurrentText("倍数:1");
    ui->volume_slider->setValue(30);
    QPixmap volume_pix(":/image/image/volume.png");
    ui->volume_icon->setPixmap(volume_pix);


    ui->start->setEnabled(false);
    ui->restart->setEnabled(false);
    ui->multiple->setEnabled(false);
    ui->volume_slider->setEnabled(false);
    ui->video_slider->setEnabled(false);

    set_loadingimage(true);

    //获取当前视频项名称
    QString filename=index.data().toString();
    QString url=_fileinfo_list.value(filename);

    ui->title->setText(filename);

    t=new MyPlayThread();

    connect(t,&MyPlayThread::setTotalTime,this,[&](qint64 totaltime){
        ui->video_slider->setTracking(true);//启用滑块追踪
        ui->video_slider->setRange(0,totaltime);
        qDebug()<<"总时长："<<totaltime<<Qt::endl;
                                               this->_totaltime=totaltime;
        //        totaltime=getShowTime(totalMs);
        ui->video_progress->setText(QString("%1/%2").arg("00:00").arg(getShowTime(totaltime)));
        ui->video_slider->setValue(0);
    });

    connect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));

    connect(t,&MyPlayThread::finished,this,[&](){
        ui->start->setIcon(QIcon(":/image/image/start.png"));
        ui->video_slider->setValue(_totaltime);

        qDebug()<<"finished";
        _is_start=false;
        _is_finish=true;
        //        _is_open=false;
        //        if(t){
        //            t->deleteLater();
        //            t->wait();  //必须等待线程结束;
        //            delete t;
        //            t=nullptr;
        //        }
    });

    set_loadingimage(false);
    t->open(url,this->ui->video,false,_audioplay,_useSDL);
    _is_open=true;

    bool is_start=t->start();
    if(!t->isRunning() || !is_start){
        QMessageBox::warning(this,"error","视频启动失败");
        return;
    }

}

void MainWindow::on_video_slider_sliderPressed()
{
    if(!_is_open){
        return;
    }
    //    ui->video_slider->setCursor(QCursor(Qt::PointingHandCursor));
    disconnect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
    t->pause();

    if(!_is_finish){
        _is_start=false;
    }

}

void MainWindow::on_video_slider_sliderReleased()
{
    if(!_is_open){
        return;
    }
    if(_is_finish){
        t->start();
        t->pause();
    }

    set_loadingimage(false);

    t->seek();
    t->_isSeek=true;
    connect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
    t->resume();

    //尝试精确seek，但效率变低
//    t->resumeSelf();

//    set_loadingimage(false);

//    connect(t,&MyPlayThread::seekPreciseFrame,this,[&](){
//        set_loadingimage(true);
//        t->setSeek_time(-1);
//        qDebug()<<"receive";
//        t->resumeOther();
//    });


    if(!_is_start){
        ui->start->setIcon(QIcon(":/image/image/stop.png"));
        _is_start=true;
    }
}

void MainWindow::on_video_slider_valueChanged(int value)
{
//    qDebug()<<value<<ui->video_slider->pageStep()<<totaltime;
    if(value >= _totaltime){

    }
    ui->video_progress->setText(QString("%1/%2").arg(getShowTime(value)).arg(getShowTime(_totaltime)));
    t->setSeek_time(value);
    _currenttime=value;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(!_is_open){
        return;
    }


    if(event->key() == Qt::Key_Right){
        disconnect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
        t->pause();

        ui->video_progress->setText(QString("%1/%2").arg(getShowTime(_currenttime+5)).arg(getShowTime(_totaltime)));
        ui->video_slider->setSliderPosition(_currenttime+5);
        t->setSeek_time(_currenttime+5);
        _currenttime+=(5);
    }
    else if(event->key() == Qt::Key_Left){
        disconnect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
        t->pause();

        ui->video_progress->setText(QString("%1/%2").arg(getShowTime(_currenttime-5)).arg(getShowTime(_totaltime)));
        ui->video_slider->setSliderPosition(_currenttime-5);
        t->setSeek_time(_currenttime-5);
        _currenttime-=(5);
    }

}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(!_is_open){
        return;
    }

    if(event->key() == Qt::Key_Right){
        set_loadingimage(false);

        t->seek();
        t->_isSeek=true;
        connect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
        t->resume();
    }
    else if(event->key() == Qt::Key_Left){
        set_loadingimage(false);

        t->seek();
        t->_isSeek=true;
        connect(t,SIGNAL(setProgressTime(qint64)),this,SLOT(setProgressTime(qint64)));
        t->resume();
    }
    else if(event->key() == Qt::Key_Space){
        on_start_clicked();
    }
}


void MainWindow::on_volume_slider_sliderMoved(int position)
{
    if(!_is_open){
        return;
    }

    //获取线性音量
    if(position<50 && position%10!=0){
        position=position+10-position%10;
    }
    qreal linearVolume =position/100.0;


    if(position == 0){
        QPixmap volume_pix(":/image/image/mute.png");
        volume_pix.scaled(ui->volume_icon->size(),Qt::KeepAspectRatio);
        ui->volume_icon->setPixmap(volume_pix);
    }
    else{
        QPixmap volume_pix(":/image/image/volume.png");
        volume_pix.scaled(ui->volume_icon->size(),Qt::KeepAspectRatio);
        ui->volume_icon->setPixmap(volume_pix);
    }

//    qDebug()<<linearVolume;
    if(!_useSDL){
        _audioplay->setVolume(linearVolume);
    }
    else{
        _audioplay->setVolume(position);
    }
}



void MainWindow::set_loadingimage(bool finished)
{
    static QMovie movie(":/image/image/loading.gif");
    static QLabel loading(ui->video);


        if(finished){
            movie.setPaused(true);
            loading.close();
            ui->start->setEnabled(true);
            ui->restart->setEnabled(true);
            ui->multiple->setEnabled(true);
            ui->video_slider->setEnabled(true);
        }
        else{
            ui->start->setEnabled(false);
            ui->restart->setEnabled(false);
            ui->multiple->setEnabled(false);
            ui->video_slider->setEnabled(false);
            loading.setGeometry(ui->video->width()/2-10,ui->video->height()/2-10,20,20);
            movie.setScaledSize(loading.size());
            loading.show();
            loading.setMovie(&movie);
            movie.start();
        }
}

void MainWindow::clearMember()
{
    if(_video_list){
        delete _video_list;
        _video_list=nullptr;
    }
    if(_border){
        delete _border;
        _border=nullptr;
    }
    if(t && t->isRunning()){
        t->disconnect();
        t->stop();
        t->deleteLater();
        t->wait();  //必须等待线程结束;
        delete t;
        t=nullptr;
    }
    if(_audioplay){
        delete _audioplay;
        _audioplay=nullptr;
    }
}


void MainWindow::on_multiple_currentTextChanged(const QString &arg1)
{
    if(!_is_open){
        return;
    }

    double speed=arg1.right(arg1.length()-arg1.indexOf(":")-1).toDouble();
    if(t && t->isRunning()){
        t->setSpeed(speed);
    }
}

