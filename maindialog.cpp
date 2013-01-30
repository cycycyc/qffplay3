#include "maindialog.h"
#include "ui_maindialog.h"
#include <ctime>
#include <QFile>
#include <QPainter>
#include <QProgressBar>
#include <QDesktopWidget>
#include <QMessageBox>

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
        ui->tableWidget->setColumnWidth(1,0);
        while ((uri = QString(file.readLine())).size() > 1)
        {
            uri = uri.trimmed();
            uris_ori.append(uri);
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
    if (uris_ori.size() == 0) return;
    QStringList tempuris;
    srand((unsigned int)time(0));
    for (int i = 0; i < ui->spinBox->value(); i++)
    {
        int tmp = rand() % uris_ori.size();
        //int tmp = i;
        tempuris.append(uris_ori[tmp]);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        QTableWidgetItem* itemUri = new QTableWidgetItem(uris_ori[tmp]);
        itemUri->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, itemUri);
    }

    uris_cur = tempuris;

    ui->tableWidget->resizeColumnsToContents();
    ui->initBtn->setText("Initializing ...");

    workThread = new WorkThread(decoders, uris_cur);
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
    delete workThread;

    if (curNum > 0)
    {
        for (int i = curNum; i < decoders.size(); i++)
        {
            if (decoders[i]->isOk())
            {
                decoders[i]->start();
                decoders[i]->genVideoThread();
                connect(decoders[i], SIGNAL(finished()), this, SLOT(OnExit()));
            }
            else
            {
                QTableWidgetItem* itemUri = ui->tableWidget->item(i, 0);
                QString itemString = "(open uri error)" + itemUri->text();
                itemUri->setText(itemString);
            }
        }
        ui->beginBtn->setEnabled(false);
    }
    curNum += uris_cur.size();
    if (more)
    {
        ui->initBtn->setEnabled(true);
        ui->initBtn->setText("More");
    }
}

void MainDialog::OnProgress(int val)
{
    QString str = "Initializing %1/%2";
    str = str.arg(val).arg(uris_cur.size());
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
    for (int i = 0; i < decoders.size(); i++)
    {
        DecodeThread* dt = decoders[i];
        if (dt->isOk())
        {
            dt->start();
            dt->genVideoThread();
            connect(dt, SIGNAL(finished()), this, SLOT(OnExit()));
        }
        else
        {
            QTableWidgetItem* itemUri = ui->tableWidget->item(i, 0);
            QString itemString = "(open uri error)" + itemUri->text();
            itemUri->setText(itemString);
        }
    }
    if (decoders.isEmpty())
    {
        QMessageBox::warning(this, "error", "cannot open uri");
        return;
    }
    curVideoThread = decoders[0]->getVideoThread();
    if (curVideoThread)
    {
        curVideoThread->setActived(true);
        connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));
    }

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
    if (curVideoThread)
    {
        curVideoThread->setActived(false);
        disconnect(curVideoThread, SIGNAL(display()), this, SLOT(update()));
    }

    curVideoThread = decoders[row]->getVideoThread();
    if (curVideoThread)
    {
        curVideoThread->setActived(true);
        connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));
    }

    currentRow = row;
    needResize = true;
}

void MainDialog::OnExit()
{
    cout << "decodethread exit" << endl;
    DecodeThread* dt = (DecodeThread *) sender();
    ReopenThread* thread = new ReopenThread(dt);
    if (curVideoThread == dt->getVideoThread())
    {
        curVideoThread->setActived(false);
        disconnect(curVideoThread, SIGNAL(display()), this, SLOT(update()));
        curVideoThread = NULL;
    }
    dt->deleteVideoThread();
    connect(thread, SIGNAL(finished()), this, SLOT(OnReopen()));
    thread->start();
}

void MainDialog::OnReopen()
{
    cout << "Reopen" << endl;
    ReopenThread* thread = (ReopenThread *) sender();
    if (thread->dt->isOk())
    {
        thread->dt->start();
        thread->dt->genVideoThread();
        if (curVideoThread == NULL)
        {
            cout << "null" << endl;
            curVideoThread = thread->dt->getVideoThread();
            curVideoThread->setActived(true);
            connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));
        }
    }

    delete thread;
}
