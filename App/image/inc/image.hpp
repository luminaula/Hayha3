#ifndef IMAGE_HPP
#define IMAGE_HPP

float inline lerp(float s, float e, float t){
    return s+(e-s)*t;
}

float inline blerp(float c00, float c10, float c01, float c11, float tx, float ty){
    return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

#endif