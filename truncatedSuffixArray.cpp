#include "truncatedSuffixArray.hpp"

#include <limits>

#include "libsais.h"
TruncatedSuffixArray::TruncatedSuffixArray(const std::string& s)
    : T(s), V(s.size()), rankA(s.size()) {
  SA = new int[s.size()];

  // divsufsort(reinterpret_cast<const unsigned char*>(s.c_str()), SA,
  // s.size());
  libsais(reinterpret_cast<const uint8_t*>(s.c_str()), SA, s.size(), 0, 0);

  for (uInt i = 0; i < s.size(); i++) rankA[SA[i]] = i;
}

TruncatedSuffixArray::~TruncatedSuffixArray() { delete[] SA; }

void TruncatedSuffixArray::setLength(uInt pos, uInt len) {
  // std::cout << "setting length for pos = " << pos << " to " << len <<
  // std::endl; std::cout << "rankA[pos] = " << rankA[pos] << std::endl;
  V.set(rankA[pos], len);
}

uInt TruncatedSuffixArray::getOcc(std::pair<uInt, uInt> rng, uInt len) const {
  auto elm = V.prod(rng.first, rng.second);
  if (elm < len) return -1;
  auto lt_elm = [&](uInt val) { return val < elm; };
  uInt r = V.max_right(rng.first, lt_elm);
  return (SA[r]);
}

std::vector<uInt> TruncatedSuffixArray::getOccs(std::pair<uInt, uInt> rng,
                                                uInt len) const {
  std::vector<uInt> res;
  std::vector<std::pair<uInt, uInt>> stack;
  stack.push_back(rng);
  while (!stack.empty()) {
    auto rng2 = stack.back();
    stack.pop_back();
    uInt elm = V.prod(rng2.first, rng2.second);
    if (elm >= len) {
      auto lt_elm = [&](uInt val) { return (val < elm); };
      uInt r = V.max_right(rng2.first, lt_elm);
      res.push_back(SA[r]);
      stack.push_back(std::make_pair(rng2.first, r));
      stack.push_back(std::make_pair(r + 1, rng2.second));
    }
  }
  return res;
}

// assumes SA[rng.first..rng.second) is the maximal range for
// T[SA[rank]..SA[rank]+len-1)
std::pair<uInt, uInt> TruncatedSuffixArray::longestPrefixAux(
    uInt rank, std::pair<uInt, uInt> rng, uInt len) const {
  std::pair<uInt, uInt> res = rng;
  if (SA[rank] + len >= T.size()) {  // we can't extend T[SA[rank]..] any more
    return std::make_pair(rank, rank);
  }
  unsigned char c = T[SA[rank] + len];
  // find smallest i >= rng.first such that T[SA[i]+len] = T[SA[rank]+len]
  if (SA[rng.first] + len < T.size() &&
      ((unsigned char)T[SA[rng.first] + len]) == c)
    res.first = rng.first;  // in case rng.first is answer.
  else {  // we know i > rng.first. When r-l == 1, answer will be r.
    uInt l = rng.first, r = rank;
    while (r - l > 1) {
      uInt m = l + (r - l) / 2;
      if (((unsigned char)T[SA[m] + len]) < c) {
        l = m;
      } else {
        r = m;
      }
    }
    res.first = r;
  }
  // find largest j <= rng.second such that T[SA[j]+len] = T[SA[rank]+len]
  uInt l = rank, r = rng.second;
  // When r-l == 1, answer will be r.
  while (r - l > 1) {
    uInt m = l + (r - l) / 2;
    if (((unsigned char)T[SA[m] + len]) <= c) {
      l = m;
    } else {
      r = m;
    }
  }
  res.second = r;
  return res;
}

std::pair<std::pair<uInt, uInt>, uInt> TruncatedSuffixArray::longestPrefix(
    uInt pos) const {
  return longestPrefix(pos, std::numeric_limits<uInt>::max());
}

std::pair<std::pair<uInt, uInt>, uInt> TruncatedSuffixArray::longestPrefix(
    uInt pos, uInt len) const {
  std::pair<std::pair<uInt, uInt>, uInt> res;
  uInt rank = rankA[pos];
  uInt l = 0;
  auto rng = std::make_pair(0, T.size());
  while (l < len) {
    rng = longestPrefixAux(rank, rng, l);
    auto elm = V.prod(rng.first, rng.second);
    if (elm < l + 1) break;
    res.first = rng;
    res.second = ++l;
  }
  return res;
}
