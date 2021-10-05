#include "IP.h"
#include <math.h>
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_gammaCorrect:
//
// Gamma correct image I1. Output is in I2.
//
void
HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{
    // copy image header ( width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);
    
    // init vars for height, width, and toal number of pixels
    int width = I1->width();
    int height = I1->height();
    int total = width * height;
    
    // int LUT
    int i, LUT[MXGRAY];
    for (i = 0; i < MXGRAY; i++) LUT[i] = MaxGray * pow((double) i/MaxGray, 1 / gamma);
    
    // declare image channel pointer and datatype
    ChannelPtr<uchar> p1, p2;
    int type;
    
    for (int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        for(i = 0; i < total; i++) *p2++ = LUT[*p1++];
    }
}
