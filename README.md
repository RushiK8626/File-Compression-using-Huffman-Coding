# C++ File Compressor (Huffman Coding)

Lossless file compression and decompression using Huffman Coding. Compressed files use the `.rsk` extension, and decompression restores the original file extension. Built as a student project for learning purposes.

## Features
- Lossless compression using Huffman Coding, Burrows–Wheeler Transform (BWT) and Move-to-Front (MTF) encoding
- Stores original file extension in the compressed file; restores on decompress
- Works with text and binary files
- Simple CLI: compress (`-c`) or decompress (`-d`)

## Project Structure
- `main.cpp`: Program entry and argument handling
- `compressor.h` / `compressor.cpp`: Compression pipeline
- `decompressor.h` / `decompressor.cpp`: Decompression pipeline
- `huffmanTree.h` / `huffmanTree.cpp`: Huffman tree and code generation
- Sample files: `test.txt`, `test.rsk`, `bigfile.txt`, etc.

## Build
Requires a C++17 compiler (GCC, Clang, or MSVC). Example commands:

- Windows (MinGW/MSYS2 or similar):
```powershell
g++ -std=c++17 -O2 main.cpp compressor.cpp decompressor.cpp huffmanTree.cpp -o main.exe
```

- Linux/macOS:
```bash
g++ -std=c++17 -O2 main.cpp compressor.cpp decompressor.cpp huffmanTree.cpp -o main
```

## Usage
- Compress a file:
	- Windows:
		```powershell
		.\main.exe <input-file> -c
		```
	- Linux/macOS:
		```bash
		./main <input-file> -c
		```
	Output: `<input-file>` is compressed to `<name>.rsk`.

- Decompress a file:
	- Windows:
		```powershell
		.\main.exe <name>.rsk -d
		```
	- Linux/macOS:
		```bash
		./main <name>.rsk -d
		```
	Output: Restores the original extension (e.g., `test.rsk` → `test.txt`).

### Examples
```powershell
# Windows examples
.\main.exe test.txt -c        # produces test.rsk
.\main.exe test.rsk -d        # restores test.txt

.\main.exe compressor.rsk -d  # decompress provided sample
```
```bash
# Linux/macOS examples
./main test.txt -c
./main test.rsk -d
```

## How It Works (Brief)
1. Frequency analysis of input bytes
2. Apply Burrows–Wheeler Transform (BWT), then Move-to-Front (MTF)
3. Build Huffman tree and derive prefix codes over the MTF output
4. Write a compact header (original extension + BWT index + metadata) and the encoded bitstream to `.rsk`
5. Decompression decodes Huffman to recover the MTF sequence, applies inverse BWT, and restores the original data

## BWT and MTF
- **Burrows–Wheeler Transform (BWT):** Rearranges input to cluster identical/similar symbols, creating longer runs and improving locality.
- **Move-to-Front (MTF):** After BWT, maps symbols to indices in a dynamic list, yielding many small numbers that Huffman encodes efficiently.
- **Benefit:** For natural-language text and similar data, applying BWT + MTF before Huffman often improves compression ratio over Huffman alone.
- **Status:** Implemented in this codebase; applied before Huffman to improve compression.


