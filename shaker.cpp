#include "shaker.h"

#include <iostream>
#include <mutex>

unsigned char Shaker::ON[]  = {0xFF, 0x01};
unsigned char Shaker::OFF[] = {0xFD, 0x01};

Shaker::Shaker()
{
    int error = libusb_init(nullptr);
    if (error != 0)
    {
        std::cerr  << "Could not initialize lib" << std::endl;
        return;
    }
    this->device = libusb_open_device_with_vid_pid(nullptr, VID, PID);

    if (this->device == nullptr)
    {
        std::cerr  << "Could not find device" << std::endl;
        return;
    }

    if (libusb_kernel_driver_active(this->device, 0))
    {
        error = libusb_detach_kernel_driver(this->device, 0);

        if (error != 0)
        {
            std::cerr  << "Could not detach kernal driver" << std::endl;
            return;
        }
    }

    error = libusb_claim_interface(this->device, 0);
    if (error != 0)
    {
        std::cerr  << "Could not claim interface" << std::endl;
        return;
    }
}

void Shaker::shake()
{
    this->start();
    while (true) {
        this->mutex.lock();
        bool need_to_shake = this->end > clock_t::now();
        this->mutex.unlock();

        if (!need_to_shake)
        {
            break;
        }
        std::this_thread::sleep_until(this->end);
    }
    this->stop();
}


void Shaker::shakeFor(int millisecs)
{   
    // shaker is running or ran
    if (this->thread.joinable())
    {
        this->mutex.lock()
        // end of shaking is in future - still running, increase end
        if (this->end >= clock_t::now())
        {
            this->end += std::chrono::milliseconds(millisecs);
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
    this->end = clock_t now + std::chrono::milliseconds(millisecs);
    this->thread = std::thread(&Shaker::_shakeFor, this);
    this->mutex.unlock();
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
    this->stop();
    libusb_release_interface(this->device, 0);
    libusb_close(this->device);
    libusb_exit(NULL);
}
