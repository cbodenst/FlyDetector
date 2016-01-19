#ifndef SETTINGS_H
#define SETTINGS_H

#include <chrono>
#include <string>

struct Settings
{
    static const std::string OUTPUT_PATH = "results";
    static const std::string OUTPUT_FILE = "results.csv";
    static const bool        SAVE_IMAGES = true;
    static const std::string TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

    static const int EPSILON             = 5;
    static const int MIN_POINTS          = 50;
    static const int PIXELS_PER_FLY      = 300;
    static const int THRESHOLD           = 100;
    static const int VEILS               = 20;

    static const Duration ROUND_TIME     = Seconds(60);
    static const Duration SHAKE_LEAD     = Seconds(10);
    static const Duration SHAKE_TIME     = Seconds(5);
};

#endif // SETTINGS_H
