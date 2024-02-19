#ifndef MYBORDERCONTAINER_H
#define MYBORDERCONTAINER_H

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QMouseEvent>

enum BorderType
{
    NULL_BORDER = 0,			// 无
    L_BORDER,		// 左
    R_BORDER,		// 右
    T_BORDER,		// 上
    B_BORDER,		// 下
    LT_BORDER,		// 左上
    LB_BORDER,      //左下
    RT_BORDER,      //右上
    RB_BORDER,      //右下
};


class MyBorderContainer : public QObject
{
    Q_OBJECT
public:
    MyBorderContainer(QWidget *parent,uint16_t minWindowHeight = 50, uint16_t minWindowWidth = 50, uint16_t borderSize = 5);
    void setMinWindowSize(uint16_t minWindowWidth, uint16_t minWindowHeight); //设置窗口最小尺寸
    void setBorderSize(uint16_t borderSize);    //设置边框捕获区域尺寸
    void DarwBorder(); //重新绘制边框（调整位置）

protected:
    void InitBorder(); //初始化边框

private:
    void getLeftScaleEvent(QPoint movPoint);

    void getRightScaleEvent(QPoint movPoint);

    void getBottomScaleEvent(QPoint movPoint);

    void getTopScaleEvent(QPoint movPoint);

    void getRBScaleEvent(QPoint movPoint);

    void getRTScaleEvent(QPoint movPoint);

    void getLTScaleEvent(QPoint movPoint);

    void getLBScaleEvent(QPoint movPoint);
    //内部边框类，防止外部创建
    class MyBorder:public QLabel
    {
    public:
        //设置边框属性
        MyBorder(QWidget *parent, BorderType type, MyBorderContainer * outClass);
    protected:
        void mousePressEvent(QMouseEvent *ev);
        void mouseMoveEvent(QMouseEvent *ev);
        void mouseReleaseEvent(QMouseEvent *ev);

    private:
        MyBorderContainer *container; //保存外部类指针
        BorderType MyType;
        QPoint mousePointOld;
        bool KeepDrag;
    };

    uint16_t borderSize; //边框捕获区大小
    uint16_t minWindowHeight; //最小窗口高度
    uint16_t minWindowWidth;   //最小窗口宽度

    QWidget *m_widget;
    //边框控件
    MyBorder *labelLft;
    MyBorder *labelRit;
    MyBorder *labelBot;
    MyBorder *labelTop;
    MyBorder *labelRB;
    MyBorder *labelRT;
    MyBorder *labelLB;
    MyBorder *labelLT;
};

#endif // MYBORDERCONTAINER_H
