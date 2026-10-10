#include "ErrorReport.h"
#include "SpikeDetector.h"
#include <iostream>
#include <unordered_map>

static bool isErrorEntry(const LogEntry& entry) {
    return entry.level == "ERROR" || entry.level == "CRITICAL";
}

static bool ranksHigher(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
    if (a.second != b.second) {
        return a.second > b.second;
    }
    return a.first < b.first;
}

std::vector<std::pair<std::string, int> > getTopErrors(const std::vector<LogEntry>& logs, int k) {
    std::vector<std::pair<std::string, int> > result;
    if (k <= 0) {
        return result;
    }

    std::unordered_map<std::string, int> messageCount;
    for (int i = 0; i < (int)logs.size(); i++) {
        if (isErrorEntry(logs[i])) {
            messageCount[logs[i].message]++;
        }
    }

    std::vector<std::pair<std::string, int> > items;
    for (std::unordered_map<std::string, int>::const_iterator it = messageCount.begin(); it != messageCount.end(); ++it) {
        items.push_back(std::make_pair(it->first, it->second));
    }

    for (int i = 0; i < (int)items.size(); i++) {
        int best = i;
        for (int j = i + 1; j < (int)items.size(); j++) {
            if (ranksHigher(items[j], items[best])) {
                best = j;
            }
        }
        std::pair<std::string, int> temp = items[i];
        items[i] = items[best];
        items[best] = temp;
    }

    for (int i = 0; i < (int)items.size() && i < k; i++) {
        result.push_back(items[i]);
    }
    return result;
}

void printReport(const std::vector<LogEntry>& logs, long long windowSeconds, int threshold) {
    std::unordered_map<std::string, int> levelCount;
    for (int i = 0; i < (int)logs.size(); i++) {
        levelCount[logs[i].level]++;
    }

    std::cout << "----- Log Report -----" << std::endl;
    std::cout << "Total entries: " << logs.size() << std::endl;

    std::cout << "Entries per level:" << std::endl;
    std::string levels[5] = {"CRITICAL", "ERROR", "WARNING", "INFO", "DEBUG"};
    for (int i = 0; i < 5; i++) {
        std::cout << "  " << levels[i] << ": " << levelCount[levels[i]] << std::endl;
    }

    std::cout << "Top 3 errors:" << std::endl;
    std::vector<std::pair<std::string, int> > top = getTopErrors(logs, 3);
    if (top.empty()) {
        std::cout << "  none" << std::endl;
    }
    for (int i = 0; i < (int)top.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << top[i].first << " (" << top[i].second << " times)" << std::endl;
    }

    std::cout << "Error spikes (window " << windowSeconds << "s, threshold " << threshold << "):" << std::endl;
    std::vector<SpikeWindow> spikes = detectSpikes(logs, windowSeconds, threshold);
    if (spikes.empty()) {
        std::cout << "  none" << std::endl;
    }
    for (int i = 0; i < (int)spikes.size(); i++) {
        std::cout << "  " << spikes[i].startTime << " to " << spikes[i].endTime
                  << ", errors: " << spikes[i].errorCount << std::endl;
    }
}
