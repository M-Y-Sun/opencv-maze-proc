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

#include "generate.hh"

static void
initscan_ (cv::Mat *img, int rows, int cols)
{
    CV_Assert (img->depth () == CV_8U && img->channels () == 1);

    assert (rows > 0 && rows & 1);
    assert (cols > 0 && cols & 1);

    for (int i = 1; i < rows; i = i + 2) {
        uchar *p = img->ptr<uchar> (i);
        for (int j = 1; j < cols; j = j + 2)
            p[j] = GN_UC_WHT;
    }
}

static void
merge_wcond_ (cv::Mat *maze, std::set<seg_t> *sets, int rows, int cols,
              int set_r, int set_c, int max, dir direction)
{
    int idx       = set_r * cols + set_c;
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

    int next       = set_r * cols + set_c;
    int mat_r_next = 2 * set_r + 1;
    int mat_c_next = 2 * set_c + 1;

    assert (in_bounds (mat_r_cur, maze->rows)
            && in_bounds (mat_c_cur, maze->cols));
    assert (in_bounds (mat_r_next, maze->rows)
            && in_bounds (mat_c_next, maze->cols));

    // if idx and next don't point to the same set (not connected in the maze)
    seg_t cur_seg, next_seg;
    for (seg_t seg : *sets) {
        if (seg.ptr->count (idx))
            cur_seg = seg;
        if (seg.ptr->count (next))
            next_seg = seg;
    }

    assert (cur_seg.ptr != nullptr && next_seg.ptr != nullptr);

    if (cur_seg.ptr != next_seg.ptr) {
        // remove the wall between the two indices in the maze matrix
        std::cout << "[  \033[37;1mINFO\033[0m  ] write to pixel ("
                  << (mat_r_cur + mat_r_next) / 2 << ','
                  << (mat_c_cur + mat_c_next) / 2 << ") of " << idx << " and "
                  << next << '\n';
        maze->ptr<uchar> ((mat_r_cur + mat_r_next)
                          / 2)[(mat_c_cur + mat_c_next) / 2]
            = GN_UC_WHT;

        // clean up
        cur_seg.ptr->merge (*next_seg.ptr);  // merge the sets
        next_seg.ptr->~set ();               // destroy the old set
        delete next_seg.ptr;                 // free the pointer to the set
        sets->erase (sets->find (next_seg)); // erase the old set
    } else {
        std::cout << "[  \033[37;1mINFO\033[0m  ] already processed " << idx
                  << " and " << next << '\n';
    }
}

static void
kruskal_ (cv::Mat *maze, int rows, int cols)
{
    int set_r = (rows - 1) / 2;
    int set_c = (rows - 1) / 2;
    int len   = set_r * set_c;
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

    std::random_device              rd;
    std::mt19937                    gen (rd ());
    std::uniform_int_distribution<> r_distr (0, set_r - 1);
    std::uniform_int_distribution<> c_distr (0, set_c - 1);
    std::shuffle (shuffled_cells.begin (), shuffled_cells.end (),
                  std::default_random_engine ());

    // join the sets

    // ------ all possible connections (n^2) ------ //
    //
    // for (std::pair<int, int> cell : shuffled_cells)
    //     for (int i = 0; i < 4; ++i)
    //         merge_wcond_ (maze, &sets, set_r, set_c, cell.first,
    //         cell.second,
    //                       len, (dir)i);

    // ------ random ------- //
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

    // initialize the maze by creating a grid of walls
    initscan_ (&maze, size, size);
    std::cout << "[   \033[32;1mOK\033[0m   ] scan maze matrix" << std::endl;

    // cv::Mat cpy;
    // cv::resize (maze, cpy, cv::Size (1024, 1024), 0, 0, cv::INTER_NEAREST);
    // cv::imshow ("initscan_ output", cpy);
    // cv::waitKey (0);

    // generate the maze using kruskal's algorithm to remove walls
    kruskal_ (&maze, size, size);
    std::cout << "[   \033[32;1mOK\033[0m   ] finish maze matrix generation"
              << std::endl;

    // add start and end as the top right and bottom left, respectively
    maze.ptr<uchar> (0)[1]                         = GN_UC_WHT;
    maze.ptr<uchar> (maze.rows - 1)[maze.cols - 2] = GN_UC_WHT;

    return maze;
}
