#include "core/sentinel_scanner.h"

#include <string>
#include <string_view>
#include <utility>

SentinelScanner::SentinelScanner(std::string sentinel) : sentinel_(std::move(sentinel)), pending_(""){}

// Feed
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk){
    Out result;
    result.safe_text = "";
    result.sentinel_found = false;
    pending_ += chunk;
    std::size_t pos = pending_.find(sentinel_);

    if (pos != std::string::npos){
        result.safe_text = pending_.substr(0, pos);
        result.sentinel_found = true;
        pending_.clear();
        return result;
    }

    if (sentinel_.empty()){
        result.safe_text = pending_;
        pending_.clear();
        return result;
    }

    std::size_t keep = sentinel_.size() - 1;

    if (pending_.size() > keep){
        std::size_t safe_count = pending_.size() - keep;
        result.safe_text = pending_.substr(0, safe_count);
        pending_.erase(0, safe_count);
    }

    return result;
}

// Flush
SentinelScanner::Out SentinelScanner::flush(){
    Out result;
    result.safe_text = pending_;
    result.sentinel_found = false;
    pending_.clear();
    return result;
}