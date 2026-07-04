#pragma once

#include <string>
#include <vector>
#include <algorithm>

namespace bbb { namespace osc { namespace path {

// --- Normalization ---

inline std::string normalize(const std::string& path) {
    if(path.empty()) return "/";
    std::string result{"/"};
    result.reserve(path.size() + 1);
    bool previous_slash = true;
    for(char c : path) {
        if(c == '/') {
            if(!previous_slash) {
                result += '/';
                previous_slash = true;
            }
        } else {
            result += c;
            previous_slash = false;
        }
    }
    while(result.size() > 1 && result.back() == '/') result.pop_back();
    return result;
}

// --- Split / Join ---

inline std::vector<std::string> split(const std::string& path) {
    std::string n = normalize(path);
    if(n == "/") return {};
    std::vector<std::string> parts;
    size_t pos = 1;
    while(pos < n.size()) {
        size_t next = n.find('/', pos);
        if(next == std::string::npos) {
            parts.push_back(n.substr(pos));
            break;
        }
        parts.push_back(n.substr(pos, next - pos));
        pos = next + 1;
    }
    return parts;
}

inline std::string join(const std::vector<std::string>& components) {
    if(components.empty()) return "/";
    std::string result;
    for(const auto& c : components) {
        if(c.empty()) continue;
        result += '/';
        size_t start = (c[0] == '/') ? 1 : 0;
        result += c.substr(start);
    }
    return result.empty() ? "/" : result;
}

// --- Component count ---

inline size_t depth(const std::string& path) {
    return split(path).size();
}

// --- Transform ---

inline std::string append(const std::string& path, const std::string& component) {
    auto parts = split(path);
    std::string c = normalize("/" + component);
    auto cparts = split(c);
    for(auto& p : cparts) parts.push_back(p);
    return join(parts);
}

inline std::string prepend(const std::string& path, const std::string& component) {
    auto cparts = split(normalize("/" + component));
    auto parts = split(path);
    cparts.insert(cparts.end(), parts.begin(), parts.end());
    return join(cparts);
}

inline std::string remove_head(const std::string& path, size_t n) {
    auto parts = split(path);
    if(n >= parts.size()) return "/";
    parts.erase(parts.begin(), parts.begin() + static_cast<ptrdiff_t>(n));
    return join(parts);
}

inline std::string remove_tail(const std::string& path, size_t n) {
    auto parts = split(path);
    if(n >= parts.size()) return "/";
    parts.erase(parts.end() - static_cast<ptrdiff_t>(n), parts.end());
    return join(parts);
}

inline std::string head(const std::string& path, size_t n) {
    auto parts = split(path);
    if(n >= parts.size()) return normalize(path);
    parts.resize(n);
    return join(parts);
}

inline std::string tail(const std::string& path, size_t n) {
    auto parts = split(path);
    if(n >= parts.size()) return normalize(path);
    parts.erase(parts.begin(), parts.end() - static_cast<ptrdiff_t>(n));
    return join(parts);
}

// --- Matching ---

inline bool starts_with_component(const std::string& path, const std::string& prefix) {
    std::string np = normalize(path);
    std::string npfx = normalize(prefix);
    if(npfx == "/") return true;
    if(np == npfx) return true;
    if(np.size() <= npfx.size()) return false;
    return np.compare(0, npfx.size(), npfx) == 0 && np[npfx.size()] == '/';
}

inline std::string strip_prefix(const std::string& path, const std::string& prefix) {
    std::string np = normalize(path);
    std::string npfx = normalize(prefix);
    if(npfx == "/") return np;
    if(np == npfx) return "/";
    if(np.size() > npfx.size() && np.compare(0, npfx.size(), npfx) == 0 && np[npfx.size()] == '/') {
        return normalize(np.substr(npfx.size()));
    }
    return {};
}

// --- Path-like detection ---

inline bool is_path_like(const std::string& s) {
    return !s.empty() && s[0] == '/';
}

}}} // namespace bbb::osc::path
