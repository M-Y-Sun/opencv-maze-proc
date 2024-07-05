#ifndef GENERATE_H
#define GENERATE_H

#include <set>

#include <opencv2/core.hpp>

// macros
#define GN_UC_WHT         255
#define GN_UC_BLK         0
#define in_bounds(n, max) (n >= 0 && n < max)

enum class dir { LEFT = 0, RIGHT = 1, UP = 2, DOWN = 3 };
struct seg_t {
    int            id;
    std::set<int> *ptr;

    bool
    operator< (const seg_t &other) const
    {
        return id < other.id;
    }
};

static void initscan_ (cv::Mat *img, int rows, int cols);
static void merge_wcond_ (cv::Mat *maze, std::set<seg_t> *sets, int rows,
                          int cols, int set_r, int set_c, int max,
                          dir direction);
static void kruskal_ (cv::Mat *maze, int rows, int cols);
cv::Mat     generate (int size);

#endif
