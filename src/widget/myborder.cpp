#include "mybordercontainer.h""

MyBorderContainer::MyBorder::MyBorder(QWidget *parent, BorderType type, MyBorderContainer * contex)
{
    this->setParent(parent);
    KeepDrag = false;
    MyType = type;
    container = contex;
    if(MyType == L_BORDER || MyType == R_BORDER)
    {
        setCursor(Qt::SizeHorCursor);
    }
    else if(MyType == T_BORDER || MyType == B_BORDER)
    {
        setCursor(Qt::SizeVerCursor);
    }
    else if(MyType == LT_BORDER || MyType == RB_BORDER)
    {
        setCursor(Qt::SizeFDiagCursor);
    }
    else if(MyType == LB_BORDER || MyType == RT_BORDER)
    {
        setCursor(Qt::SizeBDiagCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}

//鼠标点击事件
void MyBorderContainer::MyBorder::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        KeepDrag = true;
        mousePointOld = ev->globalPos();
    }
    return QWidget::mousePressEvent(ev);
}

//鼠标移动事件
void MyBorderContainer::MyBorder::mouseMoveEvent(QMouseEvent *ev)
{
    if(KeepDrag)
    {
        const QPoint position = ev->globalPos() - mousePointOld;

        //调用外部类接口
        switch (MyType) {
        case L_BORDER: //左
            container->getLeftScaleEvent(position);
            break;
        case R_BORDER: //右
            container->getRightScaleEvent(position);
            break;
        case T_BORDER: //上
            container->getTopScaleEvent(position);
            break;
        case B_BORDER: //上
            container->getBottomScaleEvent(position);
            break;
        case RB_BORDER: //右下
            container->getRBScaleEvent(position);
            break;
        case RT_BORDER: //右上
            container->getRTScaleEvent(position);
            break;
        case LB_BORDER: //左下
            container->getLBScaleEvent(position);
            break;
        case LT_BORDER: //左上
            container->getLTScaleEvent(position);
            break;
        default:
            break;
        }
        mousePointOld = ev->globalPos();
    }
    return QWidget::mouseMoveEvent(ev);
}

//鼠标释放事件
void MyBorderContainer::MyBorder::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
    KeepDrag = false;
    return QWidget::mouseReleaseEvent(ev);
}
