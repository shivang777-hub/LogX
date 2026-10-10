// Standalone test for the LogX Trie (no framework). Build: C++11.
#include <iostream>
#include <string>
#include <vector>
#include "Trie.h"

using namespace logx;

static int g_failed = 0;
static int g_total = 0;
#define CHECK(cond)                                                          \
    do {                                                                     \
        ++g_total;                                                           \
        if (!(cond)) { ++g_failed; std::cout << "  FAIL: " #cond " (line "  \
                                             << __LINE__ << ")\n"; }         \
    } while (0)

// Frequency of an exact word, read from autocomplete results (getFrequency was removed).
static int freqOf(const Trie& t, const std::string& word) {
    const std::vector<WordFrequency> r = t.autocomplete(word);
    for (std::size_t i = 0; i < r.size(); ++i)
        if (r[i].first == word) return r[i].second;
    return -1;  // not found
}

static void print(const std::string& title, const std::vector<WordFrequency>& v) {
    std::cout << title << "\n";
    if (v.empty()) std::cout << "  (none)\n";
    for (std::size_t i = 0; i < v.size(); ++i)
        std::cout << "  " << v[i].first << " [freq=" << v[i].second << "]\n";
}

int main() {
    Trie t;
    t.insertWord("error", 150);
    t.insertWord("errorcode", 20);
    t.insertWord("errorlog", 30);
    t.insertWord("exception", 40);
    t.insertWord("database", 90);
    t.insertWord("disconnect", 15);
    t.insertWord("disk", 22);

    std::cout << "--- insertion & exact search ---\n";
    CHECK(t.searchWord("error"));
    CHECK(t.searchWord("errorcode"));
    CHECK(t.searchWord("disk"));
    CHECK(!t.searchWord("err"));      // path exists but not a stored word
    CHECK(!t.searchWord("errors"));   // longer than any stored word
    CHECK(!t.searchWord("warning"));  // never inserted

    std::cout << "--- autocomplete (ordering is alphabetical via std::map) ---\n";
    std::vector<WordFrequency> r = t.autocomplete("err");
    print("autocomplete(\"err\")", r);
    CHECK(r.size() == 3);
    CHECK(r[0].first == "error"     && r[0].second == 150);
    CHECK(r[1].first == "errorcode" && r[1].second == 20);
    CHECK(r[2].first == "errorlog"  && r[2].second == 30);

    r = t.autocomplete("di");
    print("autocomplete(\"di\")", r);
    CHECK(r.size() == 2);
    CHECK(r[0].first == "disconnect" && r[1].first == "disk");

    std::cout << "--- prefix that is also a complete word ---\n";
    r = t.autocomplete("error");
    CHECK(r.size() == 3);                       // error itself + errorcode + errorlog
    CHECK(r[0].first == "error");
    r = t.autocomplete("disk");
    CHECK(r.size() == 1 && r[0].first == "disk");  // word with no extensions

    std::cout << "--- missing prefixes ---\n";
    CHECK(t.autocomplete("xyz").empty());
    CHECK(t.autocomplete("errx").empty());      // diverges mid-path
    CHECK(t.autocomplete("errorlogs").empty()); // longer than stored word

    std::cout << "--- frequency accumulation ---\n";
    CHECK(freqOf(t, "error") == 150);
    t.insertWord("error", 5);
    CHECK(freqOf(t, "error") == 155);
    t.insertWord("error");                      // default frequency = 1
    CHECK(freqOf(t, "error") == 156);
    CHECK(t.autocomplete("err").size() == 3);   // no duplicate entry created
    t.insertWord("memory");
    CHECK(freqOf(t, "memory") == 1);

    std::cout << "--- inserting a prefix of an existing word ---\n";
    t.insertWord("err", 7);
    CHECK(t.searchWord("err"));
    r = t.autocomplete("err");
    print("autocomplete(\"err\") after inserting \"err\"", r);
    CHECK(r.size() == 4);
    CHECK(r[0].first == "err" && r[0].second == 7);

    std::cout << "--- empty input ---\n";
    CHECK(t.autocomplete("").empty());
    CHECK(!t.searchWord(""));
    Trie before;
    before.insertWord("", 10);                  // ignored
    CHECK(!before.searchWord(""));
    CHECK(before.autocomplete("a").empty());
    Trie emptyTrie;
    CHECK(emptyTrie.autocomplete("a").empty());
    CHECK(!emptyTrie.searchWord("a"));

    std::cout << "\n" << (g_total - g_failed) << "/" << g_total << " checks passed\n";
    return g_failed == 0 ? 0 : 1;
}
