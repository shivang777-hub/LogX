#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

using namespace std;

class LogEntry {
private:
    string timestamp;   
    string level;       
    string message;     
    string sourceFile;  
    int lineNumber;          

public:
    LogEntry() : lineNumber(0) {}
    LogEntry(const string& ts, const string& lvl, const string& msg,
             const string& file, int line)
        : timestamp(ts), level(lvl), message(msg), sourceFile(file), lineNumber(line) {}

    const string& getTimestamp() const { return timestamp; }
    const string& getLevel() const { return level; }
    const string& getMessage() const { return message; }
    const string& getSourceFile() const { return sourceFile; }
    int getLineNumber() const { return lineNumber; }
};

#endif
