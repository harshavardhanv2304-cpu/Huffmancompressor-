#include <iostream>
#include <string>
#include <chrono>
#include "Huffman.h"

/*
 * HuffmanCompressor — CLI
 * Usage:
 *   ./huffman compress   <input_file> <output.huf>
 *   ./huffman decompress <input.huf>  <output_file>
 */

void printUsage(const std::string& prog) {
    std::cout << "\nUsage:\n"
              << "  " << prog << " compress   <input_file>   <output.huf>\n"
              << "  " << prog << " decompress <input.huf>    <output_file>\n\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }

    std::string mode      = argv[1];
    std::string inputPath = argv[2];
    std::string outPath   = argv[3];

    Huffman huffman;

    auto start = std::chrono::high_resolution_clock::now();

    bool success = false;
    if (mode == "compress") {
        std::cout << "=== Huffman Compressor ===\n";
        success = huffman.compress(inputPath, outPath);
    } else if (mode == "decompress") {
        std::cout << "=== Huffman Decompressor ===\n";
        success = huffman.decompress(inputPath, outPath);
    } else {
        std::cerr << "[ERROR] Unknown mode: " << mode << "\n";
        printUsage(argv[0]);
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    if (success) {
        std::cout << "[Time] " << elapsed << " seconds\n";
    } else {
        std::cerr << "[FAILED]\n";
        return 1;
    }

    return 0;
}
