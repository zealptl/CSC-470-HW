#include "IP.h"
#include <math.h>
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_contrast:
//
// Apply contrast enhancement to I1. Output is in I2.
// Stretch intensity difference from reference value (128) by multiplying
// difference by "contrast" and adding it back to 128. Shift result by
// adding "brightness" value.
//
void
HW_contrast(ImagePtr I1, double brightness, double contrast, ImagePtr I2)
{
    // copy image header (width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);
    
    // init vars for height, width, and total number of pixels
    int width = I1->width();
    int height = I1->height();
    int total = width * height;
    
    // init lookup table
    int i, LUT[MXGRAY];
    for(i = 0; i < MXGRAY; i++) LUT[i] = CLIP(ceil((i - 128) * contrast) + brightness + 128, 0, MaxGray);
    
    // declarations for image channel pointers and datatype
    ChannelPtr<uchar> p1, p2;
    int type;

    // visit all image channels and evaluate output image
    for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {    // get input  pointer for channel ch
        IP_getChannel(I2, ch, p2, type);        // get output pointer for channel ch
        for(i=0; i<total; i++) *p2++ = LUT[*p1++];    // use lut[] to eval output
    }
    
    
}
