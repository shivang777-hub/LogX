#include "TimeSearch.h"

int lowerBound(const std::vector<LogEntry>& entries, long long t) {
    int low = 0;
    int high = (int)entries.size();
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (entries[mid].timestamp < t) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

int upperBound(const std::vector<LogEntry>& entries, long long t) {
    int low = 0;
    int high = (int)entries.size();
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (entries[mid].timestamp <= t) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

std::vector<LogEntry> rangeSearch(const std::vector<LogEntry>& entries, long long startTime, long long endTime) {
    std::vector<LogEntry> result;
    if (entries.empty() || startTime > endTime) {
        return result;
    }
    int first = lowerBound(entries, startTime);
    int last = upperBound(entries, endTime);
    for (int i = first; i < last; i++) {
        result.push_back(entries[i]);
    }
    return result;
}
