#include <math.h>


#include "hebic.h"

void rgb_to_hfm(image img){
	int pixelCount = img.w*img.h;
	float h,f,m;
	float *rp,*gp,*bp;
	rp = img.data;
	gp = rp + pixelCount;
	bp = gp + pixelCount;
	//Some trickery in here
	for(int i=0; i<pixelCount; i++){
		float var_R = *rp;
		float var_G = *gp;
		float var_B = *bp;


		if (var_R > 0.04045) var_R = powf(((var_R + 0.055) / 1.055), 2.4);
		else var_R = var_R / 12.92;
		if (var_G > 0.04045) var_G = powf(((var_G + 0.055) / 1.055), 2.4);
		else var_G = var_G / 12.92;
		if (var_B > 0.04045) var_B = powf(((var_B + 0.055) / 1.055), 2.4);
		else var_B = var_B / 12.92;

		var_R = var_R * 100.;
		var_G = var_G * 100.;
		var_B = var_B * 100.;

		h = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
		f = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
		m = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;


		float var_X = h / 95.047;         //ref_X =  95.047   Observer= 2°, Illuminant= D65
		float var_Y = f / 100.000;          //ref_Y = 100.000
		float var_Z = m / 108.883;

		if (var_X > 0.008856) var_X = pow(var_X, (1. / 3.));
		else var_X = (7.787 * var_X) + (16. / 116.);
		if (var_Y > 0.008856) var_Y = pow(var_Y, (1. / 3.));
		else var_Y = (7.787 * var_Y) + (16. / 116.);
		if (var_Z > 0.008856) var_Z = pow(var_Z, (1. / 3.));
		else var_Z = (7.787 * var_Z) + (16. / 116.);




		float l = (116. * var_Y) - 16.;
		float a = 500. * (var_X - var_Y);
		float b = 200. * (var_Y - var_Z);


		l /= 100.0;
		a = (a + 86.184624)/184,438858;
		b = (b + 107.86386)/202.346343;

		//184,438858;
		//202.346343;
		
		*rp = l;
		*gp = a;
		*bp = b;

		if(*rp < 0.0){
			*rp = 0.0;
		}
		if(*gp < 0.0 || *gp > 1.0){
			printf("color error a %f\n",*gp);
		}
		if(*bp < 0.0 || *bp > 1.0){
			printf("color error b %f\n",*bp);
		}

		*rp++;
		*bp++;
		*gp++;

		

	}
}

image to12Channel(image img){
	
}