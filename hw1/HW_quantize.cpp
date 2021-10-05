#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
    // copy image header (width, height) of input I1 to output image I2
    IP_copyImageHeader(I1, I2);
    
    // init vars for width, height, and total numbe of pixels
    int width = I1->width();
    int height = I2->height();
    int total = width * height;
    
    srand (time(NULL));
    
    // init LUT & scale
    int scale = MXGRAY / levels;
    int bias = scale / 2;
    int i, LUT[MXGRAY];
    for (i = 0; i < MXGRAY; i++) LUT[i] = (scale * (int) (i / scale)) + bias;
    
    // declare image channel pointer and datatype
    ChannelPtr<uchar> p1, p2;
    int type;
    
    
    if (!dither) {
        for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            for (i = 0; i < total; i++) *p2++ = LUT[*p1++];
        }
    } else {
        for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            
            for (int y = 0; y < height; y++) {
                int sign = y % 2 == 0 ? 1 : -1;
                
                for (int x = 0; x < width; x++) {
                    int jitter = ((double) rand() / RAND_MAX) * bias;
                    int modified = *p1++ + (sign * jitter);
                    *p2++ = LUT[CLIP(modified, 0, MaxGray)]; // clip values outside of [0, 255] before checking in LUT
                    
                    sign *= -1;
                }
            }
        }
    }
}
