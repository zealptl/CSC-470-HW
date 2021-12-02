#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap the phase channels of I1 and I2.
// Output is in II1 and II2.
//

struct complexP {
    int len;        // length of complex number list
    float * real;    // pointer to real number list
    float * imag;    // pointer to imaginary number list
};

extern void fft1D(complexP *q1, int dir, complexP *q2);
extern void fft1DRow(ImagePtr I1, ImagePtr Image1);
extern void fft1DColumn(ImagePtr I1, ImagePtr Image1, ImagePtr Image2);
extern void fft1DMagPhase(ImagePtr I1, ImagePtr Image2, float *Magnitude, float *Phase);

void swapPhase(ImagePtr I, float * magnitude ,float * newPhase, int total); //Puts new phase on image I
void Ifft1DRow(ImagePtr I, ImagePtr IFFT, ImagePtr IFFTout);
void Ifft1DColumn(ImagePtr I, ImagePtr rowI, ImagePtr IFFTout, ImagePtr resultPtr);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap phase of I1 with I2.
// (I1_mag, I2_phase) -> II1
// (I1_phase, I2_mag) -> II2
//
void
HW_swapPhase(ImagePtr I1, ImagePtr I2, ImagePtr II1, ImagePtr II2)
{
    // init vars for width, height, and total number of pixels for I1
    int w1 = I1->width();
    int h1 = I1->height();
    int total1 = w1 * h1;

    // init vars for width, height, and total number of pixels for I2
    int w2 = I2->width();
    int h2 = I2->height();
    int total2 = w2 * h2;

    // check if both image have same dimensions
    if(w1 != w2 || h1 != h2) {
        printf("Dimensions of I1: %d x %d \nDimensions of I2: %d x %d\nAre not equal\n",w1,h1,w2,h2);
        return;
    }

    // copy image header (width, height) of input image I1 to output image II1 & II2
    IP_copyImageHeader(I1, II1);
    IP_copyImageHeader(I1, II2);

    // compute FFT of I1 and I2:
    ImagePtr I1FFTTemp;
    ImagePtr I1FFT;

    ImagePtr I2FFT;
    ImagePtr I2FFTTemp;

    // Allocate memory for FFT
    I1FFT->allocImage(w1,h1,FFT_TYPE);
    I1FFTTemp->allocImage(w1, h1, FFT_TYPE);
    I2FFT->allocImage(w2,h2,FFT_TYPE);
    I2FFTTemp->allocImage(w2, h2, FFT_TYPE);

    // Calculate FFT of I1
    fft1DRow(I1,I1FFTTemp);
    fft1DColumn(I1, I1FFTTemp, I1FFT);

    // Calculate FFT of I2
    fft1DRow(I2, I2FFTTemp);
    fft1DColumn(I2, I2FFTTemp, I2FFT);

    // Calculate magnitude and phase
    float *magnitude1 = new float[total1];
    float *phase1 = new float[total1];
    fft1DMagPhase(I1, I1FFT, magnitude1 , phase1);

    float *magnitude2 = new float[total2];
    float *phase2 = new float[total2];
    fft1DMagPhase(I2, I2FFT, magnitude2 , phase2);

    // Declare image ptrs for IFFT
    ImagePtr I1IFFTTemp;
    ImagePtr I1Ifft;

    // Swap phase of I1 with I2
    swapPhase(I1FFT,magnitude1,phase2,total1);

    // Calculate IFFT of I1 to get back image
    Ifft1DRow(I1,I1FFT, I1IFFTTemp);
    Ifft1DColumn(I1, I1IFFTTemp, I1Ifft, II1);

    // Declare image ptrs for IFFT
    ImagePtr I2IFFTTemp;
    ImagePtr I2Ifft;

    // Swap phase of I2 with I1
    swapPhase(I2FFT,magnitude2,phase1,total2);

    // Calculate IFFT of I2 to get back image
    Ifft1DRow(I2,I2FFT, I2IFFTTemp);
    Ifft1DColumn(I2, I2IFFTTemp, I2Ifft, II2);
}


void swapPhase(ImagePtr I, float * magnitude ,float * newPhase, int total) {
    ChannelPtr<float> real, img;
    real = I[0];
    img = I[1];

    for (int i = 0; i < total; ++i){
        // Replace real & imaginary parts with new phase
        *real++ = magnitude[i] * cos(newPhase[i]);
        *img++ = magnitude[i] * sin(newPhase[i]);
    }
}

// Calculate inverst FFT of a row
void Ifft1DRow(ImagePtr I, ImagePtr IFFT ,ImagePtr IFFTout) {
    int w = I->width();
    int h = I->height();
    ChannelPtr<float> real, img, real2, img2;

    IFFTout->allocImage(w, h, FFT_TYPE);

    real = IFFT[0];
    img = IFFT[1];
    real2 = IFFTout[0];
    img2 = IFFTout[1];

    complexP c1, c2, *q1, *q2;
    q1 = &c1;
    q2 = &c2;
    q1->len = w;
    q1->real = new float[w];
    q1->imag = new float[w];
    q2->len = w;
    q2->real = new float[w];
    q2->imag = new float[w];

    for (int row = 0; row < h; ++row) {
        for (int column = 0; column < w; ++column) {
            q1->real[column] = *real++;
            q1->imag[column] = *img++;
        }

        fft1D(q1, 1, q2);

        for (int i = 0; i < q2->len; ++i) {
            *real2++ = q2->real[i];
            *img2++ = q2->imag[i];
        }
    }
}

// Calculate IFFT column by column
void Ifft1DColumn(ImagePtr I, ImagePtr IFFT ,ImagePtr IFFTout, ImagePtr II1) {
    int w = I->width();
    int h = I->height();
    ChannelPtr<float> real, img, real2, img2;
    ChannelPtr<uchar> pII1;
    int type;
    IFFTout->allocImage(w, h, FFT_TYPE);

    real2 = IFFTout[0];
    img2 = IFFTout[1];


    for (int ch = 0; IP_getChannel(II1, ch, pII1, type); ++ch) {
        real = IFFT[0];
        img = IFFT[1];

        complexP c1, c2, *q1, *q2;
        q1 = &c1;
        q2 = &c2;
        q1->len = w;
        q1->real = new float[w];
        q1->imag = new float[w];
        q2->len = w;
        q2->real = new float[w];
        q2->imag = new float[w];

        for (int column = 0; column < w; ++column) {
            ChannelPtr<float> temp_real = real;
            ChannelPtr<float> temp_img = img;

            for (int row = 0; row < h; ++row) {
                q1->real[row] = *temp_real;
                q1->imag[row] = *temp_img;

                if (row < h - 1)
                {
                    temp_real += w;
                    temp_img += w;
                }
            }

            fft1D(q1, 1, q2);

            for (int i = 0; i < q2->len; ++i) {
                *pII1++ = CLIP(q2->real[i], 0, MaxGray);
            }
            real++;
            img++;
        }
    }

}
