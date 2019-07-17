#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QSettings>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

namespace Ui {
class MainWindow;
}

struct coords
{
  float x;
  float y;
  float z;
  float w;
};

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

    int connectToDevice(QString portName);
    void disconnectDevice();
    void readSettings();

    QSerialPort port;
    QLabel state;
    QLabel bytesReceived;
    QSettings * settings;
    QByteArray portData;

    int packetCounter;

    bool syncIsAlive;
};

#endif // MAINWINDOW_H
