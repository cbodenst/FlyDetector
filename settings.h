#ifndef SETTINGS_H
#define SETTINGS_H

#include <chrono>
#include <string>

struct SETTINGS
{
    static const std::string OUTPUT_PATH = "results";
    static const std::string OUTPUT_FILE = "results.csv";
    static const bool        SAVE_IMAGES = true;

    static const int VEILS           = 20;
    static const int MEASURMENT_TIME = 60; // seconds
    static const int EPSILON         = 5;
    static const int MIN_POINTS      = 50;
    static const int PIXELS_PER_FLY  = 300;
    static const int THRESHOLD       = 100;

    static const Duration ROUND_TIME = Seconds(15);
    static const Duration SHAKE_LEAD = Seconds(10);
    static const Duration SHAKE_TIME = Seconds(5);
};

#endif // SETTINGS_H
