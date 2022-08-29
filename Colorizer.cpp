#include "Colorizer.h"

using std::complex;

Colorizer::Colorizer(size_t _colors):
    val((0,0)), colors(_colors)
{ }

void Colorizer::addColor(size_t ctr, float magnitude)
{
    float phase = (ctr * 3.1415f * 2) / colors;
    val += std::polar(magnitude, phase);
}


static complex<float> normalize(complex<float> val)
{
    return val / std::abs(val);
}

void Colorizer::rgb(float peak, uint8_t& rr, uint8_t& gg, uint8_t& bb)
{
    if (std::abs(val) == 0.)
    {
        rr= 255;
        gg = 255;
        bb = 255;
        return;
    }
    float orig_abs = std::abs(val);
    val = normalize(val);
    float h = (std::arg(val) + 3.1415) * 6 / (2 * 3.1415);
    float hh = h;

    while(hh >2)
    {
        hh -= 2;
    }
    float c = std::log(std::max(1.f, orig_abs)) / std::log(peak);
    float x = c * (1 - std::abs(hh -1));
    float r,g,b;
    if(h < 1)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if(h < 2)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if(h < 3)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if(h < 4)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if(h < 5)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }
    float m = 1 - c;
    rr = 255 * (r + m);
    gg = 255 * (g + m);
    bb = 255 * (b + m);


}
