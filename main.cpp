// C++(STL) program for File Compression/Decompression using Huffman Coding with STL
// use ./a.out <filename> -c to compress file
// use ./a.out <compressed_filename> -d to decompress file

// #include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <queue>
#include <bitset>
#include <stdint.h>
#include <sys/stat.h>
using namespace std;

// A Huffman tree node
struct MinHeapNode
{
    char data;
    unsigned freq;
    MinHeapNode *left, *right;

    MinHeapNode(char data, unsigned freq)
    {
        left = right = NULL;
        this->data = data;
        this->freq = freq;
    }
};

// For comparison of
// two heap nodes (needed in min heap)
struct compare
{
    bool operator()(MinHeapNode *l, MinHeapNode *r)
    {
        return (l->freq > r->freq);
    }
};

// Function Declarations
MinHeapNode *buildHuffmanTree(map<unsigned char, int> &frequencyMap);
void saveCodes(struct MinHeapNode *root, string str, map<unsigned char, string> &huffmanCodes);
void readInputFileForCompression(const string &filename, map<unsigned char, int> &frequencyMap, string &fileContent);
pair<size_t, size_t> Compress(const string &filename);
void writeCompressedFile(const string &inputData, const map<unsigned char, int> frequencyTable, const map<unsigned char, string> huffmanCodes, const string &outputFile, const string &originalExt);
void readFromFileForDecompression(const string &inputFile, string &originalExt, map<unsigned char, int> &frequencyTable, uint32_t &originalSize, string &bitString);
pair<size_t, size_t> Decompress(const string &inputFile);
string decodeHuffmanTree(MinHeapNode *root, string &bitString, uint32_t originalSize);
size_t getFileSize(const string &filename);

// The main function that builds a Huffman Tree and
// print codes by traversing the built Huffman Tree
MinHeapNode *buildHuffmanTree(map<unsigned char, int> &frequencyMap)
{
    priority_queue<MinHeapNode *, vector<MinHeapNode *>,
                   compare>
        minHeap;

    struct MinHeapNode *left, *right, *top;

    // Create a min heap & inserts all characters of frequencyMap
    for (auto &pair : frequencyMap)
    {
        minHeap.push(new MinHeapNode(pair.first, pair.second));
    }

    // Iterate while size of heap doesn't become 1
    while (minHeap.size() != 1)
    {
        // Extract the two minimum
        // freq items from min heap
        left = minHeap.top();
        minHeap.pop();

        right = minHeap.top();
        minHeap.pop();

        top = new MinHeapNode(char(27),
                              left->freq + right->freq);

        top->left = left;
        top->right = right;

        minHeap.push(top);
    }
    return minHeap.top();
}

// Prints huffman codes from
// the root of Huffman Tree.
void saveCodes(struct MinHeapNode *root, string str, map<unsigned char, string> &huffmanCodes)
{
    if (!root)
        return;
    if (root->data != char(27))
    {
        huffmanCodes[root->data] = str;
        // cout << root->data << ": " << str << "\n";
    }
    saveCodes(root->left, str + "0", huffmanCodes);
    saveCodes(root->right, str + "1", huffmanCodes);
}

// Reads the input file for compression
// Stores the frequency of each occuring character in frequency map
void readInputFileForCompression(const string &filename, map<unsigned char, int> &frequencyMap, string &fileContent)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        throw runtime_error("Unable to open " + filename);
    }

    char ch;
    while (file.get(ch))
    {
        unsigned char uch = static_cast<unsigned char>(ch);
        ++frequencyMap[uch];
        fileContent += uch;
    }

    // Close the input file
    file.close();
}

// Main File Compression Utility
pair<size_t, size_t> Compress(const string &filename)
{
    map<unsigned char, int> frequencyMap;    // Map to stores frequency of characters in input file
    map<unsigned char, string> huffmanCodes; // Map to stores generated huffman codes
    string fileContent;                      // String to hold the content of input file
    size_t inputFileSize = getFileSize(filename);
    readInputFileForCompression(filename, frequencyMap, fileContent); // Calculate input file size

    // Extract original extension
    size_t dotPos = filename.rfind('.');
    string originalExt = (dotPos != string::npos) ? filename.substr(dotPos) : "";
    string baseFilename = (dotPos != string::npos) ? filename.substr(0, dotPos) : filename;
    string outFile = baseFilename + ".rsk";

    // Build the huffman tree and get its root node
    MinHeapNode *root = buildHuffmanTree(frequencyMap);

    // Store Huffman codes using
    // the Huffman tree built above
    saveCodes(root, "", huffmanCodes);

    // Write the output compressed file
    writeCompressedFile(fileContent, frequencyMap, huffmanCodes, outFile, originalExt);

    size_t outputFileSize = getFileSize(outFile); // Calculate output file size

    return make_pair(inputFileSize, outputFileSize);
}

// Write the new compressed file
// Write header data and then write all huffman codes
void writeCompressedFile(const string &inputData, const map<unsigned char, int> frequencyTable, const map<unsigned char, string> huffmanCodes, const string &outputFile, const string &originalExt)
{
    // open the outfile or create new
    ofstream outFile(outputFile, ios::binary);
    if (!outFile.is_open())
    {
        throw runtime_error("Failed to create output file");
    }

    // header file writing

    // Write extension length and extension first
    uint32_t extLen = originalExt.length();
    outFile.write(reinterpret_cast<const char *>(&extLen), sizeof(extLen));
    outFile.write(originalExt.c_str(), extLen);

    // Store size of frequecy table
    uint32_t tableSize = frequencyTable.size();
    outFile.write(reinterpret_cast<const char *>(&tableSize), sizeof(tableSize));

    // Store frequency table for deccompression purposes
    for (const auto &pair : frequencyTable)
    {
        outFile.write(reinterpret_cast<const char *>(&pair.first), sizeof(pair.first));
        outFile.write(reinterpret_cast<const char *>(&pair.second), sizeof(pair.second));
    }

    // Store size of contents of original file
    uint32_t originalSize = inputData.size();
    outFile.write(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));

    // Write main encoded character data
    string encodedBits;
    for (unsigned char c : inputData)
    {
        auto it = huffmanCodes.find(c);
        if (it == huffmanCodes.end())
        {
            throw runtime_error("Character not found in the huffman codes\n");
        }
        encodedBits += it->second;
    }

    // Calculating the padding for 8 bits
    unsigned char paddingBits = (8 - (encodedBits.length() % 8)) % 8;
    outFile.write(reinterpret_cast<char *>(&paddingBits), sizeof(paddingBits));
    int padding = paddingBits;

    // Adding padding bits to encoded bits
    encodedBits += string(paddingBits, '0');

    for (size_t i = 0; i < encodedBits.length(); i += 8)
    {
        bitset<8> bits(encodedBits.substr(i, 8));
        unsigned char byte = static_cast<unsigned char>(bits.to_ulong());
        outFile.write(reinterpret_cast<char *>(&byte), 1);
    }
    cout << "File has been successfully compressed and saved as " + outputFile << endl;
    // Close the output file
    outFile.close();
}

// Read the compressed input file for decompreesion
// Read the header and encoded content
// Create Frequency table from header data
void readFromFileForDecompression(const string &inputFile, string &originalExt, map<unsigned char, int> &frequencyTable, uint32_t &originalSize, string &bitString)
{
    // Open the file
    ifstream inFile(inputFile, ios::binary);
    if (!inFile)
    {
        throw runtime_error("Failded to open file " + inputFile);
    }

    // Read the header

    // Read the original Extension
    uint32_t extLen;
    inFile.read(reinterpret_cast<char *>(&extLen), sizeof(extLen));
    char *extBuffer = new char[extLen + 1];
    inFile.read(extBuffer, extLen);
    extBuffer[extLen] = '\0';
    originalExt = string(extBuffer);
    delete[] extBuffer;

    // Read the table size
    uint32_t tableSize;
    inFile.read(reinterpret_cast<char *>(&tableSize), sizeof(tableSize));

    // Read the frequency table
    for (uint32_t i = 0; i < tableSize; i++)
    {
        unsigned char character;
        int frequency;
        inFile.read(reinterpret_cast<char *>(&character), sizeof(character));
        inFile.read(reinterpret_cast<char *>(&frequency), sizeof(frequency));
        frequencyTable[character] = frequency;
    }

    // Read the original content size
    inFile.read(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));

    // Read the padding size
    unsigned char paddingBits;
    inFile.read(reinterpret_cast<char *>(&paddingBits), sizeof(paddingBits));

    // Read the encoded data
    vector<unsigned char> bytes;
    unsigned char byte;
    while (inFile.read(reinterpret_cast<char *>(&byte), 1))
    {
        bytes.push_back(byte);
    }

    // Convert bytes to binary string
    for (unsigned char byte : bytes)
    {
        bitset<8> bits(byte);
        bitString += bits.to_string();
    }

    // Remove padding
    if (paddingBits > 0 && !bitString.empty())
    {
        bitString = bitString.substr(0, bitString.size() - paddingBits);
    }

    // Close the decompressed input file
    inFile.close();
}

// Main Decompression utility
pair<size_t, size_t> Decompress(const string &inputFile)
{
    map<unsigned char, int> frequencyTable; // Map to store frequency table from header of the input file
    uint32_t originalSize;                  // Size of the original file without compression
    string bitString;
    string originalExt;                            // Original extension of the file
    size_t inputFileSize = getFileSize(inputFile); // Size of input file
    // Read the input file for decompression
    readFromFileForDecompression(inputFile, originalExt, frequencyTable, originalSize, bitString);

    // Create output filename with original extension
    size_t dotPos = inputFile.rfind('.');
    string baseFilename = (dotPos != string::npos) ? inputFile.substr(0, dotPos) : inputFile;
    string outputFile = "decompressed_" + baseFilename + originalExt;

    // Build the huffman tree from extracted header data
    MinHeapNode *root = buildHuffmanTree(frequencyTable);

    // Decode file and get string of original content
    string decodedString = decodeHuffmanTree(root, bitString, originalSize);

    // Open new output file
    ofstream outFile(outputFile, ios::binary);
    if (!outFile)
    {
        throw runtime_error("Failed to open the output file for decompression\n");
    }

    // Write decoded data to output file
    outFile.write(decodedString.c_str(), decodedString.size());

    size_t outputFileSize = getFileSize(outputFile); // Calculate size of output file
    cout << "File has been successfully decompressed and saved as " + outputFile << endl;

    // Close the output file
    outFile.close();

    return make_pair(inputFileSize, outputFileSize);
}

// Decode character from the huffman codes
// Start from the root node and traverse the tree based on the bit string
// If current node is leaf node append its character data into decoded string
string decodeHuffmanTree(MinHeapNode *root, string &bitString, uint32_t originalSize)
{
    string decodedString = "";
    MinHeapNode *current = root;

    // Read each bit and traverse the huffman tree accordingly
    for (char bit : bitString)
    {
        if (decodedString.length() >= originalSize)
            break;

        current = (bit == '0') ? current->left : current->right;

        if (!current->left && !current->right)
        {
            decodedString += current->data;
            current = root;
        }
    }
    return decodedString;
}

// Utility function to calculate the size of file
size_t getFileSize(const string &filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

// Driver Code
int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <filename> [-m]" << std::endl;
            return 1;
        }
        string filename = argv[1];

        string arg = argv[2];
        pair<size_t, size_t> sizes;
        if (arg == "-c" || arg == "-C")
        {
            sizes = Compress(filename);
            cout << "Compression complete\n";
            cout << "Initial size: " << sizes.first << " bytes\n";
            cout << "Final size: " << sizes.second << " bytes\n";
            cout << "Compression ratio: " << (100.0 * sizes.second / sizes.first) << "%\n";
        }
        else if (arg == "-d" || arg == "-D")
        {
            sizes = Decompress(filename);
            cout << "Decompression complete\n";
            cout << "Initial size: " << sizes.first << " bytes\n";
            cout << "Final size: " << sizes.second << " bytes\n";
        }
        else
        {
            cout << "Invalid choice\n";
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}