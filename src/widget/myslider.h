#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QObject>
#include <QSlider>

class MySlider: public QSlider
{
    Q_OBJECT
public:
    explicit MySlider(QWidget *parent=nullptr);
    MySlider();


    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

#endif // MYSLIDER_H
