#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H
#include <string>
#include <map>
#include <utility>
#include <vector>

class Decompressor {
    static void readFromFileForDecompression(
        const std::string &inputFile, 
        std::string &originalExt, 
        std::map<uint8_t, size_t> &frequencyTable, 
        uint32_t &originalSize, 
        std::string &bitString, 
        size_t &lastCol
    );
    static size_t getFileSize(const std::string &filename);
    static std::string inverseBWT(std::string &encodedString, int idx);
    static std::string MTFDecoding(const std::vector<uint8_t>& encodedInput);
public:
    static std::pair<size_t, size_t> Decompress(const std::string &inputFile);
};

#endif // DECOMPRESSOR_H