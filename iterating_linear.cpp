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

    Point move(Point& p) const{
        return Point(a* p.x + b * p.y + c,
                     d* p.x + e* p.y + f);
    }
};

template<size_t transform_count>
struct TransformGroup{
    Linear transforms[transform_count];

    Point move(Point& p) const{
            size_t idx = random() % transform_count;
            return transforms[idx].move(p);
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
        loop_ctr++;
        Point p(0,0);

        for(size_t point_ctr(0); point_ctr < 100000; ++point_ctr) {
            p = transforms.move(p);
            w.mark(p);
        }
    }
    std::cout << (loop_ctr / time_passed(start_program)) << " loops/sec " <<std::endl;
    w.save(target_name.c_str());
}
