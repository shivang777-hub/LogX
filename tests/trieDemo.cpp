// TrieDemo.cpp - small demo of SearchLogX::Trie
#include "Trie.h"

#include <iostream>
#include <string>
#include <vector>

// Split a log line into lower-case words (anything that is not a letter,
// digit or '_' separates words; apostrophes are dropped).
std::vector<std::string> getWords(const std::string& line) {
    std::vector<std::string> words;
    std::string word;
    for (char c : line) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
            word += c;
        } else if (c != '\'') {
            if (!word.empty()) words.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) words.push_back(word);
    return words;
}

void printList(const std::vector<std::string>& list) {
    for (const std::string& w : list) std::cout << w << ' ';
    std::cout << '\n';
}

int main() {
    // Input given by the program itself: four sample log lines.
    std::vector<std::string> logLines = {
        "2024-03-15 10:42:01 INFO  Server started on port 8080",
        "2024-03-15 10:42:05 WARN  Login failed for user Alice, retrying...",
        "2024-03-15 10:42:06 ERROR Login failed for user alice: invalid password",
        "2024-03-15 10:42:09 error Connection timeout, retry scheduled"
    };

    SearchLogX::Trie trie;

    // 1) insertWord(): add every word of every line
    std::cout << "--- insertWord ---\n";
    for (const std::string& line : logLines) {
        std::cout << line << "\n   ->  ";
        std::vector<std::string> words = getWords(line);
        for (const std::string& w : words) trie.insertWord(w);
        printList(words);
    }

    // 2) searchWord(): exact word lookup
    std::cout << "\n--- searchWord ---\n";
    for (const std::string w : {"error", "alice", "retry", "retr", "fatal"})
        std::cout << w << " -> " << (trie.searchWord(w) ? "FOUND" : "not found") << '\n';

    // 3) autocomplete(): all words starting with a prefix
    std::cout << "\n--- autocomplete ---\n";
    for (const std::string p : {"re", "log", "20", "zzz"}) {
        std::cout << p << " -> ";
        printList(trie.autocomplete(p));
    }
    return 0;
}