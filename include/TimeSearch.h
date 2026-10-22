#ifndef TIMESEARCH_H
#define TIMESEARCH_H

#include <vector>
#include "LogEntry.h"

int lowerBound(const std::vector<LogEntry>& entries, long long t);
int upperBound(const std::vector<LogEntry>& entries, long long t);
std::vector<LogEntry> rangeSearch(const std::vector<LogEntry>& entries, long long startTime, long long endTime);

#endif
