#include "pretty.hpp"

using namespace Upp;

namespace pretty {
std::map<String, const int> colours {
    // foreground [0-14]
    {"black", 30}, {"red", 31}, {"green", 32},
    {"orange", 33}, {"blue", 34}, {"magenta", 35},
    {"cyan", 36}, {"white", 37}, {"grey", 90},
    {"lightred", 91}, {"lightgreen", 92}, {"yellow", 93},
    {"lightblue", 94}, {"lightmagenta", 95}, {"turqoise", 96},
    // background [15-29]
    {"blackbg", 40}, {"redbg", 41}, {"greenbg", 42},
    {"orangebg", 43}, {"bluebg", 44}, {"purplebg", 45},
    {"cyanbg", 46}, {"whitebg", 47}, {"greybg", 100},
    {"lightredbg", 101}, {"lightgreenbg", 102}, {"yellowbg", 103},
    {"lightbluebg", 104}, {"lightpurplebg", 105}, {"turquoisebg", 106},
    // style [30-35]
    {"normal", 0}, {"bold", 1}, {"dim", 2},
    {"italic", 3}, {"underlined", 4}, {"reversefield", 7},
    //{"riverced", 26}, {"framed", 51}, {"flashing", 5}
};

int MapColor(String color)
{
  return colours.at(color);
}


};
