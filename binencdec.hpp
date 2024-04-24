#ifndef __BINENCDEC_HPP__
#define __BINENCDEC_HPP__
#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lzhb_common.hpp"

typedef uint32_t phrase_t;

typedef uint32_t bitsize_t;

const uint16_t block_size_byte = 1;
const uint16_t block_size_bit = 8 * block_size_byte;

struct BinaryPhrases {
  phrase_t phrase_size;
  bitsize_t bitsize_pos;
  bitsize_t bitsize_src;
  std::vector<uint8_t> bin_pos;
  std::vector<uint8_t> bin_src;
};

struct BinaryPhrasesC {
  phrase_t phrase_size;
  bitsize_t bitsize_pos;
  bitsize_t bitsize_src;
  std::vector<uint8_t> bin_pos;
  std::vector<uint8_t> bin_src;
  std::vector<char> phrase_c;
};

struct BinaryPhrasesMod {
  phrase_t phrase_size;
  bitsize_t bitsize_pos;
  bitsize_t bitsize_src;
  bitsize_t bitsize_per;
  std::vector<uint8_t> bin_pos;
  std::vector<uint8_t> bin_src;
  std::vector<uint8_t> bin_per;
};

struct BinaryPhrasesModC {
  phrase_t phrase_size;
  bitsize_t bitsize_pos;
  bitsize_t bitsize_src;
  bitsize_t bitsize_per;
  std::vector<uint8_t> bin_pos;
  std::vector<uint8_t> bin_src;
  std::vector<uint8_t> bin_per;
  std::vector<char> phrase_c;
};

namespace binencdec {
uInt encodePhrase(std::vector<lzhb::Phrase>& phrase,
                  const std::string& filename, bool output, bool position);

uInt encodePhraseC(std::vector<lzhb::PhraseC>& phrase,
                   const std::string& filename, bool output, bool position);

std::vector<lzhb::Phrase> decodetoPhrase(const std::string& filename,
                                         bool position);

std::vector<lzhb::PhraseC> decodetoPhraseC(const std::string& filename,
                                           bool position);

BinaryPhrases getBinary(const std::string& filename);
BinaryPhrasesC getBinaryC(const std::string& filename);
BinaryPhrasesMod getBinaryMod(const std::string& filename);
BinaryPhrasesModC getBinaryModC(const std::string& filename);

char accessBinary(const std::string& filename, uInt pos);
std::string accessBinary(const std::string& filename, uInt pos, uInt len);
std::string accessBinary(const std::string& filename,
                         std::vector<uInt> positions);
char accessBinaryFile(const std::string& filename, uInt pos);

char accessBinary(const BinaryPhrases& binphrase, uInt pos);
char accessBinary(const BinaryPhrasesC& binphrase, uInt pos);
char accessBinary(const BinaryPhrasesMod& binphrase, uInt pos);
char accessBinary(const BinaryPhrasesModC& binphrase, uInt pos);

size_t encodeTripletPhrase(std::vector<lzhb::PhraseMod>& phrase,
                           const std::string& filename, bool output,
                           bool position);

size_t encodeTripletPhraseC(std::vector<lzhb::PhraseModC>& phrase,
                            const std::string& filename, bool output,
                            bool position);

std::vector<lzhb::PhraseMod> decodetoTripletPhrase(const std::string& filename,
                                                   bool position);

std::vector<lzhb::PhraseModC> decodetoTripletPhraseC(
    const std::string& filename, bool position);

char accessBinaryTriplet(const std::string& filename, uInt pos);
std::string accessBinaryTriplet(const std::string& filename, uInt pos,
                                uInt len);
std::string accessBinaryTriplet(const std::string& filename,
                                std::vector<uInt> positions);

char accessBinaryTripletC(const std::string& filename, size_t pos);

void setBit(uint8_t& block, uint8_t bit, uint8_t pos);

uint8_t getBit(const uint8_t& block, uint8_t pos);

void encodeBlock(bitsize_t bitsize, std::vector<uint8_t>& bin_blocks,
                 std::vector<uInt>& val_blocks);

void decodeBlock(std::vector<uint8_t>& bin_blocks,
                 std::vector<uInt>& val_blocks, uint32_t bitsize, uInt tail);
};  // namespace binencdec

#endif  // __BINENCDEC_HPP__
