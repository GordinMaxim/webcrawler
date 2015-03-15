#ifndef WEBCRAWLER_H
#define WEBCRAWLER_H
#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include <unordered_set>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <set>

#define EOU "EOF"

struct Page {
    std::string url;
    std::string content;
};

class WebCrawler
{
public:
    std::atomic<int> page_id_counter;

public:
    WebCrawler(std::string root_url, int urls_left, int level, std::string dir);
    void start(int, int);
    std::string get_url();
    void put_url(const std::set<std::string>& url);
    Page get_parse_page();
    void put_parse_page(Page page);
    Page get_write_page();
    void put_write_page(Page page);
    ~WebCrawler();

private:
    std::atomic<int> urls_left;
    std::atomic<int> pages_left;
    std::atomic<int> level;
    std::string dir;
    std::vector<std::thread> readers;
    std::vector<std::thread> parsers;
    unsigned int semaphore;
    std::condition_variable sem_condvar;
    std::unordered_set<std::string> visited_urls;
    std::queue<std::string> curr_urls;
    std::queue<std::string> next_urls;
    std::queue<Page> parse_pages;
    std::queue<Page> write_pages;
    std::atomic<bool> stop_read;
    std::atomic<bool> stop_write;
    std::atomic<bool> stop_parse;
    std::mutex urls_mutex;
    std::mutex parse_page_mutex;
    std::mutex write_page_mutex;
    std::condition_variable parse_pages_not_empty;
    std::condition_variable write_pages_not_empty;
};

#endif // WEBCRAWLER_H
