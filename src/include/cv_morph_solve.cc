#include <cassert>
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "solve.hh"

cv::Mat
cv_morph_solve (cv::Mat src)
{
    if (src.depth () != CV_8U) {
        std::cerr << "[ \033[31;1mFAILED\033[0m ] image depth must be CV_8U "
                     "(uchar)\n";
        abort ();
    }

    if (src.channels () != 1) {
        std::cerr << "[ \033[31;1mFAILED\033[0m ] image must have 1 channel "
                     "(grayscale)\n";
        abort ();
    }

    cv::cvtColor (src, src, cv::COLOR_GRAY2BGR);

    // convert to binary image
    cv::Mat bin;
    cv::cvtColor (src, bin, cv::COLOR_BGR2GRAY);
    cv::threshold (bin, bin, 10, 255, cv::THRESH_BINARY_INV);

    // find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours (bin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    assert (contours.size () == 2);

    // draw the contours on a new image
    cv::Mat path = cv::Mat::zeros (src.size (), CV_8UC1);
    cv::drawContours (path, contours, 0, cv::Scalar (255, 255, 255),
                      cv::FILLED);

    // dilate and erode
    cv::Mat kernel = cv::Mat::ones (3, 3, CV_8UC1);
    cv::dilate (path, path, kernel);
    cv::Mat eroded;
    cv::erode (path, eroded, kernel);

    // cv::imshow ("dilated", path);
    // cv::waitKey (0);
    // cv::imshow ("eroded", eroded);
    // cv::waitKey (0);

    // subtract
    cv::absdiff (path, eroded, path);

    // cv::Mat elem = cv::getStructuringElement (cv::MORPH_RECT, cv::Size (3,
    // 3)); cv::morphologyEx (path, path, cv::MORPH_GRADIENT, elem);

    // draw the path
    std::vector<cv::Mat> channels;
    cv::split (src, channels);
    channels[0] = channels[0] & ~path;
    channels[1] = channels[1] & ~path;
    channels[2] = channels[2] | path;

    cv::Mat dst;
    cv::merge (channels, dst);

    return dst;
}
