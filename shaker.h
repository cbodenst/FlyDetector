#ifndef SHAKER_H
#define SHAKER_H

#include "timer.h"

class Shaker
{
public:
    virtual bool isAccessable() = 0;
    virtual void shakeFor(const Duration& shakeTime) = 0;
    virtual ~Shaker() {}
};

#endif // SHAKER_H
