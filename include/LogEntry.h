#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

struct LogEntry {
    int id;
    long long timestamp;
    std::string level;
    std::string message;
    std::string source;
};

#endif
