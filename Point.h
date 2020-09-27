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
    // Helper class for operator[](MultiPoint)
    class PointRef{
        MultiPoint& parent;
        size_t idx;
        public:
            PointRef(MultiPoint& _parent, size_t _idx):
            parent(_parent),
            idx(_idx)
            {}
                
            operator Point() const{
                return Point(parent.x[idx], parent.y[idx]);
            }
            void operator=(Point p){
                parent.x[idx] = p.x;
                parent.y[idx] = p.y;
            }
    };
    vector_type x;
    vector_type y;

    MultiPoint(float init_x, float init_y) {
        x = x * 0 + init_x;
        y = y * 0 + init_y;
    }

    size_t size() {
        return sizeof(vector_type) / sizeof(float);
    }

    PointRef operator[](size_t idx) {
        return PointRef(*this, idx);
    }
};



#endif
