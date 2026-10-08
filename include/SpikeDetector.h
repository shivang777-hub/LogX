#ifndef SPIKEDETECTOR_H
#define SPIKEDETECTOR_H

#include <vector>
#include "LogEntry.h"

struct SpikeWindow {
    long long startTime;
    long long endTime;
    int errorCount;
};

std::vector<SpikeWindow> detectSpikes(const std::vector<LogEntry>& logs, long long windowSeconds, int threshold);
int countErrorsInWindow(const std::vector<LogEntry>& logs, long long startTime, long long windowSeconds);

#endif
