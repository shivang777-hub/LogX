#include "priorityQueue.h"

#include <algorithm>
#include <cctype>

using namespace std;

namespace SearchLogX {

string severityToString(Severity s) {
    switch (s) {
        case Severity::emergency: return "EMERGENCY";
        case Severity::alert:     return "ALERT";
        case Severity::critical:  return "CRITICAL";
        case Severity::error:     return "ERROR";
        case Severity::warning:   return "WARNING";
        case Severity::notice:    return "NOTICE";
        case Severity::info:      return "INFO";
        case Severity::debug:     return "DEBUG";
        default:                  return "UNKNOWN";
    }
}

Severity severityFromString(const string& name) {
    string upper = name;
    transform(upper.begin(), upper.end(), upper.begin(),
              [](unsigned char c) { return static_cast<char>(toupper(c)); });

    if (upper == "EMERGENCY") return Severity::emergency;
    if (upper == "ALERT")     return Severity::alert;
    if (upper == "CRITICAL")  return Severity::critical;
    if (upper == "ERROR")     return Severity::error;
    if (upper == "WARNING")   return Severity::warning;
    if (upper == "NOTICE")    return Severity::notice;
    if (upper == "INFO")      return Severity::info;
    if (upper == "DEBUG")     return Severity::debug;
    return Severity::unknown;
}

bool LogPriorityQueue::LowerPriority::operator()(const Item& a, const Item& b) const {
    int sa = static_cast<int>(a.entry.severity);
    int sb = static_cast<int>(b.entry.severity);

    // Different severity: the larger number is less severe, so it has lower priority.
    if (sa != sb)
        return sa > sb;

    // Same severity: the later insertion has lower priority (FIFO).
    return a.seq > b.seq;
}

Severity severityFromInt(int value) {
    if (value < 0 || value > 7)
        return Severity::unknown;
    return static_cast<Severity>(value);
}

void LogPriorityQueue::push(const std::string& timestamp,
                            const std::string& level,
                            const std::string& message) {
    LogEntry entry;
    entry.timestamp = timestamp;
    entry.severity = severityFromString(level);
    entry.message = message;

    queue_.push(Item{entry, nextSeq_++});
}


bool LogPriorityQueue::top(LogEntry& entry) const {
    if (queue_.empty())
        return false;

    entry = queue_.top().entry;
    return true;
}

bool LogPriorityQueue::pop() {
    if (queue_.empty())
        return false;
    queue_.pop();
    return true;
}

bool LogPriorityQueue::empty() const { 
    return queue_.empty(); 
}

size_t LogPriorityQueue::size() const {
    return queue_.size(); 
}

}