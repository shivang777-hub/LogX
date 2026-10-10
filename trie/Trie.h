#pragma once
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace logx {

// One node = one character on a path from the root. Children are owned by
// unique_ptr, so destroying the root frees the whole tree (no manual delete).
struct TrieNode {
    std::map<char, std::unique_ptr<TrieNode>> children;  // ordered => deterministic traversal
    bool isEndOfWord = false;
    int frequency = 0;  // only meaningful when isEndOfWord is true
};

using WordFrequency = std::pair<std::string, int>;

class Trie {
public:
    Trie();

    // O(L). Adds `frequency` to the word's count (repeat inserts accumulate).
    void insertWord(const std::string& word, int frequency = 1);

    // O(L). True only if the exact word was inserted.
    bool searchWord(const std::string& word) const;

    // O(P + K): walk down the prefix (P chars), then DFS the subtree (K nodes).
    // Returns every stored word starting with `prefix`, unranked.
    std::vector<WordFrequency> autocomplete(const std::string& prefix) const;

private:
    std::unique_ptr<TrieNode> root_;
    std::size_t wordCount_ = 0;

    const TrieNode* findNode(const std::string& prefix) const;
    static void collect(const TrieNode* node, std::string& current,
                        std::vector<WordFrequency>& out);
};

}  // namespace logx
