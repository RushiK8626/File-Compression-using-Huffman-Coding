#include "huffmanTree.h"
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <stdexcept>

// For comparison of two heap nodes
struct Compare {
    bool operator()(const std::shared_ptr<minHeapNode> &l, 
        const std::shared_ptr<minHeapNode> &r
    ) {
        return l->freq > r->freq;
    }
};

// The main function that builds a Huffman Tree and
// print codes by traversing the built Huffman Tree
std::shared_ptr<minHeapNode> huffmanTree::buildHuffmanTree(std::map<uint8_t, size_t> &frequencyMap) {
    std::priority_queue<std::shared_ptr<minHeapNode>, 
                    std::vector<std::shared_ptr<minHeapNode>>, 
                    Compare> minHeap;

    if (frequencyMap.empty()) throw std::runtime_error("Cannot build Huffman tree from empty frequency map");
    
    // Create a min heap & inserts all characters of frequencyMap
    for (auto &pair : frequencyMap) {
        minHeap.push(std::make_shared<minHeapNode>(pair.first, pair.second));
    }

    while (minHeap.size() != 1) {
        auto left = minHeap.top();
        minHeap.pop();
        auto right = minHeap.top();
        minHeap.pop();
        auto top = std::make_shared<minHeapNode>(left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }
    if (minHeap.empty()) throw std::runtime_error("Huffman tree construction failed: empty heap");
    return minHeap.top();
}

// Saves huffman codes from the Huffman Tree. 
void huffmanTree::saveCodes(const minHeapNode *root, const std::string &str, std::unordered_map<uint8_t, std::string> &huffmanCodes) {
    if (!root) return;
    // Check if it is leaf node from flag
    if (root->isLeaf) {
        huffmanCodes[root->data] = str.empty() ? "0" : str;
        return;
    }
    saveCodes(root->left.get(), str + "0", huffmanCodes);
    saveCodes(root->right.get(), str + "1", huffmanCodes);
}

// Decode character from the huffman codes
// Start from the root node and traverse the tree based on the bit string
// If current node is leaf node append its character data into decoded string
std::vector<uint8_t> huffmanTree::decodeHuffmanTree(const std::shared_ptr<minHeapNode> &root, const std::string &bitString, uint32_t originalSize) {
    std::vector<uint8_t> decodedMTF;
    std::shared_ptr<minHeapNode> current = root;

    // Read each bit and traverse the huffman tree accordingly
    for (char bit : bitString) {
        if (decodedMTF.size() >= originalSize) break;
        if (!current) break;

        current = (bit == '0') ? current->left : current->right;
        if (!current) break;

        if (!current->left && !current->right) {
            decodedMTF.push_back(current->data);
            current = root;
        }
    }
    return decodedMTF;
}