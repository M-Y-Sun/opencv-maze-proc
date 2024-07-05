#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <utility>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define UC_WHT 255
#define UC_BLK 0

#define in_bounds(n, max) (n >= 0 && n < max)

enum class dir { LEFT = 0, RIGHT = 1, UP = 2, DOWN = 3 };

struct seg_t {
    int id;
    std::set<int> *ptr;

    bool
    operator< (const seg_t &other) const
    {
        return id < other.id;
    }
};

static void
initscan_ (cv::Mat *img, int rows, int cols)
{
    CV_Assert (img->depth () == CV_8U && img->channels () == 1);

    assert (rows > 0 && rows & 1);
    assert (cols > 0 && cols & 1);

    for (int i = 1; i < rows; i = i + 2) {
        uchar *p = img->ptr<uchar> (i);
        for (int j = 1; j < cols; j = j + 2)
            p[j] = UC_WHT;
    }
}

static void
merge_wcond_ (cv::Mat *maze, std::set<seg_t> *sets, int rows, int cols,
              int set_r, int set_c, int max, dir direction)
{
    int idx = set_r * cols + set_c;
    int mat_r_cur = 2 * set_r + 1;
    int mat_c_cur = 2 * set_c + 1;
    switch (direction) {
        case dir::LEFT:
            set_c = set_c - 1;
            break;
        case dir::RIGHT:
            set_c = set_c + 1;
            break;
        case dir::UP:
            set_r = set_r - 1;
            break;
        case dir::DOWN:
            set_r = set_r + 1;
            break;
    }

    if (!in_bounds (set_r, rows) || !in_bounds (set_c, cols)) {
        std::cout << "[  \033[37;1mINFO\033[0m  ] ignored neighbor (" << set_r
                  << ',' << set_c << ")\n";
        return;
    }

    int next = set_r * cols + set_c;
    int mat_r_next = 2 * set_r + 1;
    int mat_c_next = 2 * set_c + 1;

    assert (in_bounds (mat_r_cur, maze->rows)
            && in_bounds (mat_c_cur, maze->cols));
    assert (in_bounds (mat_r_next, maze->rows)
            && in_bounds (mat_c_next, maze->cols));

    // if idx and next don't point to the same set (not connected in the maze)
    std::set<seg_t>::iterator it_cur = sets->lower_bound ({ idx, nullptr });
    std::set<seg_t>::iterator it_next = sets->lower_bound ({ next, nullptr });
    if (it_cur != it_next) {
        // next must be less than idx since we are deleting next
        if (it_next->id > it_cur->id)
            swap (it_cur, it_next);

        // remove the wall between the two indices in the maze matrix
        std::cout << "[  \033[37;1mINFO\033[0m  ] write to pixel ("
                  << (mat_r_cur + mat_r_next) / 2 << ','
                  << (mat_c_cur + mat_c_next) / 2 << ") of " << idx << " and "
                  << next << '\n';
        maze->ptr<uchar> ((mat_r_cur + mat_r_next)
                          / 2)[(mat_c_cur + mat_c_next) / 2]
            = 127;

        // clean up
        it_cur->ptr->merge (*it_next->ptr); // merge the sets
        it_next->ptr->~set ();              // destroy the old set
        delete it_next->ptr;                // free the pointer to the set
        sets->erase (it_next);              // erase the old set
    } else {
        std::cout << "[  \033[37;1mINFO\033[0m  ] already processed " << idx
                  << " and " << next << " with lower bound " << it_cur->id
                  << '\n';
    }
}

static void
kruskal_ (cv::Mat *maze, int rows, int cols)
{
    int set_r = (rows - 1) / 2;
    int set_c = (rows - 1) / 2;
    int len = set_r * set_c;
    // vector of heap pointers to the sets
    std::set<seg_t> sets;
    for (int i = 0; i < len; ++i)
        sets.insert ({ i, new std::set<int> ({ i }) });

    // random order of traversal for the random maze
    std::vector<std::pair<int, int> > shuffled_cells (len,
                                                      std::pair<int, int> ());
    for (int i = 0; i < set_r; ++i)
        for (int j = 0; j < set_c; ++j)
            shuffled_cells[i * set_c + j] = { i, j };

    std::random_device rd;
    std::mt19937 gen (rd ());
    std::uniform_int_distribution<> r_distr (0, set_r - 1);
    std::uniform_int_distribution<> c_distr (0, set_c - 1);
    // std::shuffle (shuffled_cells.begin (), shuffled_cells.end (),
    //               std::default_random_engine ());

    // join the sets
    // for (std::pair<int, int> cell : shuffled_cells)
    //     for (int i = 0; i < 4; ++i)
    //         merge_wcond_ (maze, &sets, set_r, set_c, cell.first,
    //         cell.second,
    //                       len, (dir)i);
    while (sets.size () > 1)
        for (int i = 0; i < 4; ++i)
            merge_wcond_ (maze, &sets, set_r, set_c, r_distr (gen),
                          c_distr (gen), len, (dir)i);

    // free the set's memory
    delete sets.begin ()->ptr;
}

cv::Mat
generate (int size)
{
    if (size < 3) {
        std::cerr << "[ \033[31;1mFAILED\033[0m ] `size' must be at least 3 "
                     "(received `"
                  << size << "')\n";
        abort ();
    }

    if (!(size & 1)) {
        std::cerr
            << "[ \033[31;1mFAILED\033[0m ] `size' must be odd (recieved `"
            << size << "')\n";
        abort ();
    }

    cv::Mat maze = cv::Mat::zeros (size, size, CV_8UC1);
    std::cout << "[   \033[32;1mOK\033[0m   ] initialize maze matrix"
              << std::endl;

    initscan_ (&maze, size, size);
    std::cout << "[   \033[32;1mOK\033[0m   ] scan maze matrix" << std::endl;
    // cv::Mat cpy;
    // cv::resize (maze, cpy, cv::Size (256, 256), 0, 0, cv::INTER_NEAREST);
    // cv::imshow ("initscan_ output", cpy);
    // cv::waitKey (0);

    kruskal_ (&maze, size, size);
    std::cout << "[   \033[32;1mOK\033[0m   ] finish maze matrix generation"
              << std::endl;

    return maze;
}

int
main ()
{
    cv::Mat maze = generate (7);
    cv::resize (maze, maze, cv::Size (256, 256), 0, 0, cv::INTER_NEAREST);
    cv::imshow ("maze output", maze);
    cv::waitKey (0);

    return 0;
}
