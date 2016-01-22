#include "usbshaker.h"

#include <iostream>
#include <mutex>

unsigned char USBShaker::ON[]  = {0xFF, 0x01};
unsigned char USBShaker::OFF[] = {0xFD, 0x01};

/** private **/

void USBShaker::shake()
{
    // start shaking, wait till end is reached, stop shaking
    this->start();
    while (true) {
        this->mutex.lock();
        // check whether the shaking deadline has been extended
        bool needToShake = this->end > Clock::now();
        this->mutex.unlock();

        if (!needToShake)
        {
            break;
        }
        std::this_thread::sleep_until(this->end);
    }
    this->stop();
}

void USBShaker::start()
{
     int written;

     written = libusb_control_transfer(device, LIBUSB_REQUEST_TYPE_CLASS, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_REQUEST_SET_FEATURE, 0, USBShaker::ON, USBShaker::MSG_LEN, USBShaker::TIMEOUT);
     if (written != USBShaker::MSG_LEN)
     {
         std::cerr  << "Could not write data" << std::endl;
     }
}


void USBShaker::stop()
{
     int written;

     written = libusb_control_transfer(device, LIBUSB_REQUEST_TYPE_CLASS, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_REQUEST_SET_FEATURE, 0, USBShaker::OFF, USBShaker::MSG_LEN, USBShaker::TIMEOUT);
     if (written != USBShaker::MSG_LEN)
     {
         std::cerr  << "Could not write data" << std::endl;
     }
}

/** public **/

USBShaker::USBShaker()
{
    // initialize libusb
    int error = libusb_init(nullptr);
    if (error != 0)
    {
        std::cerr  << "Could not initialize lib" << std::endl;
        return;
    }

    // retrieve the device by vendor and product ID (fixed for USB relays)
    this->device = libusb_open_device_with_vid_pid(nullptr, VID, PID);
    if (this->device == nullptr)
    {
        std::cerr  << "Could not find device" << std::endl;
        return;
    }

    // detach the kernel USB driver/mount if needed
    if (libusb_kernel_driver_active(this->device, 0))
    {
        error = libusb_detach_kernel_driver(this->device, 0);
        if (error != 0)
        {
            std::cerr  << "Could not detach kernal driver" << std::endl;
            return;
        }
    }

    // claim the USB relay for us
    error = libusb_claim_interface(this->device, 0);
    if (error != 0)
    {
        std::cerr  << "Could not claim interface" << std::endl;
        return;
    }
}

bool USBShaker::isAccessable()
{
    return this->device != nullptr;
}

void USBShaker::shakeFor(const Duration& shakeTime)
{   
    // shaker is running or ran
    if (this->thread.joinable())
    {
        this->mutex.lock();
        // end of shaking is in future - still running, increase end
        if (this->end >= Clock::now())
        {
            this->end += shakeTime;
            this->mutex.unlock();
            return;
        }
        // shaker ran is in the past, join old thread and create a new one
        else
        {
            this->thread.join();
        }
    }
    // SHAKE IT BABY!
    this->end = Clock::now() + shakeTime;
    this->thread = std::thread(&USBShaker::shake, this);
    this->mutex.unlock();
}

USBShaker::~USBShaker()
{
    if (this->device)
    {
        this->stop();
        libusb_release_interface(this->device, 0);
        libusb_close(this->device);
    }
    libusb_exit(nullptr);
}
