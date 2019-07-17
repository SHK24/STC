#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define OPEN_DEVICE_ERROR 1

#include <QLabel>
#include <QSettings>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>


///Структура для расшифровки пакета
#pragma pack(push, 1)
struct coords
{
  float x;
  float y;
  float z;
  float w;
};
#pragma pack(pop)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectToDevice_toggled(bool checked);

    void processData();

    void on_save_clicked();

private:
    Ui::MainWindow *ui;

    ///Счетчик принятых пакетов
    int packetCounter;

    ///Признак обнаружения байта синхронизации
    bool syncIsAlive;

    ///Экземпляр порта для обмена с прибором
    QSerialPort port;

    ///Элемент отображения состояния порта
    QLabel state;

    ///Элемент отображения количества принятых пакетов
    QLabel bytesReceived;

    ///Экземпляр для взаимодействия с файлом настроек
    QSettings * settings;

    ///Массив для хранения пакетов приходящих с прибора. Очищается при вычитывании всего пакета.
    QByteArray portData;

    ///Метод для подключения к прибору
    int connectToDevice();

    ///Метод для отключения от прибора
    void disconnectDevice();

    ///Метод чтения сохраненных настроек
    void readSettings();
};

#endif // MAINWINDOW_H
