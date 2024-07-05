#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "include/generate.hh"
#include "include/solve.hh"

int
main ()
{
    cv::Mat maze = generate (45);
    cv::resize (maze, maze, cv::Size (1024, 1024), 0, 0, cv::INTER_NEAREST);
    cv::imshow ("maze output", maze);
    char key = cv::waitKey (0);
    if (key == 's') {
        cv::imwrite ("assets/maze.jpg", maze);
        std::cout << "[   \033[32;1mOK\033[0m   ] write maze image to "
                     "`assets/maze.jpg'"
                  << std::endl;
    }

    cv::imshow ("solved maze (using opencv morphology)",
                cv_morph_solve (maze));
    cv::waitKey (0);
    if (key == 's') {
        cv::imwrite ("assets/solved_maze.jpg", maze);
        std::cout << "[   \033[32;1mOK\033[0m   ] write solved maze image to "
                     "`assets/solved_maze.jpg'"
                  << std::endl;
    }

    return 0;
}
