#include "LogParser.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

using namespace std;

string LogParser::toUpper(string s) {
    transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return toupper(c); });
    return s;
}

bool LogParser::isValidLevel(const string& s) {
    return s == "INFO" || s == "WARNING" || s == "WARN" || s == "ERROR" ||
           s == "DEBUG" || s == "CRITICAL" || s == "FATAL";
}

// If a line has no level word (like Steam logs), guess it from the message.
string LogParser::inferLevel(const string& message) {
    string m = message;
    transform(m.begin(), m.end(), m.begin(),
              [](unsigned char c) { return tolower(c); });
    if (m.find("error") != string::npos || m.find("failed") != string::npos ||
        m.find("exception") != string::npos || m.find("fatal") != string::npos)
        return "ERROR";
    if (m.find("warn") != string::npos)
        return "WARNING";
    return "INFO";
}

// Two formats are supported:
//   2024-05-12 10:32:45 ERROR Database connection failed     (level written in the line)
//   [2026-05-27 13:50:57] Client version: 1779486452         (Steam style, no level)
// The level may also be written as [ERROR] or error:
bool LogParser::parseLine(const string& line, const string& file,
                          int lineNo, LogEntry& out) const {
    istringstream iss(line);
    string date, time;
    if (!(iss >> date >> time)) return false;

    // remove the [ ] around the timestamp, if there are any
    if (!date.empty() && date.front() == '[') date.erase(0, 1);
    if (!time.empty() && time.back() == ']') time.pop_back();

    // simple check for the date and time shape
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    if (time.size() < 8 || time[2] != ':' || time[5] != ':') return false;

    // everything after the timestamp, without the spaces at the front
    string rest;
    getline(iss, rest);
    size_t start = rest.find_first_not_of(" \t");
    rest = (start == string::npos) ? "" : rest.substr(start);

    // does the line start with a level word?
    istringstream rs(rest);
    string first;
    rs >> first;

    string word = first;
    word.erase(remove(word.begin(), word.end(), '['), word.end());
    word.erase(remove(word.begin(), word.end(), ']'), word.end());
    bool hasColon = !word.empty() && word.back() == ':';
    if (hasColon) word.pop_back();
    bool hasBrackets = !first.empty() && first.front() == '[';
    string upper = toUpper(word);

    // a normal word like "Error" at the start of a sentence is not taken as a level,
    // only ERROR, [Error] or error: are
    bool looksLikeLevel = hasBrackets || hasColon || word == upper;

    string level, message;
    if (!first.empty() && looksLikeLevel && isValidLevel(upper)) {
        level = upper;
        getline(rs, message);
        size_t s = message.find_first_not_of(" \t");
        message = (s == string::npos) ? "" : message.substr(s);
    } else {
        level = inferLevel(rest);   // no level in the line, so guess it
        message = rest;
    }

    out = LogEntry(date + " " + time, level, message, file, lineNo);
    return true;
}

bool LogParser::parseFile(const string& path) {
    ifstream in(path);
    if (!in.is_open()) return false;

    string line;
    int lineNo = 0;
    while (getline(in, line)) {
        ++lineNo;
        if (!line.empty() && line.back() == '\r') line.pop_back();  // Windows files
        if (line.empty()) continue;

        LogEntry e;
        if (parseLine(line, path, lineNo, e))
            entries.push_back(e);
        else
            ++skippedLines;
    }
    return true;
}

int LogParser::parseFiles(const vector<string>& paths) {
    int opened = 0;
    for (const auto& p : paths)
        if (parseFile(p)) ++opened;
    return opened;
}
