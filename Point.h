#ifndef POINT_H
#define POINT_H

#include <immintrin.h>

struct Point {
    float x,y;
    size_t z;

    Point(float _x, float _y, size_t _z):
        x(_x), y(_y), z(_z)
    {}
};

/* Stores several points together.
 * This class is intended for an SIMD approach.
 */
template<typename float_vector_type, typename int_vector_type>
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
                return Point(parent.x[idx], parent.y[idx], parent.z[idx]);
            }
            void operator=(Point p){
                parent.x[idx] = p.x;
                parent.y[idx] = p.y;
                parent.z[idx] = p.z;
            }
    };
    float_vector_type x;
    float_vector_type y;
    int_vector_type z;


    MultiPoint(float_vector_type _x, float_vector_type _y, int_vector_type _z):
        x(_x), y(_y), z(_z)
    {}
    MultiPoint(float init_x, float init_y) {
        x = x * 0 + init_x;
        y = y * 0 + init_y;
        z = z * 0;
    }

    size_t size() {
        return sizeof(float_vector_type) / sizeof(float);
    }

    PointRef operator[](size_t idx) {
        return PointRef(*this, idx);
    }
};



#endif
