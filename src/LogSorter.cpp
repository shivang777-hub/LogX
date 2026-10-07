#include "LogSorter.h"

int severityRank(const std::string& level) {
    if (level == "CRITICAL") return 5;
    if (level == "ERROR") return 4;
    if (level == "WARNING") return 3;
    if (level == "INFO") return 2;
    if (level == "DEBUG") return 1;
    return 0;
}

static int compareEntries(const LogEntry& a, const LogEntry& b, int mode) {
    if (mode == 0) {
        if (a.timestamp < b.timestamp) return -1;
        if (a.timestamp > b.timestamp) return 1;
        return 0;
    }
    if (mode == 1) {
        int ra = severityRank(a.level);
        int rb = severityRank(b.level);
        if (ra < rb) return -1;
        if (ra > rb) return 1;
        return 0;
    }
    int c = a.message.compare(b.message);
    if (c < 0) return -1;
    if (c > 0) return 1;
    return 0;
}

static void merge(std::vector<LogEntry>& entries, int left, int mid, int right, int mode, bool ascending) {
    std::vector<LogEntry> temp;
    int i = left;
    int j = mid + 1;

    while (i <= mid && j <= right) {
        int c = compareEntries(entries[j], entries[i], mode);
        bool takeRight;
        if (ascending) {
            takeRight = (c < 0);
        } else {
            takeRight = (c > 0);
        }
        if (takeRight) {
            temp.push_back(entries[j]);
            j++;
        } else {
            temp.push_back(entries[i]);
            i++;
        }
    }
    while (i <= mid) {
        temp.push_back(entries[i]);
        i++;
    }
    while (j <= right) {
        temp.push_back(entries[j]);
        j++;
    }
    for (int k = 0; k < (int)temp.size(); k++) {
        entries[left + k] = temp[k];
    }
}

static void mergeSort(std::vector<LogEntry>& entries, int left, int right, int mode, bool ascending) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(entries, left, mid, mode, ascending);
    mergeSort(entries, mid + 1, right, mode, ascending);
    merge(entries, left, mid, right, mode, ascending);
}

void sortByTimestamp(std::vector<LogEntry>& entries, bool ascending) {
    if (entries.size() < 2) return;
    mergeSort(entries, 0, (int)entries.size() - 1, 0, ascending);
}

void sortBySeverity(std::vector<LogEntry>& entries, bool ascending) {
    if (entries.size() < 2) return;
    mergeSort(entries, 0, (int)entries.size() - 1, 1, ascending);
}

void sortByMessage(std::vector<LogEntry>& entries, bool ascending) {
    if (entries.size() < 2) return;
    mergeSort(entries, 0, (int)entries.size() - 1, 2, ascending);
}
