#include "INeuralNet.hpp"


void toHFM(image_t img){
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

		*rp++ = m / 108.900002;
		*gp++ = h / 95.050003;
		*bp++ = f / 100.000000;
	}
}

void toNorm(image_t img){
    
}