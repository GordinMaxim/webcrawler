#ifndef ROUTINES_H
#define ROUTINES_H
#include "webcrawler.h"

void reader_routine(WebCrawler* crawler);
void parser_routine(WebCrawler* crawler, std::atomic<int>& urls_left);
void writer_routine(WebCrawler* crawler, std::string dir);

#endif // ROUTINES_H
