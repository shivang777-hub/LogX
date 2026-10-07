#ifndef LOGSORTER_H
#define LOGSORTER_H

#include <string>
#include <vector>
#include "LogEntry.h"

int severityRank(const std::string& level);
void sortByTimestamp(std::vector<LogEntry>& entries, bool ascending = true);
void sortBySeverity(std::vector<LogEntry>& entries, bool ascending = true);
void sortByMessage(std::vector<LogEntry>& entries, bool ascending = true);

#endif
