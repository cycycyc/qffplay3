#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>
#include <QList>
#include <QMutex>
#include <QImage>
#include <QTimer>
#include "ffmpeg.h"

class VideoThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoThread(QList<AVPacket>& q, QMutex* m, AVCodecContext* _pCodecCtx, AVFormatContext* _pFormatCtx, int vs, QObject *parent = 0);
    ~VideoThread();

    bool getFrame(QImage&img,qint64 *effectiveframenumber=0,qint64 *effectiveframetime=0,qint64 *desiredframenumber=0,qint64 *desiredframetime=0);
    bool seekNextFrame();
    bool decodeSeekFrame(qint64 after);
    int getVideoLengthMs();
    int getCurrentMs();
    void setActived(bool flag);

    QImage LastFrame;

signals:
    void display();
    
public slots:
    void OnPlayTimeout();
    
protected:
    virtual void run();

private:
    // State infos for the wrapper

    qint64 LastFrameTime,LastLastFrameTime,LastLastFrameNumber,LastFrameNumber;
    qint64 DesiredFrameTime,DesiredFrameNumber;
    bool LastFrameOk;                // Set upon start or after a seek we don't have a frame yet

    AVFrame         *pFrame;
    AVFrame         *pFrameRGB;
    SwsContext      *img_convert_ctx;
    uint8_t         *buffer;
    int             numBytes;
    AVCodecContext* pCodecCtx;
    AVFormatContext* pFormatCtx;
    int             videoStream;
    QList<AVPacket>& queue;
    QMutex* mutex;
    QTimer          playTimer;
    int             timeoffset;
    bool            actived;

};

#endif // VIDEOTHREAD_H
