#include "maindialog.h"
#include "ui_maindialog.h"
#include <ctime>
#include <QFile>
#include <QPainter>
#include <QProgressBar>
#include <QDesktopWidget>

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    QFile file("URIs.txt");
    if (file.open(QIODevice::ReadOnly))
    {
        QString uri;
        ui->tableWidget->setColumnCount(2);
        while ((uri = QString(file.readLine())).size() > 1)
        {
            uri = uri.trimmed();
            uris.append(uri);
            cout << uri.toStdString() << endl;

        }
    }

    connect(ui->initBtn, SIGNAL(clicked()), this, SLOT(OnInit()));
    connect(ui->beginBtn, SIGNAL(clicked()), this, SLOT(OnAllBegin()));
    connect(ui->stopBtn, SIGNAL(clicked()), this, SLOT(OnAllStop()));
    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(OnSelectVideo(int,int,int,int)));

    currentRow = 0;
    curNum = 0;
    curVideoThread = NULL;
    needResize = true;

    ui->beginBtn->setEnabled(false);
    ui->stopBtn->setEnabled(false);

    initializing = false;
    more = false;
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::OnInit()
{
    if (initializing) return;
    if (uris.size() == 0) return;
    QStringList tempuris;
    srand((unsigned int)time(0));
    for (int i = 0; i < ui->spinBox->value(); i++)
    {
        int tmp = rand() % uris.size();
        //int tmp = i;
        tempuris.append(uris[tmp]);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        QTableWidgetItem* itemUri = new QTableWidgetItem(uris[tmp]);
        itemUri->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, itemUri);
    }

    uris = tempuris;

    ui->tableWidget->resizeColumnsToContents();
    ui->initBtn->setText("Initializing ...");

    workThread = new WorkThread(decoders, uris);
    connect(workThread, SIGNAL(finished()), this, SLOT(OnInitFinised()));
    connect(workThread, SIGNAL(progress(int)), this, SLOT(OnProgress(int)));
    workThread->start();
    initializing = true;
}

void MainDialog::OnInitFinised()
{
    ui->beginBtn->setEnabled(true);
    ui->initBtn->setEnabled(false);
    ui->initBtn->setText("Initialized");
    initializing = false;

    if (curNum > 0)
    {
        for (int i = curNum; i < decoders.size(); i++)
        {
            if (decoders[i]->isOk())
            {
                decoders[i]->start();
                decoders[i]->genVideoThread();
            }
        }
        ui->beginBtn->setEnabled(false);
    }
    curNum += uris.size();
    if (more)
    {
        ui->initBtn->setEnabled(true);
        ui->initBtn->setText("More");
    }
}

void MainDialog::OnProgress(int val)
{
    QString str = "Initializing %1/%2";
    str = str.arg(val).arg(uris.size());
    ui->initBtn->setText(str);
}

void MainDialog::paintEvent(QPaintEvent *evt)
{
    QDialog::paintEvent(evt);
    if (!curVideoThread || curVideoThread->LastFrame.isNull()) return;
    QRect r, c;
    r.setTopLeft(ui->screen->pos()+ui->screenBox->pos());
    c = r;
    QSize s = curVideoThread->LastFrame.size();
    s.scale(ui->screen->size(), Qt::KeepAspectRatio);
    c.setSize(ui->screen->size());
    r.setSize(s);
    /*if (needResize)
    {
        s = curVideoThread->LastFrame.size();
        if (s.width() > 1000 || s.height() > 800)
            s.scale(1000, 800, Qt::KeepAspectRatio);
        r.setSize(s);
        ui->screen->setGeometry(r);
        needResize = false;
    }*/
    r.moveCenter(c.center());

    QPainter p(this);
    p.drawImage(r, curVideoThread->LastFrame);

    int curSec = curVideoThread->getCurrentMs()/1000;
    int totalSec = curVideoThread->getVideoLengthMs()/1000;

    //QString text;
    //text = text.sprintf("Time: %d:%02d/%d:%02d", curSec/60, curSec%60, totalSec/60, totalSec%60);
    //ui->label->setText(text);
}

void MainDialog::OnAllBegin()
{
    DecodeThread* dt;
    int count = 0;
    foreach (dt, decoders) {
        cout << count++ << endl;
        if (dt->isOk())
        {
            dt->start();
            dt->genVideoThread();
        }
    }

    curVideoThread = decoders[0]->getVideoThread();
    curVideoThread->setActived(true);
    connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));


    currentRow = 0;
    ui->tableWidget->setCurrentCell(0,0);
    ui->tableWidget->setFocus();
    ui->beginBtn->setEnabled(false);
    ui->stopBtn->setEnabled(true);
    ui->initBtn->setText("More");
    ui->initBtn->setEnabled(true);
    needResize = true;
    more = true;
}

void MainDialog::OnAllStop()
{
    DecodeThread* dt;
    foreach (dt, decoders) {
        dt->terminate();
    }
    foreach (dt, decoders) {
        dt->wait();
    }
    close();
}

void MainDialog::OnSelectVideo(int row, int, int, int)
{
    if (initializing) return;
    if (currentRow == row) return;
    if (!decoders[row]->isOk()) return;
    curVideoThread->setActived(false);
    disconnect(curVideoThread, SIGNAL(display()), this, SLOT(update()));

    curVideoThread = decoders[row]->getVideoThread();
    curVideoThread->setActived(true);
    connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));

    currentRow = row;
    needResize = true;
}
