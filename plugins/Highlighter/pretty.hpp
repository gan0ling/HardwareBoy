// https://github.com/jibstack64/pretty
// a simple, lightweight, unix console-colouring library for C++, all in one header file!

#ifndef PRETTY_HPP
#define PRETTY_HPP

// #include <iostream>
// #include <sstream>
// #include <vector>
// #include <map>
#include <CtrlLib/CtrlLib.h>
using namespace Upp;

namespace pretty {

    // holds stylings for later use through the 'apply' function or paint(ColourSet)
    class ColourSet;

    // dims value
    template<typename T>
    const String dim(T value);

    // brightens value
    template<typename T>
    const String bright(T value);

    // removes all escape sequences from value
    template<typename T>
    const String normal(T value);

    // paints the given value with the the fore/back/style names provided
    template<typename T>
    const String paint(T& value, Vector<int>& cns);
    //
    // template<typename T>
    // const String paint(T value, std::initializer_list<const char *> cns);
    //
    // template<typename T>
    // const String paint(T value, const char * cn);
    //
    template<typename T>
    const String paint(T& value, const String cn);
    //
    // template<typename T>
    // const String paint(T value, ColourSet& cs);

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

    const char * TEMPLATE = "\x1B[%dm";
    const char * RESET = "\033[0m";

    class ColourSet {
        private:
            const std::vector<String>_styles;

        public:
            ColourSet(std::initializer_list<String> cns);

            // applies stored stylings to value
            template<typename T>
            const String apply(T value);
    };

    ColourSet::ColourSet(std::initializer_list<String> cns) : _styles(cns) {}

    template<typename T>
    const String ColourSet::apply(T value) {
        return paint(value, this->_styles);
    }

    template<typename T>
    const String dim(T value) {
        return paint(value, "dim");
    }

    template<typename T>
    const String bright(T value) {
        return paint(value, "bold");
    }
    
    template<typename T>
    const String normal(T value) {
        // std::ostringstream oss;
        String oss;
        bool zon = false;
        for (int i = 0; i < value.size(); i++) {
            if (value[i] == '\x1B') {
                zon = true;
            }
            if (zon) {
                if (value[i] == 'm') {
                    zon = false;
                }
                continue;
            } else {
                oss << value[i];
            }
        }
        return oss;
    }

    template<typename T>
    const String paint(T& value, Vector<int>& cns) {
        // std::ostringstream oss;
        String oss;
        for (const auto& cn : cns) {
            char c[15];
            sprintf(c, TEMPLATE, cn);
            oss << c;
        }
        oss << value << RESET;
        return oss;
    }
    //
    template<typename T>
    const String paint(T& value, std::initializer_list<int> cns) {
        Vector<int> s;
        for (auto& cn : cns) {
            s.push_back(cn);
        }
        return paint(value, s);
    }
    //
    // template<typename T>
    // const String paint(T value, const char * cn) {
        // return paint(value, {cn});
    // }
    //
    template<typename T>
    const String paint(T& value, const String cn) {
        return paint(value, {colours.at(cn)});
    }
    //
    template<typename T>
    const String paint(T value, ColourSet& cs) {
        return cs.apply(value);
    }
}

#endif
