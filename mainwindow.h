#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <sensor.h>
#include <can_interface.h>
#include <QCanBusDevice>
#include <QTimer>
#include <QListWidgetItem>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void time_to_write_msg(double,double);

private slots:
    void on_pushButton_clicked();

    void on_doubleSpinBox_textChanged(const QString &arg1);

    void on_doubleSpinBox_2_textChanged(const QString &arg1);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void give_required_data();

    void reaction_on_unsended_msg(const QString&);


private:
    Ui::MainWindow *ui;
    spinbox box_1,box_2;
    //  QCanBusDevice* current_device;
    QTimer* timer;
    CanDevice* device;
    QString physical_name="can0";
    const QString plugin_names[2]={QStringLiteral("socketcan"),QStringLiteral("virtualcan")};
    const QStringList virtual_names{"can0","can1"};
    const QStringList titles{"PHYSICAL","VIRTUAL"};
    //   const int bit_rate=250000;
    //bool VFLAG;
    QString password;

    void executeSudoCommand(const QString& command);
    void ask_for_password();

};

static QByteArray int64ToByteArray(int64_t number){
    QByteArray array;
    array.resize(sizeof(number));
    memcpy(array.data(),&number,sizeof(number));
    return array;
}

static int64_t byte_array_to_int64(QByteArray array){
    int64_t value;
    memcpy(&value,array.data(),sizeof(array.data()));
    return value;
}

#endif // MAINWINDOW_H
