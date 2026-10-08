#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "LogEntry.h"
#include "PatternMatcher.h"
#include "SpikeDetector.h"
#include "ErrorReport.h"

using namespace std;

void addEntry(vector<LogEntry>& entries, int id, long long timestamp, string level, string message, string source) {
    LogEntry e;
    e.id = id;
    e.timestamp = timestamp;
    e.level = level;
    e.message = message;
    e.source = source;
    entries.push_back(e);
}

vector<LogEntry> makeSampleEntries() {
    vector<LogEntry> entries;
    addEntry(entries, 1, 1700000050, "INFO", "Server started successfully", "system.log");
    addEntry(entries, 2, 1700000400, "WARNING", "Database connection timeout", "db.log");
    addEntry(entries, 3, 1700000603, "ERROR", "Connection refused by payment gateway", "api.log");
    addEntry(entries, 4, 1700000601, "ERROR", "Connection refused by payment gateway", "api.log");
    addEntry(entries, 5, 1700000100, "INFO", "User login successful", "auth.log");
    addEntry(entries, 6, 1700000605, "ERROR", "Database connection timeout", "db.log");
    addEntry(entries, 7, 1700000602, "ERROR", "Database query failed", "db.log");
    addEntry(entries, 8, 1700000900, "DEBUG", "Cache miss for key user42", "cache.log");
    addEntry(entries, 9, 1700000604, "CRITICAL", "Server out of memory", "system.log");
    addEntry(entries, 10, 1700000600, "ERROR", "Connection refused by payment gateway", "api.log");
    addEntry(entries, 11, 1700000606, "ERROR", "Database connection timeout", "db.log");
    addEntry(entries, 12, 1700000200, "INFO", "User logout", "auth.log");
    addEntry(entries, 13, 1700001500, "ERROR", "Failed to open config file", "app.log");
    addEntry(entries, 14, 1700000607, "ERROR", "Connection refused by payment gateway", "api.log");
    addEntry(entries, 15, 1700001000, "WARNING", "Disk usage above 80 percent", "system.log");
    addEntry(entries, 16, 1700001200, "INFO", "Backup completed", "system.log");
    addEntry(entries, 17, 1700000300, "DEBUG", "Cache hit for key user42", "cache.log");
    addEntry(entries, 18, 1700001800, "ERROR", "Database connection timeout", "db.log");
    addEntry(entries, 19, 1700001900, "INFO", "User login successful", "auth.log");
    addEntry(entries, 20, 1700002000, "WARNING", "Slow response from payment service", "api.log");
    addEntry(entries, 21, 1700000700, "INFO", "Scheduled cleanup finished", "system.log");
    addEntry(entries, 22, 1700002100, "ERROR", "Failed to open config file", "app.log");
    addEntry(entries, 23, 1700002200, "INFO", "Server shutting down", "system.log");
    addEntry(entries, 24, 1700001300, "ERROR", "Database connection timeout", "db.log");
    return entries;
}

bool compareByTimestamp(const LogEntry& a, const LogEntry& b) {
    return a.timestamp < b.timestamp;
}

void printEntry(const LogEntry& e) {
    cout << "  id=" << e.id << " ts=" << e.timestamp << " [" << e.level << "] "
         << e.message << " (" << e.source << ")" << endl;
}

void printEntries(const string& title, const vector<LogEntry>& entries) {
    cout << title << " (" << entries.size() << " entries)" << endl;
    for (int i = 0; i < (int)entries.size(); i++) {
        printEntry(entries[i]);
    }
    cout << endl;
}

void printPositions(const string& title, const vector<int>& positions) {
    cout << title << ":";
    if (positions.empty()) {
        cout << " none";
    }
    for (int i = 0; i < (int)positions.size(); i++) {
        cout << " " << positions[i];
    }
    cout << endl;
}

void testPatternMatcher() {
    cout << "===== PatternMatcher =====" << endl;

    printPositions("buildLps(ababaca)", buildLps("ababaca"));
    printPositions("buildLps(aabaaab)", buildLps("aabaaab"));
    cout << endl;

    string text = "Database connection timeout on database node";
    printPositions("kmp 'database' in text", kmpSearch(text, "database"));
    printPositions("rabin-karp 'database' in text", rabinKarpSearch(text, "database"));
    printPositions("kmp 'TIMEOUT' (upper case pattern)", kmpSearch(text, "TIMEOUT"));
    printPositions("rabin-karp 'TIMEOUT' (upper case pattern)", rabinKarpSearch(text, "TIMEOUT"));
    printPositions("kmp 'aa' in 'aaaa' (overlapping)", kmpSearch("aaaa", "aa"));
    printPositions("rabin-karp 'aa' in 'aaaa' (overlapping)", rabinKarpSearch("aaaa", "aa"));
    printPositions("kmp 'refused' in text (no match)", kmpSearch(text, "refused"));
    printPositions("rabin-karp 'refused' in text (no match)", rabinKarpSearch(text, "refused"));
    printPositions("kmp with empty pattern", kmpSearch(text, ""));
    printPositions("rabin-karp with empty pattern", rabinKarpSearch(text, ""));
    printPositions("kmp with pattern longer than text", kmpSearch("abc", "abcdef"));
    printPositions("rabin-karp with pattern longer than text", rabinKarpSearch("abc", "abcdef"));
    printPositions("kmp on empty text", kmpSearch("", "abc"));
    cout << endl;

    vector<LogEntry> logs = makeSampleEntries();
    printEntries("searchLogs 'connection refused' with KMP", searchLogs(logs, "connection refused", true));
    printEntries("searchLogs 'connection refused' with Rabin-Karp", searchLogs(logs, "connection refused", false));
    printEntries("searchLogs 'TIMEOUT' with KMP", searchLogs(logs, "TIMEOUT", true));
    printEntries("searchLogs 'database' with Rabin-Karp", searchLogs(logs, "database", false));
    printEntries("searchLogs 'nothing here' with KMP", searchLogs(logs, "nothing here", true));
    printEntries("searchLogs empty pattern", searchLogs(logs, "", true));

    bool same = true;
    string words[4] = {"database", "payment", "user", "file"};
    for (int i = 0; i < 4; i++) {
        vector<LogEntry> a = searchLogs(logs, words[i], true);
        vector<LogEntry> b = searchLogs(logs, words[i], false);
        if (a.size() != b.size()) {
            same = false;
        }
    }
    cout << "kmp and rabin-karp give same number of results on 4 words: ";
    if (same) cout << "yes" << endl;
    else cout << "NO" << endl;
    cout << endl;
}

void printSpikes(const string& title, const vector<SpikeWindow>& spikes) {
    cout << title << " (" << spikes.size() << " spikes)" << endl;
    for (int i = 0; i < (int)spikes.size(); i++) {
        cout << "  from " << spikes[i].startTime << " to " << spikes[i].endTime
             << " errors=" << spikes[i].errorCount << endl;
    }
}

void testSpikeDetector() {
    cout << "===== SpikeDetector =====" << endl;
    vector<LogEntry> logs = makeSampleEntries();
    sort(logs.begin(), logs.end(), compareByTimestamp);

    printSpikes("window 10s, threshold 5", detectSpikes(logs, 10, 5));
    printSpikes("window 10s, threshold 8", detectSpikes(logs, 10, 8));
    printSpikes("window 10s, threshold 9", detectSpikes(logs, 10, 9));
    printSpikes("window 3s, threshold 3", detectSpikes(logs, 3, 3));
    printSpikes("window 600s, threshold 3", detectSpikes(logs, 600, 3));
    printSpikes("window 1s, threshold 1", detectSpikes(logs, 1, 1));
    cout << endl;

    cout << "countErrorsInWindow(start 1700000600, 10s) = " << countErrorsInWindow(logs, 1700000600, 10) << endl;
    cout << "countErrorsInWindow(start 1700000600, 4s) = " << countErrorsInWindow(logs, 1700000600, 4) << endl;
    cout << "countErrorsInWindow(start 1700001300, 600s) = " << countErrorsInWindow(logs, 1700001300, 600) << endl;
    cout << "countErrorsInWindow(start 1700000000, 50s) = " << countErrorsInWindow(logs, 1700000000, 50) << endl;
    cout << "countErrorsInWindow with window 0 = " << countErrorsInWindow(logs, 1700000600, 0) << endl;
    cout << endl;

    vector<LogEntry> empty;
    printSpikes("empty vector", detectSpikes(empty, 10, 5));
    printSpikes("window 0", detectSpikes(logs, 0, 5));
    printSpikes("negative window", detectSpikes(logs, -5, 5));
    printSpikes("threshold 0", detectSpikes(logs, 10, 0));
    printSpikes("negative threshold", detectSpikes(logs, 10, -1));
    cout << endl;
}

void testErrorReport() {
    cout << "===== ErrorReport =====" << endl;
    vector<LogEntry> logs = makeSampleEntries();
    sort(logs.begin(), logs.end(), compareByTimestamp);

    vector<pair<string, int> > top = getTopErrors(logs, 5);
    cout << "top 5 error messages" << endl;
    for (int i = 0; i < (int)top.size(); i++) {
        cout << "  " << (i + 1) << ". " << top[i].first << " = " << top[i].second << endl;
    }
    cout << "getTopErrors with k = 100 returns " << getTopErrors(logs, 100).size() << " messages" << endl;
    cout << "getTopErrors with k = 0 returns " << getTopErrors(logs, 0).size() << " messages" << endl;
    cout << endl;

    printReport(logs, 10, 5);
    cout << endl;
    printReport(logs, 600, 3);
    cout << endl;

    vector<LogEntry> empty;
    printReport(empty, 10, 5);
    cout << endl;
}

int main() {
    testPatternMatcher();
    testSpikeDetector();
    testErrorReport();
    return 0;
}
