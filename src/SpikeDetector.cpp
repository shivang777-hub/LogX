#include "SpikeDetector.h"

static bool isErrorEntry(const LogEntry& entry) {
    return entry.level == "ERROR" || entry.level == "CRITICAL";
}

std::vector<SpikeWindow> detectSpikes(const std::vector<LogEntry>& logs, long long windowSeconds, int threshold) {
    std::vector<SpikeWindow> spikes;
    if (logs.empty() || windowSeconds <= 0 || threshold <= 0) {
        return spikes;
    }

    int left = 0;
    int errors = 0;
    for (int right = 0; right < (int)logs.size(); right++) {
        if (isErrorEntry(logs[right])) {
            errors++;
        }
        while (logs[right].timestamp - logs[left].timestamp >= windowSeconds) {
            if (isErrorEntry(logs[left])) {
                errors--;
            }
            left++;
        }

        if (errors >= threshold) {
            long long start = logs[left].timestamp;
            long long end = logs[right].timestamp;
            if (!spikes.empty() && start <= spikes.back().endTime + 1) {
                if (end > spikes.back().endTime) {
                    spikes.back().endTime = end;
                }
                if (errors > spikes.back().errorCount) {
                    spikes.back().errorCount = errors;
                }
            } else {
                SpikeWindow w;
                w.startTime = start;
                w.endTime = end;
                w.errorCount = errors;
                spikes.push_back(w);
            }
        }
    }
    return spikes;
}

int countErrorsInWindow(const std::vector<LogEntry>& logs, long long startTime, long long windowSeconds) {
    if (windowSeconds <= 0) {
        return 0;
    }
    int count = 0;
    long long endTime = startTime + windowSeconds;
    for (int i = 0; i < (int)logs.size(); i++) {
        if (logs[i].timestamp >= startTime && logs[i].timestamp < endTime && isErrorEntry(logs[i])) {
            count++;
        }
    }
    return count;
}
