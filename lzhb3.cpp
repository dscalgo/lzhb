#include "lzhb3.hpp"

#include "lzhb_common.hpp"
#include "twodminheap.hpp"

// find leftmost occurrence of f in s, where all values in corresponding range
// of h is below hl
static uInt find_leftmost_naive(const std::string& s, uInt cur_size,
                                const std::string& f,
                                const std::vector<uInt>& h, uInt hl) {
  uInt startpos = 0;
  size_t cand = std::string::npos;
  while (true) {
    cand = s.find(f, startpos);
    if (cand == std::string::npos || cand >= cur_size) return -1;
    uInt i;
    for (i = 0; i < f.size(); i++)
      if (h[cand + (i % (cur_size - cand))] >= hl) break;
    if (i == f.size()) return cand;
    startpos = cand + 1;
  }
}

std::vector<lzhb::Phrase> lzhb3::parse_naive(const std::string& s,
                                             uInt height_bound) {
  std::vector<lzhb::Phrase> res;
  size_t phrase_start = 0;
  std::vector<uInt> h;
  while (phrase_start < s.size()) {
    size_t max_len = 0, max_prev_occ = std::string::npos;
    for (size_t l = 2; phrase_start + l <= s.size(); l++) {
      uInt lmostocc = find_leftmost_naive(
          s, phrase_start, s.substr(phrase_start, l), h, height_bound);
      if (lmostocc == (uInt)-1 || lmostocc >= phrase_start) break;
      uInt i;
      for (i = 0; i < l; i++) {  // check height
        if (h[lmostocc + (i % (phrase_start - lmostocc))] >= height_bound)
          break;
      }
      if (i < l) break;
      max_len = l;
      max_prev_occ = lmostocc;
    }
    if (max_len < 2) {
      res.push_back(lzhb::Phrase{.len = 1, .src = (uint8_t)s[phrase_start]});
      phrase_start += 1;
      h.push_back(0);
    } else {
      for (size_t i = 0; i < max_len; i++) {
        h.push_back(h[max_prev_occ + i % (phrase_start - max_prev_occ)] + 1);
      }
      res.push_back(
          lzhb::Phrase{.len = (uInt)max_len, .src = (uInt)max_prev_occ});
      phrase_start += max_len;
    }
    std::cerr << "\r" << phrase_start << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::Phrase> lzhb3::parseGreedier_naive(const std::string& s,
                                                     uInt height_bound) {
  std::vector<lzhb::Phrase> res;
  size_t phrase_start = 0;
  std::vector<uInt> h;
  while (phrase_start < s.size()) {
    uInt max_len = 0, min_max_val = std::numeric_limits<uInt>::max(),
         min_max_pos = -1;
    for (uInt pos = 0; pos < phrase_start; pos++) {
      uInt max_val = 0, l;
      for (l = 0; phrase_start + l < s.size(); l++) {
        uInt src = pos + (l % (phrase_start - pos));
        if (s[pos + l] != s[phrase_start + l] || h[src] >= height_bound) break;
        max_val = std::max(max_val, h[src] + 1);
      }
      if (l >= max_len) {
        if (l > max_len || max_val < min_max_val) {
          min_max_val = max_val;
          min_max_pos = pos;
        }
        max_len = l;
      }
    }
    if (max_len < 2) {
      res.push_back(lzhb::Phrase{.len = 1, .src = (uint8_t)s[phrase_start]});
      phrase_start += 1;
      h.push_back(0);
    } else {
      for (uInt i = 0; i < max_len; i++) {
        h.push_back(h[min_max_pos + i % (phrase_start - min_max_pos)] + 1);
      }
      res.push_back(
          lzhb::Phrase{.len = (uInt)max_len, .src = (uInt)min_max_pos});
      phrase_start += max_len;
    }
    std::cerr << "\r" << phrase_start << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::PhraseC> lzhb3::parseC_naive(const std::string& s,
                                               uInt height_bound) {
  std::vector<lzhb::PhraseC> res;
  uInt phrase_start = 0;
  std::vector<uInt> h;
  while (phrase_start < s.size()) {
    uInt max_len = 0, max_prev_occ = -1;
    for (size_t l = 1; phrase_start + l < s.size(); l++) {
      uInt lmostocc = find_leftmost_naive(
          s, phrase_start, s.substr(phrase_start, l), h, height_bound);
      if (lmostocc == (uInt)-1 || lmostocc >= phrase_start) break;
      size_t i;
      for (i = 0; i < l; i++) {  // check height
        if (h[lmostocc + (i % (phrase_start - lmostocc))] >= height_bound)
          break;
      }
      if (i < l) break;
      max_len = l;
      max_prev_occ = lmostocc;
    }
    if (max_len == 0) {
      res.push_back(lzhb::PhraseC{.len = 1, .src = 0, .c = s[phrase_start]});
      phrase_start += 1;
      h.push_back(0);
    } else {
      for (size_t i = 0; i < max_len; i++) {
        h.push_back(h[max_prev_occ + i % (phrase_start - max_prev_occ)] + 1);
      }
      phrase_start += max_len;
      if (phrase_start < s.size()) {
        h.push_back(0);
        max_len++;
        phrase_start++;
      }
      res.push_back(lzhb::PhraseC{
          .len = max_len, .src = max_prev_occ, .c = s[phrase_start - 1]});
    }
    std::cerr << "\r" << phrase_start << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::Phrase> lzhb3::parse(const string& s, uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::Phrase> res;
  std::vector<uInt> h;
  uInt pos = 0, t = 0;
  while (pos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, pos);
    uInt len = lce.first, src = lce.second - len + 1;
    if (len > 0) {
      string x = s.substr(pos, len);
      uInt candpos = s.find(x, std::max(t, pos - len));
      if (candpos < pos)
        while (pos + len < s.size() && s[candpos + len] == s[pos + len]) len++;
      src = (len > lce.first) ? candpos : src;
    }
    if (len <= 1) {
      res.push_back(lzhb::Phrase{.len = 1, .src = (uint8_t)s[pos]});
      h.push_back(0);
      if (h.back() == height_bound) {
        stree.terminal();
        t = h.size();
      } else {
        stree.addChar(s[pos]);
      }
      pos += 1;
    } else {  // len > 1
      res.push_back(lzhb::Phrase{.len = len, .src = src});
      for (uInt i = 0; i < len; i++) {
        h.push_back(h[src + i % (pos - src)] + 1);
        if (h.back() == height_bound) {
          stree.terminal();
          t = h.size();
        } else {
          stree.addChar(s[pos + i]);
        }
      }
      pos += len;
    }
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::Phrase> lzhb3::parseGreedier(const string& s,
                                               uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::Phrase> res;
  std::vector<uInt> h;
  TwoDMinHeap rmq(s.size());
  uInt pos = 0, t = 0;
  while (pos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, pos);
    uInt len = lce.first, src = lce.second - len + 1;
    if (len > 0) {
      string x = s.substr(pos, len);
      uInt candpos = s.find(x, std::max(t, pos - len));
      if (candpos < pos)
        while (pos + len < s.size() && s[candpos + len] == s[pos + len]) len++;

      uInt minmaxh = 0;

      if (len <= lce.first) {
        vector<uInt> occ = stree.findOccurrences(s, pos, len);
        std::sort(occ.begin(), occ.end());
        minmaxh = rmq.RMQ(occ[0], occ[0] + len - 1);
        src = occ[0];
        for (uInt i = 1; i < occ.size(); i++) {
          uInt maxh = rmq.RMQ(occ[i], occ[i] + len - 1);
          if (maxh > minmaxh) {
            src = occ[i];
            minmaxh = maxh;
          }
        }
      }
      if (candpos < pos) {
        borderArray ba(s, candpos);
        for (uInt i = 0; i < len + (pos - candpos); i++) {
          ba.add();
        }
        uInt per = ba.prefixPeriod();
        for (uInt i = candpos; i < pos; i += per) {
          uInt maxh = rmq.RMQ(i, pos - 1);
          if (maxh > minmaxh) {
            src = i;
            minmaxh = maxh;
          }
        }
      }
    }
    if (len <= 1) {
      res.push_back(lzhb::Phrase{.len = 1, .src = (uint8_t)s[pos]});
      h.push_back(0);
      rmq.addNumber(s.size() - h[h.size() - 1]);
      if (h.back() == height_bound) {
        stree.terminal();
        t = h.size();
      } else {
        stree.addChar(s[pos]);
      }
      pos += 1;
    } else {  // len > 1
      res.push_back(lzhb::Phrase{.len = len, .src = src});
      for (uInt i = 0; i < len; i++) {
        h.push_back(h[src + i % (pos - src)] + 1);
        rmq.addNumber(s.size() - h[h.size() - 1]);
        if (h.back() == height_bound) {
          stree.terminal();
          t = h.size();
        } else {
          stree.addChar(s[pos + i]);
        }
      }
      pos += len;
    }
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::PhraseC> lzhb3::parseC(const string& s, uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::PhraseC> res;
  std::vector<uInt> h;
  uInt pos = 0, t = 0;
  while (pos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, pos, s.size() - 1);
    uInt len = lce.first, src = lce.second - len + 1;
    if (len > 0) {
      string x = s.substr(pos, len);
      uInt candpos = s.find(x, std::max(t, pos - len));
      if (candpos < pos) {
        while (pos + len < s.size() && s[candpos + len] == s[pos + len]) len++;
        if (pos + len == s.size()) len--;
      }
      src = (len > lce.first) ? candpos : src;
    }
    if (len == 0) {
      res.push_back(lzhb::PhraseC{.len = 1, .src = 0, .c = s[pos]});
      h.push_back(0);
      if (h.back() == height_bound) {
        stree.terminal();
        t = h.size();
      } else {
        stree.addChar(s[pos]);
      }
      pos += 1;
    } else {  // len > 0
      for (uInt i = 0; i < len; i++) {
        h.push_back(h[src + i % (pos - src)] + 1);
        if (h.back() == height_bound) {
          stree.terminal();
          t = h.size();
        } else {
          stree.addChar(s[pos + i]);
        }
      }
      pos += len;
      if (pos < s.size()) {
        h.push_back(0);
        if (h.back() == height_bound) {
          stree.terminal();
          t = h.size();
        } else {
          stree.addChar(s[pos]);
        }
        len++;
        pos++;
      }
      res.push_back(lzhb::PhraseC{.len = len, .src = src, .c = s[pos - 1]});
    }
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::PhraseC> lzhb3::parseGreedierC(const std::string& s,
                                                 uInt height_bound) {
  UkkonenSTree stree;
  std::vector<lzhb::PhraseC> res;
  std::vector<uInt> h;
  TwoDMinHeap rmq(s.size());
  uInt pos = 0, t = 0;
  while (pos < s.size()) {
    std::pair<uInt, uInt> lce = stree.matching(s, pos, s.size() - 1);
    uInt len = lce.first, src = lce.second - len + 1;
    if (len > 0) {
      string x = s.substr(pos, len);
      uInt candpos = s.find(x, std::max(t, pos - len));
      if (candpos < pos) {
        while (pos + len < s.size() && s[candpos + len] == s[pos + len]) len++;
        if (pos + len == s.size()) len--;
      }

      uInt minmaxh = 0;

      if (len <= lce.first) {
        vector<uInt> occ = stree.findOccurrences(s, pos, len);
        std::sort(occ.begin(), occ.end());
        minmaxh = rmq.RMQ(occ[0], occ[0] + len - 1);
        src = occ[0];
        for (uInt i = 1; i < occ.size(); i++) {
          uInt maxh = rmq.RMQ(occ[i], occ[i] + len - 1);
          if (maxh > minmaxh) {
            src = occ[i];
            minmaxh = maxh;
          }
        }
      }
      if (candpos < pos) {
        borderArray ba(s, candpos);
        for (uInt i = 0; i < len + (pos - candpos); i++) {
          ba.add();
        }
        uInt per = ba.prefixPeriod();
        for (uInt i = candpos; i < pos; i += per) {
          uInt maxh = rmq.RMQ(i, pos - 1);
          if (maxh > minmaxh) {
            src = i;
            minmaxh = maxh;
          }
        }
      }
    }
    if (len == 0) {
      res.push_back(lzhb::PhraseC{.len = 1, .src = 0, .c = s[pos]});
      h.push_back(0);
      rmq.addNumber(s.size() - h.back());
      if (h.back() == height_bound) {
        stree.terminal();
        t = h.size();
      } else {
        stree.addChar(s[pos]);
      }
      pos += 1;
    } else {  // len >= 1
      for (uInt i = 0; i < len; i++) {
        h.push_back(h[src + i % (pos - src)] + 1);
        rmq.addNumber(s.size() - h.back());
        if (h.back() == height_bound) {
          stree.terminal();
          t = h.size();
        } else {
          stree.addChar(s[pos + i]);
        }
      }
      pos += len;
      if (pos < s.size()) {
        h.push_back(0);
        rmq.addNumber(s.size() - h.back());
        if (h.back() == height_bound) {
          stree.terminal();
          t = h.size();
        } else {
          stree.addChar(s[pos]);
        }
        len++;
        pos++;
      }
      if (len == 1) src = 0;
      res.push_back(lzhb::PhraseC{.len = len, .src = src, .c = s[pos - 1]});
    }
    std::cerr << "\r" << pos << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}

std::vector<lzhb::PhraseC> lzhb3::parseGreedierC_naive(const std::string& s,
                                                       uInt height_bound) {
  std::vector<lzhb::PhraseC> res;
  size_t phrase_start = 0;
  std::vector<uInt> h;
  while (phrase_start < s.size()) {
    uInt max_len = 0, min_max_val = std::numeric_limits<uInt>::max(),
         min_max_pos = -1;
    for (uInt pos = 0; pos < phrase_start; pos++) {
      uInt max_val = 0, l;
      for (l = 0; phrase_start + l + 1 < s.size(); l++) {
        uInt src = pos + (l % (phrase_start - pos));
        if (s[pos + l] != s[phrase_start + l] || h[src] >= height_bound) break;
        max_val = std::max(max_val, h[src] + 1);
      }
      if (l >= max_len) {
        if (l > max_len || max_val < min_max_val) {
          min_max_val = max_val;
          min_max_pos = pos;
        }
        max_len = l;
      }
    }
    if (max_len == 0) {
      res.push_back(lzhb::PhraseC{.len = 1, .src = 0, .c = s[phrase_start]});
      phrase_start += 1;
      h.push_back(0);
    } else {
      for (uInt i = 0; i < max_len; i++) {
        h.push_back(h[min_max_pos + i % (phrase_start - min_max_pos)] + 1);
      }
      phrase_start += max_len;
      if (phrase_start < s.size()) {
        h.push_back(0);
        max_len++;
        phrase_start++;
      }
      res.push_back(lzhb::PhraseC{
          .len = max_len, .src = min_max_pos, .c = s[phrase_start - 1]});
    }
    std::cerr << "\r" << phrase_start << "/" << s.size();
  }
  std::cerr << std::endl;
  return res;
}
