#ifndef trie_h
#define trie_h

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace SearchLogX {

struct TrieNode {
    std::map<char, std::unique_ptr<TrieNode>> children;
    bool endOfWord = false;
};


class Trie {
    std::unique_ptr<TrieNode> root_;

    const TrieNode* findNode(const std ::string& prefix) const;
    static void collect(const TrieNode* node, std::string& current, std::vector<std::string>& out);

public:
    Trie();
    void insertWord(const std::string &word);
    bool searchWord(const std::string &word) const;
    std::vector<std::string> autocomplete(const std::string& prefix) const;
};
}   //end of SearchLogX

#endif