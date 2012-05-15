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
    QFile file("URIs");
    file.open(QIODevice::ReadOnly);
    QString uri;
    ui->tableWidget->setColumnCount(2);
    while ((uri = QString(file.readLine())).size() > 1)
    {
        uri = uri.trimmed();
        uris.append(uri);
        cout << uri.toStdString() << endl;

    }

    connect(ui->initBtn, SIGNAL(clicked()), this, SLOT(OnInit()));
    connect(ui->beginBtn, SIGNAL(clicked()), this, SLOT(OnAllBegin()));
    connect(ui->stopBtn, SIGNAL(clicked()), this, SLOT(OnAllStop()));
    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(OnSelectVideo(int,int,int,int)));

    currentRow = 0;
    curVideoThread = NULL;
    needResize = true;

    ui->beginBtn->setEnabled(false);
    ui->stopBtn->setEnabled(false);
    initializing = false;
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::OnInit()
{
    if (initializing) return;
    QStringList tempuris;
    srand((unsigned int)time(0));
    for (int i = 0; i < ui->spinBox->value(); i++)
    {
        int tmp = rand() % uris.size();
        tempuris.append(uris[tmp]);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        QTableWidgetItem* itemUri = new QTableWidgetItem(uris[tmp]);
        itemUri->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 0, itemUri);
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

    curVideoThread->setActived(false);
    disconnect(curVideoThread, SIGNAL(display()), this, SLOT(update()));

    curVideoThread = decoders[row]->getVideoThread();
    curVideoThread->setActived(true);
    connect(curVideoThread, SIGNAL(display()), this, SLOT(update()));

    currentRow = row;
    needResize = true;
}
