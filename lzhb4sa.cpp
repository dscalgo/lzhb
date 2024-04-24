#include "lzhb4sa.hpp"

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

std::vector<lzhb::PhraseMod> lzhb4sa::parse(const std::string& s,
                                            uInt height_bound) {
  std::vector<lzhb::PhraseMod> phrases;
  TruncatedSuffixArray stree(s);
  std::vector<uInt> h(s.size(), 0);

  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce = stree.longestPrefix(pos);
    uInt maxper = std::max(1u, lce.second);
    borderArray ba(s, pos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src;  // find source for s[pos..pos+per)
    if (per == 1)
      src = (uint8_t)s[pos];
    else {
      lce = stree.longestPrefix(pos, per);
      src = stree.getOcc(lce.first, lce.second);
    }
    for (size_t i = 0; i < len; i++) {
      uInt v = (per == 1) ? 0 : h[src + (i % per) % (pos - src)] + 1;
      h[pos + i] = v;
      truncateStree(stree, pos + i, h, height_bound);
    }
    phrases.push_back(lzhb::PhraseMod{.len = len, .src = src, .per = per});
    pos += len;
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseModC> lzhb4sa::parseC(const std::string& s,
                                              uInt height_bound) {
  std::vector<lzhb::PhraseModC> phrases;
  TruncatedSuffixArray stree(s);
  std::vector<uInt> h(s.size(), 0);

  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce = stree.longestPrefix(pos);
    uInt maxper = std::max(1u, lce.second);
    borderArray ba(s, pos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src;  // find source for s[pos..pos+per)
    if (per == 1)
      src = (uint8_t)s[pos];
    else {
      lce = stree.longestPrefix(pos, per);
      src = stree.getOcc(lce.first, lce.second);
    }
    for (size_t i = 0; i < len; i++) {
      uInt v = (per == 1) ? 0 : h[src + (i % per) % (pos - src)] + 1;
      h[pos + i] = v;
      truncateStree(stree, pos + i, h, height_bound);
    }
    pos += len;
    if (pos < s.size()) {
      truncateStree(stree, pos, h, height_bound);
      pos++;
      len++;
    }
    if (len == 1) src = 0;
    phrases.push_back(
        lzhb::PhraseModC{.len = len, .src = src, .per = per, .c = s[pos - 1]});
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseMod> lzhb4sa::parseGreedier(const std::string& s,
                                                    uInt height_bound) {
  std::vector<lzhb::PhraseMod> phrases;
  TruncatedSuffixArray stree(s);
  atcoder::segtree<uInt, _max, _e> h(s.size());

  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce = stree.longestPrefix(pos);
    uInt maxper = std::max(1u, lce.second);
    borderArray ba(s, pos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src = -1;  // find source for s[pos..pos+per)
    if (per == 1)
      src = (uint8_t)s[pos];
    else {
      lce = stree.longestPrefix(pos, per);
      auto occs = stree.getOccs(lce.first, lce.second);
      uInt minmaxh = std::numeric_limits<uInt>::max();
      for (auto occ : occs) {
        uInt maxh = h.prod(occ, std::min(occ + per, pos));
        if (maxh < minmaxh || (maxh <= minmaxh && occ < src)) {
          src = occ;
          minmaxh = maxh;
        }
      }
    }
    for (size_t i = 0; i < len; i++) {
      uInt v = (per == 1) ? 0 : h.get(src + (i % per) % (pos - src)) + 1;
      h.set(pos + i, v);
      truncateStree(stree, pos + i, h, height_bound);
    }
    phrases.push_back(lzhb::PhraseMod{.len = len, .src = src, .per = per});
    pos += len;
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseModC> lzhb4sa::parseGreedierC(const std::string& s,
                                                      uInt height_bound) {
  std::vector<lzhb::PhraseModC> phrases;
  TruncatedSuffixArray stree(s);
  atcoder::segtree<uInt, _max, _e> h(s.size());

  uInt pos = 0;
  while (pos < s.size()) {
    std::pair<std::pair<uInt, uInt>, uInt> lce = stree.longestPrefix(pos);
    uInt maxper = std::max(1u, lce.second);
    borderArray ba(s, pos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src = -1;  // find source for s[pos..pos+per)
    if (per == 1)
      src = (uint8_t)s[pos];
    else {
      lce = stree.longestPrefix(pos, per);
      auto occs = stree.getOccs(lce.first, lce.second);
      uInt minmaxh = std::numeric_limits<uInt>::max();
      for (auto occ : occs) {
        uInt maxh = h.prod(occ, std::min(occ + per, pos));
        if (maxh < minmaxh || (maxh <= minmaxh && occ < src)) {
          src = occ;
          minmaxh = maxh;
        }
      }
    }
    for (size_t i = 0; i < len; i++) {
      uInt v = (per == 1) ? 0 : h.get(src + (i % per) % (pos - src)) + 1;
      h.set(pos + i, v);
      truncateStree(stree, pos + i, h, height_bound);
    }
    pos += len;
    if (pos < s.size()) {
      truncateStree(stree, pos, h, height_bound);
      pos++;
      len++;
    }
    if (len == 1) src = 0;
    phrases.push_back(
        lzhb::PhraseModC{.len = len, .src = src, .per = per, .c = s[pos - 1]});
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}
