#include "lzhb3sa.hpp"

#include <iostream>
#include <limits>

#include "segtree.hpp"
#include "truncatedSuffixArray.hpp"

static inline uInt _e() { return 0; }
static inline uInt _max(uInt a, uInt b) { return std::max(a, b); }

// truncate the suffix tree if necessary as when T[pos] becomes terminal
static void truncateStree(TruncatedSuffixArray& stree, uInt pos,
                          const std::vector<uInt>& h, uInt height_bound) {
  if (h[pos] < height_bound) {
    stree.setLength(pos, stree.size() - pos);
  } else {
    uInt k = pos;
    stree.setLength(k, 0);
    while (k-- > 0) {
      if (h[k] >= height_bound) return;
      stree.setLength(k, pos - k);
    }
  }
  return;
}
static void truncateStree(TruncatedSuffixArray& stree, uInt pos,
                          atcoder::segtree<uInt, _max, _e>& h,
                          uInt height_bound) {
  if (h.get(pos) < height_bound) {
    stree.setLength(pos, stree.size() - pos);
  } else {
    uInt k = pos;
    stree.setLength(k, 0);
    while (k-- > 0) {
      if (h.get(k) >= height_bound) return;
      stree.setLength(k, pos - k);
    }
  }
  return;
}

std::vector<lzhb::Phrase> lzhb3sa::parse(const std::string& s,
                                         uInt height_bound) {
  std::vector<lzhb::Phrase> res;
  TruncatedSuffixArray stree(s);
  std::vector<uInt> h(s.size(), 0);
  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce = stree.longestPrefix(pos);
    uInt len = lce.second;
    len = std::max(1u, len);
    uInt src =
        (len > 1) ? stree.getOcc(lce.first, lce.second) : (uint8_t)s[pos];
    if (len <= 1) {
      truncateStree(stree, pos, h, height_bound);
    } else {
      for (uInt i = 0; i < len; i++) {
        h[pos + i] = h[src + i % (pos - src)] + 1;
        truncateStree(stree, pos + i, h, height_bound);
      }
    }
    res.push_back(lzhb::Phrase{.len = len, .src = src});
    pos += len;
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::Phrase> lzhb3sa::parseGreedier(const std::string& s,
                                                 uInt height_bound) {
  std::vector<lzhb::Phrase> res;
  TruncatedSuffixArray stree(s);
  atcoder::segtree<uInt, _max, _e> h(s.size());

  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce = stree.longestPrefix(pos);
    uInt len = lce.second;
    uInt src = (uint8_t)s[pos];
    if (len <= 1) {
      len = 1;
      truncateStree(stree, pos, h, height_bound);
    } else {
      src = std::numeric_limits<uInt>::max();
      auto occs = stree.getOccs(lce.first, lce.second);
      uInt minmaxh = std::numeric_limits<uInt>::max();
      for (auto occ : occs) {
        uInt maxh = h.prod(occ, std::min(occ + len, pos));
        if (maxh < minmaxh || (maxh <= minmaxh && occ < src)) {
          src = occ;
          minmaxh = maxh;
        }
      }
      for (uInt i = 0; i < len; i++) {
        h.set(pos + i, h.get(src + i % (pos - src)) + 1);
        truncateStree(stree, pos + i, h, height_bound);
      }
    }
    res.push_back(lzhb::Phrase{.len = len, .src = src});
    pos += len;
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::PhraseC> lzhb3sa::parseC(const std::string& s,
                                           uInt height_bound) {
  std::vector<lzhb::PhraseC> res;
  TruncatedSuffixArray stree(s);
  std::vector<uInt> h(s.size(), 0);

  uInt pos = 0;
  while (pos < s.size()) {
    // std::cout << "pos = " << pos << std::endl;
    std::pair<std::pair<uInt, uInt>, uInt> lce =
        stree.longestPrefix(pos, s.size() - pos - 1);
    // std::cout << "lce: (" << lce.first.first << "," << lce.first.second <<
    // "),"
    //           << lce.second << std::endl;
    uInt len = lce.second, src = 0;
    if (len == 0) {
      truncateStree(stree, pos, h, height_bound);
      res.push_back(lzhb::PhraseC{.len = 1, .src = 0, .c = s[pos]});
      pos += 1;
    } else {  // len > 0
      src = stree.getOcc(lce.first, lce.second);
      for (uInt i = 0; i < len; i++) {
        h[pos + i] = h[src + i % (pos - src)] + 1;
        truncateStree(stree, pos + i, h, height_bound);
      }
      pos += len;
      truncateStree(stree, pos, h, height_bound);
      len++;
      pos++;
      if (len == 1) src = 0;
      //   std::cout << "{ .len = " << len << ", .src = " << src
      //             << ", .c = " << s[pos - 1] << "}" << std::endl;
      res.push_back(lzhb::PhraseC{.len = len, .src = src, .c = s[pos - 1]});
    }
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::PhraseC> lzhb3sa::parseGreedierC(const std::string& s,
                                                   uInt height_bound) {
  std::vector<lzhb::PhraseC> res;
  TruncatedSuffixArray stree(s);
  atcoder::segtree<uInt, _max, _e> h(s.size());

  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce =
        stree.longestPrefix(pos, s.size() - pos - 1);
    uInt len = lce.second;

    uInt src = (uint8_t)s[pos];
    if (len == 0) {
      truncateStree(stree, pos, h, height_bound);
      res.push_back(lzhb::PhraseC{.len = 1, .src = 0, .c = s[pos]});
      pos += 1;
    } else {
      src = std::numeric_limits<uInt>::max();
      auto occs = stree.getOccs(lce.first, lce.second);
      uInt minmaxh = std::numeric_limits<uInt>::max();
      for (auto occ : occs) {
        uInt maxh = h.prod(occ, std::min(occ + len, pos));
        if (maxh < minmaxh || (maxh <= minmaxh && occ < src)) {
          src = occ;
          minmaxh = maxh;
        }
      }
      for (uInt i = 0; i < len; i++) {
        h.set(pos + i, h.get(src + i % (pos - src)) + 1);
        truncateStree(stree, pos + i, h, height_bound);
      }
      pos += len;
      truncateStree(stree, pos, h, height_bound);
      len++;
      pos++;
      if (len == 1) src = 0;
      res.push_back(lzhb::PhraseC{.len = len, .src = src, .c = s[pos - 1]});
    }
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}
