#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include "webcrawler.h"

int main(int argc, char* argv[])
{
    if(5 > argc) {
        std::cout << "use: ./crawler [url] [depth] [count] [dir]" << std::endl;
        return 0;
    }
    std::string root_url(argv[1]);
    int depth = std::atoi(argv[2]);
    int count = std::atoi(argv[3]);
    std::string dir(argv[4]);
    WebCrawler crawler(root_url, count, depth, dir);
    int thread_num = std::thread::hardware_concurrency();
    crawler.start(1, 1);
    return 0;
}

