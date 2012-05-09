#include "maindialog.h"
#include "ui_maindialog.h"
#include <QFile>
#include <QPainter>
#include <QProgressBar>
#include <QDesktopWidget>

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    QFile file("URIs");
    file.open(QIODevice::ReadOnly);
    QString uri;
    int count = 0;
    ui->tableWidget->setColumnCount(2);
    QProgressBar bar;bar.adjustSize();
    QRect rect = bar.geometry();
    rect.moveCenter(QApplication::desktop()->geometry().center());
    bar.setGeometry(rect);
    bar.show();bar.repaint();
    while ((uri = QString(file.readLine())).size() > 1)
    {
        uri = uri.trimmed();
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ++count;
        QTableWidgetItem* itemUri = new QTableWidgetItem(uri);
        itemUri->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(count-1, 0, itemUri);
        uris.append(uri);
        cout << uri.toStdString() << endl;

    }
    bar.setFormat("%v/%m");bar.setRange(0, uris.size());
    for (int i = 0; i < uris.size(); i++)
    {
        bar.setValue(i+1);bar.repaint();
        DecodeThread* dt = new DecodeThread(this);
        dt->openFile(uris[i]);
        decoders.append(dt);
    }
    bar.close();
    ui->tableWidget->resizeColumnsToContents();

    connect(ui->beginBtn, SIGNAL(clicked()), this, SLOT(OnAllBegin()));
    connect(ui->stopBtn, SIGNAL(clicked()), this, SLOT(OnAllStop()));
    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(OnSelectVideo(int,int,int,int)));

    currentRow = 0;
    curVideoThread = NULL;
    needResize = true;
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::paintEvent(QPaintEvent *evt)
{
    QDialog::paintEvent(evt);
    if (!curVideoThread || curVideoThread->LastFrame.isNull()) return;
    if (needResize)
    {
        ui->screen->setFixedSize(curVideoThread->LastFrame.size());
        needResize = false;
    }
    QPainter p(this);
    p.drawImage(ui->screen->pos()+ui->screenBox->pos(), curVideoThread->LastFrame);

    int curSec = curVideoThread->getCurrentMs()/1000;
    int totalSec = curVideoThread->getVideoLengthMs()/1000;

    QString text;
    text = text.sprintf("Time: %d:%02d/%d:%02d", curSec/60, curSec%60, totalSec/60, totalSec%60);
    ui->label->setText(text);
}

void MainDialog::OnAllBegin()
{
    DecodeThread* dt;
    foreach (dt, decoders) {
        if (dt->isOk()) dt->start();
    }

    curVideoThread = decoders[0]->getVideoThread();
    curVideoThread->setActived(true);
    connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));


    currentRow = 0;
    ui->tableWidget->setCurrentCell(0,0);
    ui->tableWidget->setFocus();
}

void MainDialog::OnAllStop()
{

}

void MainDialog::OnSelectVideo(int row, int, int, int)
{
    if (currentRow == row) return;

    curVideoThread->setActived(false);
    disconnect(curVideoThread, SIGNAL(display()), this, SLOT(update()));

    curVideoThread = decoders[row]->getVideoThread();
    curVideoThread->setActived(true);
    connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));

    currentRow = row;
    needResize = true;
}
