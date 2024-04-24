#ifndef __LZH_COMMON_HPP__
#define __LZH_COMMON_HPP__
#include <cinttypes>
#include <iostream>
#include <string>
#include <vector>

typedef uint32_t uInt;

class borderArray {
  const std::string& s;
  std::vector<size_t> a;  // border array for s[bp..ep)
  size_t _bp, _ep;

 public:
  // maintain border array for prefix of str[i..] (initially empty)
  // str must remain valid for add()/naivePrefixPeriod() to function
  borderArray(const std::string& str, size_t i);

  // add next char to prefix
  void add();

  // smallest period of str[i..i+x) where x is the number of times add()
  // is called
  size_t prefixPeriod() const;
  size_t prev_prefixPeriod() const;
  size_t length() const { return (_ep - _bp); }
  size_t ep() const { return _ep; };
  size_t bp() const { return _bp; };
  size_t naivePrefixPeriod() const;
};

namespace lzhb {

// classic LZ phrase
struct Phrase {
  uInt len;
  uInt src;  // interpreted as char if len <= 1
  friend bool operator==(struct Phrase const&, struct Phrase const&) = default;
};

// classic LZ phrase by pos (not length)
struct PhrasePos {
  uInt pos;  // end position of phrase
  uInt src;  // interpreted as char if len <= 1
  friend bool operator==(struct PhrasePos const&,
                         struct PhrasePos const&) = default;
};

// classic LZ phrase with extra char
struct PhraseC {
  uInt len;  // length including c (c is always used)
  uInt src;  // ignored if len <= 1
  char c;    // extra char
  friend bool operator==(struct PhraseC const&,
                         struct PhraseC const&) = default;
};

// classic LZ phrase by pos with extra char
struct PhrasePosC {
  uInt pos;  // end position of phrase including c (c is always used)
  uInt src;  // ignored if len <= 1
  char c;    // extra char
  friend bool operator==(struct PhrasePosC const&,
                         struct PhrasePosC const&) = default;
};

// modified LZ-like phrase
struct PhraseMod {
  // public:
  //  PhraseMod(uInt len, uInt src, uInt per) : len(len), src(src), per(per) {}
  uInt len;
  uInt src;  // interpreted as char if len <= 1
  uInt per;  // period
  friend bool operator==(struct PhraseMod const&,
                         struct PhraseMod const&) = default;
};

// modified LZ-like phrase by pos
struct PhraseModPos {
  // public:
  //  PhraseMod(uInt len, uInt src, uInt per) : len(len), src(src), per(per) {}
  uInt pos;
  uInt src;  // interpreted as char if len <= 1
  uInt per;  // period
  friend bool operator==(struct PhraseModPos const&,
                         struct PhraseModPos const&) = default;
};

// modified LZ-like phrase with extra char
struct PhraseModC {
  uInt len;  // length including c (c is always used)
  uInt src;  // interpreted as char if per = 1. ignored if len <= 1.
  uInt per;  // period
  char c;    // extra char
  friend bool operator==(struct PhraseModC const&,
                         struct PhraseModC const&) = default;
};

// modified LZ-like phrase by pos with extra char
struct PhraseModPosC {
  uInt pos;  // length including c (c is always used)
  uInt src;  // interpreted as char if per = 1. ignored if len <= 1.
  uInt per;  // period
  char c;    // extra char
  friend bool operator==(struct PhraseModPosC const&,
                         struct PhraseModPosC const&) = default;
};

std::string fileread(const std::string& fname);

std::string decode(std::vector<Phrase>& encoding);
std::string decode(std::vector<PhraseC>& encoding);
std::string decode(std::vector<PhraseMod>& encoding);
std::string decode(std::vector<PhraseModC>& encoding);

// returns height array and max height of encoding
std::pair<std::vector<uInt>, uInt> height_array(
    const std::vector<Phrase>& encoding);

std::pair<std::vector<uInt>, uInt> height_array(
    const std::vector<PhraseC>& encoding);

std::pair<std::vector<uInt>, uInt> height_array(
    const std::vector<PhraseMod>& encoding);

std::pair<std::vector<uInt>, uInt> height_array(
    const std::vector<PhraseModC>& encoding);

std::vector<PhrasePos> phrasetoPos(const std::vector<Phrase>& phrase);
std::vector<PhrasePosC> phrasetoPos(const std::vector<PhraseC>& phrase);
std::vector<PhraseModPos> phrasetoPos(const std::vector<PhraseMod>& phrase);
std::vector<PhraseModPosC> phrasetoPos(const std::vector<PhraseModC>& phrase);

char access(const std::vector<Phrase>& phrase, uInt pos);
std::string access(const std::vector<Phrase>& phrase, uInt pos, uInt len);
std::string access(const std::vector<Phrase>& phrase,
                   std::vector<uInt> positions);

char access(const std::vector<PhrasePos>& phrase_pos, uInt pos);
char access(const std::vector<PhrasePosC>& phrase_pos, uInt pos);
char access(const std::vector<PhraseModPos>& phrase_pos, uInt pos);
char access(const std::vector<PhraseModPosC>& phrase_pos, uInt pos);

double average(const std::vector<uInt>& a);
double variance(const std::vector<uInt>& a);

};  // namespace lzhb

#endif  // __LZH_COMMON_HPP__