#include "IP.h"
#include <stdio.h>
#include <algorithm>

using namespace IP;
using namespace std;

struct complexP {
    int len; // length of complex number list
    float *real; // pointer to real part of complex number
    float *imag; // pointer to imaginary part of complex number
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Convolve magnitude and phase spectrum from image I1.
// Output is in Imag and Iphase.
//

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
void paddedImage(ImagePtr I1, ImagePtr I1Padded);
void fft1D(complexP *q1, int dir, complexP *q2);
void fft1DRow(ImagePtr I1, ImagePtr Image1);
void fft1DColumn(ImagePtr I1, ImagePtr Image1, ImagePtr Image2);
void fft1DMagPhase(ImagePtr I1, ImagePtr Image2, float *Magnitude, float *Phase);
float min(float arr[], int total);
float max(float arr[], int total);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Compute magnitude and phase spectrum from input image I1.
// Save results in Imag and Iphase.
//
void
HW_spectrum(ImagePtr I1, ImagePtr Imag, ImagePtr Iphase)
{
    // init vars for width, height, and total number of pixels
    int w = I1->width();
    int h = I1->height();
    int total = w * h;

    // compute FFT of the input image
    ImagePtr I2;

    // if input is not power of 2 and padding is needed
    if (ceil(log2(w)) != floor(log2(w)) || ceil(log2(h)) != floor(log2(h))) {
        paddedImage(I1, I2);

        // update dimensions with padded image's dimensions
        w = I2->width();
        h = I2->height();
        total = w * h;
    }
    else {
        I2 = I1;
    }

    if (0) { // 0 For Spectrum

     // declare channel ptrs
     ChannelPtr<uchar> p1, p2, p3;
     int type;

     // copy image header (width, height) of input image I2 to output image Imag
     IP_copyImageHeader(I2, Imag);

     // copy image header (width, height) of input image I2 to output image Iphase
     IP_copyImageHeader(I2, Iphase);

     int newTotal = I2->width() * I2->height();

     // visit all channels of image
     for (int ch = 0; IP_getChannel(I2, ch, p1, type); ch++) {
         IP_getChannel(Imag, ch, p2, type);
         IP_getChannel(Iphase, ch, p3, type);
         for (int i = 0; i < newTotal; i++) {
             *p2++ = *p1++;
             *p3++ = 0;
         }
     }
    } else {
        // Init Image1 for row fft & Image2 for column fft
        ImagePtr Image1, Image2;
        Image1->allocImage(w, h, FFT_TYPE);
        Image2->allocImage(w, h, FFT_TYPE);

        // Calculate FFT for each row
        fft1DRow(I2, Image1);

        // Take output of row FFT & perform column FFT on it
        fft1DColumn(I2, Image1, Image2);

        // init magnitude and phase vars
        float *Magnitude = new float[total];
        float *Phase = new float[total];

        // compute magnitude and phase spectrum from FFT image
        fft1DMagPhase(I2, Image2, Magnitude, Phase);

        // find min and max of magnitude and phase spectrum
        float minMagnitude = min(Magnitude, total);
        float maxMagnitude = max(Magnitude, total);
        float minPhase = min(Phase, total);
        float maxPhase = max(Phase, total);

        IP_copyImageHeader(I2, Imag);
        IP_copyImageHeader(I2, Iphase);
        ChannelPtr<uchar> Pmag, Pphase;
        int type;

        // scale magnitude and phase to fit between [0, 255]
        maxMagnitude /= 256;

        for (int ch = 0; IP_getChannel(Imag, ch, Pmag, type); ch++) {
            for (int i = 0; i < total; i++) {
                *Pmag++ = CLIP((Magnitude[i] - minMagnitude) / (maxMagnitude - minMagnitude) * MaxGray, 0, MaxGray);     // Scales Magnitude and Clipped
            }
        }

        for (int ch = 0; IP_getChannel(Iphase, ch, Pphase, type); ch++) {
            for (int i = 0; i < total; i++) {
                *Pphase++ = CLIP((Phase[i] - minPhase) / (maxPhase - minPhase) * MaxGray, 0, MaxGray);                   // Scales Phase and Clipped
            }
        }
    }

}

void paddedImage(ImagePtr I1, ImagePtr I1Padded) {
    if (0) {
        // 0 to padding all sides
        int w = I1->width();
        int h = I1->height();
        int zerosW = 0;
        int upperBase = floor(log2(w)) + 1;

        // Number of 0s to append along width
        zerosW = pow(2, upperBase) - w;

        // Number of 0s to append along height
        int zerosH = 0;
        upperBase = floor(log2(h)) + 1;
        zerosH = pow(2, upperBase) - h;

        // Padding needed for top and bottom of image
        int paddingH = zerosH / 2;

        // Padding need for left and right of image
        int paddingW = zerosW / 2;

        // Height & width after padding
        int newH = h + zerosH;
        int newW = w + zerosW;

        // new image after padding
        I1Padded->allocImage(newW, newH, BW_TYPE);
        ChannelPtr<uchar> in, out, start;
        int type;

        // visit all channels
        for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {
            IP_getChannel(I1Padded, ch, out, type);

            // visit pixels row after row
            for (int i = 0; i < newH; i++) {

                // visit pixels column after column
                for (int j = 0; j < newW; j++) {
                    if (i < paddingH || i >= paddingH + h || j < paddingW || j >= paddingW + w) {
                        *out++ = 0;
                    }
                    else {
                        *out++ = *in++;
                    }
                }
            }
        }

    } else {
        int w = I1->width();
        int h = I1->height();

        // Number of 0s to append along width
        int zerosW = 0;
        int upperBase = floor(log2(w)) + 1;
        zerosW = pow(2, upperBase) - w;

        // Number of 0s to append along height
        int zerosH = 0;
        upperBase = floor(log2(h)) + 1;
        zerosH = pow(2, upperBase) - h;

        // Height and width after padding
        int newH = h + zerosH;
        int newW = w + zerosW;
        I1Padded->allocImage(newW, newH, BW_TYPE);
        ChannelPtr<uchar> in, out, start;

        int type;
        //Initialize buffer
        for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {
            IP_getChannel(I1Padded, ch, out, type);
            for (int i = 0; i < newH; i++) {
                for (int j = 0; j < newW; j++) {
                    if(i < h && j < w){
                        *out++ = *in++;
                    }
                    else {
                        *out++ = 0;
                    }
                }
            }
        }
    }
}

// from fft1D.c
void fft1D(complexP *q1, int dir, complexP *q2) {
    int i, N, N2;
    float *r1, *i1, *r2, *i2, *ra, *ia, *rb, *ib;
    float FCTR, fctr, a, b, c, s;
    complexP *qa, *qb;

    N = q1->len;
    r1 = q1->real;
    i1 = q1->imag;
    r2 = q2->real;
    i2 = q2->imag;

    if (N == 2) {
        a = r1[0] + r1[1];
        b = i1[0] + i1[1];
        r2[1] = r1[0] - r1[1];
        i2[1] = i1[0] - i1[1];
        r2[0] = a;
        i2[0] = b;
    }
    else {
        N2 = N / 2;

        complexP first;
        qa = &first;
        qa->len = N2;
        qa->real = new float[N2];
        qa->imag = new float[N2];

        complexP second;
        qb = &second;
        qb->len = N2;
        qb->real = new float[N2];
        qb->imag = new float[N2];

        ra = qa->real;
        ia = qa->imag;
        rb = qb->real;
        ib = qb->imag;

        for (i = 0; i < N2; i++) {
            ra[i] = *r1++;
            ia[i] = *i1++;
            rb[i] = *r1++;
            ib[i] = *i1++;
        }

        fft1D(qa, dir, qa);
        fft1D(qb, dir, qb);


        if (!dir)
            FCTR = -2 * PI / N;
        else
            FCTR = 2 * PI / N;
        for (fctr = i = 0; i < N2; i++, fctr += FCTR) {
            c = cos(fctr);
            s = sin(fctr);
            a = c * rb[i] - s * ib[i];
            r2[i] = ra[i] + a;
            r2[i + N2] = ra[i] - a;

            a = s * rb[i] + c * ib[i];
            i2[i] = ia[i] + a;
            i2[i + N2] = ia[i] - a;
        }
        delete[] qa->real;
        delete[] qa->imag;
        delete[] qb->real;
        delete[] qb->imag;
    }

    if (!dir) {
        for (i = 0; i < N; i++) {
            q2->real[i] = q2->real[i] / 2;
            q2->imag[i] = q2->imag[i] / 2;
        }
    }
}

// Calculate FFT of each row in image
void fft1DRow(ImagePtr I1, ImagePtr Image1) {
    // init vars
    int w = I1->width();
    int h = I1->height();
    ChannelPtr<float> real, img;
    ChannelPtr<uchar> p1;
    int type;
    real = Image1[0];
    img = Image1[1];

    complexP c1, c2, *q1, *q2;
    q1 = &c1;
    q2 = &c2;

    // length of input for row is w
    q1->len = w;
    q1->real = new float[w];
    q1->imag = new float[w];
    q2->len = w;
    q2->real = new float[w];
    q2->imag = new float[w];

    // Calculate FFT row by row
    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
        for (int row = 0; row < h; row++) {
            for (int column = 0; column < q1->len; column++){
                q1->real[column] = *p1++;
                q1->imag[column] = 0;
            }

            // calculate fft1D for current row of q1 and store output in q2
            fft1D(q1, 0, q2);

            for (int i = 0; i < q2->len; i++){
                *real++ = q2->real[i];
                *img++ = q2->imag[i];
            }
        }
        delete[] q1->real;
        delete[] q1->imag;
        delete[] q2->real;
        delete[] q2->imag;
    }
}

// Calculate FFT of each column in image
void fft1DColumn(ImagePtr I1, ImagePtr Image1, ImagePtr Image2) {
    int w = I1->width();
    int h = I1->height();
    ChannelPtr<float> real, img, real2, img2;
    real = Image1[0];
    img = Image1[1];
    real2 = Image2[0];
    img2 = Image2[1];

    complexP c1, c2, *q1, *q2;
    q1 = &c1;
    q2 = &c2;
    q1->len = w;
    q1->real = new float[w];
    q1->imag = new float[w];
    q2->len = w;
    q2->real = new float[w];
    q2->imag = new float[w];

    // Calculate FFT column by column
    for (int column = 0; column < w; column++) {
        ChannelPtr<float> temp_real = real;
        ChannelPtr<float> temp_img = img;

        for (int row = 0; row < h; row++) {
            q1->real[row] = *temp_real;
            q1->imag[row] = *temp_img;
            if (row < h - 1) {
                temp_real += w;
                temp_img += w;
            }
        }

        // Calculate FFT of current column
        fft1D(q1, 0, q2);

        for (int i = 0; i < h; i++) {
            *real2++ = q2->real[i];
            *img2++ = q2->imag[i];
        }
        real++;
        img++;
    }
    delete[] q1->real;
    delete[] q1->imag;
    delete[] q2->real;
    delete[] q2->imag;
}

// Calculate Magnitude & Phare of image
void fft1DMagPhase(ImagePtr I1, ImagePtr Image2, float *Magnitude, float *Phase) {
    // init vars
    int w = I1->width();
    int h = I1->height();
    int total = w * h;
    ChannelPtr<float> real2, img2;
    real2 = Image2[0];
    img2 = Image2[1];

    for (int i = 0; i < total; i++, real2++, img2++) {
        // calculate magnitude & phase of each pixel
        Magnitude[i] = sqrt(pow(*real2, 2) + pow(*img2, 2));
        Phase[i] = atan2(*img2, *real2);
    }
}

// find min in array
float min(float arr[], int total) {
    float minValue = arr[0];
    for (int i = 1; i < total; i++)
        minValue = min(minValue, arr[i]);
        return minValue;
}

// find max in array
float max(float arr[], int total) {
    float maxValue = arr[0];
    for (int i = 1; i < total; i++)
        maxValue = max(maxValue, arr[i]);
        return maxValue;
}
