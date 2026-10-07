#ifndef TOPKHEAP_H
#define TOPKHEAP_H

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class TopKHeap {
private:
    std::vector<std::pair<std::string, int> > data;

    bool isSmaller(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) const;
    void heapifyUp(int index);
    void heapifyDown(int index);

public:
    void push(const std::pair<std::string, int>& item);
    std::pair<std::string, int> top() const;
    std::pair<std::string, int> pop();
    void replaceTop(const std::pair<std::string, int>& item);
    bool beatsTop(const std::pair<std::string, int>& item) const;
    int size() const;
    bool empty() const;
};

std::vector<std::pair<std::string, int> > topK(const std::unordered_map<std::string, int>& counts, int k);

#endif
