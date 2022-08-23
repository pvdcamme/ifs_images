#ifndef WORLD_H
#define WORLD_H

#include <random>
#include <fstream>
#include <string>
#include <immintrin.h>
#include <cassert>

#include <cstdint>

#include "Point.h"
#include <complex>
#include "jpeglib.h"

#include <vector>
#include <limits>
#include <bitset>

#include <string>


using std::cout;
using std::endl;

using std::complex;


/**
  Creates the colors for a single pixel.
 */
struct Colorizer {
    complex<float> val;
    size_t colors;
    Colorizer(size_t _colors):
        val((0,0)), colors(_colors)
    {
    }
    void addColor(size_t ctr, float magnitude) {
        float phase = (ctr * 3.1415f * 2) / colors;
        val += std::polar(magnitude, phase);
    }
    void normalize() {
        val /= std::abs(val);
    }

    void rgb(float peak, uint8_t& rr, uint8_t& gg, uint8_t& bb) {
        if (std::abs(val) == 0.) {
            rr= 255;
            gg = 255;
            bb = 255;
            return;
        }
        float orig_abs = std::abs(val);
        normalize();
        float h = (std::arg(val) + 3.1415) * 6 / (2 * 3.1415);
        float hh = h;

        while(hh >2) {
            hh -= 2;
        }
        float c = std::log(std::max(1.f, orig_abs)) / std::log(peak);
        float x = c * (1 - std::abs(hh -1));
        float r,g,b;
        if(h < 1) {
            r = c;
            g = x;
            b = 0;
        } else if(h < 2) {
            r = x;
            g = c;
            b = 0;
        } else if(h < 3) {
            r = 0;
            g = c;
            b = x;
        } else if(h < 4) {
            r = 0;
            g = x;
            b = c;
        } else if(h < 5) {
            r = x;
            g = 0;
            b = c;
        } else {
            r = c;
            g = 0;
            b = x;
        }
        float m = 1 - c;
        rr = 255 * (r + m);
        gg = 255 * (g + m);
        bb = 255 * (b + m);


    }
};
/** Represents a world canvas to collect
    the traversal.

    To render prettier pictures, this world is
    3D.
      x & Y: Match the drawing canvas.
      Z: Affine transform that created the new
         position.

    In postprocessing these coordinates can be
    mapped to the appropriate color.
*/
template<size_t size, size_t height>
struct World {
    static constexpr size_t line_vals = 16;
    static constexpr size_t total_line_size = 4 * 1024;
public:
    World():
        data(new uint8_t[XYZ_count]),
        full_data(new uint64_t[XYZ_count])
    {
        std::fill(data, data + XYZ_count, 0);
        std::fill(full_data,full_data + XYZ_count, 0);
    }

    ~World() {
        delete[] data;
        delete[] full_data;
    }

    template<size_t cnt>
    void mark(MultiPoint* pps) {
        __v4si previous = {0, 0, 0, 0};

        // Don't care about pixel at (0,0).
        // Used to dump the invalid coordinates.
        data[0] = 0;
        for(auto inner(0); inner < cnt; ++inner) {
            const auto p = pps[inner];
            const auto pz = p.z;

            __v4sf res_x = (p.x + 1.f) * (0.5f * size);
            __v4sf res_y = (p.y + 1.f) * (0.5f * size);

            __v4si ix = __builtin_ia32_cvtps2dq(res_x);
            __v4si iy = __builtin_ia32_cvtps2dq(res_y);

            __v4si good = (0 <= ix) & (ix < int32_t(size)) & (0 <= iy) & (iy < int32_t(size)) & (pz < int32_t(height));

            __v4si offset = pz * int32_t(XY_count);
            __v4si base_idx = (ix + int32_t(size) * iy + offset);

            auto idx = base_idx & good;
            for(size_t ctr(0); ctr < 4; ++ctr) {
                int32_t pos = previous[ctr];
                data[pos] += 1;
                if(data[pos] == 0) {
                    full_data[pos] += (uint64_t) std::numeric_limits<typeof(data[pos])>::max();
                }
            }
            previous = idx;


        }
        for(size_t ctr(0); ctr < 4; ++ctr) {
            int32_t pos = previous[ctr];
            data[pos] += 1;
            if(data[pos] == 0) {
                full_data[pos] += (uint64_t) std::numeric_limits<typeof(data[pos])>::max();
            }
        }
    }

    void mark(MultiPoint p) {
        mark<1>(&p);
    }


    void mark(struct Point p) {
        size_t ix = ((p.x + 1) * 0.5) * size;
        size_t iy = ((p.y + 1) * 0.5) * size;
        if (ix < size && iy < size && p.z < height) {
            auto pos = ix + size * iy + XY_count * p.z;
            data[pos] +=+ 1;
            if(data[pos] == 0) {
                full_data[pos] += (uint64_t) std::numeric_limits<typeof(data[pos])>::max();
            }
        }
    }
    void reduce() {
        dump();
        for(auto ctr(0); ctr < XYZ_count; ++ctr) {
            full_data[ctr] = ceil(full_data[ctr] * 0.99);
        }
    }

    void save(std::string name) {
        std::ofstream out(name.c_str());

        dump();
        full_data[0] = 0;
        out << size << std::endl;
        for(size_t y(0); y < size; ++y) {
            for(size_t x(0); x < size; ++x) {
                out << full_data[y * size + x] << ", ";
            }
            out << std::endl;
        }
    }
    void print_stats() {
        uint64_t total_marks(0);
        uint64_t least_marks(100000000);
        uint64_t spread_out = 0;
        dump();
        for(size_t ctr(0); ctr < XYZ_count; ++ctr) {
            total_marks += full_data[ctr];
            least_marks = std::min(least_marks, full_data[ctr]);
        }

        for(size_t ctr(0); ctr < XY_count; ++ctr) {
            for(size_t h(0); h < height; ++h) {
                if (full_data[ctr + h * XY_count] > 0) {
                    spread_out++;
                }
            }
        }

        cout << "Total: " << (float) total_marks << endl;
        cout << "Average: " << (1. * total_marks) / XYZ_count << endl;
        cout << "Peak: " << peak() << endl;
        cout << "Least: " << least_marks << endl;
        cout << "Spread: " << float(spread_out) / XY_count << endl;
    }
    void save_to_jpg(std::string name) {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE * outfile;
        JSAMPROW row_pointer[1];
        constexpr int row_stride = size * 3;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        if ((outfile = fopen(name.c_str(), "wb")) == NULL) {
            std::cerr << "Can't open file for writing" << std::endl;
            return;
        }
        jpeg_stdio_dest(&cinfo, outfile);
        cinfo.image_width= size;
        cinfo.image_height= size;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
        jpeg_set_defaults(&cinfo);

        jpeg_start_compress(&cinfo, TRUE);

        JSAMPLE image_row[row_stride];


        auto peak_val = peak();
        for(size_t row_ctr(0); row_ctr < size; ++row_ctr) {
            for(size_t col_ctr(0); col_ctr < size; ++col_ctr) {

                Colorizer color(height);
                for(size_t z(0); z < height; ++z) {
                    auto vv = full_data[row_ctr * size + col_ctr + z * XY_count];
                    color.addColor(z,vv);
                }
                auto current = full_data[row_ctr * size + col_ctr];
                size_t base_addr = col_ctr * 3;
                color.rgb(peak_val, image_row[base_addr+0], image_row[base_addr+1], image_row[base_addr + 2]);

            }
            row_pointer[0] = image_row;
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        jpeg_finish_compress(&cinfo);
        fclose(outfile);
        jpeg_destroy_compress(&cinfo);
    }

private:
    /**
        The data is stored into two arrays.
         1. A small one to minimize
            cache misses. This type will overflow.

         2. A large one for the full the data.
            This one should never overflow.

         When the first type is close to overflow
         it should be dumped into the larger one.
     */
    static constexpr size_t XY_count = size *size;
    static constexpr size_t XYZ_count = size *size *height;
    uint8_t* data;
    uint64_t* full_data;

    size_t peak() {
        uint64_t peak_marks(0);
        for(size_t ctr(0); ctr < XYZ_count; ++ctr) {
            peak_marks = std::max(peak_marks, full_data[ctr]);
        }
        return peak_marks;

    }

    void dump() {
        for(size_t ctr(0); ctr < XYZ_count; ++ ctr) {
            full_data[ctr] += data[ctr];
        }
        std::fill(data, data +XYZ_count, 0);

    }
};

#endif
