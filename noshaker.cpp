#include "noshaker.h"

NoShaker::NoShaker()
{}

bool NoShaker::isAccessable()
{
    return true;
}

void NoShaker::shakeFor(const Duration&)
{}

NoShaker::~NoShaker()
{}
