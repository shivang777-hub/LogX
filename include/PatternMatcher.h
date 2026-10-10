#ifndef PATTERNMATCHER_H
#define PATTERNMATCHER_H

#include <string>
#include <vector>
#include "LogEntry.h"

std::vector<int> buildLps(const std::string& pattern);
std::vector<int> kmpSearch(const std::string& text, const std::string& pattern);
std::vector<int> rabinKarpSearch(const std::string& text, const std::string& pattern);
std::vector<LogEntry> searchLogs(const std::vector<LogEntry>& logs, const std::string& pattern, bool useKmp);

#endif
