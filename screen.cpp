#include "screen.h"
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>

Screen::Screen(QWidget *parent) :
    QWidget(parent)
{
    vthread = NULL;
    needResize = true;
}

void Screen::paintEvent(QPaintEvent *)
{
    if (!vthread) return;
    QPainter p(this);
    p.drawImage(0,0,vthread->LastFrame);
    if (needResize)
    {
        resize(vthread->LastFrame.size());
        needResize = false;
    }
}

void Screen::setVideoThread(VideoThread *vt)
{
    vthread = vt;
    connect(vt, SIGNAL(display()), this, SLOT(update()));
    needResize = true;
}

void Screen::unsetVideoThread(VideoThread *vt)
{
    vthread = NULL;
    disconnect(vt, SIGNAL(display()), this, SLOT(update()));
}
