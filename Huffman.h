#pragma once
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <fstream>
#include <functional>

// Node structure for the Huffman prefix tree
struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, int f, HuffmanNode* l = nullptr, HuffmanNode* r = nullptr)
        : ch(c), freq(f), left(l), right(r) {}
};

// Comparator for min-heap priority queue
struct NodeComparator {
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const {
        return a->freq > b->freq;
    }
};

/*
 * Huffman class encapsulates the entire compression/decompression pipeline.
 * Uses OOP principles to abstract away tree construction, bit-level I/O,
 * binary file header serialization, and lossless text recovery.
 */
class Huffman {
public:
    Huffman();
    ~Huffman();

    // Core public interface
    bool compress(const std::string& inputPath, const std::string& outputPath);
    bool decompress(const std::string& inputPath, const std::string& outputPath);

private:
    HuffmanNode* root;

    // --- Tree construction ---
    std::map<char, int> buildFrequencyTable(const std::string& text);
    HuffmanNode* buildTree(const std::map<char, int>& freqTable);
    void buildCodeMap(HuffmanNode* node, const std::string& code, std::map<char, std::string>& codeMap);

    // --- Encoding / Decoding ---
    std::string encode(const std::string& text, const std::map<char, std::string>& codeMap);
    std::string decode(const std::string& bitString, HuffmanNode* treeRoot);

    // --- Bit-level I/O ---
    void writeBits(std::ofstream& out, const std::string& bitString, int& padding);
    std::string readBits(std::ifstream& in, int padding);

    // --- Binary file header (frequency table + padding metadata) ---
    void writeHeader(std::ofstream& out, const std::map<char, int>& freqTable, int padding);
    bool readHeader(std::ifstream& in, std::map<char, int>& freqTable, int& padding);

    // --- Memory cleanup ---
    void freeTree(HuffmanNode* node);
};
