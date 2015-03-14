#include "webcrawler.h"
#include <curl/curl.h>
#include <iostream>
#include "routines.h"

WebCrawler::WebCrawler(std::string root_url, int count, int level, std::string dir): level(level), dir(dir)
{
    curr_urls.push(root_url);
    stop_flag.store(false);
    semaphore = 0;
    page_id_counter = 0;
    urls_left = count;
    pages_left = count;
}

void WebCrawler::start(int readers_num, int parsers_num)
{
    std::cout << "Starting..." << std::endl;
    if(curl_global_init(CURL_GLOBAL_ALL)) {
        std::cerr << "curl init failed" << std::endl;
    }
    for(int i = 0; i < readers_num; i++) {
        readers.push_back(std::thread(reader_routine, this, std::ref(stop_flag)));
    }
    for(int i = 0; i < parsers_num; i++) {
        parsers.push_back(std::thread(parser_routine, this, std::ref(stop_flag), std::ref(urls_left)));
    }
    std::cout << "Crawling..." << std::endl;

    writer_routine(this, stop_flag, pages_left, dir);

    for(int i = 0; i < readers_num; i++) {
        readers[i].join();
    }
    for(int i = 0; i < parsers_num; i++) {
        parsers[i].join();
    }
    std::cout << "Stopping..." << std::endl;
    curl_global_cleanup();
}

std::string WebCrawler::get_url()
{
    std::unique_lock<std::mutex> ulock(urls_mutex);
    std::cout << "[get url] curr_urls before == " << curr_urls.size() << std::endl;
    if(curr_urls.empty()) {
        level--;
        if(0 == level) {
            stop_flag.store(true);
            return "";
        }
        sem_condvar.wait(ulock, [this]()->bool{ return 0 == semaphore; });
        curr_urls.swap(next_urls);
    }
    if(curr_urls.empty()) {
        //дерево закончилось, остаток ненулевой и глубина ненулевая
        return "";
    }
    std::string url = curr_urls.front();
    curr_urls.pop();
    std::cout << "[semaphore] before " << semaphore << std::endl;
    semaphore++;
    std::cout << "[semaphore] after " << semaphore << std::endl;
    std::cout << "[get url] curr_urls after == " << curr_urls.size() << std::endl;
    return url;
}

void WebCrawler::put_url(const std::set<std::string>& urls)
{
    std::lock_guard<std::mutex> lock(urls_mutex);
    std::cout << "[put url] next_urls before == " << next_urls.size() << std::endl;
    for(std::string url : urls) {
        if(visited_urls.end() != visited_urls.find(url)) {
            continue;
        }
        visited_urls.insert(url);
        next_urls.push(url);
    }

    std::cout << "[semaphore] before " << semaphore << std::endl;
    semaphore--;
    std::cout << "[semaphore] after " << semaphore << std::endl;
    sem_condvar.notify_one();
    std::cout << "[put url] next_urls after == " << next_urls.size() << std::endl;
}

Page WebCrawler::get_parse_page()
{
    std::unique_lock<std::mutex> ulock(parse_page_mutex);
    parse_pages_not_empty.wait(ulock, [this]()->bool{ return !parse_pages.empty(); });
    std::cout << "[get page] pages before == " << parse_pages.size() << std::endl;
    Page page = parse_pages.front();
    parse_pages.pop();
    std::cout << "[get page] pages after == " << parse_pages.size() << std::endl;
    return page;
}

void WebCrawler::put_parse_page(Page page)
{
    std::lock_guard<std::mutex> lock(parse_page_mutex);
    std::cout << "[put page] pages before == " << parse_pages.size() << std::endl;
    parse_pages.push(page);
    std::cout << "[put page] pages after == " << parse_pages.size() << std::endl;
    parse_pages_not_empty.notify_one();
}

Page WebCrawler::get_write_page()
{
    std::unique_lock<std::mutex> ulock(write_page_mutex);
    write_pages_not_empty.wait(ulock, [this]()->bool{ return !write_pages.empty(); });
    Page page = write_pages.front();
    write_pages.pop();
    return page;
}

void WebCrawler::put_write_page(Page page)
{
    std::lock_guard<std::mutex> lock(write_page_mutex);
    write_pages.push(page);
    write_pages_not_empty.notify_one();
}

WebCrawler::~WebCrawler()
{
}
