#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("settings.ini", QSettings::IniFormat);

    ui->statusBar->addWidget(&state);
    ui->statusBar->addWidget(&bytesReceived);

    for(auto port :QSerialPortInfo::availablePorts())
        ui->ports->addItem(port.portName());

    readSettings();

    connect(&port, SIGNAL(readyRead()), this, SLOT(processData()));

    syncIsAlive = false;
    packetCounter = 0;
}

MainWindow::~MainWindow()
{
    settings->endGroup();
    delete settings;
    delete ui;
}

int MainWindow::connectToDevice(QString portName)
{
    if(port.isOpen())
        disconnectDevice();

    port.setPortName(portName);

    if (!port.open(QIODevice::ReadWrite))
    {
        state.setText("Ошибка: Не удалось установить подключение!");
        return 1;
    }

    state.setText("Подключение успешно установлено!");
    return 0;
}

void MainWindow::disconnectDevice()
{
    port.close();
}

void MainWindow::readSettings()
{
    settings->beginGroup("GENERAL");

    ui->parity->setCurrentIndex(settings->value("parity", 0).toInt());
    ui->speed->setCurrentIndex(settings->value("speed", 0).toInt());
    ui->stopBits->setCurrentIndex(settings->value("stopBits", 0).toInt());

    settings->endGroup();
}

void MainWindow::on_connectToDevice_toggled(bool checked)
{
    if(checked)
    {        
        ///Установка четности
        if(ui->parity->currentIndex() == 0)
            port.setParity(static_cast<QSerialPort::Parity>(ui->parity->currentIndex()));
        else
            port.setParity(static_cast<QSerialPort::Parity>(ui->parity->currentIndex() + 1));

        ///Установка скорости
        port.setBaudRate(ui->speed->currentText().toInt());

        ///Установка стоп-битов
        port.setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBits->currentIndex()));

        connectToDevice(ui->ports->currentText());
    }
    else
        disconnectDevice();
}


void MainWindow::processData()
{
    if(!syncIsAlive)
    while((!(syncIsAlive = (port.read(1).at(0) == 0x40))) && (port.bytesAvailable() > 0));

    while((portData.count() < (sizeof(float) * 4)) && (port.bytesAvailable() > 0))
    {
        portData.append(port.read(1));
    }

    if(portData.count() == (sizeof(float) * 4))
    {
        coords * coordData = (coords*)portData.data();

        ui->portMonitor->append(QString("X:%1 Y:%2 Z:%3 W:%4").arg(QString::number(coordData->x,'f',3)).arg(QString::number(coordData->y,'f',3)).arg(QString::number(coordData->z,'f',3)).arg(QString::number(coordData->w,'f',3)));
        syncIsAlive = false;

        portData.remove(0, (sizeof(float) * 4));

        packetCounter++;
        bytesReceived.setText("Принятых пакетов:" + QString::number(packetCounter));
    }
}

void MainWindow::on_save_clicked()
{
    settings->beginGroup("GENERAL");

    settings->setValue("parity", QString::number(ui->parity->currentIndex()));
    settings->setValue("speed", QString::number(ui->speed->currentIndex()));
    settings->setValue("stopBits", QString::number(ui->stopBits->currentIndex()));

    settings->endGroup();

}
