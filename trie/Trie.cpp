#include "Trie.h"

namespace logx {

Trie::Trie() : root_(new TrieNode()) {}

void Trie::insertWord(const std::string& word, int frequency) {
    if (word.empty()) return;
    TrieNode* node = root_.get();
    for (char c : word) {
        auto& child = node->children[c];          // creates slot if missing
        if (!child) child.reset(new TrieNode());
        node = child.get();
    }
    if (!node->isEndOfWord) {
        node->isEndOfWord = true;
        ++wordCount_;
    }
    node->frequency += frequency;
}

// Follow one edge per character; nullptr means that path does not exist.
const TrieNode* Trie::findNode(const std::string& prefix) const {
    const TrieNode* node = root_.get();
    for (char c : prefix) {
        auto it = node->children.find(c);
        if (it == node->children.end()) return nullptr;
        node = it->second.get();
    }
    return node;
}

bool Trie::searchWord(const std::string& word) const {
    if (word.empty()) return false;
    const TrieNode* node = findNode(word);
    return node != nullptr && node->isEndOfWord;
}

std::vector<WordFrequency> Trie::autocomplete(const std::string& prefix) const {
    std::vector<WordFrequency> result;
    if (prefix.empty()) return result;
    const TrieNode* start = findNode(prefix);   // step 1: walk the prefix
    if (start == nullptr) return result;        // prefix not in Trie
    std::string current = prefix;
    collect(start, current, result);            // step 2: DFS the subtree
    return result;
}

// DFS: every node flagged isEndOfWord contributes the path string built so far.
void Trie::collect(const TrieNode* node, std::string& current,
                   std::vector<WordFrequency>& out) {
    if (node->isEndOfWord) out.emplace_back(current, node->frequency);
    for (const auto& entry : node->children) {
        current.push_back(entry.first);
        collect(entry.second.get(), current, out);
        current.pop_back();  // backtrack
    }
}

}  // namespace logx
