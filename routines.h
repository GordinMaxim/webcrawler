#ifndef ROUTINES_H
#define ROUTINES_H
#include "webcrawler.h"

void reader_routine(WebCrawler* crawler, std::atomic<bool>& stop_flag);
void parser_routine(WebCrawler* crawler, std::atomic<bool>& stop_flag, std::atomic<int>& urls_left);
void writer_routine(WebCrawler* crawler, std::atomic<bool>& stop_flag, std::atomic<int>& pages_left, std::string dir);

#endif // ROUTINES_H
