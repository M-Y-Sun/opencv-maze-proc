#include <getopt.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "include/generate.hh"
#include "include/solve.hh"

static int log_level = 1;

static struct option opts[] = { { "verbose", no_argument, &log_level, 2 },
                                { "brief", no_argument, &log_level, 1 },
                                { "silent", no_argument, &log_level, 0 },
                                { "size", required_argument, 0, 'S' },
                                { 0, 0, 0, 0 } };

int
main (int argc, char **argv)
{
    int size = -1;

    int opt;
    while (true) {
        int idx = 0;
        opt     = getopt_long (argc, argv, "vbsS:", opts, &idx);

        if (opt == -1)
            break;

        switch (opt) {
            case 'v':
                log_level = 2;
                break;
            case 'b':
                log_level = 1;
                break;
            case 's':
                log_level = 0;
                break;
            case 'S':
                size = std::stoi (optarg);
                break;
            case '?':
                break;
        }
    }

    if (size == -1) {
        std::cerr << argv[0] << ": missing argument `--size' (aka `-S')\n";
        abort ();
    }

    if (log_level > 1) {
        std::cout << "[  \033[37;1mINFO\033[0m  ] set log level to "
                  << log_level << std::endl;
        if (size > 160) {
            std::cout << "\033[33;1mwarning:\033[0m high log levels may cause "
                         "significant I/O pressure and reduce performance. "
                         "continue? [Y/n]"
                      << std::flush;

            system ("stty raw");
            char conf = getchar ();
            system ("stty cooked");
            putchar (conf);
            if (conf != 'Y')
                return 0;
        }
    }

    cv::Mat maze = generate (size, log_level);

    cv::Mat resized;
    cv::resize (maze, resized, cv::Size (1024, 1024), 0, 0, cv::INTER_NEAREST);
    cv::imshow ("maze output", resized);
    char key = cv::waitKey (0);
    if (key == 's') {
        cv::imwrite ("assets/maze.jpg", resized);
        std::cout << "[   \033[32;1mOK\033[0m   ] write maze image to "
                     "`assets/maze.jpg'"
                  << std::endl;
    }

    resized = cv_morph_solve (maze);
    cv::resize (resized, resized, cv::Size (1024, 1024), 0, 0,
                cv::INTER_NEAREST);
    cv::imshow ("solved maze (using opencv morphology)", resized);
    cv::waitKey (0);
    if (key == 's') {
        cv::imwrite ("assets/solved_maze.jpg", resized);
        std::cout << "[   \033[32;1mOK\033[0m   ] write solved maze image to "
                     "`assets/solved_maze.jpg'"
                  << std::endl;
    }

    return 0;
}
