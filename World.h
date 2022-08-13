#ifndef WORLD_H
#define WORLD_H

#include <fstream>
#include <string>
#include <immintrin.h>

#include <cstdint>

#include "Point.h"

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
        full_data(new uint32_t[XYZ_count])
    {
        std::fill(data, data + XYZ_count, 0);
        std::fill(full_data,full_data + XYZ_count, 0);
    }

    ~World() {
        delete[] data;
        delete[] full_data;
    }

    void mark(MultiPoint<__v4sf, __v4si> p) {
        __v4sf res_x = (p.x + 1.f) * (0.5f * size);
        __v4sf res_y = (p.y + 1.f) * (0.5f * size);

        __v4si ix = __builtin_ia32_cvtps2dq(res_x);
        __v4si iy = __builtin_ia32_cvtps2dq(res_y);
        __v4si good = (0 <= ix) & (ix < int32_t(size)) & (0 <= iy) & (iy < int32_t(size));

        __v4si idx = good & (ix + int32_t(size) * iy);

        data[0] = 0;
        bool needs_to_dump= false;
        for(size_t ctr(0); ctr < 4; ++ctr) {
            int32_t pos = idx[ctr];
            needs_to_dump = (250 < data[pos]++);
        }
        if(needs_to_dump) {
            dump();
        }
    }


    void mark(struct Point p) {
        size_t ix = ((p.x + 1) * 0.5) * size;
        size_t iy = ((p.y + 1) * 0.5) * size;
        if (ix < size && iy < size) {
            auto val = data[ix + size * iy]++;
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
    uint32_t* full_data;

    void dump() {
        for(size_t ctr(0); ctr < XY_count; ++ ctr) {
            full_data[ctr] += data[ctr];
            data[ctr] = 0;
        }
    }

};

#endif
