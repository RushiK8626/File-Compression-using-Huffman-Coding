#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <queue>

class minHeapNode {
public:
    size_t data;
    unsigned freq;
    bool isLeaf; 
    std::shared_ptr<minHeapNode> left, right;

    minHeapNode(char data, unsigned freq) 
        : data(data), freq(freq), isLeaf(true), left(nullptr), right(nullptr) {}
    
    minHeapNode(unsigned freq) 
        : data('\0'), freq(freq), isLeaf(false), left(nullptr), right(nullptr) {}
};

class huffmanTree {
public:
    static std::shared_ptr<minHeapNode> buildHuffmanTree(std::map<uint8_t, size_t> &frequencyMap);
    static void saveCodes(const minHeapNode *root, const std::string &str, std::unordered_map<uint8_t, std::string> &huffmanCodes);
    static std::vector<uint8_t> decodeHuffmanTree(const std::shared_ptr<minHeapNode> &root, const std::string &bitString, uint32_t originalSize);
};

#endif //HUFFMAN_TREE_H