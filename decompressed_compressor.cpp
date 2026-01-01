#include "compressor.h"
#include "huffmanTree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ios>
#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <sys/stat.h>
#include <stdexcept>
#include <numeric>
#define ALPH_SIZE 256

// Reads the input file for compression
// Stores the frequency of each occuring character in frequency map
void Compressor::readInputFileForCompression(
    const std::string &filename, 
    std::string &fileContent
) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Unable to open " + filename);

    try {
        // Get file size and reserve space
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        fileContent.reserve(fileSize);  // Pre-allocate memory

        char ch;
        while (file.get(ch)) {
            unsigned char uch = static_cast<unsigned char>(ch);
            fileContent += uch;
        }
        if (file.bad()) throw std::runtime_error("I/O error while reading file " + filename);
    }
    catch(const std::exception &e) {
        throw std::runtime_error(
            std::string("Failed while reading input file: ") + e.what()
        );
    }
}

// Write the new compressed file
// Write header data and then write all huffman codes
void Compressor::writeCompressedFile(
    const std::vector<uint8_t> &mtfEncoded,
    const std::map<uint8_t, size_t> &frequencyTable, 
    const std::unordered_map<uint8_t, std::string> &huffmanCodes, 
    const std::string &outputFile, 
    const std::string &originalExt,
    const size_t lastCol
) {
    // Basic validations for header integrity
    if (frequencyTable.empty()) throw std::runtime_error("Frequency table is empty; nothing to compress");
    if (frequencyTable.size() > 256) throw std::runtime_error("Frequency table size exceeds alphabet size (256)");
    if (mtfEncoded.empty()) throw std::runtime_error("MTF-encoded content is empty; invalid input or encoding failure");
    if (lastCol >= mtfEncoded.size()) throw std::runtime_error("Invalid BWT index; header cannot be written");
    if (originalExt.length() > 64) throw std::runtime_error("Unreasonable original extension length (>64)");

    std::ofstream outFile(outputFile, std::ios::binary);
    outFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    if (!outFile.is_open()) 
        throw std::runtime_error("Failed to create output file"); 

    try {
        // Write extension length and extension first
        uint32_t extLen = originalExt.length();
        outFile.write(reinterpret_cast<const char *>(&extLen), sizeof(extLen));
        outFile.write(originalExt.c_str(), extLen);

        // Store size of frequecy table
        uint32_t tableSize = frequencyTable.size();
        outFile.write(reinterpret_cast<const char *>(&tableSize), sizeof(tableSize));

        // Store frequency table for deccompression purposes
        for (const auto &pair : frequencyTable) {
            outFile.write(reinterpret_cast<const char *>(&pair.first), sizeof(pair.first));
            outFile.write(reinterpret_cast<const char *>(&pair.second), sizeof(pair.second));
        }

        // Store size of contents of original file
        uint32_t originalSize = mtfEncoded.size();
        outFile.write(reinterpret_cast<const char *>(&originalSize), sizeof(originalSize));
        
        outFile.write(reinterpret_cast<const char *>(&lastCol), sizeof(lastCol));

        // Write main encoded character data
        unsigned char currentByte = 0;
        int bitPosition = 7;
        size_t totalBits = 0;

        // Encode and write
        for (auto c : mtfEncoded) {
            auto it = huffmanCodes.find(c);
            if(it == huffmanCodes.end()) 
                throw std::runtime_error("Character not found in huffman codes");
           
            const std::string &code = it->second;
            for(char bit : code) {
                if(bit == '1') currentByte |= (1 << bitPosition);
                
                bitPosition--;
                totalBits++;

                if (bitPosition < 0) {
                    outFile.write(reinterpret_cast<char*>(&currentByte), 1);
                    currentByte = 0;
                    bitPosition = 7;
                }
            }
        }

        if(bitPosition != 7) 
            outFile.write(reinterpret_cast<const char*>(&currentByte), 1);

        // Calculating the padding for 8 bits
        uint8_t paddingBits = (8 - (totalBits % 8)) % 8;
        if (paddingBits > 7) throw std::runtime_error("Calculated invalid padding bits");
        outFile.write(reinterpret_cast<const char *>(&paddingBits), sizeof(paddingBits));

        std::cout << "File has been successfully compressed and saved as " 
             << outputFile << std::endl;
    }
    catch(const std::exception &e) {
        throw std::runtime_error(
            std::string("Failed writing compressed output file: ") + e.what()
        );
    }
}

// Utility function to calculate the size of file
size_t Compressor::getFileSize(const std::string &filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

// Burrows–Wheeler Transform (BWT)
// Rearranges data so similar characters cluster together
// Makes data more repetitive without losing information
std::pair<std::string, size_t> Compressor::BWTEncoding(std::string &fileContent) {
    size_t n = fileContent.size();
    if (n == 0) return {std::string(), static_cast<size_t>(-1)};

    // Sort rotation indices instead of building all rotations to save memory
    std::vector<size_t> idx(n);
    std::iota(idx.begin(), idx.end(), 0);

    auto cmp = [&fileContent, n](size_t a, size_t b) {
        // Compare cyclic rotations starting at a and b
        for (size_t k = 0; k < n; ++k) {
            unsigned char ca = static_cast<unsigned char>(fileContent[(a + k) % n]);
            unsigned char cb = static_cast<unsigned char>(fileContent[(b + k) % n]);
            if (ca < cb) return true;
            if (ca > cb) return false;
        }
        return false; // equal rotations
    };

    std::sort(idx.begin(), idx.end(), cmp);

    std::string lastCol;
    lastCol.reserve(n);
    size_t originalIndex = 0;
    for (size_t i = 0; i < n; ++i) {
        size_t start = idx[i];
        lastCol.push_back(fileContent[(start + n - 1) % n]);
        if (start == 0) originalIndex = i;
    }

    return {lastCol, originalIndex};
}

// Move to Front Encoding
std::vector<uint8_t> Compressor::MTFEncoding(const std::string& inputString) {
    std::list<char> symbols;
    for(int i = 0; i < ALPH_SIZE; i++)
        symbols.push_back((char)i);

    std::vector<uint8_t> output;
    output.reserve(inputString.size()); // Pre-allocate

    for(char c : inputString) {
        uint8_t idx = 0;
        for(auto it = symbols.begin(); it != symbols.end(); ++it, ++idx) {
            if(*it == c) {
                output.push_back(idx);
                symbols.erase(it);
                symbols.push_front(c);
                break;
            }
        }
    }
    return output;
}

// Main File Compression Utility
std::pair<size_t, size_t> Compressor::Compress(const std::string &filename) {
    std::map<uint8_t, size_t> frequencyMap;    
    std::unordered_map<uint8_t, std::string> huffmanCodes; 
    std::string fileContent;                      
    size_t inputFileSize = getFileSize(filename);
    Compressor::readInputFileForCompression(filename, fileContent);
    if (fileContent.empty()) throw std::runtime_error("Input file is empty: " + filename);

    // Generate move the front encoding, highly suitable for huffman coding 
    // Huffman coding naturally exploits this skewed frequency distribution by assigning shorted codes to frequenct symbols
    std::pair<std::string, size_t> bwtEncoding = Compressor::BWTEncoding(fileContent);
    if (bwtEncoding.first.empty()) throw std::runtime_error("BWT encoding failed: produced empty output");
    if (bwtEncoding.second == static_cast<size_t>(-1)) throw std::runtime_error("BWT encoding failed: original index not found");
    std::vector<uint8_t> mtfEncoded = Compressor::MTFEncoding(bwtEncoding.first);

    // Calculate frequencies
    for(int num : mtfEncoded) frequencyMap[num]++;

    // Build the huffman tree and get its root node
    std::shared_ptr<minHeapNode> root = huffmanTree::buildHuffmanTree(frequencyMap);
    if (!root) throw std::runtime_error("Failed to build Huffman tree");
    
    // Store Huffman codes
    huffmanTree::saveCodes(root.get(), "", huffmanCodes);

    // Extract original extension
    size_t dotPos = filename.rfind('.');
    std::string originalExt = (dotPos != std::string::npos) ? filename.substr(dotPos) : "";
    std::string baseFilename = (dotPos != std::string::npos) ? filename.substr(0, dotPos) : filename;
    std::string outFile = baseFilename + ".rsk";

    // Write the output compressed file
    Compressor::writeCompressedFile(mtfEncoded, frequencyMap, huffmanCodes, outFile, originalExt, bwtEncoding.second);

    size_t outputFileSize = getFileSize(outFile); // Calculate output file size
    return std::make_pair(inputFileSize, outputFileSize);
}