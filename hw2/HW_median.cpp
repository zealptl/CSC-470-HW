#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;

extern void copyRowToBuffer(ChannelPtr<uchar>, short*, int, int); // see implementation in HW_errDiffusion.cpp
int getMedianWithK(std::vector<int>, int);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//
void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{
    if(sz % 2 == 0) sz++;
    int k = 0;
    IP_copyImageHeader(I1, I2);
    int w = I1->width();
    int h = I1->height();
    int total = w*h;

    int type;
    ChannelPtr<uchar> p1, p2, endd;

    if (sz == 1) {
      for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
          IP_getChannel(I2, ch, p2, type);
          for(endd = p1 + total; p1<endd;) *p2++ = *p1++;
      }
    } else if (sz > 1) {
        int bufSz = sz+w-1; // size of buffer for each padded row
        short* buffers[sz]; // array of sz pointers
        for (int i=0; i<sz; i++) buffers[i] = new short[bufSz];

        for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            endd = p1+total;

            // copy first row to first sz/2 buffers, where are top padded rows
            for (int i=0; i<sz/2; i++) copyRowToBuffer(p1, buffers[i], w, sz);

            // continue to rest of buffers, note here we still begin copying from frist row of I1
            for (int i=sz/2; i<sz; i++) {
                copyRowToBuffer(p1, buffers[i], w, sz);
                p1+=w;
            }

            std::vector<int> v(0);  // vector for storing neighbors
            for (int y=0; y<h; y++) {
                for (int i=0; i<sz; i++) {
                    for (int j=0; j<sz; j++) {
                        v.push_back(buffers[j][i]);
                    }
                }

                // visit each pixel in row
                for (int x=0; x<w; x++) {
                    *p2++ = getMedianWithK(v, k);  // use sorting to find median, k is default 0

                    if (x<w-1) {
                        v.erase(v.begin(), v.begin()+sz);  // delete outgoing column
                        for (int i=0; i<sz; i++) {
                            v.push_back(buffers[i][x+sz]);  // add incoming column
                        }
                    }
                }
                v.clear();

                int nextBufferIndex = (y+sz-1)%sz;
                copyRowToBuffer(p1, buffers[nextBufferIndex], w, sz);
                if (p1 < endd-w) p1+=w;
            }
        }
        for (int i=0; i<sz; i++) delete[] buffers[i];
    }
}

int
getMedianWithK(std::vector<int> v, int k) {
    int vSz = v.size();
    unsigned short sum;

    if (vSz==1) { return v[0]; }
    else {
        std::sort(v.begin(), v.end());  // sort the vector
        int middleIndex = vSz/2;
        if (k==0) { return v[middleIndex]; }
        else {
            sum=v[middleIndex];
            // add left k values and right k values to sum
            for (int i=1; i<=k; i++) { sum+= (v[middleIndex-i]+v[middleIndex+i]); }
            return sum/(k*2+1);
        }
    }
}

