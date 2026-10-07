#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "LogEntry.h"
#include "LogSorter.h"
#include "TimeSearch.h"
#include "KeywordIndex.h"
#include "TopKHeap.h"

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
    addEntry(entries, 1, 1700000500, "INFO", "User login successful", "auth.log");
    addEntry(entries, 2, 1700000100, "ERROR", "Database connection timeout", "db.log");
    addEntry(entries, 3, 1700000900, "WARNING", "Disk usage above 80 percent", "system.log");
    addEntry(entries, 4, 1700000300, "DEBUG", "Cache miss for key user42", "cache.log");
    addEntry(entries, 5, 1700001200, "CRITICAL", "Server out of memory", "system.log");
    addEntry(entries, 6, 1700000100, "ERROR", "Database connection timeout", "db.log");
    addEntry(entries, 7, 1700000700, "INFO", "User logout", "auth.log");
    addEntry(entries, 8, 1700001500, "ERROR", "Failed to open config file", "app.log");
    addEntry(entries, 9, 1700000200, "INFO", "Server started", "system.log");
    addEntry(entries, 10, 1700001100, "WARNING", "Slow response from payment service", "api.log");
    addEntry(entries, 11, 1700000800, "ERROR", "Database connection timeout", "db.log");
    addEntry(entries, 12, 1700001800, "DEBUG", "Cache hit for key user42", "cache.log");
    addEntry(entries, 13, 1700000400, "INFO", "User login successful", "auth.log");
    addEntry(entries, 14, 1700001300, "CRITICAL", "Disk failure detected", "system.log");
    addEntry(entries, 15, 1700001000, "WARNING", "Slow response from payment service", "api.log");
    addEntry(entries, 16, 1700001600, "ERROR", "Failed login attempt for admin", "auth.log");
    addEntry(entries, 17, 1700001400, "INFO", "Backup completed", "system.log");
    addEntry(entries, 18, 1700000600, "ERROR", "Failed to open config file", "app.log");
    return entries;
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

void testSorter() {
    cout << "===== LogSorter =====" << endl;
    vector<LogEntry> original = makeSampleEntries();
    printEntries("original order", original);

    vector<LogEntry> v = original;
    sortByTimestamp(v, true);
    printEntries("sorted by timestamp ascending", v);
    sortByTimestamp(v, false);
    printEntries("sorted by timestamp descending", v);

    v = original;
    sortBySeverity(v, false);
    printEntries("sorted by severity, highest first", v);
    sortBySeverity(v, true);
    printEntries("sorted by severity, lowest first", v);

    v = original;
    sortByMessage(v, true);
    printEntries("sorted by message ascending", v);
    sortByMessage(v, false);
    printEntries("sorted by message descending", v);

    v = original;
    sortByTimestamp(v, true);
    sortBySeverity(v, false);
    bool stable = true;
    for (int i = 1; i < (int)v.size(); i++) {
        if (v[i].level == v[i - 1].level && v[i].timestamp < v[i - 1].timestamp) {
            stable = false;
        }
    }
    cout << "stability check (timestamp order kept inside same severity): ";
    if (stable) cout << "passed" << endl;
    else cout << "FAILED" << endl;

    vector<LogEntry> empty;
    sortByTimestamp(empty, true);
    cout << "sorting empty vector works, size = " << empty.size() << endl;
    cout << "severityRank(CRITICAL) = " << severityRank("CRITICAL")
         << ", severityRank(DEBUG) = " << severityRank("DEBUG")
         << ", severityRank(UNKNOWN) = " << severityRank("UNKNOWN") << endl;
    cout << endl;
}

void testTimeSearch() {
    cout << "===== TimeSearch =====" << endl;
    vector<LogEntry> v = makeSampleEntries();
    sortByTimestamp(v, true);

    cout << "lowerBound(1700000100) = " << lowerBound(v, 1700000100) << endl;
    cout << "upperBound(1700000100) = " << upperBound(v, 1700000100) << endl;
    cout << "lowerBound(1700000150) = " << lowerBound(v, 1700000150) << endl;
    cout << "upperBound(1700000150) = " << upperBound(v, 1700000150) << endl;
    cout << "lowerBound(before all) = " << lowerBound(v, 1600000000) << endl;
    cout << "upperBound(after all) = " << upperBound(v, 1800000000) << endl;
    cout << endl;

    printEntries("range 1700000300 to 1700000800", rangeSearch(v, 1700000300, 1700000800));
    printEntries("range with exact boundaries 1700000100 to 1700000200", rangeSearch(v, 1700000100, 1700000200));
    printEntries("range with start > end", rangeSearch(v, 1700000800, 1700000300));
    printEntries("range with no matches", rangeSearch(v, 1700000301, 1700000399));
    printEntries("range before all entries", rangeSearch(v, 1000, 2000));

    vector<LogEntry> empty;
    printEntries("range on empty vector", rangeSearch(empty, 1, 100));
    cout << "lowerBound on empty vector = " << lowerBound(empty, 5) << endl;
    cout << endl;
}

void printIds(const string& word, const vector<int>& ids) {
    cout << "search(\"" << word << "\") -> " << ids.size() << " ids:";
    for (int i = 0; i < (int)ids.size(); i++) {
        cout << " " << ids[i];
    }
    cout << endl;
}

void testKeywordIndex() {
    cout << "===== KeywordIndex =====" << endl;
    vector<LogEntry> v = makeSampleEntries();
    KeywordIndex index;
    index.build(v);

    printIds("timeout", index.search("timeout"));
    printIds("TIMEOUT", index.search("TIMEOUT"));
    printIds("Login", index.search("Login"));
    printIds("failed", index.search("failed"));
    printIds("payment", index.search("payment"));
    printIds("missingword", index.search("missingword"));
    cout << endl;

    cout << "level counts:" << endl;
    cout << "  CRITICAL = " << index.getLevelCount("CRITICAL") << endl;
    cout << "  ERROR = " << index.getLevelCount("ERROR") << endl;
    cout << "  WARNING = " << index.getLevelCount("WARNING") << endl;
    cout << "  INFO = " << index.getLevelCount("INFO") << endl;
    cout << "  DEBUG = " << index.getLevelCount("DEBUG") << endl;
    cout << "  error (lowercase query) = " << index.getLevelCount("error") << endl;
    cout << "  FATAL = " << index.getLevelCount("FATAL") << endl;
    cout << endl;

    cout << "word counts:" << endl;
    cout << "  database = " << index.getWordCount("database") << endl;
    cout << "  user = " << index.getWordCount("user") << endl;
    cout << "  USER (uppercase query) = " << index.getWordCount("USER") << endl;
    cout << "  nothing = " << index.getWordCount("nothing") << endl;
    cout << endl;

    vector<string> words = index.getAllWords();
    sort(words.begin(), words.end());
    cout << "getAllWords() returned " << words.size() << " unique words:" << endl;
    cout << " ";
    for (int i = 0; i < (int)words.size(); i++) {
        cout << " " << words[i];
    }
    cout << endl << endl;

    vector<LogEntry> empty;
    index.build(empty);
    cout << "after building from empty vector, getAllWords size = " << index.getAllWords().size() << endl;
    cout << endl;
}

void printTopK(const string& title, const vector<pair<string, int> >& items) {
    cout << title << endl;
    for (int i = 0; i < (int)items.size(); i++) {
        cout << "  " << (i + 1) << ". " << items[i].first << " = " << items[i].second << endl;
    }
}

void testTopKHeap() {
    cout << "===== TopKHeap =====" << endl;
    vector<LogEntry> v = makeSampleEntries();
    KeywordIndex index;
    index.build(v);

    printTopK("top 5 most frequent words", topK(index.getWordCountMap(), 5));
    printTopK("top 3 levels by count", topK(index.getLevelCountMap(), 3));
    printTopK("top 100 levels (k bigger than map)", topK(index.getLevelCountMap(), 100));

    unordered_map<string, int> empty;
    cout << "topK on empty map size = " << topK(empty, 3).size() << endl;
    cout << "topK with k = 0 size = " << topK(index.getWordCountMap(), 0).size() << endl;
    cout << endl;
}

int main() {
    testSorter();
    testTimeSearch();
    testKeywordIndex();
    testTopKHeap();
    return 0;
}
