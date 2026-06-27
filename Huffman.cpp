#include "Huffman.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <bitset>

// ─────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────

Huffman::Huffman() : root(nullptr) {}

Huffman::~Huffman() {
    freeTree(root);
}

// Recursive memory cleanup — zero memory leaks
void Huffman::freeTree(HuffmanNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// ─────────────────────────────────────────────
//  Step 1: Build frequency table using STL map
// ─────────────────────────────────────────────

std::map<char, int> Huffman::buildFrequencyTable(const std::string& text) {
    std::map<char, int> freqTable;
    for (char c : text) {
        freqTable[c]++;
    }
    return freqTable;
}

// ─────────────────────────────────────────────
//  Step 2: Build prefix tree using priority_queue (min-heap)
// ─────────────────────────────────────────────

HuffmanNode* Huffman::buildTree(const std::map<char, int>& freqTable) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeComparator> pq;

    for (auto& [ch, freq] : freqTable) {
        pq.push(new HuffmanNode(ch, freq));
    }

    // Edge case: single unique character
    if (pq.size() == 1) {
        HuffmanNode* only = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode('\0', only->freq, only, nullptr);
        return parent;
    }

    while (pq.size() > 1) {
        HuffmanNode* left  = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq, left, right);
        pq.push(merged);
    }

    return pq.top();
}

// ─────────────────────────────────────────────
//  Step 3: Generate variable-length prefix codes (STL map)
// ─────────────────────────────────────────────

void Huffman::buildCodeMap(HuffmanNode* node, const std::string& code,
                            std::map<char, std::string>& codeMap) {
    if (!node) return;

    // Leaf node — store its prefix code
    if (!node->left && !node->right) {
        codeMap[node->ch] = code.empty() ? "0" : code;
        return;
    }

    buildCodeMap(node->left,  code + "0", codeMap);
    buildCodeMap(node->right, code + "1", codeMap);
}

// ─────────────────────────────────────────────
//  Step 4: Encode text to bit string
// ─────────────────────────────────────────────

std::string Huffman::encode(const std::string& text,
                             const std::map<char, std::string>& codeMap) {
    std::string bitString;
    bitString.reserve(text.size() * 4);  // rough reserve
    for (char c : text) {
        bitString += codeMap.at(c);
    }
    return bitString;
}

// ─────────────────────────────────────────────
//  Step 5: Bit-level I/O — write variable-length codes into binary streams
//          Uses 8-bit buffer packing
// ─────────────────────────────────────────────

void Huffman::writeBits(std::ofstream& out, const std::string& bitString, int& padding) {
    // Pad to make length a multiple of 8
    std::string padded = bitString;
    padding = (8 - (padded.size() % 8)) % 8;
    padded.append(padding, '0');

    // Pack every 8 bits into one byte and write
    for (size_t i = 0; i < padded.size(); i += 8) {
        std::bitset<8> byte(padded.substr(i, 8));
        unsigned char byteVal = static_cast<unsigned char>(byte.to_ulong());
        out.put(byteVal);
    }
}

std::string Huffman::readBits(std::ifstream& in, int padding) {
    std::string bitString;
    char byte;
    while (in.get(byte)) {
        std::bitset<8> bits(static_cast<unsigned char>(byte));
        bitString += bits.to_string();
    }
    // Remove padding bits at the end
    if (padding > 0 && bitString.size() >= static_cast<size_t>(padding)) {
        bitString.erase(bitString.size() - padding);
    }
    return bitString;
}

// ─────────────────────────────────────────────
//  Step 6: Decode bit string back to original text
// ─────────────────────────────────────────────

std::string Huffman::decode(const std::string& bitString, HuffmanNode* treeRoot) {
    std::string result;
    HuffmanNode* cur = treeRoot;

    for (char bit : bitString) {
        if (bit == '0') cur = cur->left  ? cur->left  : cur;
        else            cur = cur->right ? cur->right : cur;

        if (!cur->left && !cur->right) {
            result += cur->ch;
            cur = treeRoot;
        }
    }
    return result;
}

// ─────────────────────────────────────────────
//  Binary file header: stores frequency table + bit-padding metadata
//  Format: [numEntries(4B)] [char(1B) freq(4B)] ... [padding(1B)]
// ─────────────────────────────────────────────

void Huffman::writeHeader(std::ofstream& out,
                           const std::map<char, int>& freqTable,
                           int padding) {
    // Write number of unique characters
    int size = static_cast<int>(freqTable.size());
    out.write(reinterpret_cast<const char*>(&size), sizeof(int));

    // Write each (char, frequency) pair
    for (auto& [ch, freq] : freqTable) {
        out.put(ch);
        out.write(reinterpret_cast<const char*>(&freq), sizeof(int));
    }

    // Write padding metadata (1 byte)
    unsigned char pad = static_cast<unsigned char>(padding);
    out.put(pad);
}

bool Huffman::readHeader(std::ifstream& in,
                          std::map<char, int>& freqTable,
                          int& padding) {
    int size = 0;
    if (!in.read(reinterpret_cast<char*>(&size), sizeof(int))) return false;

    for (int i = 0; i < size; ++i) {
        char ch;
        int  freq = 0;
        if (!in.get(ch)) return false;
        if (!in.read(reinterpret_cast<char*>(&freq), sizeof(int))) return false;
        freqTable[ch] = freq;
    }

    unsigned char pad = 0;
    if (!in.get(reinterpret_cast<char&>(pad))) return false;
    padding = static_cast<int>(pad);

    return true;
}

// ─────────────────────────────────────────────
//  Public: Compress
// ─────────────────────────────────────────────

bool Huffman::compress(const std::string& inputPath, const std::string& outputPath) {
    // Read input file
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        std::cerr << "[ERROR] Cannot open input file: " << inputPath << "\n";
        return false;
    }
    std::string text((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();

    if (text.empty()) {
        std::cerr << "[ERROR] Input file is empty.\n";
        return false;
    }

    // Build frequency table, tree, and code map
    auto freqTable = buildFrequencyTable(text);
    freeTree(root);
    root = buildTree(freqTable);

    std::map<char, std::string> codeMap;
    buildCodeMap(root, "", codeMap);

    // Encode text to bit string
    std::string bitString = encode(text, codeMap);

    // Open output and write header + compressed bits
    std::ofstream out(outputPath, std::ios::binary);
    if (!out) {
        std::cerr << "[ERROR] Cannot open output file: " << outputPath << "\n";
        return false;
    }

    int padding = 0;
    // We need padding before writing bits, so we write bits to a temp stream first
    std::ofstream tempOut(outputPath + ".tmp", std::ios::binary);
    writeBits(tempOut, bitString, padding);
    tempOut.close();

    // Now write final file: header first, then compressed data
    writeHeader(out, freqTable, padding);

    std::ifstream tempIn(outputPath + ".tmp", std::ios::binary);
    out << tempIn.rdbuf();
    tempIn.close();
    out.close();

    // Remove temp file
    std::remove((outputPath + ".tmp").c_str());

    // Stats
    long long originalSize   = static_cast<long long>(text.size());
    long long compressedSize = static_cast<long long>(bitString.size() / 8 + 1);
    double ratio = 100.0 * (1.0 - (double)compressedSize / originalSize);

    std::cout << "[Compress] Input  : " << originalSize   << " bytes\n";
    std::cout << "[Compress] Output : " << compressedSize << " bytes\n";
    std::cout << "[Compress] Ratio  : " << ratio          << "% size reduction\n";

    return true;
}

// ─────────────────────────────────────────────
//  Public: Decompress
// ─────────────────────────────────────────────

bool Huffman::decompress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        std::cerr << "[ERROR] Cannot open compressed file: " << inputPath << "\n";
        return false;
    }

    // Reconstruct frequency table and padding from binary header
    std::map<char, int> freqTable;
    int padding = 0;
    if (!readHeader(in, freqTable, padding)) {
        std::cerr << "[ERROR] Failed to read file header.\n";
        return false;
    }

    // Rebuild Huffman tree from frequency table
    freeTree(root);
    root = buildTree(freqTable);

    // Read remaining compressed bits
    std::string bitString = readBits(in, padding);
    in.close();

    // Decode and write original text
    std::string original = decode(bitString, root);

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) {
        std::cerr << "[ERROR] Cannot open output file: " << outputPath << "\n";
        return false;
    }
    out << original;
    out.close();

    std::cout << "[Decompress] Recovered " << original.size() << " bytes\n";
    std::cout << "[Decompress] Output written to: " << outputPath << "\n";

    return true;
}
