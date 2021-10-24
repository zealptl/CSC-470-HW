#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//

void
blur1D(ChannelPtr<uchar> src, int len, int stride, int ww, ChannelPtr<uchar> dest);

void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
    // copy image header (width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);
    
    // copy image header to an intermediate buffer
    ImagePtr I3;
    IP_copyImageHeader(I1, I3);

    // init vars for width, height, and total number of pixels
    int w = I1->width ();
    int h = I1->height();
    int total = w * h;
    
    // declare channel ptrs
    int type;
    ChannelPtr<uchar> p1, p2, p3, end;
    
    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        IP_getChannel(I3, ch, p3, type);
        
        if (filterW == 1) for (end = p1 + total; p1 < end;) *p3++ = *p1++;
        if (filterW > 1) {
            // blur rows one by one
            for (int y = 0; y < h; y++) {
                blur1D(p1, w, 1, filterW, p3);
                p1 += w;
                p3 += w;
            }
        }
        
        p3 -= total;
        
        if (filterH == 1) for (end = p3 + total; p3 < end;) *p2++ = *p3++;
        if (filterH > 1) {
            for (int x = 0; x < w; x++) {
                // blur columns one by one
                blur1D(p3, h, w, filterH, p2);
                p3 += 1;
                p2 += 1;
            }
        }
    }
    
    
}

void
blur1D(ChannelPtr<uchar> src, int len, int stride, int ww, ChannelPtr<uchar> dest) {
    if (ww % 2 == 0) ww++; // ensure ww is always odd by adding 1 if ww is even
    int padSize = ww / 2; // size of padding left and right of pixel
    int buffSize = len + ww - 1; // size of padded buffer
    short *buff = new short[buffSize];
    
    // copy to padded pixels to buffer
    for (int i = 0; i < len + padSize; i++) buff[i] = *src;
    int index = 0;
    for (int i = padSize; i < len + padSize; i++) {
        buff[i] = src[index];
        index += stride;
    }
    
    for (int i = len + padSize; i < buffSize; i++) buff[i] = src[index - stride];
    
    unsigned short sum = 0;
    for (int i = 0; i < ww; i++) sum += buff[i];
    for (int i = 0; i < len; i++) {
        dest[i*stride] = sum / ww;
        sum += (buff[i + ww] - buff[i]);
    }
    
    delete [] buff;
}
