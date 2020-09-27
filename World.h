#ifndef WORLD_H
#define WORLD_H

#include <fstream>
#include <string>
#include <immintrin.h>

#include <cstdint>

#include "Point.h"


// World from -1 to 1
template<size_t size>
struct World {
public:
    World():
        data(new uint8_t[size * size]),
        full_data(new uint32_t[size * size])
    {
        for(size_t ctr(0); ctr < size * size; ++ ctr) {
            data[ctr] = 0;
            full_data[ctr] = 0;
        }
    }

    ~World() {
        delete[] data;
    }

    void mark(MultiPoint<__v4sf> p){
        __v4sf res_x = (p.x + 1.f) * (0.5f * size);
        __v4sf res_y = (p.y + 1.f) * (0.5f * size);

        __v4si ix = __builtin_ia32_cvtps2dq(res_x);
        __v4si iy = __builtin_ia32_cvtps2dq(res_y);
        __v4si good = (0 <= ix) & (ix < int32_t(size)) & (0 <= iy) & (iy < int32_t(size));

        __v4si idx = good & (ix + int32_t(size) * iy);

        data[0] = 0;
        bool needs_to_dump= false;
        for(size_t ctr(0); ctr < 4; ++ctr){
            int32_t pos = idx[ctr];
            needs_to_dump = (250 < data[pos]++);
        }
        if(needs_to_dump){
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
    uint8_t* data;
    uint32_t* full_data;

    void dump() {
        for(size_t ctr(0); ctr < size * size; ++ ctr) {
            full_data[ctr] += data[ctr];
            data[ctr] = 0;
        }
    }

};

#endif
