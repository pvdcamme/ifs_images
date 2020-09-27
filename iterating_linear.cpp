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
#include <fstream>
#include <string>
#include <chrono>

#include <cstdlib>
#include <cstdint>


float normal(){
    double upper = random() / double(RAND_MAX);
    return float(2 * upper - 1);
}

struct Point{
    float x,y;

    Point(float _x, float _y):
        x(_x), y(_y)
    {}
};

struct Linear{
    float a,b,c;
    float d,e,f;

    Linear():
        a(normal()), b(normal()), c(normal()),
        d(normal()), e(normal()), f(normal())
    { }

   struct Point move(struct Point& p){
        return Point(a* p.x + b * p.y + c,
                     d* p.x + e* p.y + f);
    }
};


// World from -1 to 1
template<size_t size>
struct World{
    uint8_t* data;
    uint32_t* full_data;

    World():
        data(new uint8_t[size * size]),
        full_data(new uint32_t[size * size])
    {
        for(size_t ctr(0); ctr < size * size; ++ ctr){
            data[ctr] = 0;
            full_data[ctr] = 0;
        }
    }

    ~World(){
        delete[] data;
    }

    void dump(){
        for(size_t ctr(0); ctr < size * size; ++ ctr){
            full_data[ctr] += data[ctr];
            data[ctr] = 0;
        }
    }


    void mark(struct Point& p){
        size_t ix = ((p.x + 1) * 0.5) * size;
        size_t iy = ((p.y + 1) * 0.5) * size;
        if (ix < size && iy < size){
            auto val = data[ix + size * iy]++;
            if(val > 250){
                dump();
            }
        }
    }

    void save(std::string name){
        std::ofstream out(name.c_str());

        dump();
        full_data[0] = 0;
        out << size << std::endl;
        for(size_t y(0); y < size; ++y){
            for(size_t x(0); x < size; ++x){
                out << full_data[y * size + x] << ", ";
            }
            out << std::endl;
        }
    }
};

template<typename clock>
double time_passed(std::chrono::time_point<clock> start){
    auto now = clock::now();
    std::chrono::duration<double> elapsed = now - start;
    return elapsed.count();
}

int main(int argc, char** argv){
    std::string target_name("result.txt");
    if(argc > 1){
        target_name = argv[1];
   }

    World<1024> w;
    const size_t transform_count= 16;
    Linear transforms[transform_count];

    auto start_program= std::chrono::steady_clock::now();
    size_t loop_ctr(0);
    while(time_passed(start_program) < 6){
        loop_ctr++;
        Point p(0,0);

        for(size_t point_ctr(0); point_ctr < 100000; ++point_ctr){
            size_t idx = random() % transform_count;
            p = transforms[idx].move(p);
            w.mark(p);
        }
    }
    std::cout << (loop_ctr / time_passed(start_program)) << " loops/sec " <<std::endl;
    w.save(target_name.c_str());
}
