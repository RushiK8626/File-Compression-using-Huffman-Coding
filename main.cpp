// C++ program for File Compression/Decompression using Huffman Coding with STL
// use ./a.out <filename> -c to compress file
// use ./a.out <compressed_filename> -d to decompress file

#include <iostream>
#include <string>
#include <utility>
#include <stdexcept>
#include "compressor.h"
#include "decompressor.h"

int main(int argc, char *argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " <filename> [-c|-d]" << std::endl;
            return 1;
        }
        std::string filename = argv[1];
        std::string arg = argv[2];
        std::pair<size_t, size_t> sizes;
            if (arg == "-c" || arg == "-C") {
                Compressor C;
                sizes = Compressor::Compress(filename);
                std::cout << "Compression complete\n";
                std::cout << "Initial size: " << sizes.first << " bytes\n";
                if(sizes.second)
                    std::cout << "Final size: " << sizes.second << " bytes\n";
                else
                    std::cout << "Final size: Invalid bytes\n";
                if (sizes.first > 0)
                    std::cout << "Compression ratio: " << (100.0 * sizes.second / sizes.first) << "%\n";
                else
                    std::cout << "Compression ratio: N/A (zero input size)\n";
            }
            else if (arg == "-d" || arg == "-D") {
                sizes = Decompressor::Decompress(filename);
                std::cout << "Decompression complete\n";
                std::cout << "Initial size: " << sizes.first << " bytes\n";
                if(sizes.second)
                    std::cout << "Final size: " << sizes.second << " bytes\n";
                else
                    std::cout << "Final size: Invalid bytes\n";
        }
        else {
            std::cout << "Invalid choice. Use -c to compress or -d to decompress.\n";
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}