#include "Trie.h"

using namespace std;

namespace SearchLogX {

Trie::Trie() {
    root_ = make_unique<TrieNode>();
}


void Trie::insertWord(const string& word) {
    if(word.empty())
        return;
    TrieNode* node = root_.get();
    for(char c : word) {
        auto& child = node->children[c];    //create slot if not exist
        if(!child)
            child = make_unique<TrieNode>();
        node = child.get();
    }
    node->endOfWord = true;
}

const TrieNode* Trie::findNode(const string& prefix) const {
    const TrieNode* node = root_.get();
    for(char c : prefix) {
        auto it = node->children.find(c);
        if(it == node->children.end())
            return nullptr;
        node = it->second.get();
    }
    return node;
}

bool Trie::searchWord(const std::string& word) const {
    if (word.empty()) 
        return false;
    const TrieNode* node = findNode(word);
    return node != nullptr && node->endOfWord;
}

vector<string> Trie::autocomplete(const std::string& prefix) const {
    vector<string> result;
    if (prefix.empty()) 
        return result;
    const TrieNode* start = findNode(prefix);   // step 1: walk the prefix
    if (start != nullptr) {      // step 2: DFS the subtree
        string current = prefix;
        collect(start, current, result);  
    }          
    return result;
}


void Trie::collect(const TrieNode* node, string& current, vector<std::string>& out) {
    if (node->endOfWord)
        out.emplace_back(current);
    for(const auto& entry : node->children) {
        current.push_back(entry.first);
        collect(entry.second.get(), current, out);
        current.pop_back(); //backtrack
    }
}

}