#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QList>
#include "decodethread.h"

namespace Ui {
class MainDialog;
}

class WorkThread : public QThread
{
    Q_OBJECT
public:
    WorkThread(QList<DecodeThread*>& d, const QStringList& u) : decoders(d), uris(u){}
signals:
    void progress(int);
protected:
    virtual void run()
    {
        for (int i = 0; i < uris.size(); i++)
        {
            DecodeThread* dt = new DecodeThread;
            dt->openFile(uris[i]);
            decoders.append(dt);
            emit progress(i+1);
        }
    }

private:
    QList<DecodeThread*>& decoders;
    QStringList uris;
};

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

public slots:
    void OnInit();
    void OnInitFinised();
    void OnAllBegin();
    void OnAllStop();
    void OnProgress(int);
    void OnSelectVideo(int,int,int,int);

protected:
    void paintEvent(QPaintEvent *evt);

private:
    Ui::MainDialog *ui;
    QList<DecodeThread*> decoders;
    QStringList uris;
    int currentRow;
    VideoThread* curVideoThread;
    bool needResize;
    WorkThread* workThread;
    bool initializing;
    int curNum;
};

#endif // MAINDIALOG_H
