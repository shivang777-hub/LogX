#ifndef ERRORREPORT_H
#define ERRORREPORT_H

#include <string>
#include <utility>
#include <vector>
#include "LogEntry.h"

std::vector<std::pair<std::string, int> > getTopErrors(const std::vector<LogEntry>& logs, int k);
void printReport(const std::vector<LogEntry>& logs, long long windowSeconds, int threshold);

#endif
