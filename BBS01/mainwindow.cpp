#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,serial(new QSerialPort)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_19->hide();
    ui->comboBox_3->hide();
    qApp->setApplicationName("Bafang");
    qApp->setOrganizationName("ConfigTool_BBS01");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    serial_scan();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::serial_scan()
{
    QString port_name;
    unsigned char i;
    for(i=0;i<ui->cbPort->count();i++)
        ui->cbPort->removeItem(i);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        /*  qDebug() << "Name : " << info.portName();
            qDebug() << "Description : " << info.description();
            qDebug() << "Manufacturer: " << info.manufacturer();
            qDebug() << "VID " << info.vendorIdentifier();
            qDebug() << "PID" << info.productIdentifier();
            qDebug() << index << "\r\n"; */
        if(info.portName()=="ttyS0")
        {

        }
        else
            ui->cbPort->addItem(info.portName()+":"+info.manufacturer());
    }
}

void MainWindow::serial_connect()
{
    QString portname;
    if(ui->cbPort->currentText()=="")
    {
        serial->disconnect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()));


        if(serial->isOpen())
        {
            serial->putChar('S');
            while(!serial->flush());
            serial->clear(QSerialPort::AllDirections);
            serial->close();
        }
    }
    else
    {
        portname="/dev/"+ ui->cbPort->currentText().left(ui->cbPort->currentText().lastIndexOf(':'));
        serial->setPortName(portname);
        serial->setBaudRate(1200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->setReadBufferSize(0);
        serial->open(QIODevice::ReadWrite);
        serial->connect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()),Qt::UniqueConnection);
        commande=rdGEN;
        connect_Dev();
    }

}
void MainWindow::on_pbScanPort_clicked()
{
    serial_scan();
}


void MainWindow::on_rbconnect_toggled(bool checked)
{
    if(checked)
    {
        if (ui->cbPort->count()>0)
        {
            serial_connect();
            ui->rbconnect->setStyleSheet("background-color: red");
        }
        else
        {
            serial->disconnect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()));
            if(serial->isOpen())
            {
                // serial->putChar('S');
                while(!serial->flush());
                serial->clear(QSerialPort::AllDirections);
                serial->close();
            }
            ui->rbconnect->setChecked(false);
        }

    }
    else
    {
        ui->rbconnect->setStyleSheet("background-color: ");
        serial->disconnect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()));
        if(serial->isOpen())
        {
            while(!serial->flush());
            serial->clear(QSerialPort::AllDirections);
            serial->close();
        }
    }
}
void MainWindow::serialreceived()
{
    QByteArray rec_tab;
    rec_tab=serial->readAll();
    tabok+=rec_tab;
    if(tabok.size()>1)
    {
        ui->temesInfo->append(tabok.toHex(' '));
        switch (tabok.at(0))
        {
        case GENERAL:
            if(commande==rdGEN)
            {
                DecodeGen(tabok);
            }
            break;
        case BASIC:
            if ((commande == rdSingle) || (commande == rdAll))
            {
                DecodeBas(tabok);
            }
            else if ((commande == wrSingle) or (commande == wrAll))
            {
                DecodeBas_w(tabok);
            }
            break;
        case PEDAL_ASSIST:
            if ((commande == rdSingle) || (commande == rdAll))
            {
                DecodePedAs(tabok);
            }
            else if ((commande == wrSingle) or (commande == wrAll))
            {
                DecodePedAs_w(tabok);
            }
            break;
        case THROTTLE:
            if ((commande == rdSingle) || (commande == rdAll))
            {
                DecodeThr(tabok);
            }
            else if ((commande == wrSingle) or (commande == wrAll))
            {
                DecodeThr_w(tabok);
            }
            break;
        default:
            //        command="ERROR";
            break;

        }
        tabok.clear();
    }
    QThread::msleep(500);
}

void MainWindow::connect_Dev()
{
    char byteSendData[12];
    tabok.clear();
    commande=rdGEN;
    byteSendData[0] = 0x11; // Read command
    byteSendData[1] = 0x51; // Read General location
    byteSendData[2] = 0x04; // Data bytes count ???
    byteSendData[3] = 0xB0; // Unknown
    byteSendData[4] = (byteSendData[1] + byteSendData[2] + byteSendData[3]) % 256; // Data verification b
    ui->temesInfo->append("Send Connect_Dev");
    serial->write(byteSendData,5);
    QThread::msleep(500);
}

void MainWindow::Read_Dev(unsigned char cmd)
{
    char byteSendData[12];
    QString command;
    byteSendData[0] = 0x11;   // Read command
    byteSendData[1] = cmd; // Read location (0x5X)
    switch (cmd)
    {
    case GENERAL:
        command="GENERAL";
        break;
    case BASIC:
        command="BASIC";
        break;
    case PEDAL_ASSIST:
        command="PEDAL_ASSIST";
        break;
    case THROTTLE:
        command="THROTTLE";
        break;
    default:
        command="ERROR";
        break;
    }
    ui->temesInfo->append("Send Read_Dev + " + command);
    serial->write(byteSendData,2);
    QThread::msleep(500);                // Wait for controller to prepare for sending
}

void MainWindow::Write_Dev(unsigned char cmd)
{
    unsigned char byteSendData[50];
    unsigned short i,st,taille=0;
    switch (cmd)
    {
    case BASIC:
        ui->temesInfo->append("Send Write_Dev Basic");
        byteSendData[0]  = 0x16; // Write command
        byteSendData[1]  = 0x52; // Write location - Basic block
        byteSendData[2]  = 24;  // 24 bytes of settings to be written to flash
        // Add Basic block settings
        byteSendData[3]  = ui->cbBasLowBat->currentText().toInt();
        byteSendData[4]  = ui->spBasCurrLim->value();
        byteSendData[5]  = ui->spBasLvl0CurrLim->value();
        byteSendData[6]  = ui->spBasLvl1CurrLim->value();
        byteSendData[7]  = ui->spBasLvl2CurrLim->value();
        byteSendData[8]  = ui->spBasLvl3CurrLim->value();
        byteSendData[9]  = ui->spBasLvl4CurrLim->value();
        byteSendData[10] = ui->spBasLvl5CurrLim->value();
        byteSendData[11] = ui->spBasLvl6CurrLim->value();
        byteSendData[12] = ui->spBasLvl7CurrLim->value();
        byteSendData[13] = ui->spBasLvl8CurrLim->value();
        byteSendData[14] = ui->spBasLvl9CurrLim->value();
        byteSendData[15] = ui->spBasLvl0SpdLim->value();
        byteSendData[16] = ui->spBasLvl1SpdLim->value();
        byteSendData[17] = ui->spBasLvl2SpdLim->value();
        byteSendData[18] = ui->spBasLvl3SpdLim->value();
        byteSendData[19] = ui->spBasLvl4SpdLim->value();
        byteSendData[20] = ui->spBasLvl5SpdLim->value();
        byteSendData[21] = ui->spBasLvl6SpdLim->value();
        byteSendData[22] = ui->spBasLvl7SpdLim->value();
        byteSendData[23] = ui->spBasLvl8SpdLim->value();
        byteSendData[24] = ui->spBasLvl9SpdLim->value();

        if (ui->cbBasWheelDiam->currentIndex() == 12)// if wheel size is 700C
            byteSendData[25] = 55;
        else
            byteSendData[25] = ui->cbBasWheelDiam->currentText().toInt() * 2;

        if (ui->cbBasSpdMtrType->currentIndex() == 2 ) // if speed meter is by motor phase (for hub motors)
            byteSendData[26] = (3 * 64) + ui->spBasSpdMtrSig->value();
        else
            byteSendData[26] = (ui->cbBasSpdMtrType->currentIndex() * 64) + ui->spBasSpdMtrSig->value();
        // End Add Basic block settings

        st = 0;
        for (i= 0;i<= 25;i++)
        {              // cycle from 0 to data lenght + 1 (24 + 1 = 25)
            st = st + byteSendData[i + 1]; // sum all data bytes including location and lenght bytes and excluding the command byte
        }
        byteSendData[27] = st % 256;   // Data verification byte - equals the remainder of sum of all data bytes divided by 256
        taille= 28; // size to writre
        break;
    case PEDAL_ASSIST:
        ui->temesInfo->append("Send Write_Dev Pedal Assist");
        byteSendData[0] = 0x16; // Write command
        byteSendData[1] = 0x53; // Write location - PAS block
        byteSendData[2] = 11;  // 11 bytes of settings to be written to flash
        // Add PAS block settings
        byteSendData[3] = ui->cbPASPedalSensType->currentIndex();

        if (ui->cbPASDesigAssist->currentIndex() == 0) // if PAS designated assist is set by LCD
            byteSendData[4] = 255;
        else
            byteSendData[4] = ui->cbPASDesigAssist->currentIndex() - 1;

        if (ui->cbPASSpdLim->currentIndex() == 0) // if PAS speed limit is set by LCD
            byteSendData[5] = 255;
        else
            byteSendData[5] = ui->cbPASSpdLim->currentIndex() + 14;

        byteSendData[6] = ui->spPASStartCurr->value();
        byteSendData[7] = ui->cbPASSlowStartMode->currentIndex() + 1;
        byteSendData[8] = ui->spPASStartDeg->value();

        if (ui->cbPASWorkMode->currentIndex() == 0)// if PAS work mode is undetermined
            byteSendData[9] = 255;
        else
            byteSendData[9] = ui->cbPASWorkMode->currentIndex() + 9;

        byteSendData[10] = ui->spPASStopDelay->value();
        byteSendData[11] = ui->spPASCurrDecay->value();
        byteSendData[12] = ui->spPASStopDecay->value();
        byteSendData[13] = ui->spPASKeepCurr->value();
        // End Add PAS block settings
        st=0;
        for (i=0;i<=12;i++)    // cycle from 0 to data lenght + 1 (11 + 1 = 12)
        {
            st = st + byteSendData[i + 1]; // sum all data bytes including location and lenght bytes and excluding the command byte
        }
        byteSendData[14] = st % 256;   // Data verification byte - equals the remainder of sum of all data bytes divided by 256
        taille = 15; // size to writre
        break;
    case THROTTLE:
        ui->temesInfo->append("Send Write_Dev Throttle");
        byteSendData[0] = 0x16; // Write command
        byteSendData[1] = 0x54; // Write location - Throttle block
        byteSendData[2] = 6;   // 6 bytes of settings to be written to flash
        // Add Throttle block settings
        byteSendData[3] = ui->spThrStartVolt->value();
        byteSendData[4] = ui->spThrEndVolt->value();
        byteSendData[5] = ui->cbThrMode->currentIndex();

        if (ui->cbThrAssistLvl->currentIndex()== 0) // if Throttle assist level is set by LCD
            byteSendData[6] = 255;
        else
            byteSendData[6] = ui->cbThrAssistLvl->currentIndex() - 1;

        if (ui->cbThrSpeedLim->currentIndex() == 0) // if Throttle speed limit is set by LCD
            byteSendData[7] = 255;
        else
            byteSendData[7] = ui->cbThrSpeedLim->currentIndex() + 14;

        byteSendData[8] = ui->spThrStartCurr->value();
        // End Add Throttle block settings

        st=0;
        for( i=0;i<= 7;i++)
        {// cycle from 0 to data lenght + 1 (6 + 1 = 7)
            st = st + byteSendData[i + 1]; // sum all data bytes including location and lenght bytes and excluding the command byte
        }
        byteSendData[9] = st  % 256;   // Data verification byte - equals the remainder of sum of all data bytes divided by 256
        taille = 10; // size to writre
        break;
    }
    serial->write((char *)byteSendData,taille);
    QThread::msleep(500);
}
void MainWindow::on_pbRead_clicked()
{
    tabok.clear();
    commande=rdSingle;
    if(ui->Onglet->currentIndex()==0)
    {
        Read_Dev(BASIC);
    }
    else if(ui->Onglet->currentIndex()==1)
    {
        Read_Dev(PEDAL_ASSIST);
    }
    else if(ui->Onglet->currentIndex()==2)
    {
        Read_Dev(THROTTLE);
    }
}

void MainWindow::on_Onglet_currentChanged(int index)
{
    if(index==3)
    {
        ui->pbRead->hide();
        ui->pbWrite->hide();
    }
    else
    {
        ui->pbRead->show();
        ui->pbWrite->show();
    }
}
void MainWindow::DecodeBas_w(QByteArray rec_tab)
{
    switch ( rec_tab[1])
    {
    case 0: //Low Battery Protect
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Low Battery Protect: valeur hors limite");
        break;
    case 1://Current Limit
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant limite: valeur hors limite");
        break;
    case 2: //Current Limit 0
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 0: valeur hors limite");
        break;
    case 4: //Current Limit 1
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 1: valeur hors limite");
        break;
    case 6: //Current Limit 2
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 2: valeur hors limite");
        break;
    case 8: //Current Limit 3
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 3: valeur hors limite");
        break;
    case 10: //Current Limit 4
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 4: valeur hors limite");
        break;
    case 12: //Current Limit 5
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 5: valeur hors limite");
        break;
    case 14: //Current Limit 6
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 6: valeur hors limite");
        break;
    case 16: //Current Limit 7
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 7: valeur hors limite");
        break;
    case 18: //Current Limit 8
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 8: valeur hors limite");
        break;
    case 20: //Current Limit 9
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Courant Limite 9: valeur hors limite");
        break;
    case 3: //Limit SPD0
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 0: valeur hors limite");
        break;
    case 5: //Limit SPD1
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 1: valeur hors limite");
        break;
    case 7: //Limit SPD2
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 2: valeur hors limite");
        break;
    case 9: //Limit SPD3
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 3: valeur hors limite");
        break;
    case 11: //Limit SPD4
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 4: valeur hors limite");
        break;
    case 13: //Limit SPD5
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 5: valeur hors limite");
        break;
    case 15: //Limit SPD6
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 6: valeur hors limite");
        break;
    case 17: //Limit SPD7
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 7: valeur hors limite");
        break;
    case 19: //Limit SPD8
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 8: valeur hors limite");
        break;
    case 21: //Limit SPD9
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Limite 9: valeur hors limite");
        break;
    case 22: //Wheel Diameter
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Diametre Roue: valeur hors limite");
        break;
    case 23: //SpdMeter Signal
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("BAS:Speed Meter Signal: valeur hors limite");
        break;
    case 24:
        if (commande == wrAll)
        {
            ui->temesInfo->append("BAS:Parametre Basic Write OK");
            Write_Dev(PEDAL_ASSIST);
        }
        else
        {
            commande = rdIgnore; // ignore further data
            ui->lblerror->setText("");
            ui->lblerror->setStyleSheet("background-color: ");
            ui->temesInfo->append("BAS:Parametre Basic Write OK");
        }
        break; // case
    }
}
void MainWindow::DecodePedAs_w(QByteArray rec_tab)
{
    switch(rec_tab[1])
    {
    case 0: //Pedal Sensor Type
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Pedal Sensor Type: valeur erronnee");
        break;
    case 1: //Designated Assist Level
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Designated Assist Level: valeur erronnee");
        break;
    case 2: //Speed Limit
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Speed Limit: valeur erronnee");
        break;
    case 3: //Start Current
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Start Current: valeur hors limite");
        break;
    case 4: //Slow-start Mode
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Slow-Start Mode: mode erronne");
        break;
    case 5: //Start Degree
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Start Degree: valeur hors limite");
        break;
    case 6: //Work Mode
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Work Mode: mode erronne");
        break;
    case 7: //Stop Delay
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Stop Delay: valeur hors limite");
        break;
    case 8: //Current Decay
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Current Decay: valeur hors limite");
        break;
    case 9: //Stop Decay
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Stop Decay: valeur hors limite");
        break;
    case 10: //Keep Current
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("PAS: Keep Current: valeur hors limite");
        break;
    case 11:
        if (commande == wrAll)
        {
            ui->temesInfo->append("PAS:Parametre Pedal Assit Write OK");
            Write_Dev(THROTTLE);
        }
        else
        {
            commande =rdIgnore;
            ui->lblerror->setText("");
            ui->lblerror->setStyleSheet("background-color: ");
            ui->temesInfo->append("PAS:Parametre Pedal Assist Write OK");
        }
        break;

    } // case
}
void MainWindow::DecodeThr_w(QByteArray rec_tab)
{
    switch (rec_tab[1])
    {
    case 0: //Start Voltage
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("THR: Start Voltage: valeur hors limite");
        break;
    case 1: //End Voltage
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("THR: End Voltage: valeur hors limite");
        break;
    case 2: //Mode
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("THR: Mode: mode erronne");
        break;
    case 3: //Designated Assist
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("THR: Designated Assist: valeur erronnee");
        break;
    case 4: //Speed Limit
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("THR: Speed Limit: valeur hors limite");
        break;
    case 5: //Start Current
        ui->lblerror->setText("Erreur Exist: See Infos Tab");
        ui->lblerror->setStyleSheet("background-color: red");
        ui->temesInfo->append("THR: Start Current: valeur hors limite");
        break;
    case 6:
        if (commande == wrSingle)
        {
            ui->lblerror->setText("");
            ui->lblerror->setStyleSheet("background-color: ");
            ui->temesInfo->append("THR:Parametre Throttle Write OK");
            commande =rdIgnore;
        }
        else
        {
            ui->lblerror->setText("");
            ui->lblerror->setStyleSheet("background-color: ");
            ui->temesInfo->append("THR:Parametre Throttle Write OK");
            ui->temesInfo->append("ALL: Ecriture Flash complete OK");
            commande = rdIgnore; // ignore further data
        }
        break;
    }
}

void MainWindow::DecodePedAs(QByteArray rec_tab)
{
    ui->cbPASPedalSensType->setCurrentIndex(rec_tab[2]);
    if ((unsigned char)rec_tab[3] == 255)
        ui->cbPASDesigAssist->setCurrentIndex(0);
    else
        ui->cbPASDesigAssist->setCurrentIndex(rec_tab[3] + 1);

    if ((unsigned char)rec_tab.at(4) == 0xff)
        ui->cbPASSpdLim->setCurrentIndex(0);
    else
        ui->cbPASSpdLim->setCurrentIndex(rec_tab[4] - 14);

    ui->spPASStartCurr->setValue(rec_tab[5]);
    ui->cbPASSlowStartMode->setCurrentIndex(rec_tab[6] - 1);
    ui->spPASStartDeg->setValue(rec_tab[7]);
    if ((unsigned char)rec_tab[8] == 255)
        ui->cbPASWorkMode->setCurrentIndex(0);
    else
        ui->cbPASWorkMode->setCurrentIndex(rec_tab[8] - 9);

    ui->spPASStopDelay->setValue(rec_tab[9]);
    ui->spPASCurrDecay->setValue(rec_tab[10]);
    ui->spPASStopDecay->setValue(rec_tab[11]);
    ui->spPASKeepCurr->setValue(rec_tab[12]);

    if (commande == rdSingle)
    {
        commande = rdIgnore; // ignore further data
        // Application.MessageBox(PAnsiChar(GetErrorText('err33')), PAnsiChar(GetErrorTitle('err33')), mb_ICONINFORMATION+mb_Ok);
    }
    else
    {
        commande = rdAll;
        Read_Dev(THROTTLE);
    }
}

void MainWindow::DecodeThr(QByteArray rec_tab)
{
    ui->spThrStartVolt->setValue(rec_tab[2]);
    ui->spThrEndVolt->setValue(rec_tab[3]);
    ui->cbThrMode->setCurrentIndex(rec_tab[4]);

    if ((unsigned char) rec_tab[5] == 255)
        ui->cbThrAssistLvl->setCurrentIndex(0);
    else
        ui->cbThrAssistLvl->setCurrentIndex(rec_tab[5] + 1);

    if ((unsigned char) rec_tab[6] == 255)
        ui->cbThrSpeedLim->setCurrentIndex(0);
    else
        ui->cbThrSpeedLim->setCurrentIndex(rec_tab[6] - 14);

    ui->spThrStartCurr->setValue(rec_tab[7]);

    if (commande == rdSingle)
    {}
    else
    {
        commande= rdIgnore; // ignore further data
    }
}

void MainWindow::DecodeBas(QByteArray rec_tab)
{
    unsigned char i,t,m;
    ui->cbBasLowBat->clear();
    if (ui->lbNomVolt->text()== "24V" )
    {
        for(i=18;i<=22;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
    }
    if (ui->lbNomVolt->text() == "36V")
    {
        for(i= 28;i<=  32;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
    }
    if (ui->lbNomVolt->text() == "48V")
    {
        for (i = 38;i<=43;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
    }
    if (ui->lbNomVolt->text() =="60V")
    {
        for(i = 48;i<=55;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
    }
    if (ui->lbNomVolt->text() == "24-48V")
    {
        for( i = 18;i<=43;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
    }
    if (ui->lbNomVolt->text() == "24-60V")
    {
        for( i = 18;i<=55;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
    }
    ui->cbBasLowBat->setCurrentText(QString::number(rec_tab[2]));
    //cbbBasLowBat.ItemIndex        := cbbBasLowBat.Items.IndexOf(IntToStr(data_in[2]));
    ui->spBasCurrLim->setValue(rec_tab[3]);
    ui->spBasLvl0CurrLim->setValue(rec_tab[4]);
    ui->spBasLvl1CurrLim->setValue(rec_tab[5]);
    ui->spBasLvl2CurrLim->setValue(rec_tab[6]);
    ui->spBasLvl3CurrLim->setValue(rec_tab[7]);
    ui->spBasLvl4CurrLim->setValue(rec_tab[8]);
    ui->spBasLvl5CurrLim->setValue(rec_tab[9]);
    ui->spBasLvl6CurrLim->setValue(rec_tab[10]);
    ui->spBasLvl7CurrLim->setValue(rec_tab[11]);
    ui->spBasLvl8CurrLim->setValue(rec_tab[12]);
    ui->spBasLvl9CurrLim->setValue(rec_tab[13]);
    ui->spBasLvl0SpdLim->setValue(rec_tab[14]);
    ui->spBasLvl1SpdLim->setValue(rec_tab[15]);
    ui->spBasLvl2SpdLim->setValue(rec_tab[16]);
    ui->spBasLvl3SpdLim->setValue(rec_tab[17]);
    ui->spBasLvl4SpdLim->setValue(rec_tab[18]);
    ui->spBasLvl5SpdLim->setValue(rec_tab[19]);
    ui->spBasLvl6SpdLim->setValue(rec_tab[20]);
    ui->spBasLvl7SpdLim->setValue(rec_tab[21]);
    ui->spBasLvl8SpdLim->setValue(rec_tab[22]);
    ui->spBasLvl9SpdLim->setValue(rec_tab[23]);

    // Convert wheel size
    t= rec_tab[24]% 2;
    m= (rec_tab[24]% 2) + (rec_tab[24]/2);
    if (m > 27)
    {
        if ((m == 28) && (t == 1))
            ui->cbBasWheelDiam->setCurrentIndex(12);
        else
            ui->cbBasWheelDiam->setCurrentIndex(m - 15);
    }
    else
        ui->cbBasWheelDiam->setCurrentIndex(m - 16);

    // Convert speed meter type and signals
    m = rec_tab[25] / 64;
    if (m == 3)
        ui->cbBasSpdMtrType->setCurrentIndex(2);
    else
        ui->cbBasSpdMtrType->setCurrentIndex(m);
    ui->spBasSpdMtrSig->setValue(rec_tab[25]/ 64);

    if (commande == rdSingle)
    {
        commande = rdIgnore; // ignore further data
    }
    else
    {
        commande = rdAll;
        Read_Dev(PEDAL_ASSIST);
    }
}

void MainWindow::DecodeGen(QByteArray rec_tab)
{
    unsigned char i;
    QString str_tmp;
    str_tmp=QString(rec_tab);
    ui->lbManuf->setText(str_tmp.mid(2,4)); //  +rec_tab.at(3)+rec_tab.at(4); // Manufacturer
    ui->lbModel->setText(str_tmp.mid(6, 4)); // Model
    ui->lbHWVer->setText('V' + QString(rec_tab.at(10)) + '.' +QString(rec_tab.at(11))); // Hardware verion
    ui->lbFWVer->setText('V' + QString(rec_tab.at(12)) + '.' + QString(rec_tab.at(13)) + '.' + QString(rec_tab.at(14)) + '.' + QString(rec_tab.at(15))); // Firmware version
    ui->cbBasLowBat->clear();
    switch(rec_tab[16])
    { // Nominal voltage
    case 0:
        ui->lbNomVolt->setText("24V");
        for (i=18;i<=22;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
        break;
    case 1:
        ui->lbNomVolt->setText("36V");
        for (i=28;i<=32;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
        break;
    case 2:
        ui->lbNomVolt->setText("48V");
        for (i=38;i<=43;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
        break;
    case 3:
        ui->lbNomVolt->setText("60V");
        for (i=48;i<=55;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
        break;
    case 4:
        ui->lbNomVolt->setText("24V-48V");
        for (i=18;i<=43;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
        break;
    default:
        ui->lbNomVolt->setText("24-60V");
        for (i=18;i<=55;i++)
        {
            ui->cbBasLowBat->addItem(QString::number(i));
        }
        break;
    }  // Nominal Voltage case end
    ui->lbMaxCurr->setText(QString::number(rec_tab[17])+'A');
    ui->spBasCurrLim->setMaximum(rec_tab[17]); // limit control
    commande= rdIgnore; // ignore further data
}

void MainWindow::on_pbWrite_clicked()
{
    tabok.clear();
    commande=wrSingle;
    if(ui->Onglet->currentIndex()==0)
    {
        Write_Dev(BASIC);
    }
    else if(ui->Onglet->currentIndex()==1)
    {
        Write_Dev(PEDAL_ASSIST);
    }
    else if(ui->Onglet->currentIndex()==2)
    {
        Write_Dev(THROTTLE);
    }
}

void MainWindow::on_pbReadFlash_clicked()
{
    commande=rdAll;
    Read_Dev(BASIC); // Write settings to controller
}


void MainWindow::on_pbWriteFlash_clicked()
{
    commande=wrAll;
    Write_Dev(BASIC);
    // Write_Dev(PEDAL_ASSIST);
    // Write_Dev(THROTTLE);
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionLoad_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);
    fichier_nom = dialog.getOpenFileName(this);
    if(!fichier_nom.isEmpty())
    {
        ui->temesInfo->append("Load config file : "+ fichier_nom);
        QSettings fichier_el(fichier_nom, QSettings::IniFormat);

        // Load from file
        // Load basic settings
        ui->cbBasLowBat->setCurrentText(fichier_el.value("Basic/LBP",0).toString());
        ui->spBasCurrLim->setValue(fichier_el.value("Basic/LC",0).toUInt());
        ui->spBasLvl0CurrLim->setValue(fichier_el.value("Basic/ALC0",0).toUInt());//('Basic','ALC0',10);
        ui->spBasLvl1CurrLim->setValue(fichier_el.value("Basic/ALC1",0).toUInt());//('Basic','ALC1',10);
        ui->spBasLvl2CurrLim->setValue(fichier_el.value("Basic/ALC2",0).toUInt());//('Basic','ALC2',10);
        ui->spBasLvl3CurrLim->setValue(fichier_el.value("Basic/ALC3",0).toUInt());//('Basic','ALC3',10);
        ui->spBasLvl4CurrLim->setValue(fichier_el.value("Basic/ALC4",0).toUInt());//('Basic','ALC4',10);
        ui->spBasLvl5CurrLim->setValue(fichier_el.value("Basic/ALC5",0).toUInt());//('Basic','ALC5',10);
        ui->spBasLvl6CurrLim->setValue(fichier_el.value("Basic/ALC6",0).toUInt());//('Basic','ALC6',10);
        ui->spBasLvl7CurrLim->setValue(fichier_el.value("Basic/ALC7",0).toUInt());//('Basic','ALC7',10);
        ui->spBasLvl8CurrLim->setValue(fichier_el.value("Basic/ALC8",0).toUInt());//('Basic','ALC8',10);
        ui->spBasLvl9CurrLim->setValue(fichier_el.value("Basic/ALC9",0).toUInt());//('Basic','ALC9',10);
        ui->spBasLvl0SpdLim->setValue(fichier_el.value("Basic/ALBP0",0).toUInt());// ('Basic','ALBP0',10);
        ui->spBasLvl1SpdLim->setValue(fichier_el.value("Basic/ALBP1",0).toUInt());// ('Basic','ALBP1',10);
        ui->spBasLvl2SpdLim->setValue(fichier_el.value("Basic/ALBP2",0).toUInt());// ('Basic','ALBP2',10);
        ui->spBasLvl3SpdLim->setValue(fichier_el.value("Basic/ALBP3",0).toUInt());// ('Basic','ALBP3',10);
        ui->spBasLvl4SpdLim->setValue(fichier_el.value("Basic/ALBP4",0).toUInt());// ('Basic','ALBP4',10);
        ui->spBasLvl5SpdLim->setValue(fichier_el.value("Basic/ALBP5",0).toUInt());// ('Basic','ALBP5',10);
        ui->spBasLvl6SpdLim->setValue(fichier_el.value("Basic/ALBP6",0).toUInt());// ('Basic','ALBP6',10);
        ui->spBasLvl7SpdLim->setValue(fichier_el.value("Basic/ALBP7",0).toUInt());// ('Basic','ALBP7',10);
        ui->spBasLvl8SpdLim->setValue(fichier_el.value("Basic/ALBP8",0).toUInt());// ('Basic','ALBP8',10);
        ui->spBasLvl9SpdLim->setValue(fichier_el.value("Basic/ALBP9",0).toUInt());// ('Basic','ALBP9',10);
        ui->cbBasWheelDiam->setCurrentIndex(fichier_el.value("Basic/WD",0).toUInt());//('Basic','WD',10);
        ui->cbBasSpdMtrType->setCurrentIndex(fichier_el.value("Basic/SMM",0).toUInt());//('Basic','SMM',0);
        ui->spBasSpdMtrSig->setValue(fichier_el.value("Basic/SMS",0).toUInt());//('Basic','SMS',10);
        // Load pedal assist settings
        ui->cbPASPedalSensType->setCurrentIndex(fichier_el.value("Pedal Assist/PT",0).toUInt());//('Pedal Assist','PT',0);
        ui->cbPASDesigAssist->setCurrentIndex(fichier_el.value("Pedal Assist/DA",0).toUInt());//('Pedal Assist','DA',0);
        ui->cbPASSpdLim->setCurrentIndex(fichier_el.value("Pedal Assist/SL",0).toUInt());//('Pedal Assist','SL',0);
        ui->cbPASSlowStartMode->setCurrentIndex(fichier_el.value("Pedal Assist/SSM",0).toUInt());//('Pedal Assist','SSM',0);
        ui->cbPASWorkMode->setCurrentIndex(fichier_el.value("Pedal Assist/WM",0).toUInt());//('Pedal Assist','WM',0);
        ui->spPASStartCurr->setValue(fichier_el.value("Pedal Assist/SC",0).toUInt());  //('Pedal Assist','SC',10);
        ui->spPASStartDeg->setValue(fichier_el.value("Pedal Assist/SDN",0).toUInt());//('Pedal Assist','SDN',10);
        ui->spPASStopDelay->setValue(fichier_el.value("Pedal Assist/TS",0).toUInt());//('Pedal Assist','TS',10);
        ui->spPASCurrDecay->setValue(fichier_el.value("Pedal Assist/CD",0).toUInt());//('Pedal Assist','CD',10);
        ui->spPASStopDecay->setValue(fichier_el.value("Pedal Assist/SD",0).toUInt());//('Pedal Assist','SD',10);
        ui->spPASKeepCurr->setValue(fichier_el.value("Pedal Assist/KC",0).toUInt());//('Pedal Assist','KC',10);
        // Load throttle handle settings
        ui->spThrStartVolt->setValue(fichier_el.value("Throttle Handle/SV",0).toUInt());//('Throttle Handle','SV',10);
        ui->spThrEndVolt->setValue(fichier_el.value("Throttle Handle/EV",0).toUInt());//('Throttle Handle','EV',10);
        ui->cbThrMode->setCurrentIndex(fichier_el.value("Throttle Handle/MODE",0).toUInt());//('Throttle Handle','MODE',0);
        ui->cbThrAssistLvl->setCurrentIndex(fichier_el.value("Throttle Handle/DA",0).toUInt());//'Throttle Handle','DA',0);
        ui->cbThrSpeedLim->setCurrentIndex(fichier_el.value("Throttle Handle/SL",0).toUInt());//('Throttle Handle','SL',0);
        ui->spThrStartCurr->setValue(fichier_el.value("Throttle Handle/SC",0).toUInt());//('Throttle Handle','SC',10);
    }
    else {}
}

//// set the setting
//s.setValue("group_name/setting_name", new_value);

//// retrieve the setting
//// Note: Change "toType" to whatever type that this setting is supposed to be
//current_value = s.value("group_name/setting_name", default_value).toType();

void MainWindow::on_actionSave_As_triggered()
{
    QFileDialog dialog(this);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);
    fichier_nom = dialog.getSaveFileName(this);
    if(!fichier_nom.isEmpty())
    {
        QSettings fichier_el(fichier_nom, QSettings::IniFormat);
        fichier_el.setValue("Basic/LBP",ui->cbBasLowBat->currentText());
        fichier_el.setValue("Basic/LC",ui->spBasCurrLim->value());
        fichier_el.setValue("Basic/ALC0",ui->spBasLvl0CurrLim->value());
        fichier_el.setValue("Basic/ALC1",ui->spBasLvl1CurrLim->value());
        fichier_el.setValue("Basic/ALC2",ui->spBasLvl2CurrLim->value());
        fichier_el.setValue("Basic/ALC3",ui->spBasLvl3CurrLim->value());
        fichier_el.setValue("Basic/ALC4",ui->spBasLvl4CurrLim->value());
        fichier_el.setValue("Basic/ALC5",ui->spBasLvl5CurrLim->value());
        fichier_el.setValue("Basic/ALC6",ui->spBasLvl6CurrLim->value());
        fichier_el.setValue("Basic/ALC7",ui->spBasLvl7CurrLim->value());
        fichier_el.setValue("Basic/ALC8",ui->spBasLvl8CurrLim->value());
        fichier_el.setValue("Basic/ALC9",ui->spBasLvl9CurrLim->value());
        fichier_el.setValue("Basic/ALBP0",ui->spBasLvl0SpdLim->value());
        fichier_el.setValue("Basic/ALBP1",ui->spBasLvl1SpdLim->value());
        fichier_el.setValue("Basic/ALBP2",ui->spBasLvl2SpdLim->value());
        fichier_el.setValue("Basic/ALBP3",ui->spBasLvl3SpdLim->value());
        fichier_el.setValue("Basic/ALBP4",ui->spBasLvl4SpdLim->value());
        fichier_el.setValue("Basic/ALBP5",ui->spBasLvl5SpdLim->value());
        fichier_el.setValue("Basic/ALBP6",ui->spBasLvl6SpdLim->value());
        fichier_el.setValue("Basic/ALBP7",ui->spBasLvl7SpdLim->value());
        fichier_el.setValue("Basic/ALBP8",ui->spBasLvl8SpdLim->value());
        fichier_el.setValue("Basic/ALBP9",ui->spBasLvl9SpdLim->value());
        fichier_el.setValue("Basic/WD",ui->cbBasWheelDiam->currentIndex());
        fichier_el.setValue("Basic/SMM",ui->cbBasSpdMtrType->currentIndex());
        fichier_el.setValue("Basic/SMS",ui->spBasSpdMtrSig->value());
        // Save pedal assist settings
        fichier_el.setValue("Pedal Assist/PT",ui->cbPASPedalSensType->currentIndex());
        fichier_el.setValue("Pedal Assist/DA",ui->cbPASDesigAssist->currentIndex());
        fichier_el.setValue("Pedal Assist/SL",ui->cbPASSpdLim->currentIndex());
        fichier_el.setValue("Pedal Assist/SSM",ui->cbPASSlowStartMode->currentIndex());
        fichier_el.setValue("Pedal Assist/WM",ui->cbPASWorkMode->currentIndex());
        fichier_el.setValue("Pedal Assist/SC",ui->spPASStartCurr->value());
        fichier_el.setValue("Pedal Assist/SDN",ui->spPASStartDeg->value());
        fichier_el.setValue("Pedal Assist/TS",ui->spPASStopDelay->value());
        fichier_el.setValue("Pedal Assist/CD",ui->spPASCurrDecay->value());
        fichier_el.setValue("Pedal Assist/SD",ui->spPASStopDecay->value());
        fichier_el.setValue("Pedal Assist/KC",ui->spPASKeepCurr->value());
        // Save throttle handle settings
        fichier_el.setValue("Throttle Handle/SV",ui->spThrStartVolt->value());
        fichier_el.setValue("Throttle Handle/EV",ui->spThrEndVolt->value());
        fichier_el.setValue("Throttle Handle/MODE",ui->cbThrMode->currentIndex());
        fichier_el.setValue("Throttle Handle/DA",ui->cbThrAssistLvl->currentIndex());
        fichier_el.setValue("Throttle Handle/SL",ui->cbThrSpeedLim->currentIndex());
        fichier_el.setValue("Throttle Handle/SC",ui->spThrStartCurr->value());
        fichier_el.sync();
        ui->temesInfo->append("Save config file As : "+ fichier_nom);
        QByteArray fileData;
        QFile file(fichier_nom);
        file.open(QIODevice::ReadWrite); // open for read and write
        fileData = file.readAll(); // read all the data into the byte array
        QString text(fileData); // add to text string for easy string replace
        text.replace(QString("%20"), QString(" ")); // replace text in string
        file.resize(0);
        file.seek(0); // go to the beginning of the file
        file.write(text.toUtf8()); // write the new text back to the file

        file.close(); // close the file handle.
    }
    else {}
}

void MainWindow::on_actionSave_triggered()
{
    if(!fichier_nom.isEmpty())
    {
        QSettings fichier_el(fichier_nom, QSettings::IniFormat);
        fichier_el.setValue("Basic/LBP",ui->cbBasLowBat->currentText());
        fichier_el.setValue("Basic/LC",ui->spBasCurrLim->value());
        fichier_el.setValue("Basic/ALC0",ui->spBasLvl0CurrLim->value());
        fichier_el.setValue("Basic/ALC1",ui->spBasLvl1CurrLim->value());
        fichier_el.setValue("Basic/ALC2",ui->spBasLvl2CurrLim->value());
        fichier_el.setValue("Basic/ALC3",ui->spBasLvl3CurrLim->value());
        fichier_el.setValue("Basic/ALC4",ui->spBasLvl4CurrLim->value());
        fichier_el.setValue("Basic/ALC5",ui->spBasLvl5CurrLim->value());
        fichier_el.setValue("Basic/ALC6",ui->spBasLvl6CurrLim->value());
        fichier_el.setValue("Basic/ALC7",ui->spBasLvl7CurrLim->value());
        fichier_el.setValue("Basic/ALC8",ui->spBasLvl8CurrLim->value());
        fichier_el.setValue("Basic/ALC9",ui->spBasLvl9CurrLim->value());
        fichier_el.setValue("Basic/ALBP0",ui->spBasLvl0SpdLim->value());
        fichier_el.setValue("Basic/ALBP1",ui->spBasLvl1SpdLim->value());
        fichier_el.setValue("Basic/ALBP2",ui->spBasLvl2SpdLim->value());
        fichier_el.setValue("Basic/ALBP3",ui->spBasLvl3SpdLim->value());
        fichier_el.setValue("Basic/ALBP4",ui->spBasLvl4SpdLim->value());
        fichier_el.setValue("Basic/ALBP5",ui->spBasLvl5SpdLim->value());
        fichier_el.setValue("Basic/ALBP6",ui->spBasLvl6SpdLim->value());
        fichier_el.setValue("Basic/ALBP7",ui->spBasLvl7SpdLim->value());
        fichier_el.setValue("Basic/ALBP8",ui->spBasLvl8SpdLim->value());
        fichier_el.setValue("Basic/ALBP9",ui->spBasLvl9SpdLim->value());
        fichier_el.setValue("Basic/WD",ui->cbBasWheelDiam->currentIndex());
        fichier_el.setValue("Basic/SMM",ui->cbBasSpdMtrType->currentIndex());
        fichier_el.setValue("Basic/SMS",ui->spBasSpdMtrSig->value());
        // Save pedal assist settings
        fichier_el.setValue("Pedal Assist/PT",ui->cbPASPedalSensType->currentIndex());
        fichier_el.setValue("Pedal Assist/DA",ui->cbPASDesigAssist->currentIndex());
        fichier_el.setValue("Pedal Assist/SL",ui->cbPASSpdLim->currentIndex());
        fichier_el.setValue("Pedal Assist/SSM",ui->cbPASSlowStartMode->currentIndex());
        fichier_el.setValue("Pedal Assist/WM",ui->cbPASWorkMode->currentIndex());
        fichier_el.setValue("Pedal Assist/SC",ui->spPASStartCurr->value());
        fichier_el.setValue("Pedal Assist/SDN",ui->spPASStartDeg->value());
        fichier_el.setValue("Pedal Assist/TS",ui->spPASStopDelay->value());
        fichier_el.setValue("Pedal Assist/CD",ui->spPASCurrDecay->value());
        fichier_el.setValue("Pedal Assist/SD",ui->spPASStopDecay->value());
        fichier_el.setValue("Pedal Assist/KC",ui->spPASKeepCurr->value());
        // Save throttle handle settings
        fichier_el.setValue("Throttle Handle/SV",ui->spThrStartVolt->value());
        fichier_el.setValue("Throttle Handle/EV",ui->spThrEndVolt->value());
        fichier_el.setValue("Throttle Handle/MODE",ui->cbThrMode->currentIndex());
        fichier_el.setValue("Throttle Handle/DA",ui->cbThrAssistLvl->currentIndex());
        fichier_el.setValue("Throttle Handle/SL",ui->cbThrSpeedLim->currentIndex());
        fichier_el.setValue("Throttle Handle/SC",ui->spThrStartCurr->value());
        fichier_el.sync();
        ui->temesInfo->append("Save config file : "+ fichier_nom);
        QByteArray fileData;
        QFile file(fichier_nom);
        file.open(QIODevice::ReadWrite | QIODevice::Text); // open for read and write
        fileData = file.readAll(); // read all the data into the byte array
        QString text(fileData); // add to text string for easy string replace

        text.replace(QString("%20"), QString(" ")); // replace text in string
        file.resize(0);
        file.seek(0); // go to the beginning of the file
        file.write(text.toUtf8()); // write the new text back to the file

        file.close(); // close the file handle.
    }
    else {}
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,"QT - Bafang Config Tool",
                           "<h4>Rewrite QT C++ of Bafang Config Tool </h4>\n\n"
                           "D'apres PenOff: <a href=\"https://penoff.me/category/projects/e-bike-conversion\">PenOff's site</a><br>"
                           "Et Blitip: <a href=\"https://blitip.blogspot.com/p/velotaff.html\">Blitip modification site </a><br>"
                           "Github du projet : <a href=\"https://github.com/smile5/BafangConfigToolCpp\">Lien projet</a><br>"
                           "<h4>Rewrite in QT By smile5</h4>");
}

