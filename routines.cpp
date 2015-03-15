#include "routines.h"
#include <iostream>
#include <string>
#include <regex>
#include <set>
#include <iterator>
#include <fstream>
#include <curl/curl.h>

size_t string_write(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

CURLcode curl_read(CURL* curl, const std::string& url, std::string& buffer, long timeout = 30)
{
    CURLcode code(CURLE_FAILED_INIT);

    if (curl) {
        if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str()))
                && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_write))
                && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer))
                && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
                && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
                && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout)))
        {
            code = curl_easy_perform(curl);
        }
    }
    return code;
}

std::string merge_url(const std::string& head, const std::string& tail)
{
    if (! tail.empty()) {
        if (tail.find("http") == 0) {
            return tail;
        } else if (tail.find("//") == 0) {
            return "http:" + tail;
        } else if (tail[0] != '/') {
            return head + '/' + tail;
        }

        size_t proto_pos = head.find("//");
        size_t first_slash_pos = head.find("/", proto_pos + 2);

        if (first_slash_pos != std::string::npos) {
            return head.substr(0, first_slash_pos) + tail;
        } else {
            return head + tail;
        }
    } else {
        return head;
    }
}

std::set<std::string> extract_links(const std::string& base_url, const std::string& content)
{
    std::regex url_re("<\\s*A\\s+[^>]*href\\s*=\\s*\"([^\"]*)\"", std::regex::icase);
    std::sregex_token_iterator iter(content.begin(), content.end(), url_re, 1);
    std::sregex_token_iterator iter_end;
    std::set<std::string> extracted_urls;

    for(; iter != iter_end; iter++) {
        std::string url = (*iter);
        url = merge_url(base_url, url);
        extracted_urls.insert(url);
    }

    return extracted_urls;
}

void reader_routine(WebCrawler *crawler)
{
    std::cout << "[READER "<< std::this_thread::get_id() << " start]" << std::endl;
    long timeout = 30;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init failed" << std::endl;
        return;
    }

    std::string url;
    while(EOU != (url = crawler->get_url()))
    {
        std::string content = "";
        if(CURLE_OK != curl_read(curl, url, content, timeout)) {
            content = "failed to load page";
            std::cerr << "cannot load page with url (" << url << ")" << std::endl;
        }
        Page page = {url, content};
        crawler->put_write_page(page);
    }
    Page end_page = {EOU, ""};
    crawler->put_write_page(end_page);
    curl_easy_cleanup(curl);
    std::cout << "[READER "<< std::this_thread::get_id() << " end]" << std::endl;
}

void writer_routine(WebCrawler* crawler,
                    std::string dir)
{
    std::cout << "[WRITER "<< std::this_thread::get_id() << " start]" << std::endl;
    Page page = crawler->get_write_page();
    while(EOU != page.url) {
        std::string filename = dir + "/page" + std::to_string(crawler->page_id_counter++);
        std::ofstream ofs(filename, std::ofstream::out);
        ofs << "[url " << page.url << "]" << std::endl;
        ofs << page.content;
        ofs.close();
        crawler->put_parse_page(page);
        page = crawler->get_write_page();
    }
    page = {EOU, ""};
    crawler->put_parse_page(page);
    std::cout << "[WRITER "<< std::this_thread::get_id() << " end]" << std::endl;
}

void parser_routine(WebCrawler *crawler)
{
    std::cout << "[PARSER "<< std::this_thread::get_id() << " start]" << std::endl;
    Page page = crawler->get_parse_page();
    while(EOU != page.url) {
        std::set<std::string> links = extract_links(page.url, page.content);
        crawler->put_url(links);
        page = crawler->get_parse_page();
    }
    std::cout << "[PARSER "<< std::this_thread::get_id() << " end]" << std::endl;
}
