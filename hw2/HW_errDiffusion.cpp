#include "IP.h"
#include "math.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//

void gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2);
void copyRowToBuff(ChannelPtr<uchar> p1, short *buff, int w, int size);

void errDiffusionFS(ImagePtr I1, double gamma, bool serpentine, ImagePtr I2);
void errDiffusionJJN(ImagePtr I1, double gamma, bool serpentine, ImagePtr I2);

void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{
    if (method == 0) {
        errDiffusionFS(I1, gamma, serpentine, I2);
    } else if (method == 1) {
        errDiffusionJJN(I1, gamma, serpentine, I2);
    }
}

void
gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2) {
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

void
copyRowToBuff(ChannelPtr<uchar> p1, short *buff, int w, int size) {
    int buffSize = size + w - 1;
    for (int i = 0; i < size / 2; i++) buff[i] = *p1;
    for (int i = size / 2; i < size / 2 + w - 1; i++) buff[i] = *p1++;
    for (int i = size /2 + w - 1; i < buffSize; i++) buff[i] = *p1;
}

// Floyd-Steinberg error diffusion
void
errDiffusionFS(ImagePtr I1, double gamma, bool serpentine, ImagePtr I2) {
    ImagePtr Ig;
    IP_copyImageHeader(I1, I2);
    IP_copyImageHeader(I1, Ig);
    gammaCorrect(I1, gamma, Ig);
    
    int w = I1->width();
    int h = I1->height();
    ChannelPtr<uchar> p1, p2, end;
    int type;

    int thr = MXGRAY / 2;
    int kernelSize = 3;
    
    short *in1;
    short *in2;
    short err;
    
    int buffSize = w + 2;
    short *buff0 = new short[buffSize]; // top buff
    short *buff1 = new short[buffSize]; // bottom buff
    
    for (int ch = 0; IP_getChannel(Ig, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        
        // copy first row to top buff
        copyRowToBuff(p1, buff0, w, kernelSize);
        p1 += w;
        
        for (int y = 1; y < h; y++) {
            if (serpentine) {
                // even rows
                if (y % 2 == 0) {
                    copyRowToBuff(p1, buff0, w, kernelSize);
                    p1 += w;
                    in1 = buff1 + w + 1;
                    in2 = buff0 + w + 1;
                    
                    p2 += (w - 1);
                    
                    for (int x = 0; x < w; x++) {
                        *p2 = (*in1 < thr) ? 0 : MaxGray;
                        err = *in1 - *p2;
                        
                        *(in1 - 1) += (err * 7 / 16.);
                        *(in2 - 1) += (err * 3 / 16.);
                        *(in2) += (err * 5 / 16.);
                        *(in2 - 1) += (err * 1 / 16.);
                        
                        in1--;
                        in2--;
                        p2--;
                    }
                    p2 += (w + 1);
                } else { // odd rows
                    copyRowToBuff(p1, buff1, w, kernelSize);
                    p1 += w;
                    in1 = buff0 + 1;
                    in2 = buff1 + 1;
                    
                    for (int x = 0; x < w; x++) {
                        *p2 = (*in1 < thr) ? 0 : MaxGray;
                        err = *in1 - *p2;
                        
                        *(in1 + 1) += (err * 7 / 16.0);
                        *(in2 - 1) += (err * 3 / 16.0);
                        *(in2) += (err * 5 / 16.0);
                        *(in2 + 1) += (err * 1 / 16.0);
                        
                        in1++;
                        in2++;
                        p2++;
                    }
                }
            }
            else { // raster scan
                if (y % 2 == 0) {
                    copyRowToBuff(p1, buff0, w, kernelSize);
                    in1 = buff1 + 1;
                    in2 = buff0 + 1;
                } else {
                    copyRowToBuff(p1, buff1, w, kernelSize);
                    in1 = buff0 + 1;
                    in2 = buff1 + 1;
                }
                p1 += w;
                
                for (int x = 0; x < w; x++) {
                    *p2 = (*in1 < thr) ? 0 : MaxGray;
                    err = *in1 - *p2;
                    
                    *(in1 + 1) += (err * 7 / 16.0);
                    *(in2 - 1) += (err * 3 / 16.0);
                    *(in2) += (err * 5/ 16.0);
                    *(in2 + 1) += (err * 1 / 16.0);
                    
                    in1++;
                    in2++;
                    p2++;
                }
            }
        }
    }
    
    delete [] buff0;
    delete [] buff1;
}

// Jarvis-Juice-Ninke error diffusion
void
errDiffusionJJN(ImagePtr I1, double gamma, bool serpentine, ImagePtr I2) {
    ImagePtr Ig;
    IP_copyImageHeader(I1, I2);
    IP_copyImageHeader(I1, Ig);
    gammaCorrect(I1, gamma, Ig);
    
    int w = I1->width();
    int h = I1->height();
    ChannelPtr<uchar> p1, p2, end;
    int type;

    int thr = MXGRAY / 2;
    int kernelSize = 5;
    
    short **in = new short *[3];
    short err;
    
    int buffSize = w + 4;
    short **buffers = new short *[3];
    for (int i = 0; i < 3; i++) buffers[i] = new short[buffSize];
    
    for (int ch = 0; IP_getChannel(Ig, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        
        copyRowToBuff(p1, buffers[0], w, kernelSize);
        copyRowToBuff(p1, buffers[1], w, kernelSize);
        p1 += w;
        
        for (int y = 2; y < h; y++) {
            if (y % 3 == 0) copyRowToBuff(p1, buffers[0], w, kernelSize);
            else if (y % 3 == 1) copyRowToBuff(p1, buffers[1], w, kernelSize);
            else if (y % 3 == 2) copyRowToBuff(p1, buffers[2], w, kernelSize);
            p1 += w;
            
            if (serpentine) {
                if (y % 2 == 0) {
                    if (y % 3 == 0) {
                        in[0] = buffers[1] + w + 2;
                        in[1] = buffers[2] + w + 2;
                        in[2] = buffers[0] + w + 2;
                    } else if (y % 3 == 1) {
                        in[0] = buffers[2] + w + 2;
                        in[1] = buffers[0] + w + 2;
                        in[2] = buffers[1] + w + 2;
                    } else {
                        in[0] = buffers[0] + w + 2;
                        in[1] = buffers[1] + w + 2;
                        in[2] = buffers[2] + w + 2;
                    }
                    p2 += (w - 1);
                    
                    for (int x = 0; x < w; x++) {
                        *p2 = (*in[0] < thr) ? 0 : MaxGray;
                        err = *in[0] - *p2;
                        
                        *(in[0] - 1) += (err * 7 / 48.);
                        *(in[0] - 2) += (err * 5 / 48.);
                        *(in[1]) += (err * 7 / 48.);
                        *(in[1] + 1) += (err * 5 / 48.);
                        *(in[1] + 2) += (err * 3 / 48.);
                        *(in[1] - 1) += (err * 5 / 48.);
                        *(in[1] - 2) += (err * 3 / 48.);
                        *(in[2]) += (err * 5 / 48.);
                        *(in[2] + 1) += (err * 3 / 48.);
                        *(in[2] + 2) += (err * 1 / 48.);
                        *(in[2] - 1) += (err * 3 / 48.);
                        *(in[2] - 2) += (err * 1 / 48.);
                        
                        in[0]--;
                        in[1]--;
                        in[2]--;
                        p2--;
                    }
                    p2 += (w + 1);
                } else {
                    if (y % 3 == 0) {
                        in[0] = buffers[1] + 2;
                        in[1] = buffers[2] + 2;
                        in[2] = buffers[0] + 2;

                    } else if (y % 3 == 1) {
                        in[0] = buffers[2] + 2;
                        in[1] = buffers[0] + 2;
                        in[2] = buffers[1] + 2;

                    } else {
                        in[0] = buffers[0] + 2;
                        in[1] = buffers[1] + 2;
                        in[2] = buffers[2] + 2;
                    }
                    
                    for (int x = 0; x < w; x++) {
                        *p2 = (*in[0] < thr) ? 0 : MaxGray;
                        err = *in[0] - *p2;
                        
                        *(in[0] + 1) += (err * 7 / 48.);
                        *(in[0] + 2) += (err * 5 / 48.);
                        *(in[1]) += (err * 7 / 48.);
                        *(in[1] + 1) += (err * 5 / 48.);
                        *(in[1] + 2) += (err * 3 / 48.);
                        *(in[1] - 1) += (err * 5 / 48.);
                        *(in[1] - 2) += (err * 3 / 48.);
                        *(in[2]) += (err * 5 / 48.);
                        *(in[2] + 1) += (err * 3 / 48.);
                        *(in[2] + 2) += (err * 1 / 48.);
                        *(in[2] - 1) += (err * 3 / 48.);
                        *(in[2] - 2) += (err * 1 / 48.);
                        
                        in[0]++;
                        in[1]++;
                        in[2]++;
                        p2++;
                    }
                }
            }
            else {
                if (y % 3 == 0) {
                    in[0] = buffers[1] + 2;
                    in[1] = buffers[2] + 2;
                    in[2] = buffers[0] + 2;

                } else if(y % 3 == 1) {
                    in[0] = buffers[2] + 2;
                    in[1] = buffers[0] + 2;
                    in[2] = buffers[1] + 2;

                } else {
                    in[0] = buffers[0] + 2;
                    in[1] = buffers[1] + 2;
                    in[2] = buffers[2] + 2;

                }
                
                for (int x = 0; x < w; x++) {
                    *p2 = (*in[0] < thr) ? 0 : MaxGray;
                    err = *in[0] - *p2;
                    
                    *(in[0] + 1) += (err * 7 / 48.);
                    *(in[0] + 2) += (err * 5 / 48.);
                    *(in[1]) += (err * 7 / 48.);
                    *(in[1] + 1) += (err * 5 / 48.);
                    *(in[1] + 2) += (err * 3 / 48.);
                    *(in[1] - 1) += (err * 5 / 48.);
                    *(in[1] - 2) += (err * 3 / 48.);
                    *(in[2]) += (err * 5 / 48.);
                    *(in[2] + 1) += (err * 3 / 48.);
                    *(in[2] + 2) += (err * 1 / 48.);
                    *(in[2] - 1) += (err * 3 / 48.);
                    *(in[2] - 2) += (err * 1 / 48.);
                    
                    in[0]++;
                    in[1]++;
                    in[2]++;
                    p2++;
                }
            }
        }
    }
    
    delete [] buffers;
    delete [] in;
}
