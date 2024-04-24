#include "lzhb_common.hpp"

#include <cstdlib>
#include <fstream>

borderArray::borderArray(const std::string& str, size_t i)
    : s(str), _bp(i), _ep(i) {}

void borderArray::add() {
  if (_ep == s.size()) {
    std::abort();
  }
  char c = s[_ep];
  size_t len = (_ep - _bp);
  _ep++;
  if (len == 0) {
    a.push_back(0);
    return;
  }
  size_t p = len - 1;
  while (true) {
    if (s[_bp + a[p]] == c) {
      a.push_back(a[p] + 1);
      return;
    }
    if (a[p] == 0) {
      a.push_back(0);
      return;
    }
    p = a[p] - 1;
  }
  return;
}

size_t borderArray::prefixPeriod() const {
  if (a.empty())
    return 0;
  else
    return (_ep - _bp) - a.back();
}

size_t borderArray::prev_prefixPeriod() const {
  if (a.size() < 1)
    return 0;
  else
    return (_ep - _bp) - a[a.size() - 1];
}

size_t borderArray::naivePrefixPeriod() const {
  if (_ep == _bp)
    return 0;
  else {
    size_t res = _ep - _bp;
    for (size_t p = 1; p < res; p++) {
      size_t i;
      for (i = 0; i + p < res; i++) {
        if (s[_bp + i] != s[_bp + i + p])
          break;
      }
      if (i + p == res)
        return p;
    }
    return res;
  }
}

std::string lzhb::fileread(const std::string& fname) {
  std::ifstream ifs(fname);  // 読み込むファイルのパスを指定
  if (!ifs)
    throw std::system_error(errno, std::system_category(),
                            "failed to open " + fname);

  std::string str((std::istreambuf_iterator<char>(ifs)),
                  std::istreambuf_iterator<char>());

  return str;
}

std::string lzhb::decode(std::vector<Phrase>& encoding) {
  std::string res;
  for (size_t i = 0; i < encoding.size(); i++) {
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    if (len <= 1) {
      res.push_back(static_cast<char>(src));
    } else {
      for (size_t j = 0; j < len; j++)
        res.push_back(res[src++]);
    }
  }
  return res;
}

std::string lzhb::decode(std::vector<PhraseC>& encoding) {
  std::string res;
  for (size_t i = 0; i < encoding.size(); i++) {
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    if (len <= 1) {
      res.push_back(static_cast<char>(encoding[i].c));
    } else {
      for (size_t j = 0; j < len - 1; j++)
        res.push_back(res[src++]);
      res.push_back(encoding[i].c);
    }
  }
  return res;
}
std::string lzhb::decode(std::vector<PhraseMod>& encoding) {
  std::string res;
  for (size_t i = 0; i < encoding.size(); i++) {
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    uInt per = encoding[i].per;
    if (len < per)
      throw("error: phrase length < period");
    if (per == 1) {
      for (size_t j = 0; j < len; j++)
        res.push_back(static_cast<char>(src));
    } else {
      for (size_t j = 0; j < len; j++)
        res.push_back(res[src + (j % per)]);
    }
  }
  return res;
}

std::string lzhb::decode(std::vector<PhraseModC>& encoding) {
  std::string res;
  for (size_t i = 0; i < encoding.size(); i++) {
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    uInt per = encoding[i].per;
    char c = encoding[i].c;
    if (len < per)
      throw("error: phrase length < period");
    if (per == 1) {
      for (size_t j = 0; j < len - 1; j++)
        res.push_back(static_cast<char>(src));
    } else {
      for (size_t j = 0; j < len - 1; j++)
        res.push_back(res[src + (j % per)]);
    }
    res.push_back(c);
  }
  return res;
}

// compute maximum height of referencing with shortcuts
std::pair<std::vector<uInt>, uInt> lzhb::height_array(
    const std::vector<Phrase>& encoding) {
  std::vector<uInt> h;
  uInt maxh = 0;
  for (size_t i = 0; i < encoding.size(); i++) {
    size_t q = h.size();
    uInt j;
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    if (len <= 1) {
      h.push_back(0);
    } else {
      for (j = 0; j < len; j++) {
        h.push_back(h[src++] + 1);
        maxh = std::max(maxh, h.back());
        if (src == q)
          src = encoding[i].src;
      }
    }
  }
  return std::pair<std::vector<uInt>, uInt>(h, maxh);
}

// compute maximum height of referencing with shortcuts
std::pair<std::vector<uInt>, uInt> lzhb::height_array(
    const std::vector<PhraseMod>& encoding) {
  std::vector<uInt> h;
  uInt maxh = 0, pstart = 0;
  for (size_t i = 0; i < encoding.size(); i++) {
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    uInt per = encoding[i].per;
    if (len < per)
      throw("error: phrase length < period");
    if (per == 1) {
      for (size_t j = 0; j < len; j++)
        h.push_back(0);
    } else {
      for (size_t j = 0; j < len; j++) {
        h.push_back(h[src + (j % per) % (pstart - src)] + 1);
        maxh = std::max(maxh, h.back());
      }
    }
    pstart += len;
  }
  return std::pair<std::vector<uInt>, uInt>(h, maxh);
}

// compute maximum height of referencing with shortcuts
std::pair<std::vector<uInt>, uInt> lzhb::height_array(
    const std::vector<PhraseC>& encoding) {
  std::vector<uInt> h;
  uInt maxh = 0;
  for (size_t i = 0; i < encoding.size(); i++) {
    size_t q = h.size();
    uInt x = encoding[i].len;
    uInt y = encoding[i].src;
    if (x == 0)
      throw("error: phrase length = 0");
    for (size_t j = 0; j < x - 1; j++) {
      h.push_back(h[y++] + 1);
      maxh = std::max(maxh, h.back());
      if (y == q)
        y = encoding[i].src;
    }
    h.push_back(0);
  }
  return std::pair<std::vector<uInt>, uInt>(h, maxh);
}

// compute maximum height of referencing with shortcuts
std::pair<std::vector<uInt>, uInt> lzhb::height_array(
    const std::vector<PhraseModC>& encoding) {
  std::vector<uInt> h;
  uInt maxh = 0, pstart = 0;
  for (size_t i = 0; i < encoding.size(); i++) {
    uInt len = encoding[i].len;
    uInt src = encoding[i].src;
    uInt per = encoding[i].per;
    if (len == 0)
      throw("error: phrase length = 0");
    if (per == 1) {
      for (size_t j = 0; j < len; j++)
        h.push_back(0);
    } else {
      for (size_t j = 0; j < len - 1; j++) {
        h.push_back(h[src + (j % per) % (pstart - src)] + 1);
        maxh = std::max(maxh, h.back());
      }
      h.push_back(0);
    }
    pstart += len;
  }
  return std::pair<std::vector<uInt>, uInt>(h, maxh);
}

std::vector<lzhb::PhrasePos> lzhb::phrasetoPos(
    const std::vector<lzhb::Phrase>& phrase) {
  std::vector<lzhb::PhrasePos> phrase_pos;
  uInt p = 0;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(lzhb::PhrasePos{.pos = p, .src = phrase[i].src});
  }
  return phrase_pos;
}

std::vector<lzhb::PhrasePosC> lzhb::phrasetoPos(
    const std::vector<lzhb::PhraseC>& phrase) {
  std::vector<lzhb::PhrasePosC> phrase_pos;
  uInt p = 0;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(
        lzhb::PhrasePosC{.pos = p, .src = phrase[i].src, .c = phrase[i].c});
  }
  return phrase_pos;
}

std::vector<lzhb::PhraseModPos> lzhb::phrasetoPos(
    const std::vector<lzhb::PhraseMod>& phrase) {
  std::vector<lzhb::PhraseModPos> phrase_pos;
  uInt p = 0;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(lzhb::PhraseModPos{
        .pos = p, .src = phrase[i].src, .per = phrase[i].per});
  }
  return phrase_pos;
}

std::vector<lzhb::PhraseModPosC> lzhb::phrasetoPos(
    const std::vector<lzhb::PhraseModC>& phrase) {
  std::vector<lzhb::PhraseModPosC> phrase_pos;
  uInt p = 0;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(lzhb::PhraseModPosC{.pos = p,
                                             .src = phrase[i].src,
                                             .per = phrase[i].per,
                                             .c = phrase[i].c});
  }
  return phrase_pos;
}

char lzhb::access(const std::vector<Phrase>& phrase, uInt pos) {
  char c = 0;
  uInt p = 0;
  std::vector<uInt> phrase_pos;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(p);
  }
  while (true) {
    size_t pos_min = 0;
    size_t pos_max = phrase.size();
    // size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t pos_string = phrase_pos[pos_phrase];
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    pos_cur = phrase_pos[pos_phrase];

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_phrase = pos_min - 1;
      pos_prev = phrase_pos[pos_phrase];
    }

    size_t val = phrase[pos_min].src;

    if (pos_cur - pos_prev <= 1) {
      c = (char)val;
      return c;
    } else {
      size_t per = pos_prev - val;
      pos = val + (pos - pos_prev) % per;
    }
  }
  return c;
}

std::string lzhb::access(const std::vector<Phrase>& phrase,
                         uInt pos,
                         uInt len) {
  std::string s(len, 'a');
  uInt p = 0;
  uInt head = pos;
  std::vector<uInt> phrase_pos;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(p);
  }
  for (uInt j = 0; j < len; j++) {
    pos = head + j;
    while (true) {
      size_t pos_min = 0;
      size_t pos_max = phrase.size();
      // size_t pos_string = 0;
      while (pos_min < pos_max) {
        size_t pos_phrase = (pos_min + pos_max) / 2;
        size_t pos_string = phrase_pos[pos_phrase];
        if (pos >= pos_string) {
          pos_min = pos_phrase + 1;
        } else {
          pos_max = pos_phrase;
        }
      }

      size_t pos_prev = 0;
      size_t pos_cur = 0;

      size_t pos_phrase = pos_min;
      pos_cur = phrase_pos[pos_phrase];

      if (pos_min == 0) {
        pos_prev = 0;
      } else {
        pos_phrase = pos_min - 1;
        pos_prev = phrase_pos[pos_phrase];
      }

      size_t val = phrase[pos_min].src;

      if (pos_cur - pos_prev <= 1) {
        s[j] = (char)val;
        break;
      } else {
        size_t per = pos_prev - val;
        pos = val + (pos - pos_prev) % per;
      }
    }
  }
  return s;
}

std::string lzhb::access(const std::vector<Phrase>& phrase,
                         std::vector<uInt> positions) {
  std::string s(positions.size(), 'a');
  uInt p = 0;
  std::vector<uInt> phrase_pos;
  for (size_t i = 0; i < phrase.size(); i++) {
    p += std::max((uInt)1, phrase[i].len);
    phrase_pos.push_back(p);
  }
  for (uInt j = 0; j < positions.size(); j++) {
    uInt pos = positions[j];
    while (true) {
      size_t pos_min = 0;
      size_t pos_max = phrase.size();
      // size_t pos_string = 0;
      while (pos_min < pos_max) {
        size_t pos_phrase = (pos_min + pos_max) / 2;
        size_t pos_string = phrase_pos[pos_phrase];
        if (pos >= pos_string) {
          pos_min = pos_phrase + 1;
        } else {
          pos_max = pos_phrase;
        }
      }

      size_t pos_prev = 0;
      size_t pos_cur = 0;

      size_t pos_phrase = pos_min;
      pos_cur = phrase_pos[pos_phrase];

      if (pos_min == 0) {
        pos_prev = 0;
      } else {
        pos_phrase = pos_min - 1;
        pos_prev = phrase_pos[pos_phrase];
      }

      size_t val = phrase[pos_min].src;

      if (pos_cur - pos_prev <= 1) {
        s[j] = (char)val;
        break;
      } else {
        size_t per = pos_prev - val;
        pos = val + (pos - pos_prev) % per;
      }
    }
  }
  return s;
}

char lzhb::access(const std::vector<PhrasePos>& phrase_pos, uInt pos) {
  char c = 0;
  size_t pos_min = 0;
  size_t pos_max = phrase_pos.size();
  while (true) {
    pos_min = 0;
    // size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t pos_string = phrase_pos[pos_phrase].pos;
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    pos_cur = phrase_pos[pos_phrase].pos;

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_phrase = pos_min - 1;
      pos_prev = phrase_pos[pos_phrase].pos;
    }

    size_t val = phrase_pos[pos_min].src;

    if (pos_cur - pos_prev <= 1) {
      c = (char)val;
      return c;
    } else {
      size_t per = pos_prev - val;
      pos = val + (pos - pos_prev) % per;
    }
  }
  return c;
}

char lzhb::access(const std::vector<PhrasePosC>& phrase_pos, uInt pos) {
  char c = 0;
  size_t pos_min = 0;
  size_t pos_max = phrase_pos.size();
  while (true) {
    pos_min = 0;
    // size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t pos_string = phrase_pos[pos_phrase].pos;
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    pos_cur = phrase_pos[pos_phrase].pos;

    if (pos == pos_cur - 1) {
      return phrase_pos[pos_min].c;
    }

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_phrase = pos_min - 1;
      pos_prev = phrase_pos[pos_phrase].pos;
    }

    size_t val = phrase_pos[pos_min].src;

    if (pos_cur - pos_prev <= 1) {
      return phrase_pos[pos_min].c;
    } else {
      size_t per = pos_prev - val;
      pos = val + (pos - pos_prev) % per;
    }
  }
  return c;
}

char lzhb::access(const std::vector<PhraseModPos>& phrase_pos, uInt pos) {
  char c = 0;
  size_t pos_min = 0;
  size_t pos_max = phrase_pos.size();
  while (true) {
    pos_min = 0;
    // size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t pos_string = phrase_pos[pos_phrase].pos;
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    // size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    // pos_cur = phrase_pos[pos_phrase].pos;

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_phrase = pos_min - 1;
      pos_prev = phrase_pos[pos_phrase].pos;
    }

    size_t val = phrase_pos[pos_min].src;
    size_t per = phrase_pos[pos_min].per;

    if (per == 1) {
      c = (char)val;
      return c;
    } else {
      size_t per2 = pos_prev - val;
      pos = val + ((pos - pos_prev) % per) % per2;
    }
  }
  return c;
}

char lzhb::access(const std::vector<PhraseModPosC>& phrase_pos, uInt pos) {
  char c = 0;
  size_t pos_min = 0;
  size_t pos_max = phrase_pos.size();
  while (true) {
    pos_min = 0;
    // size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t pos_string = phrase_pos[pos_phrase].pos;
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    pos_cur = phrase_pos[pos_phrase].pos;

    if (pos == pos_cur - 1) {
      return phrase_pos[pos_min].c;
    }

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_phrase = pos_min - 1;
      pos_prev = phrase_pos[pos_phrase].pos;
    }

    size_t val = phrase_pos[pos_min].src;
    size_t per = phrase_pos[pos_min].per;

    if (per == 1) {
      c = (char)val;
      return c;
    } else {
      size_t per2 = pos_prev - val;
      pos = val + ((pos - pos_prev) % per) % per2;
    }
  }
  return c;
}

double lzhb::average(const std::vector<uInt>& a) {
  size_t sum = 0;
  double ave = 0;
  for (size_t i = 0; i < a.size(); i++) {
    sum += a[i];
  }

  ave = static_cast<double>(sum) / a.size();

  return ave;
}

double lzhb::variance(const std::vector<uInt>& a) {
  double ave = lzhb::average(a);
  double sum2 = 0;
  double var = 0;

  for (size_t i = 0; i < a.size(); i++) {
    sum2 += a[i] * a[i];
  }

  var = sum2 / a.size() - ave * ave;

  return var;
}