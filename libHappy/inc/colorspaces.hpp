#ifndef COLORSPACE_HPP
#define COLORSPACE_HPP
#include <iostream>

namespace HEBI{

namespace Colors{


struct Color{
    float values[3];

    float &operator[](int ind){
        return values[ind];
    }

    friend std::ostream &operator<<(std::ostream &out, const Color &c);
};



extern Color *xyz;
extern Color *lab;
extern Color *rgb;

void init();
void checkColors();
void findMinMax();

}

}

#endif