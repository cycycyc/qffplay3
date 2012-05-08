#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QList>
#include "decodethread.h"

namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

public slots:
    void OnAllBegin();
    void OnAllStop();
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
};

#endif // MAINDIALOG_H
