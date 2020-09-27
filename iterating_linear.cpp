/**
   A small program that generates images using
   an iterating system.

   The basis is a number of linear transforms.
   These are applied in random order while
   the density of the results are saved. Some
   regions have much higher chance than others.

   The program iterates for a fixed number of
   seconds before saving the image.

   The result of this program is text file with
   a 2D matrix with counts. This file has the folloing
   format:
     line 1: <size>
     line 2: <first row, <size> values seperated by commas>
     ..   <size> + 1: last row

   The post_process.py script can process this file to
   a pretty immage.

   Normally the result file is saved to 'result.txt'. This
   name can be changed with the first command argument.

*/
#include <iostream>
#include <string>
#include <chrono>

#include <cstdlib>
#include <cstdint>

#include "World.h"


float normal() {
    double upper = random() / double(RAND_MAX);
    return float(2 * upper - 1);
}

struct Linear {
    float a,b,c;
    float d,e,f;

    Linear():
        a(normal()), b(normal()), c(normal()),
        d(normal()), e(normal()), f(normal())
    { }

    Point move(Point p) const {
        return Point(a* p.x + b * p.y + c,
                     d* p.x + e* p.y + f);
    }
    static MultiPoint<__v4sf> move(MultiPoint<__v4sf> pp,
                const Linear& l1,
                const Linear& l2,
                const Linear& l3,
                const Linear& l4)
    {
        __v4sf aa = {l1.a, l2.a, l3.a, l4.a};
        __v4sf bb = {l1.b, l2.b, l3.b, l4.b};
        __v4sf cc = {l1.c, l2.c, l3.c, l4.c};
        __v4sf dd = {l1.d, l2.d, l3.d, l4.d};
        __v4sf ee = {l1.e, l2.e, l3.e, l4.e};
        __v4sf ff = {l1.f, l2.f, l3.f, l4.f};
        
        __v4sf x = aa * pp.x + bb * pp.y + cc;
        __v4sf y = dd * pp.x + ee * pp.y + ff;

        pp.x = x;
        pp.y = y;
        return pp;
    }
};

template<size_t exp>
size_t pow(size_t val){
    return val * pow<exp -1>(val);
}

template<>
size_t pow<0>(size_t val){
    return 1;
}

template<size_t transform_count>
struct TransformGroup {
    Linear transforms[transform_count];

    Point move(Point p) const {
        size_t idx = random() % transform_count;
        return transforms[idx].move(p);
    }

    MultiPoint<__v4sf> move(MultiPoint<__v4sf>& p) const {
        size_t rr = random();
        size_t idx0 = (rr /pow<0>(transform_count)) % transform_count;
        size_t idx1 = (rr /pow<1>(transform_count)) % transform_count;
        size_t idx2 = (rr /pow<2>(transform_count)) % transform_count;
        size_t idx3 = (rr /pow<3>(transform_count)) % transform_count;

        return Linear::move(p,
                    transforms[idx0],
                    transforms[idx1],
                    transforms[idx2],
                    transforms[idx3]);
    }
};


template<typename clock>
double time_passed(std::chrono::time_point<clock> start) {
    auto now = clock::now();
    std::chrono::duration<double> elapsed = now - start;
    return elapsed.count();
}

void init_seed() {
    auto some_moment = std::chrono::steady_clock::now();
    unsigned int some_value = some_moment.time_since_epoch().count();
    srand(some_value);
}

int main(int argc, char** argv) {
    std::string target_name("result.txt");
    if(argc > 1) {
        target_name = argv[1];
    }

    init_seed();

    World<1024> w;
    const size_t transform_count= 16;
    TransformGroup<16> transforms;

    auto start_program= std::chrono::steady_clock::now();
    size_t loop_ctr(0);
    size_t max_runtime= 4;
    while(time_passed(start_program) < max_runtime) {
        MultiPoint<__v4sf> p(0,0);
        loop_ctr+= p.size();

        for(size_t point_ctr(0); point_ctr < 100000; ++point_ctr) {
            p = transforms.move(p);
            w.mark(p);
        }
    }
    std::cout << (loop_ctr / time_passed(start_program)) << " loops/sec " <<std::endl;
    w.save(target_name.c_str());
}
