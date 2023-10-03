#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H
#include <QObject>
#include <QCanBusDevice>
#include <libusb-1.0/libusb.h>

class CanDevice:public QObject{
    Q_OBJECT
private:
    const int bit_rate=250000;
    uint8_t FLAG;
    QCanBusDevice* current_device;
    libusb_device_descriptor device_descriptor;

public:
    enum Device_Type{
        VIRTUAL_CAN=0x01,
        VCAN=0x02,
        PHYSICAL_INACTIVE=0x03,
        PHYSICAL_ACTIVE=0xFF
    };
    CanDevice(uint8_t FLAG=Device_Type::VIRTUAL_CAN){
        this->FLAG=FLAG;
        current_device=nullptr;
        device_descriptor.idProduct=0;
        device_descriptor.idVendor=1;
    }
    CanDevice(uint16_t idVendor ,uint16_t idProduct,QCanBusDevice* device=nullptr,uint8_t FLAG=Device_Type::VIRTUAL_CAN){
        this->FLAG=FLAG;
        current_device=device;
        device_descriptor.idProduct=idProduct;
        device_descriptor.idVendor=idVendor;
    }
    void set_FLAG(Device_Type is_virtual){
        FLAG=is_virtual;
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
    uint8_t get_flag() const{
        return FLAG;
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
