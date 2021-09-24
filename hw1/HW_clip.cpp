#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_clip:
//
// Clip intensities of image I1 to [t1,t2] range. Output is in I2.
// If    input<t1: output = t1;
// If t1<input<t2: output = input;
// If      val>t2: output = t2;
//
void
HW_clip(ImagePtr I1, int t1, int t2, ImagePtr I2)
{
    // copy image header (width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);
    
    // int vars for height, width, and total number of pixels
    int width = I1->width();
    int height = I1->height();
    int total = width * height;
    
    // init LUT
    int i, LUT[MXGRAY];
    for (i = 0; i < t1 && i < MXGRAY; i++) LUT[i] = t1;
    for ( ; i < t2; i++) LUT[i] = i;
    for (i = t2; i < MXGRAY; i++) LUT[i] = t2;
    
    // declare image channel pointer and datatype;
    ChannelPtr<uchar> p1, p2;
    int type;
    
    for (int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        for(i = 0; i < total; i++) *p2++ = LUT[*p1++];
    }
}
