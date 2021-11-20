#include "colorspaces.hpp"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

namespace HEBI{

namespace Colors{


std::ostream &operator<<(std::ostream &out, const Color &c){
    out << c.values[0] << " " << c.values[1] << " " << c.values[2];
}

Color *xyz;
Color *lab;
Color *rgb;

bool colorsInited = false;

float clamp(float val){
    return MAX(MIN(1.0,val),0.0);
}

void clampColors(Color *col){
    for(int i=0;i<0xFFFFFF;i++){
        for(int c=0;c<3;c++){
            col[i][c] = clamp(col[i][c]);
        }
    }
}

void init(){
    if(colorsInited)
        return;
    xyz = (Color*)calloc(0xFFFFFF,sizeof(Color));
    lab = (Color*)calloc(0xFFFFFF,sizeof(Color));
    rgb = (Color*)calloc(0xFFFFFF,sizeof(Color));

    for(int i=0;i<0xFFFFFF;i++){
        float r,g,b,x,y,z,l,a,bb;
        b = (float)(i & 0xFF);
        g = (float)((i >> 8) & 0xFF);
        r = (float)((i >> 16) & 0xFF);

        rgb[i].values[0] = r / 255.0;
        rgb[i].values[1] = g / 255.0;
        rgb[i].values[2] = b / 255.0;

        float var_R = r / 255.0;
        float var_G = g / 255.0;
        float var_B = b / 255.0;


        if (var_R > 0.04045) var_R = powf(((var_R + 0.055) / 1.055), 2.4);
        else var_R = var_R / 12.92;
        if (var_G > 0.04045) var_G = powf(((var_G + 0.055) / 1.055), 2.4);
        else var_G = var_G / 12.92;
        if (var_B > 0.04045) var_B = powf(((var_B + 0.055) / 1.055), 2.4);
        else var_B = var_B / 12.92;

        var_R = var_R * 100.;
        var_G = var_G * 100.;
        var_B = var_B * 100.;

        x = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
        y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
        z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

        float var_X = x / 94.8894;         //ref_X =  95.047   Observer= 2°, Illuminant= D65
        float var_Y = y / 99.9358;          //ref_Y = 100.000
        float var_Z = z / 108.883;

        xyz[i].values[0] = var_X;
        xyz[i].values[1] = var_Y;
        xyz[i].values[2] = var_Z;


        if (var_X > 0.008856) var_X = pow(var_X, (1. / 3.));
        else var_X = (7.787 * var_X) + (16. / 116.);
        if (var_Y > 0.008856) var_Y = pow(var_Y, (1. / 3.));
        else var_Y = (7.787 * var_Y) + (16. / 116.);
        if (var_Z > 0.008856) var_Z = pow(var_Z, (1. / 3.));
        else var_Z = (7.787 * var_Z) + (16. / 116.);


        l = (116. * var_Y) - 16.;
        a = 500. * (var_X - var_Y);
        bb = 200. * (var_Y - var_Z);


        

        l /= 100.0;
        a = (a + 86.184624)/184,438858;
        bb = (bb + 107.86386)/202.346343;

        lab[i].values[0] = l;
        lab[i].values[1] = a;
        lab[i].values[2] = bb;


    }

    clampColors(rgb);
    clampColors(xyz);
    clampColors(lab);
    
    colorsInited = true;

}

bool checkColor(Color c){
    for(int i=0;i<3;i++){
        if(c.values[i] < 0.0 | c.values[i] > 1.0)
            return false;
    }
    return true;
}

void checkColorSpace(Color *c, std::string name){
    for(int i=0;i<0xFFFFFF;i++){
        if(!checkColor(c[i])){
            printf("%s color Error %x  %f %f %f \n",name.c_str(),i,c[i].values[0],c[i].values[1],c[i].values[2]);
        }
    }
}

void checkColors(){
    checkColorSpace(rgb,"rgb");
    checkColorSpace(xyz,"xyz");
    checkColorSpace(lab,"lab");
}



void minMax(Color *col){
    Color min = {{1.0,1.0,1.0}};
    Color max = {{0.0,0.0,0.0}};
    int minind,maxind;

    for(int i=0;i<0xFFFFFF;i++){
        for(int c=0;c<3;c++){
            if(col[i][c] > max[c]){
                max[c] = col[i][c];
                maxind = i;
            }
            if(col[i][c] < min[c]){
                min[c] = col[i][c];
                minind = i;
            }
        }
    }

    std::cout << min << " " << max << std::endl;



}

void findMinMax(){
    std::cout << "rgb" << std::endl;
    minMax(rgb);
    std::cout << "xyz" << std::endl;
    minMax(xyz);
    std::cout << "lab" << std::endl;
    minMax(lab);
}


}

}