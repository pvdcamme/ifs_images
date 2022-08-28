#ifndef COLORIZER_H
#define COLORIZER_H
#include <cstdint>

#include <complex>

#include <vector>
#include <limits>



using std::endl;

using std::complex;

#
/**
  Creates the colors for a single pixel.
 */
struct Colorizer {
    complex<float> val;
    size_t colors;
    Colorizer(size_t _colors);
    void addColor(size_t ctr, float magnitude);
    void normalize();

    void rgb(float peak, uint8_t& rr, uint8_t& gg, uint8_t& bb);
};
#endif
