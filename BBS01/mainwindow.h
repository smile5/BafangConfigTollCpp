#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


#define GENERAL  0x51           // used in device read/write
#define BASIC  0x52             // used in device read/write
#define PEDAL_ASSIST  0x53      // used in device read/write
#define THROTTLE  0x54          // used in device read/write



class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbScanPort_clicked();
    void on_rbconnect_toggled(bool checked);
    void serialreceived();
    void on_pbRead_clicked();
    void on_Onglet_currentChanged(int index);

    void on_pbWrite_clicked();

    void on_pbReadFlash_clicked();

    void on_pbWriteFlash_clicked();

    void on_actionExit_triggered();

    void on_actionLoad_triggered();

    void on_actionSave_As_triggered();

    void on_actionSave_triggered();

    void on_actionAbout_triggered();

private:
    typedef enum
    {
        rdIgnore,     // marker - ignore further received data
        rdSingle,     // marker - single block read
        wrSingle,     // marker - single block write
        rdAll,        // marker - full flash read
        wrAll,        // marker - full flash write
        rdGEN         // marker - General data block read
    } command;

    QSerialPort *serial;
    Ui::MainWindow *ui;

    QSerialPortInfo serialPortInfo;
    command commande;
    QByteArray tabok;
    QString fichier_nom;
    void connect_Dev();
    void serial_connect();
    void serial_scan();
    void Read_Dev(unsigned char cmd);
    void Write_Dev(unsigned char cmd);
    void DecodeGen(QByteArray rec_tab);
    void DecodeBas_w(QByteArray rec_tab);
    void DecodeBas(QByteArray rec_tab);
    void DecodePedAs_w(QByteArray rec_tab);
    void DecodeThr_w(QByteArray rec_tab);
    void DecodePedAs(QByteArray rec_tab);
    void DecodeThr(QByteArray rec_tab);
};
#endif // MAINWINDOW_H
