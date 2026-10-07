#include "TopKHeap.h"

bool TopKHeap::isSmaller(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) const {
    if (a.second != b.second) {
        return a.second < b.second;
    }
    return a.first > b.first;
}

void TopKHeap::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (isSmaller(data[index], data[parent])) {
            std::pair<std::string, int> temp = data[index];
            data[index] = data[parent];
            data[parent] = temp;
            index = parent;
        } else {
            break;
        }
    }
}

void TopKHeap::heapifyDown(int index) {
    int n = (int)data.size();
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;
        if (left < n && isSmaller(data[left], data[smallest])) {
            smallest = left;
        }
        if (right < n && isSmaller(data[right], data[smallest])) {
            smallest = right;
        }
        if (smallest == index) {
            break;
        }
        std::pair<std::string, int> temp = data[index];
        data[index] = data[smallest];
        data[smallest] = temp;
        index = smallest;
    }
}

void TopKHeap::push(const std::pair<std::string, int>& item) {
    data.push_back(item);
    heapifyUp((int)data.size() - 1);
}

std::pair<std::string, int> TopKHeap::top() const {
    return data[0];
}

std::pair<std::string, int> TopKHeap::pop() {
    std::pair<std::string, int> smallest = data[0];
    data[0] = data[data.size() - 1];
    data.pop_back();
    if (!data.empty()) {
        heapifyDown(0);
    }
    return smallest;
}

void TopKHeap::replaceTop(const std::pair<std::string, int>& item) {
    data[0] = item;
    heapifyDown(0);
}

bool TopKHeap::beatsTop(const std::pair<std::string, int>& item) const {
    if (data.empty()) {
        return true;
    }
    return isSmaller(data[0], item);
}

int TopKHeap::size() const {
    return (int)data.size();
}

bool TopKHeap::empty() const {
    return data.empty();
}

std::vector<std::pair<std::string, int> > topK(const std::unordered_map<std::string, int>& counts, int k) {
    std::vector<std::pair<std::string, int> > result;
    if (k <= 0) {
        return result;
    }

    TopKHeap heap;
    for (std::unordered_map<std::string, int>::const_iterator it = counts.begin(); it != counts.end(); ++it) {
        std::pair<std::string, int> item(it->first, it->second);
        if (heap.size() < k) {
            heap.push(item);
        } else if (heap.beatsTop(item)) {
            heap.replaceTop(item);
        }
    }

    while (!heap.empty()) {
        result.push_back(heap.pop());
    }
    for (int i = 0, j = (int)result.size() - 1; i < j; i++, j--) {
        std::pair<std::string, int> temp = result[i];
        result[i] = result[j];
        result[j] = temp;
    }
    return result;
}
