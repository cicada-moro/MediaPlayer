#include "mybordercontainer.h""
MyBorderContainer::MyBorderContainer(QWidget *parent,uint16_t minWindowHeight,uint16_t minWindowWidth, uint16_t borderSize)
{
    m_widget = parent;
    this->minWindowHeight = minWindowHeight; //最小窗口高度
    this->minWindowWidth = minWindowWidth;   //最小窗口宽度
    this->borderSize = borderSize;
    this->setParent(parent);
    InitBorder();
}

void MyBorderContainer::InitBorder()
{
    //上下左右的label，为了控制界面能够拖动拉伸
    labelLft = new MyBorder(m_widget, L_BORDER, this);
    labelLft->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelLft->raise();


    labelRit = new MyBorder(m_widget, R_BORDER, this);
    labelRit->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelRit->raise();

    labelBot = new MyBorder(m_widget, B_BORDER, this);
    labelBot->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelBot->raise();

    labelTop = new MyBorder(m_widget, T_BORDER, this);
    labelTop->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelTop->raise();


    labelRB = new MyBorder(m_widget, RB_BORDER, this);
    labelRB->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelRB->raise();


    labelRT = new MyBorder(m_widget, RT_BORDER, this);
    labelRT->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelRT->raise();


    labelLB = new MyBorder(m_widget, LB_BORDER, this);
    labelLB->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelLB->raise();

    labelLT = new MyBorder(m_widget, LT_BORDER, this);
    labelLT->setStyleSheet("QLabel {background-color: transparent;}");//设置背景透明
    labelLT->raise();

    DarwBorder();
}

void MyBorderContainer::setMinWindowSize(uint16_t minWindowWidth, uint16_t minWindowHeight)
{
    this->minWindowHeight = minWindowHeight; //最小窗口高度
    this->minWindowWidth = minWindowWidth;   //最小窗口宽度
}

void MyBorderContainer::setBorderSize(uint16_t borderSize)
{
    this->borderSize = borderSize;
}

//重绘边框
void MyBorderContainer::DarwBorder()
{
    labelLft->setGeometry(0,0,this->borderSize,m_widget->height());
    labelRit->setGeometry(m_widget->width()-this->borderSize,0,this->borderSize,m_widget->height());
    labelBot->setGeometry(0,m_widget->height()-this->borderSize,m_widget->width(),this->borderSize);
    labelTop->setGeometry(0,0,m_widget->width(),this->borderSize);
    labelRB->setGeometry(m_widget->width()-this->borderSize-1,m_widget->height()-this->borderSize-1,m_widget->width(),m_widget->height());
    labelRT->setGeometry(m_widget->width()-this->borderSize-1,0,m_widget->width(),this->borderSize+1);
    labelLB->setGeometry(0,m_widget->height()-this->borderSize-1,this->borderSize+1,m_widget->width());
    labelLT->setGeometry(0,0,this->borderSize+1,this->borderSize+1);
}

void MyBorderContainer::getLeftScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().x()+movPoint.x())>(m_widget->pos().x()+m_widget->width()-minWindowWidth))
    {
        return;//保证拖动窗口左边界的时候，控件宽度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x()+movPoint.x(),m_widget->pos().y(),m_widget->width()-movPoint.x(),m_widget->height());
    DarwBorder();
}

void MyBorderContainer::getRightScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().x()+m_widget->width()+movPoint.x())<(m_widget->pos().x()+minWindowWidth))
    {
        return;//保证拖动窗口右边界的时候，控件宽度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y(),m_widget->width()+movPoint.x(),m_widget->height());
    DarwBorder();
}

void MyBorderContainer::getBottomScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().y()+m_widget->height()+movPoint.y())<(m_widget->pos().y()+minWindowHeight))
    {
        return;//保证拖动窗口下边界的时候，控件高度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y(),m_widget->width(),m_widget->height()+movPoint.y());
    DarwBorder();
}

void MyBorderContainer::getTopScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().y()+movPoint.y())>(m_widget->pos().y()+m_widget->height()-minWindowHeight))
    {
        return;//保证拖动窗口上边界的时候，控件高度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y()+movPoint.y(),m_widget->width(),m_widget->height()-movPoint.y());
    DarwBorder();
}

void MyBorderContainer::getRBScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().y()+m_widget->height()+movPoint.y())<(m_widget->pos().y()+minWindowHeight)
        || (m_widget->pos().x()+m_widget->width()+movPoint.x())<(m_widget->pos().x()+minWindowWidth))
    {
        return;//保证拖动窗口上边界的时候，控件高度和宽度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y(),m_widget->width()+movPoint.x(),m_widget->height());
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y(),m_widget->width(),m_widget->height()+movPoint.y());
    DarwBorder();
}

void MyBorderContainer::getRTScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().x()+m_widget->width()+movPoint.x())<(m_widget->pos().x()+minWindowWidth)
        || (m_widget->pos().y()+movPoint.y())>(m_widget->pos().y()+m_widget->height()-minWindowHeight) )
    {
        return;//保证拖动窗口上边界的时候，控件高度和宽度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y()+movPoint.y(),m_widget->width(),m_widget->height()-movPoint.y());
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y(),m_widget->width()+movPoint.x(),m_widget->height());
    DarwBorder();
}

void MyBorderContainer::getLTScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().x()+movPoint.x())>(m_widget->pos().x()+m_widget->width()-minWindowWidth)
        || (m_widget->pos().y()+movPoint.y())>(m_widget->pos().y()+m_widget->height()-minWindowHeight) )
    {
        return;//保证拖动窗口上边界的时候，控件高度和宽度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x()+movPoint.x(),m_widget->pos().y(),m_widget->width()-movPoint.x(),m_widget->height());
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y()+movPoint.y(),m_widget->width(),m_widget->height()-movPoint.y());
    DarwBorder();
}

void MyBorderContainer::getLBScaleEvent(QPoint movPoint)
{
    if((m_widget->pos().x()+movPoint.x())>(m_widget->pos().x()+m_widget->width()-minWindowWidth)
        || (m_widget->pos().y()+m_widget->height()+movPoint.y())<(m_widget->pos().y()+minWindowHeight))
    {
        return;//保证拖动窗口上边界的时候，控件高度和宽度至少有200
    }
    m_widget->setGeometry(m_widget->pos().x()+movPoint.x(),m_widget->pos().y(),m_widget->width()-movPoint.x(),m_widget->height());
    m_widget->setGeometry(m_widget->pos().x(),m_widget->pos().y(),m_widget->width(),m_widget->height()+movPoint.y());
    DarwBorder();
}
