#ifndef SHAKER_H
#define SHAKER_H


#include <libusb-1.0/libusb.h>
#include <thread>
class Shaker
{
protected:
    static unsigned char ON[];
    static unsigned char OFF[];
    static const int TIMEOUT = 5000;
    static const int MSG_LEN = 2;
    static const int VID = 0x16c0;
    static const int PID = 0x05df;

    std::thread t;
    libusb_device_handle* device;
    void _shakeFor(int millisecs);
public:
    Shaker();
    ~Shaker();

    void shakeFor(int millisecs=5000);
    void start();
    void stop();

};

#endif // SHAKER_H
