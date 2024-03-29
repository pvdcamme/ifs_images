/**
   A small program that generates images using
   an iterating system.

   The basis is a number of linear transforms.
   These are applied in random order while
   the density of the results are saved. Some
   regions have much higher chance than others.

   The program iterates for a fixed number of
   seconds before saving the image.

   Optionally one can supply the name of the output
   image.

   @detail.
   Most small values are passed by-value rather then by reference.
   This gives us nice-pure functions that are easy to reason
   about. Benchmarking has shown no performance degradation.

*/
#include <random>
#include <iostream>
#include <string>
#include <chrono>

#include <cstdlib>
#include <cstdint>
#include <sstream>

#include "World.h"
#include "FastRandom.h"


static std::default_random_engine generator;

float normal()
{
    double upper = random() / double(RAND_MAX);
    return float(2 * upper - 1);
}

/*
 * A linear transform. Initialized with random values.
 * for performance use the vector instructions.
 */
struct Linear
{
    int32_t id;
    float a,b,c;
    float d,e,f;

    Linear():
        id(0)
    {
        std::normal_distribution<double> distribution(0.0,0.3);

        a = distribution(generator);
        b = distribution(generator);
        c = distribution(generator);
        d = distribution(generator);
        e = distribution(generator);
        f = distribution(generator);

        if(normal() < -0.4)
        {
            std::cout << "Rotatation" << std::endl;
            float angle = normal() * 2 * 3.1415;
            float scale = std::normal_distribution<float>(0.7, 0.1)(generator);


            a = scale * cos(angle) + distribution(generator);
            b = scale * -sin(angle) +distribution(generator);

            c =  distribution(generator);
            d = scale * sin(angle) + distribution(generator);
            e = scale * cos(angle) + distribution(generator);
            f =  distribution(generator);
        }
    }

    void perturb()
    {
        std::normal_distribution<double> distribution(0.0,0.08);
        a += distribution(generator);
        b += distribution(generator);
        c += distribution(generator);
        d += distribution(generator);
        e += distribution(generator);
        f += distribution(generator);
    }

    Point move(Point p) const
    {
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
size_t pow(size_t val)
{
    return val * pow<exp -1>(val);
}

template<>
size_t pow<0>(size_t val)
{
    return 1;
}

/**
  * Groups several transforms together.
  * Makes it easy to pick a random one each time.
  */
template<size_t transform_count>
struct TransformGroup
{
    FastRandom rr;
    Linear transforms[transform_count];

    TransformGroup() : rr(random())
    {
        for(auto ctr(0); ctr < transform_count; ++ctr)
        {
            transforms[ctr].id =ctr;
        }
    }

    Point move(Point p)
    {
        size_t idx = rr.random() % transform_count;
        return transforms[idx].move(p);
    }

    MultiPoint move(MultiPoint& p)
    {
        uint64_t idxs = rr.random();
        size_t idx0 = (idxs /pow<0>(transform_count)) % transform_count;
        size_t idx1 = (idxs /pow<1>(transform_count)) % transform_count;
        size_t idx2 = (idxs /pow<2>(transform_count)) % transform_count;
        size_t idx3 = (idxs /pow<3>(transform_count)) % transform_count;

        return Linear::move(p,
                            transforms[idx0],
                            transforms[idx1],
                            transforms[idx2],
                            transforms[idx3]);
    }
    void perturb()
    {
        size_t idx = random() % transform_count;
        transforms[idx].perturb();
    }
};


template<typename clock>
double time_passed(std::chrono::time_point<clock> start)
{
    auto now = clock::now();
    std::chrono::duration<double> elapsed = now - start;
    return elapsed.count();
}

static void init_rand()
{
    auto now = std::chrono::steady_clock::now();
    auto some_val = now.time_since_epoch().count();
    srand(some_val);
}

int main(int argc, char** argv)
{
    init_rand();
    generator.seed(random());
    std::string target_name("result.jpg");
    if(argc > 1)
    {
        target_name = argv[1];
    }

    constexpr auto N_TRANSFORMS = 16;
    World<1024,N_TRANSFORMS> w;
    TransformGroup<N_TRANSFORMS> transforms;

    auto start_program= std::chrono::steady_clock::now();
    size_t loop_ctr(0);
    size_t max_runtime= 3600;

    size_t step_ctr = 0;
    while(time_passed(start_program) < max_runtime)
    {
        const size_t internal_loop = 600000000;
        cout << ++step_ctr << ":: start of a cycle (" << time_passed(start_program) << ") " <<  endl;
        MultiPoint p(0,0);
        constexpr size_t INNER_CTR = 32;
        MultiPoint saved[INNER_CTR];

        for(size_t point_ctr(0); point_ctr < internal_loop; point_ctr += INNER_CTR)
        {
            for(auto inner(0); inner < INNER_CTR; ++inner)
            {
                saved[inner] = p = transforms.move(p);
            }

            w.mark<INNER_CTR>(saved);
        }

        loop_ctr+= p.size() * internal_loop;
        std::cout << (loop_ctr / time_passed(start_program)) << " loops/sec " <<std::endl;

        w.print_stats();
        w.save_to_jpg(target_name.c_str());
    }
}
