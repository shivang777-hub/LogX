// =============================================================================
// TrieTest.cpp  -  automated tests for SearchLogX::Trie, driven by a real log file
//
// Build:  g++ -std=c++17 -Wall -Wextra -pedantic Trie.cpp TrieTest.cpp -o TrieTest
// Run:    ./TrieTest sample.log        (or just ./TrieTest -> uses "sample.log")
//
// TOKENIZATION POLICY (how a log line becomes words)
//   * Word characters : ASCII letters a-z / A-Z, digits 0-9, and underscore '_'.
//   * Everything else (space, tab, '\r', , . : ; [ ] ( ) " - = / etc.) SEPARATES words.
//       "10:42:01,123"      -> 10, 42, 01, 123
//       "[db-pool]"         -> db, pool
//       "192.168.1.10"      -> 192, 168, 1, 10
//   * Digits are kept:        "30s" stays "30s", "2024" stays "2024".
//   * Underscores are kept:   "user_id" stays one word (common in log field names).
//   * Apostrophes are REMOVED and do NOT split:  "don't" -> "dont", 'quoted' -> quoted
//   * Letters are lower-cased: ERROR, Error, error all become "error".
//   * Non-ASCII bytes (e.g. UTF-8 letters) are treated as separators.
//   * A token made only of underscores (like "___") is discarded.
//
// The Trie stores word MEMBERSHIP only. Inserting "error" 100 times still leaves
// exactly one "error" in the Trie. Occurrence counts live in this test program.
// =============================================================================

#include "Trie.h"

#include <cstddef>
#include <cstdio>   // std::remove (delete temporary files)
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using SearchLogX::Trie;

namespace {

const std::string kDefaultLogFile = "sample.log";

// =============================================================================
// PART 1 - A tiny test framework (prints PASS / FAIL and counts them)
// =============================================================================

int g_passed = 0;
int g_failed = 0;

void report(bool ok, const std::string& name) {
    if (ok) {
        ++g_passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++g_failed;
        std::cout << "[FAIL] " << name << '\n';
    }
}

void expectTrue(bool condition, const std::string& name)  { report(condition, name); }
void expectFalse(bool condition, const std::string& name) { report(!condition, name); }

void expectEqualCount(std::size_t actual, std::size_t expected, const std::string& name) {
    const bool ok = (actual == expected);
    report(ok, name);
    if (!ok)
        std::cout << "       expected " << expected << " but got " << actual << '\n';
}

std::string listToString(const std::vector<std::string>& items) {
    std::string text = "{";
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) text += ", ";
        text += items[i];
    }
    return text + "}";
}

void expectEqualList(const std::vector<std::string>& actual,
                     const std::vector<std::string>& expected,
                     const std::string& name) {
    const bool ok = (actual == expected);
    report(ok, name);
    if (!ok) {
        std::cout << "       expected " << listToString(expected) << '\n'
                  << "       but got  " << listToString(actual) << '\n';
    }
}

// =============================================================================
// PART 2 - Tokenization and normalization (NO file access, NO Trie here)
// =============================================================================

bool isWordChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

char toLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z')
        return static_cast<char>(c - 'A' + 'a');
    return c;
}

// A token is only valid if it has at least one letter or digit.
// (Tokens contain only letters, digits and '_', so "not only underscores" is enough.)
bool hasLetterOrDigit(const std::string& token) {
    for (char c : token)
        if (c != '_') return true;
    return false;
}

void finishToken(std::string& current, std::vector<std::string>& tokens) {
    if (hasLetterOrDigit(current))
        tokens.push_back(current);
    current.clear();
}

// Split ONE line into normalized words. Repeated words are all returned.
std::vector<std::string> tokenizeLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;

    for (char c : line) {
        if (isWordChar(c)) {
            current.push_back(toLowerAscii(c));   // keep, lower-cased
        } else if (c == '\'') {
            // apostrophe: skip it, but do NOT end the word ("don't" -> "dont")
        } else {
            finishToken(current, tokens);         // any other char ends the word
        }
    }
    finishToken(current, tokens);                 // last word on the line
    return tokens;
}

// =============================================================================
// PART 3 - Reading a log file and inserting words into the Trie
// =============================================================================

struct LogStats {
    std::size_t lines = 0;        // lines read from the file
    std::size_t totalWords = 0;   // every word occurrence (repeats counted)
    // Test-only reference list of distinct words, used to double-check the Trie.
    // (The Trie itself does not count words.)
    std::set<std::string> uniqueWords;
};

// Reads the WHOLE file one line at a time (never loads the entire file at once).
// Returns false and fills errorMessage if the file cannot be opened/read.
bool processLogFile(const std::string& path, Trie& trie,
                    LogStats& stats, std::string& errorMessage) {
    std::ifstream file(path);
    if (!file.is_open()) {
        errorMessage = "Cannot open log file '" + path +
                       "'. Check that the path is correct and the file is readable.";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {            // loops until the LAST line
        ++stats.lines;
        for (const std::string& word : tokenizeLine(line)) {
            trie.insertWord(word);                // membership only
            ++stats.totalWords;
            stats.uniqueWords.insert(word);
        }
    }

    if (file.bad()) {                             // real I/O error (not normal end of file)
        errorMessage = "A read error occurred while reading '" + path + "'.";
        return false;
    }
    return true;
}

// =============================================================================
// PART 4 - Helpers for the tests
// =============================================================================

bool writeTextFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);    // binary: keep "\r\n" exactly as written
    if (!out) return false;
    out << content;
    out.close();
    return !out.fail();
}

// What autocomplete(prefix) SHOULD return, computed from the sorted std::set.
std::vector<std::string> expectedAutocomplete(const std::set<std::string>& words,
                                              const std::string& prefix) {
    std::vector<std::string> result;
    for (auto it = words.lower_bound(prefix);
         it != words.end() && it->compare(0, prefix.size(), prefix) == 0; ++it)
        result.push_back(*it);
    return result;
}

void showAutocomplete(const Trie& trie, const std::string& prefix) {
    std::cout << "       autocomplete(\"" << prefix << "\") -> "
              << listToString(trie.autocomplete(prefix)) << '\n';
}

// =============================================================================
// PART 5 - Tests
// =============================================================================

// ---- 5A: Trie behaviour on hand-made words ---------------------------------

void testNormalWordAndMissingWord() {
    Trie t;
    t.insertWord("error");
    expectTrue(t.searchWord("error"),    "normal word: 'error' is found after insertion");
    expectFalse(t.searchWord("warning"), "missing word: 'warning' (never inserted) is not found");
}

void testDuplicateInsertion() {
    Trie t;
    t.insertWord("timeout");
    t.insertWord("timeout");
    t.insertWord("timeout");
    expectTrue(t.searchWord("timeout"), "duplicate insertion: word is still found");
    expectEqualList(t.autocomplete("timeout"), {"timeout"},
                    "duplicate insertion: stored once (Trie keeps membership, not counts)");
}

void testEmptyString() {
    Trie t;
    t.insertWord("");
    expectFalse(t.searchWord(""), "empty string: searchWord(\"\") is false");
    expectEqualCount(t.autocomplete("").size(), 0, "empty string: autocomplete(\"\") returns nothing");
    t.insertWord("abc");
    expectFalse(t.searchWord(""), "empty string: searchWord(\"\") still false after other inserts");
    expectEqualCount(t.autocomplete("").size(), 0,
                     "empty string: autocomplete(\"\") still empty after other inserts");
}

void testPrefixes() {
    Trie t;
    t.insertWord("error");
    t.insertWord("errors");
    t.insertWord("errand");
    expectFalse(t.searchWord("err"), "existing prefix: 'err' is a prefix, not a stored word");
    expectEqualList(t.autocomplete("err"), {"errand", "error", "errors"},
                    "existing prefix: autocomplete('err') returns all 3 words, sorted");
    expectEqualCount(t.autocomplete("xyz").size(), 0, "missing prefix: 'xyz' gives no results");
    expectEqualCount(t.autocomplete("errz").size(), 0,
                     "missing prefix: 'errz' (diverges part-way) gives no results");
    expectEqualCount(t.autocomplete("errorsx").size(), 0,
                     "missing prefix: longer than any stored word gives no results");
    expectFalse(t.searchWord("errorsx"), "search: word longer than any stored word is not found");
}

void testWordThatIsAlsoPrefix() {
    Trie t;
    t.insertWord("cart");   // inserted in scrambled order on purpose
    t.insertWord("care");
    t.insertWord("car");
    t.insertWord("card");
    expectTrue(t.searchWord("car"),  "word-and-prefix: 'car' is found although longer words exist");
    expectFalse(t.searchWord("ca"),  "word-and-prefix: 'ca' is NOT a word");
    expectEqualList(t.autocomplete("car"), {"car", "card", "care", "cart"},
                    "word-and-prefix: autocomplete('car') lists 'car' itself first, then the rest");
    expectEqualList(t.autocomplete("card"), {"card"},
                    "word-and-prefix: autocomplete('card') returns just 'card'");
}

void testAutocompleteMultipleMatchesSorted() {
    Trie t;
    t.insertWord("connection");   // scrambled order; std::map still yields sorted output
    t.insertWord("connect");
    t.insertWord("console");
    t.insertWord("conn");
    t.insertWord("connecting");
    t.insertWord("connected");
    expectEqualList(t.autocomplete("conn"),
                    {"conn", "connect", "connected", "connecting", "connection"},
                    "multiple matches: autocomplete('conn') is in sorted order");
    expectEqualList(t.autocomplete("con"),
                    {"conn", "connect", "connected", "connecting", "connection", "console"},
                    "multiple matches: autocomplete('con') includes every branch, sorted");
}

void testRawTrieIsCaseSensitive() {
    Trie t;
    t.insertWord("error");
    expectFalse(t.searchWord("ERROR"), "case: the raw Trie is case-sensitive ('ERROR' not found)");
    expectFalse(t.searchWord("Error"), "case: the raw Trie is case-sensitive ('Error' not found)");
    expectEqualCount(t.autocomplete("ERR").size(), 0,
                     "case: autocomplete('ERR') finds nothing (normalise BEFORE inserting)");
}

// ---- 5B: Tokenizer ---------------------------------------------------------

void testTokenizer() {
    expectEqualList(tokenizeLine("ERROR Error error"), {"error", "error", "error"},
                    "tokenizer: lower-cases and keeps repeated words");
    expectEqualList(tokenizeLine("Connection failed: timeout (30s), retrying..."),
                    {"connection", "failed", "timeout", "30s", "retrying"},
                    "tokenizer: colon, parentheses, comma and dots separate words");
    expectEqualList(tokenizeLine("[WARN] user=\"alice\"; path=/var/log"),
                    {"warn", "user", "alice", "path", "var", "log"},
                    "tokenizer: brackets, quotes, semicolon, '=' and '/' separate words");
    expectEqualList(tokenizeLine("time-out well-known"), {"time", "out", "well", "known"},
                    "tokenizer: hyphen separates words");
    expectEqualList(tokenizeLine("2024-03-15 10:42:01,123"),
                    {"2024", "03", "15", "10", "42", "01", "123"},
                    "tokenizer: timestamp pieces become digit words (leading zeros kept)");
    expectEqualList(tokenizeLine("user_id=42 pool_size"), {"user_id", "42", "pool_size"},
                    "tokenizer: underscores stay inside a word");
    expectEqualList(tokenizeLine("Don't stop 'quoted'"), {"dont", "stop", "quoted"},
                    "tokenizer: apostrophes are removed and do not split words");
    expectEqualList(tokenizeLine("192.168.1.10"), {"192", "168", "1", "10"},
                    "tokenizer: dots separate (IP address -> four numbers)");
    expectEqualList(tokenizeLine(""), {}, "tokenizer: empty line gives no words");
    expectEqualList(tokenizeLine("  ,;;--  ()  "), {},
                    "tokenizer: whitespace/punctuation-only line gives no words");
    expectEqualList(tokenizeLine("___ ---- '''"), {},
                    "tokenizer: underscore-only and apostrophe-only tokens are discarded");
    expectEqualList(tokenizeLine("a\tb\r"), {"a", "b"},
                    "tokenizer: tab and carriage return (Windows line ends) are separators");
    expectEqualList(tokenizeLine("caf\xC3\xA9 ok"), {"caf", "ok"},
                    "tokenizer: non-ASCII bytes act as separators");
}

void testNormalisationFeedsTrie() {
    Trie t;
    const std::vector<std::string> lines = {"ERROR disk full", "Error: DISK Full!", "error (disk) FULL"};
    for (const std::string& line : lines)
        for (const std::string& word : tokenizeLine(line))
            t.insertWord(word);

    expectTrue(t.searchWord("error"), "normalisation: 'error' found (from ERROR / Error / error)");
    expectTrue(t.searchWord("disk"),  "normalisation: 'disk' found (from separate lines)");
    expectTrue(t.searchWord("full"),  "normalisation: 'full' found (punctuation removed)");
    expectFalse(t.searchWord("ERROR"), "normalisation: upper-case form was never stored");
    expectEqualList(t.autocomplete("e"), {"error"},
                    "normalisation: three spellings of 'error' produce ONE stored word");
}

// ---- 5C: File handling (missing, empty, multi-line, large) -----------------

void testMissingFile() {
    Trie t;
    LogStats stats;
    std::string error;
    const bool ok = processLogFile("definitely_missing_file.log", t, stats, error);
    expectFalse(ok, "missing file: processLogFile reports failure");
    expectTrue(!error.empty(), "missing file: a useful error message is produced");
    std::cout << "       message: " << error << '\n';
    expectEqualCount(stats.totalWords, 0, "missing file: no words were processed");
}

void testEmptyFile() {
    const std::string path = "TrieTest_tmp_empty.log";
    expectTrue(writeTextFile(path, ""), "empty file: temporary file created");

    Trie t;
    LogStats stats;
    std::string error;
    const bool ok = processLogFile(path, t, stats, error);
    expectTrue(ok, "empty file: opens and reads without error");
    expectEqualCount(stats.lines, 0, "empty file: 0 lines");
    expectEqualCount(stats.totalWords, 0, "empty file: 0 word occurrences");
    expectEqualCount(stats.uniqueWords.size(), 0, "empty file: 0 unique words");
    expectFalse(t.searchWord("error"), "empty file: Trie contains nothing");
    expectEqualCount(t.autocomplete("e").size(), 0, "empty file: autocomplete returns nothing");
    std::remove(path.c_str());
}

void testMultiLineFile() {
    const std::string path = "TrieTest_tmp_multiline.log";
    const std::string content =
        "2024-03-15 10:42:01 ERROR Disk full, retrying...\n"
        "2024-03-15 10:42:02 error: DISK FULL!\r\n"          // Windows line ending
        "[WARN] Disk almost-full";                           // no newline at the end
    expectTrue(writeTextFile(path, content), "multi-line: temporary file created");

    Trie t;
    LogStats stats;
    std::string error;
    const bool ok = processLogFile(path, t, stats, error);
    expectTrue(ok, "multi-line: file read without error");
    expectEqualCount(stats.lines, 3, "multi-line: all 3 lines were read");
    expectEqualCount(stats.totalWords, 23, "multi-line: 23 word occurrences (10 + 9 + 4)");
    expectEqualCount(stats.uniqueWords.size(), 13, "multi-line: 13 unique words");
    expectTrue(t.searchWord("retrying"), "multi-line: word from line 1 is in the Trie");
    expectTrue(t.searchWord("almost"),   "multi-line: word from the LAST line (no newline) is in the Trie");
    expectTrue(t.searchWord("warn"),     "multi-line: 'warn' found (from '[WARN]')");
    expectTrue(t.searchWord("error"),    "multi-line: 'error' found (ERROR and error merged)");
    expectFalse(t.searchWord("ERROR"),   "multi-line: upper-case form was not stored");
    expectEqualList(t.autocomplete("d"), {"disk"}, "multi-line: autocomplete('d') -> disk");
    expectEqualList(t.autocomplete("0"), {"01", "02", "03"},
                    "multi-line: autocomplete('0') -> digit words in sorted order");
    std::remove(path.c_str());
}

void testLargeFile() {
    const std::string path = "TrieTest_tmp_large.log";
    std::string content;
    for (int i = 0; i < 5000; ++i)
        content += "entry" + std::to_string(i) + " ok\n";
    expectTrue(writeTextFile(path, content), "large file: temporary file with 5000 lines created");

    Trie t;
    LogStats stats;
    std::string error;
    const bool ok = processLogFile(path, t, stats, error);
    expectTrue(ok, "large file: read without error");
    expectEqualCount(stats.lines, 5000, "large file: all 5000 lines were read");
    expectEqualCount(stats.totalWords, 10000, "large file: 10000 word occurrences");
    expectEqualCount(stats.uniqueWords.size(), 5001, "large file: 5001 unique words (entry0..entry4999 + ok)");
    expectTrue(t.searchWord("entry0"),     "large file: first line's word is found");
    expectTrue(t.searchWord("entry4999"),  "large file: LAST line's word is found");
    expectFalse(t.searchWord("entry5000"), "large file: word beyond the file is not found");
    expectEqualList(t.autocomplete("entry4999"), {"entry4999"},
                    "large file: autocomplete('entry4999') returns exactly one word");
    expectEqualCount(t.autocomplete("entry49").size(), 111,
                     "large file: autocomplete('entry49') -> 1 + 10 + 100 = 111 words");
    std::remove(path.c_str());
}

// ---- 5D: The real log file given on the command line -----------------------

void testRealLogFile(const std::string& path, bool useSampleExpectations) {
    std::cout << "Reading log file: " << path << '\n';

    Trie trie;
    LogStats stats;
    std::string error;
    const bool ok = processLogFile(path, trie, stats, error);
    expectTrue(ok, "log file: opened and read completely");
    if (!ok) {
        std::cout << "       " << error << '\n';
        return;
    }

    std::cout << "       lines read       : " << stats.lines << '\n'
              << "       word occurrences : " << stats.totalWords << " (repeats counted)\n"
              << "       unique words     : " << stats.uniqueWords.size()
              << " (what the Trie holds; the Trie does not store counts)\n";

    // ---- Checks that work for ANY log file (compare Trie against std::set) ----
    expectTrue(stats.totalWords >= stats.uniqueWords.size(),
               "log file: occurrences >= unique words");

    std::size_t missing = 0;
    for (const std::string& w : stats.uniqueWords)
        if (!trie.searchWord(w)) ++missing;
    expectEqualCount(missing, 0, "log file: every extracted word is found by searchWord()");

    const std::string firstChars = "abcdefghijklmnopqrstuvwxyz0123456789_";
    bool autocompleteMatches = true;
    std::size_t trieWordCount = 0;
    for (char c : firstChars) {
        const std::string prefix(1, c);
        const std::vector<std::string> actual = trie.autocomplete(prefix);
        trieWordCount += actual.size();
        if (actual != expectedAutocomplete(stats.uniqueWords, prefix)) {
            autocompleteMatches = false;
            std::cout << "       mismatch for prefix '" << prefix << "'\n";
        }
    }
    expectTrue(autocompleteMatches,
               "log file: autocomplete for every 1-character prefix matches the expected sorted list");
    expectEqualCount(trieWordCount, stats.uniqueWords.size(),
                     "log file: Trie holds exactly the unique words (no duplicates, none lost)");
    expectFalse(trie.searchWord("zzzz_not_in_any_log_zzzz"),
                "log file: a word that is not in the log is not found");

    // ---- Checks that only make sense for the provided sample.log -------------
    if (!useSampleExpectations) {
        std::cout << "       (sample.log-specific checks skipped for this file)\n";
        return;
    }

    for (const char* prefix : {"user", "retry", "sched", "co", "20"})
        showAutocomplete(trie, prefix);

    expectEqualCount(stats.lines, 10,        "sample.log: 10 lines");
    expectEqualCount(stats.totalWords, 158,  "sample.log: 158 word occurrences");
    expectEqualCount(stats.uniqueWords.size(), 81, "sample.log: 81 unique words");

    expectTrue(trie.searchWord("error"),     "sample.log: 'error' found (ERROR and error merged)");
    expectTrue(trie.searchWord("warn"),      "sample.log: 'warn' found");
    expectTrue(trie.searchWord("info"),      "sample.log: 'info' found");
    expectTrue(trie.searchWord("alice"),     "sample.log: 'alice' found (Alice and alice merged)");
    expectTrue(trie.searchWord("dont"),      "sample.log: 'dont' found (from \"don't\")");
    expectTrue(trie.searchWord("user_42"),   "sample.log: 'user_42' found (underscore kept)");
    expectTrue(trie.searchWord("30s"),       "sample.log: '30s' found (digits + letters kept)");
    expectTrue(trie.searchWord("2024"),      "sample.log: '2024' found (from the timestamp)");
    expectTrue(trie.searchWord("pool_size"), "sample.log: 'pool_size' found");

    expectFalse(trie.searchWord("ERROR"),    "sample.log: 'ERROR' not stored (everything is lower-case)");
    expectFalse(trie.searchWord("don't"),    "sample.log: \"don't\" not stored (apostrophe removed)");
    expectFalse(trie.searchWord("db-pool"),  "sample.log: 'db-pool' not stored (hyphen splits it)");
    expectFalse(trie.searchWord("retr"),     "sample.log: 'retr' is only a prefix, not a word");
    expectFalse(trie.searchWord("fatal"),    "sample.log: 'fatal' never appears");

    expectEqualList(trie.autocomplete("user"), {"user", "user_42", "users"},
                    "sample.log: autocomplete('user') -> word that is also a prefix, '_' sorts before 's'");
    expectEqualList(trie.autocomplete("retry"), {"retry", "retrying"},
                    "sample.log: autocomplete('retry') -> {retry, retrying}");
    expectEqualList(trie.autocomplete("sched"), {"scheduled", "scheduler"},
                    "sample.log: autocomplete('sched') -> {scheduled, scheduler}");
    expectEqualList(trie.autocomplete("complete"), {"complete", "completed"},
                    "sample.log: autocomplete('complete') -> {complete, completed}");
    expectEqualList(trie.autocomplete("co"), {"complete", "completed", "connection"},
                    "sample.log: autocomplete('co') -> three words, sorted");
    expectEqualList(trie.autocomplete("20"), {"20", "200", "2024"},
                    "sample.log: autocomplete('20') -> digit words, sorted");
    expectEqualCount(trie.autocomplete("zzz").size(), 0,
                     "sample.log: autocomplete('zzz') -> nothing");
}

void section(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

}  // namespace

// =============================================================================
// main
// =============================================================================
int main(int argc, char* argv[]) {
    std::string logPath = kDefaultLogFile;
    if (argc > 1) logPath = argv[1];
    if (argc > 2)
        std::cerr << "Note: extra arguments ignored. Usage: " << argv[0] << " [logfile]\n";

    // The exact-number checks only make sense for the sample.log from the guide.
    const bool useSampleExpectations = (logPath == kDefaultLogFile);

    std::cout << "=== SearchLogX Trie tests ===\n";

    section("Trie: insertWord / searchWord / autocomplete");
    testNormalWordAndMissingWord();
    testDuplicateInsertion();
    testEmptyString();
    testPrefixes();
    testWordThatIsAlsoPrefix();
    testAutocompleteMultipleMatchesSorted();
    testRawTrieIsCaseSensitive();

    section("Tokenizer and normalisation");
    testTokenizer();
    testNormalisationFeedsTrie();

    section("File handling");
    testMissingFile();
    testEmptyFile();
    testMultiLineFile();
    testLargeFile();

    section("Real log file");
    testRealLogFile(logPath, useSampleExpectations);

    std::cout << "\n=== Summary: " << g_passed << " passed, " << g_failed << " failed ===\n";
    return (g_failed == 0) ? 0 : 1;
}