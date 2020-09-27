#ifndef POINT_H
#define POINT_H

#include <immintrin.h>

struct Point {
    float x,y;

    Point(float _x, float _y):
        x(_x), y(_y)
    {}
};

/* Stores several points together.
 * This class is intended for an SIMD approach.
 */
template<typename vector_type>
struct MultiPoint {
    vector_type x;
    vector_type y;

    MultiPoint(float init_x, float init_y){
        x = x ^ x + init_x;
        y = y ^ y + init_y;

    }

    size_t size(){
        return sizeof(vector_type) / sizeof(float);
    }

    Point operator[](size_t idx){
        return Point(x[idx], y[idx]);
    }
};



#endif
