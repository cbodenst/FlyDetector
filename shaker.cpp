#include "shaker.h"

#include <iostream>
#include <unistd.h>

unsigned char Shaker::ON[] = {0xFF, 0x01};
unsigned char Shaker::OFF[] = {0xFD, 0x01};

Shaker::Shaker()
{
    int error = libusb_init(NULL);
    if (error != 0)
    {
        std::cerr  << "Could not initialize lib" << std::endl;
        return;
    }

    this->device = libusb_open_device_with_vid_pid(NULL,VID,PID);

    if (this->device == NULL)
    {
        std::cerr  << "Could not find device" << std::endl;
        return;
    }
    if (libusb_kernel_driver_active(this->device,0))
    {
        error = libusb_detach_kernel_driver(this->device,0);

        if (error != 0)
        {
            std::cerr  << "Could not detach kernal driver" << std::endl;
            return;
        }
    }
    error = libusb_claim_interface(this->device,0);

    if (error != 0)
    {
        std::cerr  << "Could not claim interface" << std::endl;
        return;
    }

}


void Shaker::_shakeFor(int millisecs)
{
    this->start();
    usleep(millisecs * 1000);
    this->stop();
}


void Shaker::shakeFor(int millisecs)
{   if(this->t.joinable())
        this->t.join();
    this->t = std::thread(&Shaker::_shakeFor, this, millisecs);
}

void Shaker::start()
{
     int written;

     written = libusb_control_transfer(device, LIBUSB_REQUEST_TYPE_CLASS, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_REQUEST_SET_FEATURE, 0, Shaker::ON, Shaker::MSG_LEN, Shaker::TIMEOUT);

     if (written != Shaker::MSG_LEN)
     {
         std::cerr  << "Could not write data" << std::endl;
     }
}


void Shaker::stop()
{
     int written;

     written = libusb_control_transfer(device, LIBUSB_REQUEST_TYPE_CLASS, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_REQUEST_SET_FEATURE, 0, Shaker::OFF, Shaker::MSG_LEN, Shaker::TIMEOUT);

     if (written != Shaker::MSG_LEN)
     {
         std::cerr  << "Could not write data" << std::endl;
     }
}

Shaker::~Shaker()
{

    libusb_release_interface(this->device, 0);
    libusb_close(this->device);
    libusb_exit(NULL);
}
