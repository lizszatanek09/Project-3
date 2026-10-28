// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

TEST(reg_null_rejected){
    ToolRegistry r;

    CHECK(!r.add(std::unique_ptr<Tool>{}));
    CHECK_EQ(r.size(), std::size_t{0});
    CHECK(r.find("alpha") == nullptr);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;

    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));

    std::vector<std::string> seen;

    r.for_each([&seen](const Tool& tool) {
        seen.push_back(tool.name());
    });

    CHECK_EQ(seen.size(), std::size_t{3});

    for (const char* name : {"alpha", "beta", "gamma"}) {
        int count = 0;

        for (const std::string& visited : seen) {
            if (visited == name) {
                count++;
            }
        }

        CHECK_EQ(count, 1);
    }

    CHECK_EQ(r.size(), std::size_t{3});
}

TEST(reg_find_missing_tool) {
    ToolRegistry r;

    CHECK(r.find("missing") == nullptr);
    CHECK_EQ(r.size(), std::size_t{0});
}

TEST(reg_size_updates) {
    ToolRegistry r;

    CHECK_EQ(r.size(), std::size_t{0});

    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    CHECK_EQ(r.size(), std::size_t{1});

    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    CHECK_EQ(r.size(), std::size_t{2});

    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));
    CHECK_EQ(r.size(), std::size_t{3});
}

TEST(reg_scale_10000) {
    ToolRegistry r;

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);

        REQUIRE(r.add(std::make_unique<NamedTool>(name)));
        CHECK(r.load_factor() <= 1.0);
        CHECK(r.max_probe_length() <= 64);
    }

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);

        CHECK(r.find(name) != nullptr);
    }

    std::cout << "10,000-tool registry results:\n";
    std::cout << "  Bucket count: " << r.bucket_count() << '\n';
    std::cout << "  Load factor: " << r.load_factor() << '\n';
    std::cout << "  Maximum probe length: "
              << r.max_probe_length() << '\n';

    CHECK_EQ(r.size(), std::size_t{10000});
}

TEST(reg_weak_hash_comparison) {
    const std::size_t bucket_count = 16384;

    std::vector<std::size_t> bucket_sizes(bucket_count, 0);

    for (int i = 0; i < 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        std::size_t weak_hash = static_cast<unsigned char>(name[0]) + name.size();
        std::size_t index = weak_hash % bucket_count;
        bucket_sizes[index]++;
    }

    std::size_t longest_chain = 0;

    for (std::size_t count : bucket_sizes) {
        if (count > longest_chain) {
            longest_chain = count;
        }
    }

    std::cout << "Weak-hash comparison:\n";
    std::cout << "  Bucket count: " << bucket_count << '\n';
    std::cout << "  Longest chain: " << longest_chain << '\n';

    CHECK_EQ(bucket_sizes.size(), bucket_count);
}

TEST(ws_accepts_normal_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt");

    REQUIRE(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt");

    REQUIRE(ws.resolve("new/dir/file.txt", &out, &reason));
    CHECK(out == ws.root() / "new" / "dir" / "file.txt");

    REQUIRE(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt");
}

TEST(ws_accepts_root_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve(".", &out, &reason));
    CHECK(out == ws.root());

    REQUIRE(ws.resolve("a/..", &out, &reason));
    CHECK(out == ws.root());

    REQUIRE(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
}

TEST(ws_rejects_invalid_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    reason.clear();
    CHECK(!ws.resolve("", &out, &reason));
    CHECK(reason == "empty path");

    reason.clear();
    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());

    reason.clear();
    CHECK(!ws.resolve("..", &out, &reason));
    CHECK(!reason.empty());

    reason.clear();
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());

    reason.clear();
    CHECK(!ws.resolve("a/../../b", &out, &reason));
    CHECK(!reason.empty());
}


TEST(ws_rejects_embedded_null) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    // Create a path containing an embedded null character.
    std::string bad_path("notes\0/todo.txt", 15);

    CHECK(!ws.resolve(bad_path, &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_subtraction_left_to_right) {
    double v = 0;
    std::string err;

    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);
}

TEST(calc_division_left_to_right) {
    double v = 0;
    std::string err;

    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
}

TEST(calc_parentheses) {
    double v = 0;
    std::string err;

    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);
}

TEST(calc_nested_expression) {
    double v = 0;
    std::string err;

    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
}

TEST(calc_negative_numbers) {
    double v = 0;
    std::string err;

    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
}

TEST(calc_decimal_result) {
    double v = 0;
    std::string err;

    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
}

TEST(calc_division_by_zero_expression) {
    double v = 0;
    std::string err;

    CHECK(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
}

TEST(reg_pointer_stability_after_resize) {
    ToolRegistry r;

    REQUIRE(r.add(std::make_unique<NamedTool>("original")));
    Tool* original = r.find("original");

    REQUIRE(original != nullptr);

    for (int i = 0; i < 1000; i++) {
        std::string name = "extra_" + std::to_string(i);

        REQUIRE(r.add(std::make_unique<NamedTool>(name)));
    }

    CHECK(r.find("original") == original);
    CHECK(original->name() == "original");
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
