#ifndef WORLD_H
#define WORLD_H

#include <fstream>
#include <string>
#include <immintrin.h>

#include <cstdint>

#include "Point.h"
#include "jpeglib.h"


using std::cout;
using std::endl;
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

    void mark(MultiPoint p) {
        __v4sf res_x = (p.x + 1.f) * (0.5f * size);
        __v4sf res_y = (p.y + 1.f) * (0.5f * size);

        __v4si ix = __builtin_ia32_cvtps2dq(res_x);
        __v4si iy = __builtin_ia32_cvtps2dq(res_y);

        __v4si good = (0 <= ix) & (ix < int32_t(size)) & (0 <= iy) & (iy < int32_t(size)) & (p.z < int32_t(height));

        __v4si offset = p.z * int32_t(XY_count);
        __v4si base_idx = (ix + int32_t(size) * iy + offset);
        __v4si idx = good & base_idx;

        data[0] = 0;
        for(size_t ctr(0); ctr < p.size(); ++ctr) {
            int32_t pos = idx[ctr];
            auto new_val = data[pos] + 1;
            if(new_val < 254){
                data[pos] = new_val;
            } else {
                full_data[pos] += new_val;
                data[pos] = 0;
            }
        }
    }


    void mark(struct Point p) {
        size_t ix = ((p.x + 1) * 0.5) * size;
        size_t iy = ((p.y + 1) * 0.5) * size;
        if (ix < size && iy < size && p.z < height) {
            auto val = data[ix + size * iy + XY_count * p.z]++;
            if(val > 250) {
                dump();
            }
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
        dump();
        for(size_t ctr(0); ctr < XYZ_count; ++ctr) {
            total_marks += full_data[ctr];
        }
        cout << "Total: " << (float) total_marks << endl;
        cout << "Average: " << (1. * total_marks) / XYZ_count << endl;
        cout << "Peak: " << peak() << endl;
    }
    void save_to_jpg() {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE * outfile;
        JSAMPROW row_pointer[1];
        constexpr int row_stride = size * 3;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        if ((outfile = fopen("test.jpg", "wb")) == NULL) {
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
                auto current = full_data[row_ctr * size + col_ctr];
                size_t base_addr = col_ctr * 3;
                image_row[base_addr+0] =0;
                image_row[base_addr+1] = (UINT8) ((255. * current) / peak_val);
                image_row[base_addr+2] = (UINT8) ((255. * current) / peak_val);
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
         1. A small memory footprint to minimize
            cache misses. This type will overflow.

         2. A large memory one for all the data.
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
        for(size_t ctr(0); ctr < XY_count; ++ ctr) {
            full_data[ctr] += data[ctr];
            data[ctr] = 0;
        }
    }
};

#endif
