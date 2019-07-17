#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ///Инициализация указателя экземпляра для доступа к настройкам (парный delete в деструкторе)
    settings = new QSettings("settings.ini", QSettings::IniFormat);

    ///Добавление элементов в строку состояния
    ui->statusBar->addWidget(&state);
    ui->statusBar->addWidget(&bytesReceived);

    ///Поиск всех портов в системе
    for(auto port :QSerialPortInfo::availablePorts())
        ui->ports->addItem(port.portName());

    ///Чтение ранее сохраненных данных
    readSettings();

    ///Подключение сигнала готовности приема данных к слоту обработки пришедших данных
    connect(&port, SIGNAL(readyRead()), this, SLOT(processData()));
}

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

int MainWindow::connectToDevice()
{
    ///Проверка порта, если порт уже открыт - закрыть текущее соединение
    if(port.isOpen())
        disconnectDevice();

    ///В случае неудачи при открытии порта - выдать сообщение и вернуть код 1
    if (!port.open(QIODevice::ReadWrite))
    {
        state.setText("Ошибка: Не удалось установить подключение!");
        return OPEN_DEVICE_ERROR;
    }

    ///Сброс флага синхронизации
    syncIsAlive = false;

    ///Обнуление счетчика пакетов
    packetCounter = 0;

    //В случае успеха  - выдать сообщение и вернуть код 0
    state.setText("Подключение успешно установлено!");
    return 0;
}

void MainWindow::disconnectDevice()
{
    ///Закрытие порта
    port.close();
}

void MainWindow::readSettings()
{
    ///Чтение настроек из файла

    settings->beginGroup("GENERAL");

    ui->parity->setCurrentIndex(settings->value("parity", 0).toInt());
    ui->speed->setCurrentIndex(settings->value("speed", 0).toInt());
    ui->stopBits->setCurrentIndex(settings->value("stopBits", 0).toInt());

    settings->endGroup();
}

void MainWindow::on_connectToDevice_toggled(bool checked)
{
    ///Обработчик нажатия кнопки "Соединиться", если кнопка нажата - установить соединение с параметрами определенными в графической форме
    /// Если кнопка отжата - разорвать соединение
    if(checked)
    {        
        ///Установка четности
        /// Костыль с currentIndex + 1, возник из-за того что в enum QSerialPort::Parity отсутсвует единица
        if(ui->parity->currentIndex() == 0)
            port.setParity(static_cast<QSerialPort::Parity>(ui->parity->currentIndex()));
        else
            port.setParity(static_cast<QSerialPort::Parity>(ui->parity->currentIndex() + 1));

        ///Установка скорости
        port.setBaudRate(ui->speed->currentText().toInt());

        ///Установка стоп-битов
        port.setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBits->currentIndex()));

        ///Установка имени порта
        port.setPortName(ui->ports->currentText());

        connectToDevice();
    }
    else
        disconnectDevice();
}


void MainWindow::processData()
{
    ///Проверка флага обнаружения байта синхронизации. Если флаг сброшен, то пытаться обнаружить флаг в пакете, при этом откидывая все встретившиеся до него данные.
    if(!syncIsAlive)
    while((!(syncIsAlive = (port.read(1).at(0) == 0x40))) && (port.bytesAvailable() > 0));

    ///Читать данные до тех пор пока не заполниться пакет, или пока не закончатся данные в буфере
    while((portData.count() < (sizeof(float) * 4)) && (port.bytesAvailable() > 0))
    {
        portData.append(port.read(1));
    }

    ///Если пакет был заполнен
    if(portData.count() == (sizeof(float) * 4))
    {
        ///Привести указатель к типу coords
        coords * coordData = (coords*)portData.data();

        ///Вывести считанные значения в поле "Монитор порта"
        ui->portMonitor->append(QString("X:%1 Y:%2 Z:%3 W:%4").arg(QString::number(coordData->x,'f',3)).arg(QString::number(coordData->y,'f',3)).arg(QString::number(coordData->z,'f',3)).arg(QString::number(coordData->w,'f',3)));

        ///Сброс флага синхронизации, говорит о том что нужно вновь искать байт синхронизации
        syncIsAlive = false;

        ///Удаление обработанных данных из временного хранилища
        portData.remove(0, (sizeof(float) * 4));

        ///Увеличение счетчика пакетов
        packetCounter++;
        bytesReceived.setText("Принятых пакетов:" + QString::number(packetCounter));
    }
}

void MainWindow::on_save_clicked()
{
    ///Сохранение настроек в файл

    settings->beginGroup("GENERAL");

    settings->setValue("parity", QString::number(ui->parity->currentIndex()));
    settings->setValue("speed", QString::number(ui->speed->currentIndex()));
    settings->setValue("stopBits", QString::number(ui->stopBits->currentIndex()));

    settings->endGroup();

}
