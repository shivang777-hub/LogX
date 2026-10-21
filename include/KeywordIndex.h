#ifndef KEYWORDINDEX_H
#define KEYWORDINDEX_H

#include <string>
#include <unordered_map>
#include <vector>
#include "LogEntry.h"

class KeywordIndex {
private:
    std::unordered_map<std::string, std::vector<int> > wordToIds;
    std::unordered_map<std::string, int> levelCount;
    std::unordered_map<std::string, int> wordCount;

    std::string toLower(const std::string& text) const;
    std::string toUpper(const std::string& text) const;
    std::vector<std::string> splitWords(const std::string& message) const;

public:
    void build(const std::vector<LogEntry>& entries);
    std::vector<int> search(const std::string& word) const;
    int getLevelCount(const std::string& level) const;
    int getWordCount(const std::string& word) const;
    std::vector<std::string> getAllWords() const;
    const std::unordered_map<std::string, int>& getWordCountMap() const;
    const std::unordered_map<std::string, int>& getLevelCountMap() const;
};

#endif
