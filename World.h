#ifndef WORLD_H
#define WORLD_H

#include <fstream>
#include <string>

#include <cstdint>

#include "Point.h"


// World from -1 to 1
template<size_t size>
struct World {
    uint8_t* data;
    uint32_t* full_data;

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

    void dump() {
        for(size_t ctr(0); ctr < size * size; ++ ctr) {
            full_data[ctr] += data[ctr];
            data[ctr] = 0;
        }
    }


    void mark(struct Point& p) {
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
};

#endif 
