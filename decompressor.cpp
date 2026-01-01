#include "decompressor.h"
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <utility>
#include <sys/stat.h>
#include "huffmanTree.h"

#define ALPH_SIZE 256

// Create Frequency table from header data
// Read the compressed input file for decompreesion
// Read the header and encoded content
// Create Frequency table from header data
void Decompressor::readFromFileForDecompression(
    const std::string &inputFile, 
    std::string &originalExt, 
    std::map<uint8_t, size_t> &frequencyTable, 
    uint32_t &originalSize, 
    std::string &bitString,
    size_t &lastCol
) {
    // Open the file
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) throw std::runtime_error("Failed to open file " + inputFile); 

    try {
        // Read the original Extension
        uint32_t extLen;
        inFile.read(reinterpret_cast<char *>(&extLen), sizeof(extLen));
        if (inFile.fail()) throw std::runtime_error("Failed reading extension length from " + inputFile);
        if (extLen > 64) throw std::runtime_error("Corrupt header: unreasonable extension length (>64)");
        std::string ext;
        ext.resize(extLen);
        inFile.read(&ext[0], extLen);
        if (inFile.fail()) throw std::runtime_error("Failed reading extension data from " + inputFile);
        originalExt = ext;

        // Read the table size
        uint32_t tableSize;
        inFile.read(reinterpret_cast<char *>(&tableSize), sizeof(tableSize));
        if (inFile.fail()) throw std::runtime_error("Failed reading frequency table size from " + inputFile);
        if (tableSize > 256) throw std::runtime_error("Corrupt header: unreasonable frequency table size (>256)");

        // Read the frequency table
        for (uint32_t i = 0; i < tableSize; i++) {
            unsigned char character;
            size_t frequency;
            inFile.read(reinterpret_cast<char *>(&character), sizeof(character));
            inFile.read(reinterpret_cast<char *>(&frequency), sizeof(frequency));
            if (inFile.fail()) throw std::runtime_error("Failed reading frequency table entry from " + inputFile);
            frequencyTable[character] = frequency;
        }

        // Read the original content size
        inFile.read(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));
        if (inFile.fail()) throw std::runtime_error("Failed reading original size from " + inputFile);
        if (originalSize == 0) throw std::runtime_error("Corrupt header: original size is zero");

        inFile.read(reinterpret_cast<char *>(&lastCol), sizeof(lastCol));
        if (inFile.fail()) throw std::runtime_error("Failed column index for bwt decoding size from " + inputFile);

        // Read the encoded data FIRST
        std::vector<unsigned char> bytes;
        unsigned char byte;
        
        // Read until we hit the last byte (which is the padding info)
        while (inFile.read(reinterpret_cast<char *>(&byte), 1)) {
            bytes.push_back(byte);
        }
        
        if (inFile.bad()) throw std::runtime_error("I/O error while reading encoded data from " + inputFile);
        if (bytes.empty()) throw std::runtime_error("No encoded data found in " + inputFile);
        
        // The LAST byte is the padding information
        unsigned char paddingBits = bytes.back();
        if (paddingBits > 7) throw std::runtime_error("Corrupt trailer: invalid padding bits value");
        bytes.pop_back(); // Remove padding byte from encoded data

        // Convert bytes to binary string (MSB-first per byte)
        for (unsigned char byte : bytes) {
            for (int bit = 7; bit >= 0; --bit) {
                bitString.push_back(((byte >> bit) & 1) ? '1' : '0');
            }
        }

        // Remove padding
        if (paddingBits > 0 && !bitString.empty()) {
            bitString = bitString.substr(0, bitString.size() - paddingBits);
        }
        if (bitString.empty()) throw std::runtime_error("Encoded bitstream is empty after removing padding");
    }
    catch(const std::exception &e) {
        throw std::runtime_error(
            std::string("Failed while reading input file: ") + e.what()
        );
    }
}

// Utility function to calculate the size of file
size_t Decompressor::getFileSize(const std::string &filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

// BWT Decoding
std::string Decompressor::inverseBWT(std::string &encodedString, int idx) {
    int n = encodedString.size();
    std::vector<int> count(256, 0);
    std::vector<int> rank(n);

    for(int i = 0; i < n; i++) {
        rank[i] = count[(unsigned char)encodedString[i]]++;
    }

    std::vector<int> firstPos(256, 0);
    int sum = 0;
    for(int i = 0; i < 256; i++) {
        firstPos[i] = sum;
        sum += count[i];
    }

    std::string result(n, ' ');
    for(int i = n - 1; i >= 0; i--) {
        char c = encodedString[idx];
        result[i] = c;
        idx = firstPos[(unsigned char)c] + rank[idx];
    }

    return result;
}

std::string Decompressor::MTFDecoding(const std::vector<uint8_t>& encodedInput) {
    std::list<char> symbols;
    for(int i = 0; i < ALPH_SIZE; i++)
        symbols.push_back((char)i);

    std::string output;
    output.reserve(encodedInput.size()); // Pre-allocate

    for(uint8_t idx : encodedInput) {
        // Find the character at position 'idx'
        auto it = symbols.begin();
        std::advance(it, idx);  // Move iterator to position idx
        
        char c = *it;
        output.push_back(c);
        
        // Move this character to front
        symbols.erase(it);
        symbols.push_front(c);
    }
    
    return output;
}

// Main Decompression utility
std::pair<size_t, size_t> Decompressor::Decompress(const std::string &inputFile) {
    std::map<uint8_t, size_t> frequencyTable; 
    uint32_t originalSize;                  
    std::string bitString;
    std::string originalExt; 
    size_t lastCol;                           
    size_t inputFileSize = Decompressor::getFileSize(inputFile); 

    // Read the input file for decompression
    Decompressor::readFromFileForDecompression(inputFile, originalExt, frequencyTable, originalSize, bitString, lastCol);

    // Validate header values against simple invariants
    if (frequencyTable.empty()) throw std::runtime_error("Corrupt header: empty frequency table");
    size_t sumFreq = 0;
    for (const auto &p : frequencyTable) sumFreq += p.second;
    if (sumFreq != static_cast<size_t>(originalSize)) throw std::runtime_error("Corrupt header: frequency sum does not match original size");
    if (lastCol >= static_cast<size_t>(originalSize)) throw std::runtime_error("Corrupt header: BWT index out of bounds");

    
    std::vector<uint8_t> decodedMTF;
    if(frequencyTable.size() == 1) {
        // Handle single unique character case
        // The entire file is just this one character repeated
        decodedMTF = std::vector<uint8_t>(originalSize, frequencyTable.begin()->first);
    }
    else {
        // Build the huffman tree from extracted header data
        std::shared_ptr<minHeapNode> root = huffmanTree::buildHuffmanTree(frequencyTable);
        if (!root) throw std::runtime_error("Failed to build Huffman tree");
        
        // Decode file and get string of original content
        decodedMTF = huffmanTree::decodeHuffmanTree(root, bitString, originalSize);
    }

    if (decodedMTF.size() != static_cast<size_t>(originalSize)) throw std::runtime_error("Decoded MTF size mismatch; data may be corrupted");

    std::string mtfDecoded = Decompressor::MTFDecoding(decodedMTF);
    if (mtfDecoded.size() != static_cast<size_t>(originalSize)) throw std::runtime_error("MTF decoding produced unexpected size");
    std::string decodedString = Decompressor::inverseBWT(mtfDecoded, static_cast<int>(lastCol));
    
    // Create output filename with original extension
    size_t dotPos = inputFile.rfind('.');
    std::string baseFilename = (dotPos != std::string::npos) ? inputFile.substr(0, dotPos) : inputFile;
    std::string outputFile = "decompressed_" + baseFilename + originalExt;

    // Open new output file
    std::ofstream outFile(outputFile, std::ios::binary);
    outFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    if (!outFile) throw std::runtime_error("Failed to open the output file for decompression\n");
    
    // Write decoded data to output file
    try {
        outFile.write(decodedString.c_str(), decodedString.size());
        outFile.close(); 
    } catch(const std::exception &e) {
        throw std::runtime_error(std::string("Failed writing decompressed output file: ") + e.what());
    }
    size_t outputFileSize = Decompressor::getFileSize(outputFile); 
    std::cout << "File has been successfully decompressed and saved as " + outputFile << std::endl;

    return std::make_pair(inputFileSize, outputFileSize);
}
