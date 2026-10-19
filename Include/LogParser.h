#ifndef LOGPARSER_H
#define LOGPARSER_H

#include "LogEntry.h"
#include <string>
#include <vector>

using namespace std;

// Reads log files and stores every valid line as a LogEntry in a vector.
class LogParser {
private:
    vector<LogEntry> entries;  // all parsed logs
    int skippedLines;               // lines that did not match the format

    bool parseLine(const string& line, const string& file,
                   int lineNo, LogEntry& out) const;
    static string toUpper(string s);
    static bool isValidLevel(const string& s);
    static string inferLevel(const string& message);

public:
    LogParser() : skippedLines(0) {}

    // Parse one file. Returns false if the file could not be opened.
    bool parseFile(const string& path);

    // Parse many files (the paths given by the folder traversal module).
    int parseFiles(const vector<string>& paths);

    const vector<LogEntry>& getEntries() const { return entries; }
    int getSkippedLines() const { return skippedLines; }
    size_t size() const { return entries.size(); }
    void clear() { entries.clear(); skippedLines = 0; }
};

#endif
