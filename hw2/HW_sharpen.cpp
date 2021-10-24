#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//

extern void HW_blur (ImagePtr, int, int, ImagePtr);
void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
    // copy image header (width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);

    // init vars for width, height, and total number of pixels
    int w = I1->width ();
    int h = I1->height();
    int total = w * h;
    
    // declare channel ptrs
    int type;
    ChannelPtr<uchar> p1, p2, end;
    
    if (size == 1) {
        for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            for (end = p1 + total; p1 < end; ) *p2++ = *p1++;
        }
    } else {
        HW_blur(I1, size, size, I2);
        for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            for (end = p1 + total; p1 < end; ) {
                *p2 = CLIP(*p1 + (*p1 - *p2) * factor, 0, MaxGray);
                *p1++;
                *p2++;
            }
        }
    }
    
}
