#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include "webcrawler.h"

int main(int argc, char* argv[])
{
    if(5 > argc) {
        std::cout << "use: ./crawler [url] [depth] " <<
                     "[count] [dir] {page_readers_num parsers_num}" << std::endl;
        return 0;
    }
    std::string root_url(argv[1]);
    int depth = std::atoi(argv[2]);
    int count = std::atoi(argv[3]);
    std::string dir(argv[4]);
    WebCrawler crawler(root_url, count, depth, dir);
    int readers_num = 1;
    int parsers_num = 1;
    if(std::thread::hardware_concurrency() > 2) {
        readers_num = std::thread::hardware_concurrency() / 2;
        parsers_num = std::thread::hardware_concurrency() / 2;
    }
    if(argc == 6) {
        readers_num = std::atoi(argv[5]);
    }
    if(argc == 7) {
        parsers_num = std::atoi(argv[6]);
    }
    crawler.start(readers_num, parsers_num);
//    crawler.start(1, 1);
    return 0;
}

