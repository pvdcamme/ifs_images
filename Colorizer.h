#ifndef COLORIZER_H
#define COLORIZER_H
#include <complex>



using std::complex;

#
/**
  Blends the colors for a single pixel.
 */
class Colorizer {
    complex<float> val;
    size_t colors;
public:

    Colorizer(size_t num_oolors);

    // Adds colors for the blending.
    void addColor(size_t color_ctr, float magnitude);

    // Calculates the final Color of the pixel.
    void rgb(float peak, uint8_t& rr, uint8_t& gg, uint8_t& bb);
};
#endif
