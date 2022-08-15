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

   @detail.
   Most small values are passed by-value rather then by reference.
   This gives us nice-pure functions that are easy to reason
   about. Benchmarking has shown no performance degradation.

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

/*
 * A linear transform. Initialized with random values.
 * for performance use the vector instructions.
 */
struct Linear {
    size_t id;
    float a,b,c;
    float d,e,f;

    Linear():
        id(0),
        a(normal()), b(normal()), c(normal()),
        d(normal()), e(normal()), f(normal())
    { }

    Point move(Point p) const {
        return Point(a* p.x + b * p.y + c,
                     d* p.x + e* p.y + f,
                     id);
    }

    /* Performs multiple transforms simultaneously.
     * Offers a nice speedup, despite the gathering.
     */
    static MultiPoint move(MultiPoint pp,
                           const Linear& l1,
                           const Linear& l2,
                           const Linear& l3,
                           const Linear& l4)
    {
        float_vector_type aa = {l1.a, l2.a, l3.a, l4.a};
        float_vector_type bb = {l1.b, l2.b, l3.b, l4.b};
        float_vector_type cc = {l1.c, l2.c, l3.c, l4.c};
        float_vector_type dd = {l1.d, l2.d, l3.d, l4.d};
        float_vector_type ee = {l1.e, l2.e, l3.e, l4.e};
        float_vector_type ff = {l1.f, l2.f, l3.f, l4.f};

        float_vector_type x = aa * pp.x + bb * pp.y + cc;
        float_vector_type y = dd * pp.x + ee * pp.y + ff;
        int_vector_type z = {l1.id, l2.id, l3.id, l4.id};

        return MultiPoint(x,y, z);
    }
};

/**
 A fast power operation.
 Delegates much to the compiler, use
 only for small exponents.
*/
template<size_t exp>
size_t pow(size_t val) {
    return val * pow<exp -1>(val);
}

template<>
size_t pow<0>(size_t val) {
    return 1;
}

/**
  * Groups several transforms together.
  * Makes it easy to pick a random one each time.
  */
template<size_t transform_count>
struct TransformGroup {
    Linear transforms[transform_count];

    TransformGroup() {
        for(auto ctr(0); ctr < transform_count; ++ctr) {
            transforms[ctr].id =ctr;
        }
    }

    Point move(Point p) const {
        size_t idx = random() % transform_count;
        return transforms[idx].move(p);
    }

    MultiPoint move(MultiPoint& p) const {
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

    World<1024,1> w;
    TransformGroup<16> transforms;

    auto start_program= std::chrono::steady_clock::now();
    size_t loop_ctr(0);
    size_t max_runtime= 4;
    while(time_passed(start_program) < max_runtime) {
        const size_t internal_loop = 100000;
        //MultiPoint p(0,0);
        Point p(0,0);
        loop_ctr+= p.size() * internal_loop;

        for(size_t point_ctr(0); point_ctr < internal_loop; ++point_ctr) {
            p = transforms.move(p);
            w.mark(p);
        }
    }
    std::cout << (loop_ctr / time_passed(start_program)) << " loops/sec " <<std::endl;
    w.save(target_name.c_str());
    w.print_stats();
    w.save_to_jpg();
}
