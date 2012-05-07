#ifndef SCREEN_H
#define SCREEN_H

#include <QWidget>
#include <QLabel>
#include "videothread.h"

class Screen : public QWidget
{
    Q_OBJECT
public:
    explicit Screen(QWidget *parent = 0);
    void setVideoThread(VideoThread* vt);
    void unsetVideoThread(VideoThread* vt);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    VideoThread* vthread;
    bool needResize;

};

#endif // SCREEN_H
