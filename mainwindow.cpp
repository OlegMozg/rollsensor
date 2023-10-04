#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>
#include <QCanBusFrame>
#include <QMessageBox>
#include <QString>
#include <QListWidgetItem>
#include <QByteArray>
#include <QDataStream>
#include <QTimer>
#include <cstdlib>
#include <QProcess>
#include <QInputDialog>
#include<QCoreApplication>
#include <libusb-1.0/libusb.h>

#define MIN_ANGLE -10.0
#define MAX_ANGLE +10.0
#define sensor_step 0.1
#define DLC 8
#define FRAME_ID 0x01101805
#define PERIOD 100
#define DEBUG true
#define ID_VENDOR 0x1111//заменить на can-овские
#define ID_PRODUCT 0xc31c


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer=new QTimer(this);
    device=new CanDevice();
    ui->label_1->setText("Ось X:");
    ui->label_2->setText("Ось Y:");
    ui->doubleSpinBox->setMinimum(MIN_ANGLE);
    ui->doubleSpinBox->setMaximum(MAX_ANGLE);
    ui->doubleSpinBox_2->setMinimum(MIN_ANGLE);
    ui->doubleSpinBox_2->setMaximum(MAX_ANGLE);
    ui->doubleSpinBox->setValue(0.0);//по умолчанию
    ui->doubleSpinBox_2->setValue(0.0);//по умолчанию
    ui->doubleSpinBox->setSingleStep(sensor_step);
    ui->doubleSpinBox_2->setSingleStep(sensor_step);
    ui->doubleSpinBox->setSuffix(QChar(176));
    ui->doubleSpinBox_2->setSuffix(QChar(176));

    QFont font,font_2 =ui->label_1->font();
    font.setPixelSize(36);
    ui->label_1->setFont(font);
    ui->label_2->setFont(font);
    ui->doubleSpinBox->setFont(font);
    ui->doubleSpinBox_2->setFont(font);
    ui->pushButton->setText("Выход");
    ui->pushButton_2->setText("Старт");
    box_1.title=ui->label_1->text();
    box_2.title=ui->label_2->text();
    box_1.angle=ui->doubleSpinBox->value();
    box_2.angle=ui->doubleSpinBox_2->value();
    ui->pushButton_2->setEnabled(false);
    ui->label->setText("");
    ui->pushButton_3->setText("Устройства");
    ui->label_3->setText("Активные");
    font_2.setPixelSize(24);
    ui->label_4->setFont(font_2);
    ui->label_4->setText("Ошибки на шине:");
    ui->checkBox->setText("Физический контроллер");

    password="";

    connect(timer,SIGNAL(timeout()),this,SLOT(give_required_data()));
    connect(this,SIGNAL(time_to_write_msg(double,double)),this->device,SLOT(BCM_send(double,double)));
    connect(this->device,SIGNAL(bus_error(QString)),this,SLOT(show_error(QString)));
    connect(this->device,SIGNAL(msg_wasnt_sent(QString)),this,SLOT(reaction_on_unsended_msg(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
    delete device;
}

void MainWindow::on_pushButton_clicked()//quit
{
    if(timer->isActive()){
        timer->stop();
        ui->label->setText("Имитация остановлена!");
    }
    if(ui->listWidget->count()!=0)
        ui->listWidget->clear();

    this->close();
}

void MainWindow::on_doubleSpinBox_textChanged(const QString &arg1)
{
    bool ok;
    QMessageBox MSG;
    QString text=arg1;
    text.remove(ui->doubleSpinBox->suffix());
    text.remove(ui->doubleSpinBox->prefix());
    text.replace(",",".");
    double potential_angle=text.toDouble(&ok);
    if(ok){
        if(std::abs(ui->doubleSpinBox->value())>10.0){
            MSG.setText("Выход из допустимого диапазона");
            MSG.exec();
            ui->doubleSpinBox->setValue(box_1.angle);
        }
        box_1.angle=potential_angle;
    }
    else{
        MSG.setText("Некорректные данные!");
        MSG.exec();
        ui->doubleSpinBox->setValue(box_1.angle);
    }
}

void MainWindow::on_doubleSpinBox_2_textChanged(const QString &arg1)
{
    bool ok;
    QMessageBox MSG;
    QString text=arg1;
    text.remove(ui->doubleSpinBox_2->suffix());
    text.remove(ui->doubleSpinBox_2->prefix());
    text.replace(",",".");
    double potential_angle=text.toDouble(&ok);
    if(ok){
        if(std::abs(ui->doubleSpinBox_2->value())>10.0){
            MSG.setText("Выход из допустимого диапазона");
            MSG.exec();
            ui->doubleSpinBox_2->setValue(box_2.angle);
        }
        box_2.angle=potential_angle;
    }
    else{
        MSG.setText("Некорректные данные!");
        MSG.exec();
        ui->doubleSpinBox_2->setValue(box_2.angle);
    }
}

void CanDevice::BCM_send(double angle_x,double angle_y){

    int32_t number1=static_cast<int32_t>(angle_x*(-100));
    int32_t number2=static_cast<int32_t>(angle_y*100);
    qDebug()<<number1<<number2;
    QCanBusFrame frame;
    QByteArray data;
    int64_t payload=(static_cast<int64_t>(number2)<<32)|(static_cast<int64_t>(number1)& 0xFFFFFFFF);
    qDebug()<<"payload:"<<payload<<"size:"<<sizeof(payload);
    qDebug()<<"старшие байты"<<(payload>>32);
    int32_t val=static_cast<int32_t>(payload&0xFFFFFFFF);
    qDebug()<<"младшие байты"<<val;
    qDebug()<<"here_1";
    data=int64ToByteArray(payload);
    qDebug()<<"Data:"<<data<<"size:"<<sizeof(data.data());
    frame.setExtendedFrameFormat(true);
    frame.setFrameType(QCanBusFrame::DataFrame);
    frame.setFrameId(FRAME_ID);
    frame.setPayload(data);
    bool result=current_device->writeFrame(frame);
    qDebug()<<"here2";
    if(!result){
        if(current_device->state()==QCanBusDevice::ConnectedState)
            current_device->disconnectDevice();
        QString err_msg="Сообщение не отправлено! Проверьте активные CAN-устройства!";
        emit msg_wasnt_sent(err_msg);
    }
}

void MainWindow::on_pushButton_2_clicked()//start timer//активна только при наличии сконфигурированного интерфейса
{
    if(!timer->isActive()){
        timer->setInterval(PERIOD);
        ui->label->setText("Имитация началась!");
        ui->pushButton_2->setEnabled(false);
        timer->start();
    }
}

void MainWindow::on_pushButton_3_clicked()//view devices
{
    ui->listWidget->clear();
    QString errorstring;
    QMessageBox msg;
    bool select_pi=false;
    if(ui->checkBox->isChecked())
        select_pi=true;
    QList<QCanBusDeviceInfo> devices= QCanBus::instance()->availableDevices(plugin_names[0],&errorstring);
    if(!errorstring.isEmpty()){//плагин может не поддерживать эту функцию ^
        msg.setText(errorstring);
        msg.exec();
        //попытка
        //device->set_FLAG(CanDevice::VIRTUAL_CAN);
        goto virtual_attempt;
    }

    if(devices.size()!=0){
        QListWidgetItem* item=nullptr;
        foreach (auto element, devices) {
            if(element.isVirtual()){
                if(select_pi)
                    continue;
                else{
                    item=new QListWidgetItem(element.name()+":"+titles[1]);
                    ui->listWidget->addItem(item);
                }

            }
            else{
                item=new QListWidgetItem(element.name()+":"+titles[0]);
                ui->listWidget->addItem(item);
            }
        }
        if(ui->listWidget->count()==0){
            QListWidgetItem* item=new QListWidgetItem(physical_name+":"+titles[0]);
            ui->listWidget->addItem(item);
            ui->label_4->setText("Девайс не активен");
        }
    }
    else{
            if(select_pi){
                    QListWidgetItem* item=new QListWidgetItem(physical_name+":"+titles[0]);
                    ui->listWidget->addItem(item);
                   // device->set_FLAG(CanDevice::PHYSICAL_INACTIVE);
                    ui->label_4->setText("Девайс не активен");
            }
            else{
                    virtual_attempt:
                    QListWidgetItem* item=nullptr;
                    foreach(auto name,virtual_names){
                        item=new QListWidgetItem(name+":"+titles[1]);
                        ui->listWidget->addItem(item);
                        //device->set_FLAG(CanDevice::VIRTUAL_CAN);
                    }
            }
    }

    ui->listWidget->show();

}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString command;
    bool is_config, status;
    QString errormsg="";
    QMessageBox MSG;
    QString interface_name=item->text();
    QCanBusDevice* device=nullptr;
    QString out;
    try{
    if(interface_name.contains(titles[1])){
            if(interface_name.contains("vcan"))
            {
                    this->device->set_FLAG(CanDevice::VCAN);
                    MSG.setText("Неподдерживаемый тип устройства!");
                    MSG.exec();
                    return;
            }
            else{
                    device=QCanBus::instance()->createDevice(plugin_names[1],interface_name.remove(":"+titles[1]),&errormsg);
                    this->device->set_FLAG(CanDevice::VIRTUAL_CAN);
                    if(DEBUG)
                        device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey,true);
                    status=device->connectDevice();
            }
    }
    else if(interface_name.contains(titles[0])){
            this->device->set_FLAG(CanDevice::PHYSICAL_CAN);
            QString if_name=interface_name.remove(":"+titles[0]);
            command="ip link set can0 down";
            qDebug()<<"1";
            out=executeSudoCommand(command);
            if(out.contains("cannot find device",Qt::CaseInsensitive)){
                    MSG.setText("Проверьте наличие устройства!");
                    MSG.exec();
                    return;
            }
            qDebug()<<"2";
            device=QCanBus::instance()->createDevice(plugin_names[0],if_name,&errormsg);
            qDebug()<<"3";
            device->setConfigurationParameter(QCanBusDevice::BitRateKey,this->device->rate());
            if(DEBUG)
                device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey,true);
            command="ip link set can0 up";
            out=executeSudoCommand(command);
            status=device->connectDevice();
            // device->setConfigurationParameter(QCanBusDevice::ProtocolKey,CAN_BCM);
    }
    else{
            MSG.setText("Что-то пошло не так.Интерфейс не распознан!");
            MSG.exec();
            return;
    }
    }
    catch(const QCanBusDevice::CanBusError& error){

    }

    if(status==true){
        is_config=true;
        this->device->set_device(device);
        connect(this->device->device(),SIGNAL(framesReceived()),this->device,SLOT(read_frame()));
        connect(this->device->device(),SIGNAL(errorOccurred(QCanBusDevice::CanBusError)),this->device,SLOT(bus_error_catch(QCanBusDevice::CanBusError)));
    }
    else {
        is_config=false;
        if(device->state()==QCanBusDevice::ConnectedState)
            device->disconnectDevice();
        if(errormsg!=""){
        MSG.setText(errormsg);
        MSG.exec();
        }
    }

    if(is_config){
        ui->pushButton_2->setEnabled(true);
        ui->listWidget->setEnabled(false);
    }
}

void CanDevice::read_frame(){
    QCanBusFrame frame=current_device->readFrame();
    int64_t value=byte_array_to_int64(frame.payload());
    int64_t mask=0xFFFFFFFF;
    int32_t rnum1= (value>>32)&mask;
    int32_t rnum2= (value&mask);
    qDebug()<<"старшие байты"<<rnum1;
    qDebug()<<"младшие байты"<<rnum2;
    qDebug()<<"Получено:"<<value;
}

QString MainWindow::executeSudoCommand(const QString& command){

    if(password=="")
        ask_for_password();

    QStringList sudoArgs;
    sudoArgs<<"-k"<<"-S"<<"sh"<<"-c"<<command;
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("sudo",sudoArgs);
    process.write(password.toUtf8());
    process.closeWriteChannel();
    process.waitForFinished();
    QString output=process.readAll();
    return output;
}

void MainWindow::ask_for_password(){
    bool ok;
    QString password=QInputDialog::getText(this,"Доступ к контроллеру","Введите пароль:",QLineEdit::Password,"",&ok,Qt::Window);
    if(ok && password!=""){
        this->password=password;
        return;
    }
    else{
        QMessageBox MSG;
        MSG.setText("Доступ запрещен.Повторите попытку!");
        MSG.exec();
        this->password="";
    }
}

void MainWindow::give_required_data(){
    emit time_to_write_msg(box_1.angle,box_2.angle);
}

void MainWindow::reaction_on_unsended_msg(const QString& err_msg){
    QMessageBox MSG;
    if(err_msg!=""){
        MSG.setText(err_msg);
        MSG.exec();
    }
    timer->stop();
    ui->label->setText("Имитация остановлена!");
    ui->listWidget->setEnabled(true);

}

void CanDevice::bus_error_catch(QCanBusDevice::CanBusError error){
    QString msg_for_user="";
    bool close_app=false;
    switch(error){
    case QCanBusDevice::ReadError:
        msg_for_user="Ошибка чтения";
        break;
    case QCanBusDevice::WriteError:
        msg_for_user="Ошибка записи";
        break;
    case QCanBusDevice::OperationError:
        msg_for_user="Операция запрещена";
        break;
    case QCanBusDevice::ConfigurationError:
        msg_for_user="Ошибка конфигурации";
        close_app=true;
        break;
    case QCanBusDevice::ConnectionError:
        msg_for_user="Ошибка подключения к контроллеру";
        break;
    case QCanBusDevice::TimeoutError:
        msg_for_user="Время ожидания истекло";
        break;
    default:
        msg_for_user="Неизвестная ошибка";
        close_app=true;
        break;
    }
    if(msg_for_user!="")
        emit bus_error(msg_for_user);
    emit msg_wasnt_sent("");
    if(close_app)


}

void MainWindow::show_error(QString err){
    ui->label_4->setText(err);
}
