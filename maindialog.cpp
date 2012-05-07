#include "maindialog.h"
#include "ui_maindialog.h"
#include <QFile>

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    QFile file("URIs");
    file.open(QIODevice::ReadOnly);
    QString uri;
    int count = 0;
    ui->tableWidget->setColumnCount(1);
    while ((uri = QString(file.readLine())).size() > 1)
    {
        uri = uri.trimmed();
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ++count;
        QTableWidgetItem* itemUri = new QTableWidgetItem(uri);
        itemUri->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(count-1, 0, itemUri);
        uris.append(uri);
        DecodeThread* dt = new DecodeThread(this);
        cout << uri.toStdString() << endl;
        dt->openFile(uri);
        decoders.append(dt);
    }

    connect(ui->beginBtn, SIGNAL(clicked()), this, SLOT(OnAllBegin()));
    connect(ui->stopBtn, SIGNAL(clicked()), this, SLOT(OnAllStop()));
    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(OnSelectVideo(int,int,int,int)));

    currentRow = 0;
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::OnAllBegin()
{
    DecodeThread* dt;
    foreach (dt, decoders) {
        if (dt->isOk()) dt->start();
    }
    decoders.first()->attachVideo(ui->widget);
    currentRow = 0;
    ui->tableWidget->setCurrentCell(0,0);
    update();
}

void MainDialog::OnAllStop()
{

}

void MainDialog::OnSelectVideo(int row, int, int, int)
{
    if (currentRow == row) return;
    decoders[currentRow]->detachVideo(ui->widget);
    decoders[row]->attachVideo(ui->widget);
    currentRow = row;
}
