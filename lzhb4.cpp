#include "lzhb4.hpp"

#include <cassert>
#include <iostream>

#include "lzhb_common.hpp"
#include "suffixtree.hpp"
#include "twodminheap.hpp"

std::vector<lzhb::PhraseMod> lzhb4::parse(const std::string& s,
                                          uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::PhraseMod> phrases;
  std::vector<uInt> h;
  uInt curPos = 0, t = 0;
  while (curPos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, curPos);
    uInt nsrLen = lce.first, srLen = nsrLen, srSrc = -1;
    if (nsrLen > 0) {
      string x = s.substr(curPos, nsrLen);
      srSrc = s.find(x, std::max(t, curPos - nsrLen));
      if (srSrc < curPos)
        while (curPos + srLen < s.size() &&
               s[srSrc + srLen] == s[curPos + srLen])
          srLen++;
    }
    uInt maxper = std::max(srLen, (uInt)1);  // period is at least 1
    borderArray ba(s, curPos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src;  // find source for s[cur_pos..cur_pos+period)
    if (per == 1)
      src = (uint8_t)s[curPos];
    else if (per > nsrLen)  // self referencing period is longest
      src = srSrc;
    else {  // period <= nsrLen : no self-referencing period
      auto pt = make_pair(stree.get_root(), -1);
      for (uInt i = 0; i < per; i++)
        pt = stree.traverse(pt.first, s[curPos + i]);
      src = pt.second - per + 1;
    }

    for (size_t i = 0; i < len; i++) {
      h.push_back((per == 1) ? 0 : h[src + (i % per) % (curPos - src)] + 1);
      if (h.back() < height_bound)
        stree.addChar(s[curPos + i]);
      else {
        stree.terminal();
        t = h.size();
      }
    }
    phrases.push_back(lzhb::PhraseMod{.len = len, .src = src, .per = per});
    curPos += len;
    std::cerr << "\r" << curPos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseModC> lzhb4::parseC(const std::string& s,
                                            uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::PhraseModC> phrases;
  std::vector<uInt> h;
  uInt curPos = 0, t = 0;
  while (curPos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, curPos);
    uInt nsrLen = lce.first, srLen = nsrLen, srSrc = -1;
    if (nsrLen > 0) {
      string x = s.substr(curPos, nsrLen);
      srSrc = s.find(x, std::max(t, curPos - nsrLen));
      if (srSrc < curPos)
        while (curPos + srLen < s.size() &&
               s[srSrc + srLen] == s[curPos + srLen])
          srLen++;
    }
    uInt maxper = std::max(srLen, (uInt)1);  // period is at least 1
    borderArray ba(s, curPos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src;  // find source for s[cur_pos..cur_pos+period)
    if (per == 1)
      src = (uint8_t)s[curPos];
    else if (per > nsrLen)  // self referencing period is longest
      src = srSrc;
    else {  // period <= nsrLen : no self-referencing period
      auto pt = make_pair(stree.get_root(), -1);
      for (uInt i = 0; i < per; i++)
        pt = stree.traverse(pt.first, s[curPos + i]);
      src = pt.second - per + 1;
    }
    for (size_t i = 0; i < len; i++) {
      h.push_back((per == 1) ? 0 : h[src + (i % per) % (curPos - src)] + 1);
      if (h.back() < height_bound)
        stree.addChar(s[curPos + i]);
      else {
        stree.terminal();
        t = curPos + i + 1;
      }
    }
    curPos += len;
    if (curPos < s.size()) {
      h.push_back(0);
      if (h.back() < height_bound)
        stree.addChar(s[curPos]);
      else {
        stree.terminal();
        t = h.size();
      }
      curPos++;
      len++;
    }
    if (len == 1) src = 0;
    phrases.push_back(lzhb::PhraseModC{
        .len = len, .src = src, .per = per, .c = s[curPos - 1]});
    std::cerr << "\r" << curPos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseMod> lzhb4::parse_naive(const std::string& s,
                                                uInt height_bound) {
  std::vector<lzhb::PhraseMod> phrases;
  std::vector<uInt> h;
  uInt cur_pos = 0;
  while (cur_pos < s.size()) {  // find longest phrase starting from cur_pos
    uInt cand_length = 1,       // maximum length found so far
        cand_period = 1,        // period of the candidate phrase
        cand_source = (u_int8_t)s[cur_pos];  // source of the candidate phrase
    // we should first extend via period 1 ---
    borderArray ba(s, cur_pos);
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() != 1) break;
      cand_length = ba.length();
    }
    // if period changed ---------------------
    if (ba.prefixPeriod() > 1) {
      for (uInt source = 0; source < cur_pos; source++) {
        uInt period = 0;
        while (ba.ep() <= s.size()) {
          uInt p;
          for (p = period; p < ba.prefixPeriod(); p++) {
            if (s[source + p] != s[cur_pos + p] ||
                h[source + (p % (cur_pos - source))] >= height_bound) {
              break;  // break if we cannot extend the source
            }
          }
          if (p < ba.prefixPeriod()) break;  // we coudn't extend at source
          period = ba.prefixPeriod();
          if (period > cand_period) {
            cand_period = period;
            cand_source = source;
          }
          cand_length = ba.length();
          if (ba.ep() == s.size()) break;
          ba.add();
        }
      }
    }
    // add the phrase -------------------------
    for (size_t i = 0; i < cand_length; i++) {
      h.push_back(
          (cand_period == 1)
              ? 0
              : h[cand_source + ((i % cand_period) % (cur_pos - cand_source))] +
                    1);
    }
    phrases.push_back(lzhb::PhraseMod{
        .len = cand_length, .src = cand_source, .per = cand_period});
    cur_pos += cand_length;
    std::cerr << "\r" << cur_pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseMod> lzhb4::parseGreedier(const std::string& s,
                                                  uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::PhraseMod> phrases;
  std::vector<uInt> h;
  TwoDMinHeap rmq(s.size());
  uInt curPos = 0, t = 0;
  // std::cout << s << std::endl;
  while (curPos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, curPos);
    uInt nsrLen = lce.first, srLen = nsrLen, srSrc = -1;
    if (nsrLen > 0) {
      string x = s.substr(curPos, nsrLen);
      srSrc = s.find(x, std::max(t, curPos - nsrLen));
      if (srSrc < curPos)
        while (curPos + srLen < s.size() &&
               s[srSrc + srLen] == s[curPos + srLen])
          srLen++;
    }
    uInt maxper = std::max(srLen, (uInt)1);  // period is at least 1
    borderArray ba(s, curPos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src = -1;  // find source for s[cur_pos..cur_pos+period)
    if (per == 1)
      src = (uint8_t)s[curPos];
    else {
      uInt minmaxh = 0;
      if (per <= nsrLen) {
        auto occs = stree.findOccurrences(s, curPos, per);
        std::sort(occs.begin(), occs.end());
        for (auto occ : occs) {
          uInt maxh = rmq.RMQ(occ, occ + per - 1);
          if (maxh > minmaxh) {
            src = occ;
            minmaxh = maxh;
          }
        }
      }
      string x = s.substr(curPos, per);
      srSrc = s.find(x, std::max(t, (per > curPos) ? 0 : (curPos - per)));
      while (srSrc < curPos) {
        uInt maxh = rmq.RMQ(srSrc, curPos - 1);
        if (maxh > minmaxh) {
          src = srSrc;
          minmaxh = maxh;
        }
        srSrc = s.find(x, srSrc + 1);
      }
    }
    for (size_t i = 0; i < len; i++) {
      h.push_back((per == 1) ? 0 : h[src + (i % per) % (curPos - src)] + 1);
      rmq.addNumber(s.size() - h.back());
      if (h.back() < height_bound)
        stree.addChar(s[curPos + i]);
      else {
        stree.terminal();
        t = h.size();
      }
    }
    phrases.push_back(lzhb::PhraseMod{.len = len, .src = src, .per = per});
    curPos += len;
    std::cerr << "\r" << curPos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseMod> lzhb4::parseGreedier_naive(const std::string& s,
                                                        uInt height_bound) {
  std::vector<lzhb::PhraseMod> phrases;
  std::vector<uInt> h;
  uInt cur_pos = 0;
  while (cur_pos < s.size()) {  // find longest phrase starting from cur_pos
    uInt cand_length = 0,       // maximum length found so far
        min_max_height =
            std::numeric_limits<uInt>::max(),  // minimum of the maximum
                                               // height of longest phrase
                                               // found so far
        cand_period = 1,                       // period of the candidate phrase
        cand_source = (uint8_t)s[cur_pos];     // source of the candidate phrase
    for (uInt source = 0; source < cur_pos; source++) {
      borderArray ba(s, cur_pos);
      uInt period = 0, length = 0, maxval = 0;
      while (ba.ep() < s.size()) {
        ba.add();
        if (ba.prefixPeriod() > period) {
          uInt p;
          for (p = period; p < ba.prefixPeriod(); p++) {
            if (s[source + p] != s[cur_pos + p] ||
                h[source + (p % (cur_pos - source))] >= height_bound) {
              break;  // break if we cannot extend the source
            }
          }
          if (p < ba.prefixPeriod()) break;  // we coudn't extend at source
          for (p = period; p < ba.prefixPeriod(); p++)
            maxval = std::max(maxval, h[source + (p % (cur_pos - source))] + 1);
          period = ba.prefixPeriod();
        }
        length = ba.length();
      }
      if (period > cand_period ||
          (period == cand_period && maxval < min_max_height)) {
        cand_period = period;
        cand_source = source;
        cand_length = length;
        min_max_height = maxval;
      }
    }
    if (cand_length == 0 || cand_period == 1) {
      while (s[cur_pos] == s[cur_pos + cand_length]) cand_length++;
      cand_source = (uint8_t)s[cur_pos];
    }

    // add the phrase -------------------------
    for (size_t i = 0; i < cand_length; i++) {
      h.push_back(
          (cand_period == 1)
              ? 0
              : h[cand_source + ((i % cand_period) % (cur_pos - cand_source))] +
                    1);
    }
    phrases.push_back(lzhb::PhraseMod{
        .len = cand_length, .src = cand_source, .per = cand_period});
    cur_pos += cand_length;
    std::cerr << "\r" << cur_pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseModC> lzhb4::parseC_naive(const std::string& s,
                                                  uInt height_bound) {
  std::vector<lzhb::PhraseModC> phrases;
  std::vector<uInt> h;
  uInt cur_pos = 0;
  while (cur_pos < s.size()) {  // find longest phrase starting from cur_pos
    uInt cand_length = 1,       // maximum length found so far
        cand_period = 1,        // period of the candidate phrase
        cand_source = (u_int8_t)s[cur_pos];  // source of the candidate phrase
    // we should first extend via period 1 ---
    borderArray ba(s, cur_pos);
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() != 1) break;
      cand_length = ba.length();
    }
    // if period changed ---------------------
    if (ba.prefixPeriod() > 1) {
      for (uInt source = 0; source < cur_pos; source++) {
        uInt period = 0;
        while (ba.ep() <= s.size()) {
          uInt p;
          for (p = period; p < ba.prefixPeriod(); p++) {
            if (s[source + p] != s[cur_pos + p] ||
                h[source + (p % (cur_pos - source))] >= height_bound) {
              break;  // break if we cannot extend the source
            }
          }
          if (p < ba.prefixPeriod()) break;  // we coudn't extend at source
          period = ba.prefixPeriod();
          if (period > cand_period) {
            cand_period = period;
            cand_source = source;
          }
          cand_length = ba.length();
          if (ba.ep() == s.size()) break;
          ba.add();
        }
      }
    }
    // add the phrase -------------------------
    for (size_t i = 0; i < cand_length; i++) {
      h.push_back(
          (cand_period == 1)
              ? 0
              : h[cand_source + ((i % cand_period) % (cur_pos - cand_source))] +
                    1);
    }
    cur_pos += cand_length;
    if (cur_pos < s.size()) {
      h.push_back(0);
      cur_pos++;
      cand_length++;
    }
    if (cand_length == 1) cand_source = 0;
    phrases.push_back(lzhb::PhraseModC{.len = cand_length,
                                       .src = cand_source,
                                       .per = cand_period,
                                       .c = s[cur_pos - 1]});
    std::cerr << "\r" << cur_pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseModC> lzhb4::parseGreedierC_naive(const std::string& s,
                                                          uInt height_bound) {
  std::vector<lzhb::PhraseModC> phrases;
  std::vector<uInt> h;
  uInt cur_pos = 0;
  while (cur_pos < s.size()) {  // find longest phrase starting from cur_pos
    uInt cand_length = 0,       // maximum length found so far
        min_max_height =
            std::numeric_limits<uInt>::max(),  // minimum of the maximum
                                               // height of longest phrase
                                               // found so far
        cand_period = 1,                       // period of the candidate phrase
        cand_source = (uint8_t)s[cur_pos];     // source of the candidate phrase
    for (uInt source = 0; source < cur_pos; source++) {
      borderArray ba(s, cur_pos);
      uInt period = 0, length = 0, maxval = 0;
      while (ba.ep() < s.size()) {
        ba.add();
        if (ba.prefixPeriod() > period) {
          uInt p;
          for (p = period; p < ba.prefixPeriod(); p++) {
            if (s[source + p] != s[cur_pos + p] ||
                h[source + (p % (cur_pos - source))] >= height_bound) {
              break;  // break if we cannot extend the source
            }
          }
          if (p < ba.prefixPeriod()) break;  // we coudn't extend at source
          for (p = period; p < ba.prefixPeriod(); p++)
            maxval = std::max(maxval, h[source + (p % (cur_pos - source))] + 1);
          period = ba.prefixPeriod();
        }
        length = ba.length();
      }
      if (period > cand_period ||
          (period == cand_period && maxval < min_max_height)) {
        cand_period = period;
        cand_source = source;
        cand_length = length;
        min_max_height = maxval;
      }
    }
    if (cand_length == 0 || cand_period == 1) {
      while (s[cur_pos] == s[cur_pos + cand_length]) cand_length++;
      cand_source = (uint8_t)s[cur_pos];
    }

    // add the phrase -------------------------
    for (size_t i = 0; i < cand_length; i++) {
      h.push_back(
          (cand_period == 1)
              ? 0
              : h[cand_source + ((i % cand_period) % (cur_pos - cand_source))] +
                    1);
    }
    cur_pos += cand_length;
    if (cur_pos < s.size()) {
      h.push_back(0);
      cur_pos++;
      cand_length++;
    }
    if (cand_length == 1) cand_source = 0;
    phrases.push_back(lzhb::PhraseModC{.len = cand_length,
                                       .src = cand_source,
                                       .per = cand_period,
                                       .c = s[cur_pos - 1]});
    std::cerr << "\r" << cur_pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}

std::vector<lzhb::PhraseModC> lzhb4::parseGreedierC(const std::string& s,
                                                    uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::PhraseModC> phrases;
  std::vector<uInt> h;
  TwoDMinHeap rmq(s.size());
  uInt curPos = 0, t = 0;
  while (curPos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, curPos);
    uInt nsrLen = lce.first, srLen = nsrLen, srSrc = -1;
    if (nsrLen > 0) {
      string x = s.substr(curPos, nsrLen);
      srSrc = s.find(x, std::max(t, curPos - nsrLen));
      if (srSrc < curPos)
        while (curPos + srLen < s.size() &&
               s[srSrc + srLen] == s[curPos + srLen])
          srLen++;
    }
    uInt maxper = std::max(srLen, (uInt)1);  // period is at least 1
    borderArray ba(s, curPos);
    uInt per = 0, len = 0;
    while (ba.ep() < s.size()) {
      ba.add();
      if (ba.prefixPeriod() > maxper) break;
      per = ba.prefixPeriod();
      len = ba.length();
    }
    uInt src = -1;  // find source for s[cur_pos..cur_pos+period)
    if (per == 1)
      src = (uint8_t)s[curPos];
    else {
      uInt minmaxh = 0;
      if (per <= nsrLen) {
        auto occs = stree.findOccurrences(s, curPos, per);
        std::sort(occs.begin(), occs.end());
        for (auto occ : occs) {
          uInt maxh = rmq.RMQ(occ, occ + per - 1);
          if (maxh > minmaxh) {
            src = occ;
            minmaxh = maxh;
          }
        }
      }
      string x = s.substr(curPos, per);
      srSrc = s.find(x, std::max(t, (per > curPos) ? 0 : (curPos - per)));
      while (srSrc < curPos) {
        uInt maxh = rmq.RMQ(srSrc, curPos - 1);
        if (maxh > minmaxh) {
          src = srSrc;
          minmaxh = maxh;
        }
        srSrc = s.find(x, srSrc + 1);
      }
    }
    for (size_t i = 0; i < len; i++) {
      h.push_back((per == 1) ? 0 : h[src + (i % per) % (curPos - src)] + 1);
      rmq.addNumber(s.size() - h.back());
      if (h.back() < height_bound)
        stree.addChar(s[curPos + i]);
      else {
        stree.terminal();
        t = h.size();
      }
    }
    curPos += len;
    if (curPos < s.size()) {
      h.push_back(0);
      rmq.addNumber(s.size() - h.back());
      if (h.back() < height_bound) {
        stree.addChar(s[curPos]);
      } else {
        stree.terminal();
        t = h.size();
      }
      curPos++;
      len++;
    }
    if (len == 1) src = 0;
    phrases.push_back(lzhb::PhraseModC{
        .len = len, .src = src, .per = per, .c = s[curPos - 1]});
    std::cerr << "\r" << curPos << "/" << s.size();
  }
  std::cerr << std::endl;
  return phrases;
}
