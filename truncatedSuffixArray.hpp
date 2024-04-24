#ifndef __TRUNCATED_SUFFIX_ARRAY_HPP__
#define __TRUNCATED_SUFFIX_ARRAY_HPP__
#include <string>

#include "lzhb_common.hpp"
#include "segtree.hpp"
class TruncatedSuffixArray {
  static inline uInt e() { return 0; }
  static inline uInt max(uInt a, uInt b) { return std::max(a, b); }
  const std::string& T;
  atcoder::segtree<uInt, TruncatedSuffixArray::max, TruncatedSuffixArray::e> V;

  // return range [i'..j') in suffix array of T[SA[rank]..SA[rank]+len)
  // given range [i..j) for T[SA[rank]..SA[rank]+len-1)
  std::pair<uInt, uInt> longestPrefixAux(uInt rank, std::pair<uInt, uInt> rng,
                                         uInt len) const;
  int* SA;
  std::vector<int> rankA;

 public:
  TruncatedSuffixArray(const std::string& s);
  ~TruncatedSuffixArray();
  // set length of T[SA[rank]..n) to T[SA[rank]..SA[rank]+len)
  void setLength(uInt pos, uInt len);

  uInt getOcc(std::pair<uInt, uInt> rng, uInt len) const;
  std::vector<uInt> getOccs(std::pair<uInt, uInt> rng, uInt len) const;

  int get(uInt rank) const { return SA[rank]; };
  uInt getRank(uInt pos) const { return rankA[pos]; };
  size_t size() const { return T.size(); }
  // return range [i..j) in suffix array and length of the longest prefix of
  // T[pos..n) in the truncated suffix array
  std::pair<std::pair<uInt, uInt>, uInt> longestPrefix(uInt pos) const;

  // return range [i..j) in suffix array of T[pos..pos+len) in the truncated
  // suffix array when it is known that it occurs
  std::pair<std::pair<uInt, uInt>, uInt> longestPrefix(uInt pos,
                                                       uInt len) const;
};

#endif  //__TRUNCATED_SUFFIX_ARRAY_HPP__
