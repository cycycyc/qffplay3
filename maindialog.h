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

private:
    Ui::MainDialog *ui;
    QList<DecodeThread*> decoders;
    QStringList uris;
    int currentRow;
};

#endif // MAINDIALOG_H
