#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{
    // copy image header (width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);
    
    // int vars for height, width, and total number of pixels
    int width = I1->width();
    int height = I1->height();
    int total = width * height;
    int diff = t2 - t1;
    
    // init LUT
    int i, LUT[MXGRAY];
    for (i = 0; i < MXGRAY; i++) LUT[i] = CLIP((int)((MaxGray * (i - t1)) / diff), 0, MaxGray); // clip values outside of [0, 255]
    
    // declare image channel pointer and datatype;
    ChannelPtr<uchar> p1, p2;
    int type;
    
    for (int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        for(i = 0; i < total; i++) *p2++ = LUT[*p1++];
    }
}
