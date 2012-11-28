#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>
#include <QImage>
#include <QList>
#include <QMutex>
#include "ffmpeg.h"
#include "videothread.h"

class   DecodeThread : public QThread
{
    Q_OBJECT
public:
    explicit DecodeThread(QObject *parent = 0);
    
    ~DecodeThread();

    bool openFile() {return openFile(fileName);}
    bool openFile(QString file);
    void close();
    bool isOk();
    VideoThread* getVideoThread() const {return vthread;}
    void deleteVideoThread() {if (vthread) delete vthread; vthread = NULL;}
    void genVideoThread();
    bool seekMs(int tsms);
    bool seekFrame(qint64 frame);

protected:
    virtual void run();

private:
    // Basic FFmpeg stuff
    AVFormatContext *pFormatCtx;
    int videoStream;
    int audioStream;
    AVCodecContext  *pVideoCodecCtx;
    AVCodecContext  *pAudioCodecCtx;
    AVCodec         *pVideoCodec;
    AVCodec         *pAudioCodec;
    AVPacket        packet;

    // State infos for the wrapper
    bool ok;
    bool reachEnd;

    // Initialization functions
    bool initCodec();
    void InitVars();

    QList<AVPacket> videoQueue;
    QList<AVPacket> audioQueue;
    QMutex* videoMutex;
    QMutex* audioMutex;

    QString fileName;
    VideoThread* vthread;

};

#endif // DECODETHREAD_H
