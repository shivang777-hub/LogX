#include "priorityQueue.h"
#include <iostream>
using namespace SearchLogX;

int main() {
    LogPriorityQueue q;
    std::string keyword = "Disk";

    // Add the logs that contain the keyword
    q.push("08:00:05", "warning",  "Disk usage at 85%");
    q.push("08:02:30", "critical", "Disk failure detected");
    q.push("08:03:15", "error",    "Disk read error");
    q.push("08:06:00", "notice",   "Disk cleanup scheduled");

    std::cout << "Results for \"" << keyword << "\":\n";

    // Print in priority order
    LogEntry e;
    while (q.top(e)) {
        std::cout << "[" << severityToString(e.severity) << "] "
                  << e.timestamp << "  " << e.message << "\n";
        q.pop();
    }
    return 0;
}