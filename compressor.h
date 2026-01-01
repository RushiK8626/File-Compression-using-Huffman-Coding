#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <unordered_map>

class Compressor {
    static void readInputFileForCompression(
        const std::string &filename, 
        std::string &fileContent
    );
    
    static void writeCompressedFile(const std::vector<uint8_t> &mtfEncoded, 
        const std::map<uint8_t, size_t> &frequencyTable, 
        const std::unordered_map<uint8_t, std::string> &huffmanCodes, 
        const std::string &outputFile, 
        const std::string &originalExt,
        const size_t lastCol
    );

    static size_t getFileSize(const std::string &filename);

    static std::pair<std::string, size_t> BWTEncoding(std::string &fileContent);
    static std::vector<uint8_t> MTFEncoding(const std::string &inputString);
    
public:
    static std::pair<size_t, size_t> Compress(const std::string &filename) ;
};

#endif // COMPRESSOR_H