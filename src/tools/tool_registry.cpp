// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

ToolRegistry::ToolRegistry()    // Create initial buckets
    : buckets_(8), size_(0){

    }

ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {     // FNV-1a (spec Appendix A) or another real string hash
    std::size_t h = 14695981039346656037ULL;  // FNV offset basis

    for (char c : key) {
        h ^= static_cast<unsigned char>(c);
        h *= 1099511628211ULL;  // FNV prime
    }

    return h;
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if(tool == nullptr) {
        return false;
    }

    if(find(tool->name()) != nullptr) {
        return false;
    }

    if(size_ + 1 > buckets_.size()) {
        rehash(buckets_.size() * 2);
    }

    std::size_t index = hash(tool->name()) % buckets_.size();

    buckets_[index].push_back(std::move(tool));
    size_++;

    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    if(buckets_.empty()) {
        return nullptr;
    }

    std::size_t index = hash(name) % buckets_.size();

    for(const auto& tool : buckets_[index]) {
        if(tool->name() == name) {
            return tool.get();
        }
    }

    return nullptr;  
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;   
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_) {
        for (const auto& tool : bucket) {
            fn(*tool);
        }
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

double ToolRegistry::load_factor() const noexcept {
    if (buckets_.empty()) {
        return 0.0;
    }

    return static_cast<double>(size_) / buckets_.size();
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t max_length = 0;

    for (const auto& bucket : buckets_){
        if(bucket.size() > max_length) {
            max_length = bucket.size();
        }
    }
    return max_length;   
}

void ToolRegistry::rehash(std::size_t new_bucket_count) {
    std::vector<std::vector<std::unique_ptr<Tool>>> new_buckets(new_bucket_count);

    for (auto& bucket : buckets_) {
        for (auto& tool : bucket) {
            std::size_t index = hash(tool->name()) % new_bucket_count;
            new_buckets[index].push_back(std::move(tool));
        }
    }

    buckets_ = std::move(new_buckets);
}
