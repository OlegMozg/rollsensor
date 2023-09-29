#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H
#include <QObject>
#include <QCanBusDevice>
#include <libusb-1.0/libusb.h>

class CanDevice:public QObject{
    Q_OBJECT
private:
    const int bit_rate=250000;
    bool VFLAG;
    QCanBusDevice* current_device;
    libusb_device_descriptor device_descriptor;

public:
    CanDevice( uint16_t idVendor ,uint16_t idProduct,bool VFLAG=false,QCanBusDevice* device=nullptr){
        this->VFLAG=VFLAG;
        current_device=device;
        device_descriptor.idProduct=idProduct;
        device_descriptor.idVendor=idVendor;
    }
    void set_VFLAG(bool is_virtual){
        VFLAG=is_virtual;
    }
    void set_device(QCanBusDevice* device){
        current_device=device;
    }
    QCanBusDevice* device() const{
        return current_device;
    }
    uint16_t vendor() const{
        return device_descriptor.idVendor;
    }
    uint16_t product() const{
        return device_descriptor.idProduct;
    }
    bool get_flag() const{
        return VFLAG;
    }
    int rate() const{
        return bit_rate;
    }
public :
signals:
    void msg_wasnt_sent(QString err_msg);
private slots:
    void BCM_send(double,double);
    void read_frame();
};

#endif // CAN_INTERFACE_H
