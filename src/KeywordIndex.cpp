#include "KeywordIndex.h"
#include <cctype>

std::string KeywordIndex::toLower(const std::string& text) const {
    std::string result = text;
    for (int i = 0; i < (int)result.size(); i++) {
        result[i] = (char)std::tolower((unsigned char)result[i]);
    }
    return result;
}

std::string KeywordIndex::toUpper(const std::string& text) const {
    std::string result = text;
    for (int i = 0; i < (int)result.size(); i++) {
        result[i] = (char)std::toupper((unsigned char)result[i]);
    }
    return result;
}

std::vector<std::string> KeywordIndex::splitWords(const std::string& message) const {
    std::vector<std::string> words;
    std::string lowered = toLower(message);
    std::string current = "";
    for (int i = 0; i < (int)lowered.size(); i++) {
        if (std::isalnum((unsigned char)lowered[i])) {
            current += lowered[i];
        } else {
            if (!current.empty()) {
                words.push_back(current);
                current = "";
            }
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

void KeywordIndex::build(const std::vector<LogEntry>& entries) {
    wordToIds.clear();
    levelCount.clear();
    wordCount.clear();

    for (int i = 0; i < (int)entries.size(); i++) {
        levelCount[toUpper(entries[i].level)]++;

        std::vector<std::string> words = splitWords(entries[i].message);
        for (int j = 0; j < (int)words.size(); j++) {
            wordCount[words[j]]++;
            std::vector<int>& ids = wordToIds[words[j]];
            if (ids.empty() || ids.back() != entries[i].id) {
                ids.push_back(entries[i].id);
            }
        }
    }
}

std::vector<int> KeywordIndex::search(const std::string& word) const {
    std::unordered_map<std::string, std::vector<int> >::const_iterator it = wordToIds.find(toLower(word));
    if (it == wordToIds.end()) {
        return std::vector<int>();
    }
    return it->second;
}

int KeywordIndex::getLevelCount(const std::string& level) const {
    std::unordered_map<std::string, int>::const_iterator it = levelCount.find(toUpper(level));
    if (it == levelCount.end()) {
        return 0;
    }
    return it->second;
}

int KeywordIndex::getWordCount(const std::string& word) const {
    std::unordered_map<std::string, int>::const_iterator it = wordCount.find(toLower(word));
    if (it == wordCount.end()) {
        return 0;
    }
    return it->second;
}

std::vector<std::string> KeywordIndex::getAllWords() const {
    std::vector<std::string> words;
    for (std::unordered_map<std::string, int>::const_iterator it = wordCount.begin(); it != wordCount.end(); ++it) {
        words.push_back(it->first);
    }
    return words;
}

const std::unordered_map<std::string, int>& KeywordIndex::getWordCountMap() const {
    return wordCount;
}

const std::unordered_map<std::string, int>& KeywordIndex::getLevelCountMap() const {
    return levelCount;
}
