#include <bbb/osc_path.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failure_count{0};

void expect_true(bool condition, const char *message) {
    if(condition) {
        return;
    }
    ++failure_count;
    std::cerr << "FAIL: " << message << "\n";
}

void expect_equal_string(const std::string &actual, const std::string &expected, const char *message) {
    if(actual == expected) {
        return;
    }
    ++failure_count;
    std::cerr << "FAIL: " << message
        << " expected " << expected
        << " got " << actual << "\n";
}

void expect_equal_size(std::size_t actual, std::size_t expected, const char *message) {
    if(actual == expected) {
        return;
    }
    ++failure_count;
    std::cerr << "FAIL: " << message
        << " expected " << expected
        << " got " << actual << "\n";
}

void expect_components(
    const std::vector<std::string> &actual,
    const std::vector<std::string> &expected,
    const char *message
) {
    if(actual == expected) {
        return;
    }
    ++failure_count;
    std::cerr << "FAIL: " << message << " expected";
    for(const auto &component : expected) {
        std::cerr << " [" << component << "]";
    }
    std::cerr << " got";
    for(const auto &component : actual) {
        std::cerr << " [" << component << "]";
    }
    std::cerr << "\n";
}

void test_normalize() {
    expect_equal_string(bbb::osc::path::normalize(""), "/", "empty path normalizes to root");
    expect_equal_string(bbb::osc::path::normalize("/"), "/", "root path normalizes to root");
    expect_equal_string(bbb::osc::path::normalize("foo"), "/foo", "relative path gains leading slash");
    expect_equal_string(bbb::osc::path::normalize("foo/bar"), "/foo/bar", "relative multi-component path gains leading slash");
    expect_equal_string(bbb::osc::path::normalize("/foo/bar/"), "/foo/bar", "trailing slash is removed");
    expect_equal_string(bbb::osc::path::normalize("///foo//bar///"), "/foo/bar", "duplicate slashes collapse");
}

void test_split_join_depth() {
    expect_components(bbb::osc::path::split("/"), {}, "root split is empty");
    expect_components(bbb::osc::path::split("foo/bar"), {"foo", "bar"}, "relative split is component-safe");
    expect_components(bbb::osc::path::split("///foo//bar///"), {"foo", "bar"}, "split normalizes duplicate slashes");

    expect_equal_string(bbb::osc::path::join({}), "/", "empty component join is root");
    expect_equal_string(bbb::osc::path::join({"foo", "bar"}), "/foo/bar", "component join");
    expect_equal_string(bbb::osc::path::join({"/foo", "bar"}), "/foo/bar", "component join strips leading slash per component");
    expect_equal_size(bbb::osc::path::depth("/foo/bar/baz"), 3, "path depth");
}

void test_transform_helpers() {
    expect_equal_string(bbb::osc::path::append("/foo", "bar"), "/foo/bar", "append simple component");
    expect_equal_string(bbb::osc::path::append("/foo", "/bar/baz"), "/foo/bar/baz", "append path component");
    expect_equal_string(bbb::osc::path::prepend("/bar", "foo"), "/foo/bar", "prepend simple component");
    expect_equal_string(bbb::osc::path::remove_head("/foo/bar/baz", 1), "/bar/baz", "remove one head component");
    expect_equal_string(bbb::osc::path::remove_head("/foo/bar/baz", 3), "/", "remove all head components");
    expect_equal_string(bbb::osc::path::remove_tail("/foo/bar/baz", 1), "/foo/bar", "remove one tail component");
    expect_equal_string(bbb::osc::path::remove_tail("/foo/bar/baz", 3), "/", "remove all tail components");
    expect_equal_string(bbb::osc::path::head("/foo/bar/baz", 2), "/foo/bar", "head two components");
    expect_equal_string(bbb::osc::path::tail("/foo/bar/baz", 2), "/bar/baz", "tail two components");
}

void test_prefix_matching() {
    expect_true(bbb::osc::path::starts_with_component("/foo/bar", "/foo"), "prefix matches component boundary");
    expect_true(bbb::osc::path::starts_with_component("/foo", "/foo"), "prefix matches exact path");
    expect_true(bbb::osc::path::starts_with_component("/foo/bar", "/"), "root prefix matches any path");
    expect_true(!bbb::osc::path::starts_with_component("/foobar", "/foo"), "prefix does not match partial component");
    expect_true(!bbb::osc::path::starts_with_component("/foo", "/foo/bar"), "longer prefix does not match shorter path");

    expect_equal_string(bbb::osc::path::strip_prefix("/foo/bar", "/foo"), "/bar", "strip component prefix");
    expect_equal_string(bbb::osc::path::strip_prefix("/foo", "/foo"), "/", "strip exact prefix");
    expect_equal_string(bbb::osc::path::strip_prefix("/foo/bar", "/"), "/foo/bar", "strip root prefix returns normalized path");
    expect_equal_string(bbb::osc::path::strip_prefix("/foobar", "/foo"), "", "strip rejects partial component prefix");
}

void test_path_like_detection() {
    expect_true(bbb::osc::path::is_path_like("/foo"), "slash-prefixed value is path-like");
    expect_true(!bbb::osc::path::is_path_like("foo"), "relative value is not path-like for Max atom auto-detect");
    expect_true(!bbb::osc::path::is_path_like(""), "empty value is not path-like");
}

} // namespace

int main() {
    test_normalize();
    test_split_join_depth();
    test_transform_helpers();
    test_prefix_matching();
    test_path_like_detection();

    if(failure_count != 0) {
        std::cerr << failure_count << " bbb.osc path test(s) failed\n";
        return 1;
    }

    std::cout << "bbb.osc path tests passed\n";
    return 0;
}
