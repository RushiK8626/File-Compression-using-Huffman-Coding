
# C++ File Compressor

This program compresses and decompresses files using Huffman Coding. It stores compressed files with the extension ".rsk" and restores the original extension during decompression. This program was created me as a student project and is intended for educational purposes

## Usage
- Compression:
```bash
./a.out <filename> -c
```
- Decompression:
```bash
./a.out <filename> -d
```

## How it works:
1. Compression
- Reads the input file and calculates the frequency of each character.
- Builds a Huffman tree, where characters with higher frequencies are assigned shorter codes.
- Replaces characters with their corresponding Huffman codes.
- Writes the compressed data, Huffman code table, and the original file extension to a new file with the ".rsk" extension.

2. Decompression
- Reads the compressed file and extracts the Huffman code table and original file extension.
- Uses the Huffman code table to decode the compressed data.
- Reconstructs the original file with its original extension.

## Note
-This is a basic implementation of Huffman Coding and may not be the most efficient.
- There are many other compression algorithms available, each with its own advantages and disadvantages.


## Authors

- Rushikesh Kadepurkar

