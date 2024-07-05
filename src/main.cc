#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "include/generate.hh"

int
main ()
{
    cv::Mat maze = generate (45);
    cv::resize (maze, maze, cv::Size (1024, 1024), 0, 0, cv::INTER_NEAREST);
    cv::imshow ("maze output", maze);
    cv::waitKey (0);

    return 0;
}
