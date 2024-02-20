#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include "src/mythread/myplaythread.h"
#include <src/widget/mybordercontainer.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString getShowTime(qint64 totalTime);
    void set_loadingimage(bool finished);
    void clearMember();

public slots:
    void setProgressTime(qint64);

private slots:
    void on_window_closed_clicked();

    void on_window_expanded_clicked();

    void on_window_smalled_clicked();

    void on_open_triggered();

    void on_start_clicked();

    void on_video_slider_sliderPressed();

    void on_video_slider_sliderReleased();

    void on_video_slider_valueChanged(int value);

    void on_restart_clicked();

    void on_before_clicked();

    void on_next_clicked();

    void on_listView_doubleClicked(const QModelIndex &index);

    void on_volume_slider_sliderMoved(int position);

    void on_multiple_currentTextChanged(const QString &arg1);

    void on_window_full_clicked();

public:
    Ui::MainWindow *ui;


private:
    bool        _m_bDrag;
    QPoint      _mouseStartPoint;
    QPoint      _windowTopLeftPoint;
    MyBorderContainer *_border=nullptr;
    MyAudioPlay *_audioplay=nullptr;
    QString _videofmt;
    QString _audiofmt;


    MyPlayThread *t=nullptr;
    bool _is_open=false;
    bool _is_start=false;
    bool _is_finish=false;
    bool _useSDL=true;
    bool _isFull=false;

    qint64 _totaltime=-1;
    qint64 _currenttime=-1;

    QStringList _list;
    QStringListModel *_video_list=nullptr;
    QMap<QString,QString> _fileinfo_list;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;


    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;


    // QWidget interface
protected:
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H
