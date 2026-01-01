# File Compressor using Huffman Coding


This project implements file compression and decompression using a combination of Burrows-Wheeler Transform (BWT), Move-To-Front (MTF) encoding, and Huffman Coding in C++. This multi-stage approach efficiently reduces file sizes by transforming and encoding data based on character patterns and frequency, making it ideal for compressing large text files.

## Features
- Compresses text files using Burrows-Wheeler Transform (BWT), Move-To-Front (MTF), and Huffman Coding
- Decompresses files back to their original content
- Handles large files efficiently
- Modular C++ codebase with clear separation of logic

## File Structure
- `compressor.cpp`, `compressor.h`: Compression logic (BWT, MTF, Huffman Coding)
- `decompressor.cpp`, `decompressor.h`: Decompression logic (inverse BWT, inverse MTF, Huffman Decoding)
- `huffmanTree.cpp`, `huffmanTree.h`: Huffman tree implementation
- `main.cpp`: Entry point for running compression/decompression
- `bigfile.txt`: Example input file
- `bigfile.rsk`: Example compressed file
- `decompressed_bigfile.txt`: Example decompressed output

## Usage
1. **Build the project**
   - Use a C++ compiler (e.g., g++) to compile all `.cpp` files.
   - Example:
     ```sh
     g++ -o file_compressor main.cpp compressor.cpp decompressor.cpp huffmanTree.cpp
     ```
2. **Compress a file**
   - Run the executable and follow prompts to select compression.
3. **Decompress a file**
   - Run the executable and follow prompts to select decompression.


## Compression Pipeline
This project uses a three-stage compression pipeline:

1. **Burrows-Wheeler Transform (BWT):** Rearranges the input data to group similar characters together, making it more amenable to further compression.
2. **Move-To-Front (MTF) Encoding:** Converts sequences of repeated characters into sequences of small integers, further increasing compressibility.
3. **Huffman Coding:** Assigns shorter codes to more frequent symbols and longer codes to less frequent ones, reducing the overall file size without losing information.

Each stage contributes to improved compression efficiency, especially for large text files with repeating patterns.

## License
This project is for educational purposes.


