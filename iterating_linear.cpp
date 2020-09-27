#include <iostream>
#include <stdlib.h>
#include <cstdint>
#include <fstream>
#include <string>

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
            data[ix + size * iy]++;
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

int main(){
    World<1024> w;
    const size_t transform_count= 16;
    Linear transforms[transform_count];

    for(size_t loop_ctr(0); loop_ctr < 1000; ++loop_ctr){
        Point p(0,0);

        for(size_t point_ctr(0); point_ctr < 100000; ++point_ctr){
            size_t idx = random() % transform_count;
            p = transforms[idx].move(p);
            w.mark(p);
        }
    }
    w.save("test.txt");
}
