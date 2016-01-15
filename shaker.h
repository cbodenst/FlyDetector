#ifndef SHAKER_H
#define SHAKER_H

#include <chrono>
#include <libusb-1.0/libusb.h>
#include <mutex>
#include <thread>

typedef std::chrono::high_resolution_clock clock_t;
typedef std::chrono::time_point<clock_t> timepoint_t;

class Shaker
{
protected:
    /* USB on/off packages */
    static unsigned char ON[];
    static unsigned char OFF[];

    /* USB relay constants */
    static const int TIMEOUT = 5000;
    static const int MSG_LEN = 2;
    static const int VID     = 0x16c0;
    static const int PID     = 0x05df;

    /* synchronization and threading for shaker thread */
    std::mutex  mutex;
    std::thread thread;
    timepoint_t end;

    /* USB device */
    libusb_device_handle* device;

    void shake();

public:
    Shaker();

    void shakeFor(int millisecs=5000);
    void start();
    void stop();
    
    ~Shaker();
};

#endif // SHAKER_H
