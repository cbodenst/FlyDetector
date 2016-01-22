#ifndef NOSHAKER_H
#define NOSHAKER_H

#include "shaker.h"
#include "timer.h"

class NoShaker : public Shaker
{
public:
    NoShaker();

    virtual bool isAccessable();
    virtual void shakeFor(const Duration&);

    virtual ~NoShaker();
};

#endif // NOSHAKER_H
