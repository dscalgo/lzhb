#include "binencdec.hpp"

uInt binencdec::encodePhrase(std::vector<lzhb::Phrase>& phrase,
                             const std::string& filename, bool output,
                             bool position) {
  std::vector<uInt> phrase_lp;
  std::vector<uInt> phrase_src;
  uInt pos = 0;
  uInt max_lp = 1;
  uInt max_src = 1;

  for (size_t i = 0; i < phrase.size(); i++) {
    if (position) {
      pos += std::max((uInt)1, phrase[i].len);
      phrase_lp.push_back(pos);
    } else {
      phrase_lp.push_back(phrase[i].len);
    }
    phrase_src.push_back(phrase[i].src);
    max_lp = std::max(max_lp, phrase_lp[i]);
    max_src = std::max(max_src, phrase_src[i]);
  }
  bitsize_t bitsize_lp = (bitsize_t)log2((double)max_lp) + 1;
  bitsize_t bitsize_src = (bitsize_t)log2((double)max_src) + 1;

  std::vector<uint8_t> bin_lp;
  std::vector<uint8_t> bin_src;
  binencdec::encodeBlock(bitsize_lp, bin_lp, phrase_lp);
  binencdec::encodeBlock(bitsize_src, bin_src, phrase_src);

  phrase_t phrase_size = phrase.size();
  if (output) {
    std::string binfile = filename;
    if (position) {
      binfile += ".lzp";
    } else {
      binfile += ".lz";
    }
    std::ofstream fs(binfile, std::ofstream::binary);
    fs.write((char*)&phrase_size, sizeof(phrase_t));
    fs.write((char*)&bitsize_lp, sizeof(bitsize_t));
    fs.write((char*)&bitsize_src, sizeof(bitsize_t));
    fs.write((char*)&bin_lp[0], bin_lp.size());
    fs.write((char*)&bin_src[0], bin_src.size());
    fs.close();
  }
  return sizeof(phrase_t) + sizeof(bitsize_t) + sizeof(bitsize_t) +
         bin_lp.size() + bin_src.size();
}

uInt binencdec::encodePhraseC(std::vector<lzhb::PhraseC>& phrase,
                              const std::string& filename, bool output,
                              bool position) {
  std::vector<uInt> phrase_lp;
  std::vector<uInt> phrase_src;
  std::vector<char> phrase_c;
  uInt pos = 0;
  uInt max_lp = 1;
  uInt max_src = 1;

  for (size_t i = 0; i < phrase.size(); i++) {
    if (position) {
      pos += std::max((uInt)1, phrase[i].len);
      phrase_lp.push_back(pos);
    } else {
      phrase_lp.push_back(phrase[i].len);
    }
    phrase_src.push_back(phrase[i].src);
    phrase_c.push_back(phrase[i].c);
    max_lp = std::max(max_lp, phrase_lp[i]);
    max_src = std::max(max_src, phrase_src[i]);
  }
  bitsize_t bitsize_lp = (bitsize_t)log2((double)max_lp) + 1;
  bitsize_t bitsize_src = (bitsize_t)log2((double)max_src) + 1;

  std::vector<uint8_t> bin_lp;
  std::vector<uint8_t> bin_src;
  binencdec::encodeBlock(bitsize_lp, bin_lp, phrase_lp);
  binencdec::encodeBlock(bitsize_src, bin_src, phrase_src);

  phrase_t phrase_size = phrase.size();
  if (output) {
    std::string binfile = filename;
    if (position) {
      binfile += ".lzcp";
    } else {
      binfile += ".lzc";
    }
    std::ofstream fs(binfile, std::ofstream::binary);
    fs.write((char*)&phrase_size, sizeof(phrase_t));
    fs.write((char*)&bitsize_lp, sizeof(bitsize_t));
    fs.write((char*)&bitsize_src, sizeof(bitsize_t));
    fs.write((char*)&bin_lp[0], bin_lp.size());
    fs.write((char*)&bin_src[0], bin_src.size());
    fs.write((char*)&phrase_c[0], phrase.size());
    fs.close();
  }
  return sizeof(phrase_t) + sizeof(bitsize_t) + sizeof(bitsize_t) +
         bin_lp.size() + bin_src.size() + phrase.size();
}

std::vector<lzhb::Phrase> binencdec::decodetoPhrase(const std::string& filename,
                                                    bool position) {
  std::vector<lzhb::Phrase> phrase;

  std::string binfile = filename;
  if (position) {
    binfile += ".lzp";
  } else {
    binfile += ".lz";
  }
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.close();

  std::vector<uInt> phrase_lp;
  std::vector<uInt> phrase_src;
  binencdec::decodeBlock(bin_lp, phrase_lp, bitsize_lp, tail_lp);
  binencdec::decodeBlock(bin_src, phrase_src, bitsize_src, tail_src);

  if (position) {
    for (size_t i = 0; i < phrase_lp.size(); i++) {
      if (i == 0) {
        phrase.push_back(lzhb::Phrase{.len = 1, .src = phrase_src[i]});
      } else {
        if (phrase_lp[i] - phrase_lp[i - 1] == 1) {
          phrase.push_back(lzhb::Phrase{.len = 1, .src = phrase_src[i]});
        } else {
          phrase.push_back(lzhb::Phrase{.len = phrase_lp[i] - phrase_lp[i - 1],
                                        .src = phrase_src[i]});
        }
      }
    }
  } else {
    for (size_t i = 0; i < phrase_lp.size(); i++) {
      phrase.push_back(lzhb::Phrase{.len = phrase_lp[i], .src = phrase_src[i]});
    }
  }
  return phrase;
}

std::vector<lzhb::PhraseC> binencdec::decodetoPhraseC(
    const std::string& filename, bool position) {
  std::vector<lzhb::PhraseC> phrase;

  std::string binfile = filename;
  if (position) {
    binfile += ".lzcp";
  } else {
    binfile += ".lzc";
  }
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<char> phrase_c(phrase_size);
  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&phrase_c[0], phrase_c.size());
  fs.close();

  std::vector<uInt> phrase_lp;
  std::vector<uInt> phrase_src;
  binencdec::decodeBlock(bin_lp, phrase_lp, bitsize_lp, tail_lp);
  binencdec::decodeBlock(bin_src, phrase_src, bitsize_src, tail_src);

  if (position) {
    for (size_t i = 0; i < phrase_lp.size(); i++) {
      if (i == 0) {
        phrase.push_back(
            lzhb::PhraseC{.len = 1, .src = phrase_src[i], .c = phrase_c[i]});
      } else {
        if (phrase_lp[i] - phrase_lp[i - 1] == 1) {
          phrase.push_back(
              lzhb::PhraseC{.len = 1, .src = phrase_src[i], .c = phrase_c[i]});
        } else {
          phrase.push_back(lzhb::PhraseC{.len = phrase_lp[i] - phrase_lp[i - 1],
                                         .src = phrase_src[i],
                                         .c = phrase_c[i]});
        }
      }
    }
  } else {
    for (size_t i = 0; i < phrase_lp.size(); i++) {
      phrase.push_back(lzhb::PhraseC{
          .len = phrase_lp[i], .src = phrase_src[i], .c = phrase_c[i]});
    }
  }
  return phrase;
}

BinaryPhrases binencdec::getBinary(const std::string& filename) {
  std::string binfile = filename;
  binfile += ".lzp";

  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  // size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  // size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.close();

  return BinaryPhrases{.phrase_size = phrase_size,
                       .bitsize_pos = bitsize_lp,
                       .bitsize_src = bitsize_src,
                       .bin_pos = bin_lp,
                       .bin_src = bin_src};
}

BinaryPhrasesC binencdec::getBinaryC(const std::string& filename) {
  std::string binfile = filename;
  binfile += ".lzcp";

  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  // size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  // size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<char> phrase_c(phrase_size);
  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&phrase_c[0], phrase_c.size());
  fs.close();

  return BinaryPhrasesC{.phrase_size = phrase_size,
                        .bitsize_pos = bitsize_lp,
                        .bitsize_src = bitsize_src,
                        .bin_pos = bin_lp,
                        .bin_src = bin_src,
                        .phrase_c = phrase_c};
}

BinaryPhrasesMod binencdec::getBinaryMod(const std::string& filename) {
  std::string binfile = filename;
  binfile += ".lzxp";

  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;
  bitsize_t bitsize_per = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));
  fs.read((char*)&bitsize_per, sizeof(bitsize_t));

  // size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  // size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  // size_t tail_per = 1 + ((phrase_size * bitsize_per - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;
  size_t size_per = 1 + (phrase_size * bitsize_per - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<uint8_t> bin_per(size_per);
  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&bin_per[0], bin_per.size());
  fs.close();

  return BinaryPhrasesMod{.phrase_size = phrase_size,
                          .bitsize_pos = bitsize_lp,
                          .bitsize_src = bitsize_src,
                          .bitsize_per = bitsize_per,
                          .bin_pos = bin_lp,
                          .bin_src = bin_src,
                          .bin_per = bin_per};
}

BinaryPhrasesModC binencdec::getBinaryModC(const std::string& filename) {
  std::string binfile = filename;
  binfile += ".lzxcp";

  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;
  bitsize_t bitsize_per = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));
  fs.read((char*)&bitsize_per, sizeof(bitsize_t));

  // size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  // size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  // size_t tail_per = 1 + ((phrase_size * bitsize_per - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;
  size_t size_per = 1 + (phrase_size * bitsize_per - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<uint8_t> bin_per(size_per);
  std::vector<char> phrase_c(phrase_size);
  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&bin_per[0], bin_per.size());
  fs.read((char*)&phrase_c[0], phrase_c.size());
  fs.close();

  return BinaryPhrasesModC{.phrase_size = phrase_size,
                           .bitsize_pos = bitsize_lp,
                           .bitsize_src = bitsize_src,
                           .bitsize_per = bitsize_per,
                           .bin_pos = bin_lp,
                           .bin_src = bin_src,
                           .bin_per = bin_per,
                           .phrase_c = phrase_c};
}

char binencdec::accessBinary(const std::string& filename, uInt pos) {
  char c = 0;
  std::string binfile = filename;
  binfile += ".lzp";
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_pos, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / block_size_bit;

  std::vector<uint8_t> bin_pos(size_pos);
  std::vector<uint8_t> bin_src(size_src);
  fs.read((char*)&bin_pos[0], block_size_byte * bin_pos.size());
  fs.read((char*)&bin_src[0], block_size_byte * bin_src.size());
  fs.close();

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      pos_string = 0;
      for (size_t i = 0; i < bitsize_pos; i++) {
        // std::cout << pos_string << std::endl;
        pos_string |=
            (binencdec::getBit(
                 bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }
    // std::cout << pos << " " << pos_min << std::endl;

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * bitsize_pos;
    size_t block_pos = bit_pos / block_size_bit;
    size_t block_start = bit_pos % block_size_bit;
    // std::cout << block_pos << " " << block_start << std::endl;
    for (size_t i = 0; i < bitsize_pos; i++) {
      pos_cur |=
          (binencdec::getBit(
               bin_pos[block_pos + (block_start + i) / block_size_bit],
               (block_size_bit - 1) - ((block_start + i) % block_size_bit))
           << (bitsize_pos - i - 1));
    }
    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      size_t pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < bitsize_pos; i++) {
        pos_prev |=
            (binencdec::getBit(
                 bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
    }
    // std::cout << pos_prev << " " << pos_cur << std::endl;

    size_t bit_src = pos_min * bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < bitsize_src; i++) {
      val |= (binencdec::getBit(
                  bin_src[block_src + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (bitsize_src - i - 1));
    }

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

std::string binencdec::accessBinary(const std::string& filename, uInt pos,
                                    uInt len) {
  std::string s(len, 'a');
  std::string binfile = filename;
  binfile += ".lzp";
  std::ifstream fs(binfile, std::ifstream::binary);
  uInt head = pos;

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_pos, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / block_size_bit;

  std::vector<uint8_t> bin_pos(size_pos);
  std::vector<uint8_t> bin_src(size_src);
  fs.read((char*)&bin_pos[0], block_size_byte * bin_pos.size());
  fs.read((char*)&bin_src[0], block_size_byte * bin_src.size());
  fs.close();

  for (uInt j = 0; j < len; j++) {
    pos = head + j;
    while (true) {
      size_t pos_min = 0;
      size_t pos_max = phrase_size;
      size_t pos_string = 0;
      while (pos_min < pos_max) {
        size_t pos_phrase = (pos_min + pos_max) / 2;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        pos_string = 0;
        for (size_t i = 0; i < bitsize_pos; i++) {
          // std::cout << pos_string << std::endl;
          pos_string |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
        if (pos >= pos_string) {
          pos_min = pos_phrase + 1;
        } else {
          pos_max = pos_phrase;
        }
      }
      // std::cout << pos << " " << pos_min << std::endl;

      size_t pos_prev = 0;
      size_t pos_cur = 0;

      size_t pos_phrase = pos_min;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      // std::cout << block_pos << " " << block_start << std::endl;
      for (size_t i = 0; i < bitsize_pos; i++) {
        pos_cur |=
            (binencdec::getBit(
                 bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
      if (pos_min == 0) {
        pos_prev = 0;
      } else {
        pos_prev = 0;
        pos_phrase = pos_min - 1;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_prev |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
      }
      // std::cout << pos_prev << " " << pos_cur << std::endl;

      size_t bit_src = pos_min * bitsize_src;
      size_t block_src = bit_src / block_size_bit;
      block_start = bit_src % block_size_bit;
      size_t val = 0;
      for (size_t i = 0; i < bitsize_src; i++) {
        val |= (binencdec::getBit(
                    bin_src[block_src + (block_start + i) / block_size_bit],
                    (block_size_bit - 1) - ((block_start + i) % block_size_bit))
                << (bitsize_src - i - 1));
      }

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

std::string binencdec::accessBinary(const std::string& filename,
                                    std::vector<uInt> positions) {
  std::string s(positions.size(), 'a');
  std::string binfile = filename;
  binfile += ".lzp";
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_pos, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / block_size_bit;

  std::vector<uint8_t> bin_pos(size_pos);
  std::vector<uint8_t> bin_src(size_src);
  fs.read((char*)&bin_pos[0], block_size_byte * bin_pos.size());
  fs.read((char*)&bin_src[0], block_size_byte * bin_src.size());
  fs.close();

  for (uInt j = 0; j < positions.size(); j++) {
    uInt pos = positions[j];
    while (true) {
      size_t pos_min = 0;
      size_t pos_max = phrase_size;
      size_t pos_string = 0;
      while (pos_min < pos_max) {
        size_t pos_phrase = (pos_min + pos_max) / 2;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        pos_string = 0;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_string |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
        if (pos >= pos_string) {
          pos_min = pos_phrase + 1;
        } else {
          pos_max = pos_phrase;
        }
      }

      size_t pos_prev = 0;
      size_t pos_cur = 0;

      size_t pos_phrase = pos_min;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;

      for (size_t i = 0; i < bitsize_pos; i++) {
        pos_cur |=
            (binencdec::getBit(
                 bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
      if (pos_min == 0) {
        pos_prev = 0;
      } else {
        pos_prev = 0;
        pos_phrase = pos_min - 1;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_prev |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
      }

      size_t bit_src = pos_min * bitsize_src;
      size_t block_src = bit_src / block_size_bit;
      block_start = bit_src % block_size_bit;
      size_t val = 0;
      for (size_t i = 0; i < bitsize_src; i++) {
        val |= (binencdec::getBit(
                    bin_src[block_src + (block_start + i) / block_size_bit],
                    (block_size_bit - 1) - ((block_start + i) % block_size_bit))
                << (bitsize_src - i - 1));
      }

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

char binencdec::accessBinaryFile(const std::string& filename, uInt pos) {
  FILE* fp;
  char c = 0;
  std::string binfile = filename;
  binfile += ".lzp";

  fp = fopen(binfile.c_str(), "r");

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;

  fread(&phrase_size, sizeof(phrase_t), 1, fp);
  fread(&bitsize_pos, sizeof(bitsize_t), 1, fp);
  fread(&bitsize_src, sizeof(bitsize_t), 1, fp);

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;

  size_t head = sizeof(phrase_t) + 2 * sizeof(bitsize_t);

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;

      pos_string = 0;

      for (size_t i = 0; i < bitsize_pos; i++) {
        uint8_t buff;
        fseek(fp, head + block_pos + (block_start + i) / block_size_bit, 0);
        fread((char*)&buff, 1, 1, fp);
        pos_string |=
            (binencdec::getBit(buff, (block_size_bit - 1) -
                                         ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * bitsize_pos;
    size_t block_pos = bit_pos / block_size_bit;
    size_t block_start = bit_pos % block_size_bit;

    for (size_t i = 0; i < bitsize_pos; i++) {
      uint8_t buff;
      fseek(fp, head + block_pos + (block_start + i) / block_size_bit, 0);
      fread((char*)&buff, 1, 1, fp);
      pos_cur |=
          (binencdec::getBit(buff, (block_size_bit - 1) -
                                       ((block_start + i) % block_size_bit))
           << (bitsize_pos - i - 1));
    }
    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < bitsize_pos; i++) {
        uint8_t buff;
        fseek(fp, head + block_pos + (block_start + i) / block_size_bit, 0);
        fread((char*)&buff, 1, 1, fp);
        pos_prev |=
            (binencdec::getBit(buff, (block_size_bit - 1) -
                                         ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
    }

    size_t bit_src = pos_min * bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < bitsize_src; i++) {
      uint8_t buff;
      fseek(fp,
            head + size_pos + block_src + (block_start + i) / block_size_bit,
            0);
      fread((char*)&buff, 1, 1, fp);
      val |= (binencdec::getBit(buff, (block_size_bit - 1) -
                                          ((block_start + i) % block_size_bit))
              << (bitsize_src - i - 1));
    }

    if (pos_cur - pos_prev <= 1) {
      c = (char)val;
      fclose(fp);
      return c;
    } else {
      size_t per = pos_prev - val;
      pos = val + (pos - pos_prev) % per;
    }
  }
  fclose(fp);
  return c;
}

char binencdec::accessBinary(const BinaryPhrases& binphrase, uInt pos) {
  char c = 0;

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = binphrase.phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      pos_string = 0;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_string |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
    size_t block_pos = bit_pos / block_size_bit;
    size_t block_start = bit_pos % block_size_bit;

    for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
      pos_cur |=
          (binencdec::getBit(
               binphrase
                   .bin_pos[block_pos + (block_start + i) / block_size_bit],
               (block_size_bit - 1) - ((block_start + i) % block_size_bit))
           << (binphrase.bitsize_pos - i - 1));
    }
    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      size_t pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_prev |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
    }

    size_t bit_src = pos_min * binphrase.bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < binphrase.bitsize_src; i++) {
      val |= (binencdec::getBit(
                  binphrase
                      .bin_src[block_src + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (binphrase.bitsize_src - i - 1));
    }

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

char binencdec::accessBinary(const BinaryPhrasesC& binphrase, uInt pos) {
  char c = 0;

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = binphrase.phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      pos_string = 0;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_string |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
    size_t block_pos = bit_pos / block_size_bit;
    size_t block_start = bit_pos % block_size_bit;

    for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
      pos_cur |=
          (binencdec::getBit(
               binphrase
                   .bin_pos[block_pos + (block_start + i) / block_size_bit],
               (block_size_bit - 1) - ((block_start + i) % block_size_bit))
           << (binphrase.bitsize_pos - i - 1));
    }

    if (pos == pos_cur - 1) {
      return binphrase.phrase_c[pos_min];
    }

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      size_t pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_prev |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
    }

    size_t bit_src = pos_min * binphrase.bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < binphrase.bitsize_src; i++) {
      val |= (binencdec::getBit(
                  binphrase
                      .bin_src[block_src + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (binphrase.bitsize_src - i - 1));
    }

    if (pos_cur - pos_prev <= 1) {
      return binphrase.phrase_c[pos_min];
    } else {
      size_t per = pos_prev - val;
      pos = val + (pos - pos_prev) % per;
    }
  }
  return c;
}

char binencdec::accessBinary(const BinaryPhrasesMod& binphrase, uInt pos) {
  char c = 0;

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = binphrase.phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      pos_string = 0;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_string |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
    size_t block_start = bit_pos % block_size_bit;
    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      size_t pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_prev |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
    }

    size_t bit_src = pos_min * binphrase.bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < binphrase.bitsize_src; i++) {
      val |= (binencdec::getBit(
                  binphrase
                      .bin_src[block_src + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (binphrase.bitsize_src - i - 1));
    }

    size_t bit_per = pos_min * binphrase.bitsize_per;
    size_t block_per = bit_per / block_size_bit;
    block_start = bit_per % block_size_bit;
    size_t per = 0;
    for (size_t i = 0; i < binphrase.bitsize_per; i++) {
      per |= (binencdec::getBit(
                  binphrase
                      .bin_per[block_per + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (binphrase.bitsize_per - i - 1));
    }

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

char binencdec::accessBinary(const BinaryPhrasesModC& binphrase, uInt pos) {
  char c = 0;

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = binphrase.phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      pos_string = 0;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_string |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;
    size_t pos_cur = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
    size_t block_pos = bit_pos / block_size_bit;
    size_t block_start = bit_pos % block_size_bit;
    for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
      pos_cur |=
          (binencdec::getBit(
               binphrase
                   .bin_pos[block_pos + (block_start + i) / block_size_bit],
               (block_size_bit - 1) - ((block_start + i) % block_size_bit))
           << (binphrase.bitsize_pos - i - 1));
    }

    if (pos == pos_cur - 1) {
      return binphrase.phrase_c[pos_min];
    }

    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      size_t pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * binphrase.bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < binphrase.bitsize_pos; i++) {
        pos_prev |=
            (binencdec::getBit(
                 binphrase
                     .bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (binphrase.bitsize_pos - i - 1));
      }
    }

    size_t bit_src = pos_min * binphrase.bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < binphrase.bitsize_src; i++) {
      val |= (binencdec::getBit(
                  binphrase
                      .bin_src[block_src + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (binphrase.bitsize_src - i - 1));
    }

    size_t bit_per = pos_min * binphrase.bitsize_per;
    size_t block_per = bit_per / block_size_bit;
    block_start = bit_per % block_size_bit;
    size_t per = 0;
    for (size_t i = 0; i < binphrase.bitsize_per; i++) {
      per |= (binencdec::getBit(
                  binphrase
                      .bin_per[block_per + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (binphrase.bitsize_per - i - 1));
    }

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

size_t binencdec::encodeTripletPhrase(
    std::vector<struct lzhb::PhraseMod>& phrase, const std::string& filename,
    bool output, bool position) {
  std::vector<uInt> phrase_lp;
  std::vector<uInt> phrase_src;
  std::vector<uInt> phrase_per;

  uInt pos = 0;
  uInt max_lp = 1;
  uInt max_src = 1;
  uInt max_per = 1;
  for (size_t i = 0; i < phrase.size(); i++) {
    if (position) {
      pos += std::max((uInt)1, phrase[i].len);
      phrase_lp.push_back(pos);
    } else {
      phrase_lp.push_back(phrase[i].len);
    }
    phrase_src.push_back(phrase[i].src);
    phrase_per.push_back(phrase[i].per);
    max_lp = std::max(max_lp, phrase_lp[i]);
    max_src = std::max(max_src, phrase_src[i]);
    max_per = std::max(max_per, phrase_per[i]);
  }
  bitsize_t bitsize_lp = (bitsize_t)log2((double)max_lp) + 1;
  bitsize_t bitsize_src = (bitsize_t)log2((double)max_src) + 1;
  bitsize_t bitsize_per = (bitsize_t)log2((double)max_per) + 1;

  std::vector<uint8_t> bin_lp;
  std::vector<uint8_t> bin_src;
  std::vector<uint8_t> bin_per;
  binencdec::encodeBlock(bitsize_lp, bin_lp, phrase_lp);
  binencdec::encodeBlock(bitsize_src, bin_src, phrase_src);
  binencdec::encodeBlock(bitsize_per, bin_per, phrase_per);

  phrase_t phrase_size = phrase.size();
  if (output) {
    std::string binfile = filename;
    if (position) {
      binfile += ".lzxp";
    } else {
      binfile += ".lzx";
    }
    std::ofstream fs(binfile, std::ofstream::binary);
    fs.write((char*)&phrase_size, sizeof(phrase_t));
    fs.write((char*)&bitsize_lp, sizeof(bitsize_t));
    fs.write((char*)&bitsize_src, sizeof(bitsize_t));
    fs.write((char*)&bitsize_per, sizeof(bitsize_t));
    fs.write((char*)&bin_lp[0], bin_lp.size());
    fs.write((char*)&bin_src[0], bin_src.size());
    fs.write((char*)&bin_per[0], bin_per.size());
    fs.close();
  }
  return sizeof(phrase_t) + sizeof(bitsize_t) + sizeof(bitsize_t) +
         bin_lp.size() + bin_src.size() + bin_per.size();
}

size_t binencdec::encodeTripletPhraseC(
    std::vector<struct lzhb::PhraseModC>& phrase, const std::string& filename,
    bool output, bool position) {
  std::cerr << "sorry, not implemented yet" << std::endl;
}

std::vector<struct lzhb::PhraseMod> binencdec::decodetoTripletPhrase(
    const std::string& filename, bool position) {
  std::vector<struct lzhb::PhraseMod> phrase;

  std::string binfile = filename;
  if (position) {
    binfile += ".lzxp";
  } else {
    binfile += ".lzx";
  }
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_lp = 1;
  bitsize_t bitsize_src = 1;
  bitsize_t bitsize_per = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_lp, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));
  fs.read((char*)&bitsize_per, sizeof(bitsize_t));

  size_t tail_lp = 1 + ((phrase_size * bitsize_lp - 1) % 8);
  size_t tail_src = 1 + ((phrase_size * bitsize_src - 1) % 8);
  size_t tail_per = 1 + ((phrase_size * bitsize_per - 1) % 8);
  size_t size_lp = 1 + (phrase_size * bitsize_lp - 1) / 8;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / 8;
  size_t size_per = 1 + (phrase_size * bitsize_per - 1) / 8;

  std::vector<uint8_t> bin_lp(size_lp);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<uint8_t> bin_per(size_per);

  fs.read((char*)&bin_lp[0], bin_lp.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&bin_per[0], bin_per.size());
  fs.close();

  std::vector<uInt> phrase_lp;
  std::vector<uInt> phrase_src;
  std::vector<uInt> phrase_per;

  binencdec::decodeBlock(bin_lp, phrase_lp, bitsize_lp, tail_lp);
  binencdec::decodeBlock(bin_src, phrase_src, bitsize_src, tail_src);
  binencdec::decodeBlock(bin_per, phrase_per, bitsize_per, tail_per);

  if (position) {
    for (size_t i = 0; i < phrase_lp.size(); i++) {
      if (i == 0) {
        phrase.push_back(lzhb::PhraseMod{
            .len = phrase_lp[i], .src = phrase_src[i], .per = phrase_per[i]});
      } else {
        phrase.push_back(lzhb::PhraseMod{
            .len = phrase_lp[i] - phrase_lp[i - 1],
            .src = phrase_src[i],
            .per = phrase_per[i],
        });
      }
    }
  } else {
    for (size_t i = 0; i < phrase_lp.size(); i++) {
      phrase.push_back(lzhb::PhraseMod{
          .len = phrase_lp[i], .src = phrase_src[i], .per = phrase_per[i]});
    }
  }
  return phrase;
}

char binencdec::accessBinaryTriplet(const std::string& filename, uInt pos) {
  char c = 0;
  std::string binfile = filename;
  binfile += ".lzxp";
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;
  bitsize_t bitsize_per = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_pos, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));
  fs.read((char*)&bitsize_per, sizeof(bitsize_t));

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / block_size_bit;
  size_t size_per = 1 + (phrase_size * bitsize_per - 1) / block_size_bit;

  std::vector<uint8_t> bin_pos(size_pos);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<uint8_t> bin_per(size_per);
  fs.read((char*)&bin_pos[0], bin_pos.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&bin_per[0], bin_per.size());
  fs.close();

  while (true) {
    size_t pos_min = 0;
    size_t pos_max = phrase_size;
    size_t pos_string = 0;
    while (pos_min < pos_max) {
      size_t pos_phrase = (pos_min + pos_max) / 2;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      pos_string = 0;
      for (size_t i = 0; i < bitsize_pos; i++) {
        pos_string |=
            (binencdec::getBit(
                 bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
      if (pos >= pos_string) {
        pos_min = pos_phrase + 1;
      } else {
        pos_max = pos_phrase;
      }
    }

    size_t pos_prev = 0;

    size_t pos_phrase = pos_min;
    size_t bit_pos = pos_phrase * bitsize_pos;
    size_t block_start = bit_pos % block_size_bit;
    if (pos_min == 0) {
      pos_prev = 0;
    } else {
      pos_prev = 0;
      size_t pos_phrase = pos_min - 1;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_pos = bit_pos / block_size_bit;
      size_t block_start = bit_pos % block_size_bit;
      for (size_t i = 0; i < bitsize_pos; i++) {
        pos_prev |=
            (binencdec::getBit(
                 bin_pos[block_pos + (block_start + i) / block_size_bit],
                 (block_size_bit - 1) - ((block_start + i) % block_size_bit))
             << (bitsize_pos - i - 1));
      }
    }

    size_t bit_src = pos_min * bitsize_src;
    size_t block_src = bit_src / block_size_bit;
    block_start = bit_src % block_size_bit;
    size_t val = 0;
    for (size_t i = 0; i < bitsize_src; i++) {
      val |= (binencdec::getBit(
                  bin_src[block_src + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (bitsize_src - i - 1));
    }

    size_t bit_per = pos_min * bitsize_per;
    size_t block_per = bit_per / block_size_bit;
    block_start = bit_per % block_size_bit;
    size_t per = 0;
    for (size_t i = 0; i < bitsize_per; i++) {
      per |= (binencdec::getBit(
                  bin_per[block_per + (block_start + i) / block_size_bit],
                  (block_size_bit - 1) - ((block_start + i) % block_size_bit))
              << (bitsize_per - i - 1));
    }

    if (per == 1) {
      c = (char)val;
      return c;
    } else {
      pos = val + (pos - pos_prev) % per;
    }
  }
  return c;
}

std::string binencdec::accessBinaryTriplet(const std::string& filename,
                                           uInt pos, uInt len) {
  std::string s(len, 'a');
  std::string binfile = filename;
  binfile += ".lzxp";
  std::ifstream fs(binfile, std::ifstream::binary);
  uInt head = pos;

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;
  bitsize_t bitsize_per = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_pos, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));
  fs.read((char*)&bitsize_per, sizeof(bitsize_t));

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / block_size_bit;
  size_t size_per = 1 + (phrase_size * bitsize_per - 1) / block_size_bit;

  std::vector<uint8_t> bin_pos(size_pos);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<uint8_t> bin_per(size_per);
  fs.read((char*)&bin_pos[0], bin_pos.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&bin_per[0], bin_per.size());
  fs.close();

  for (uInt j = 0; j < len; j++) {
    pos = head + j;
    while (true) {
      size_t pos_min = 0;
      size_t pos_max = phrase_size;
      size_t pos_string = 0;
      while (pos_min < pos_max) {
        size_t pos_phrase = (pos_min + pos_max) / 2;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        pos_string = 0;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_string |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
        if (pos >= pos_string) {
          pos_min = pos_phrase + 1;
        } else {
          pos_max = pos_phrase;
        }
      }

      size_t pos_prev = 0;

      size_t pos_phrase = pos_min;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_start = bit_pos % block_size_bit;
      if (pos_min == 0) {
        pos_prev = 0;
      } else {
        pos_prev = 0;
        pos_phrase = pos_min - 1;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_prev |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
      }

      size_t bit_src = pos_min * bitsize_src;
      size_t block_src = bit_src / block_size_bit;
      block_start = bit_src % block_size_bit;
      size_t val = 0;
      for (size_t i = 0; i < bitsize_src; i++) {
        val |= (binencdec::getBit(
                    bin_src[block_src + (block_start + i) / block_size_bit],
                    (block_size_bit - 1) - ((block_start + i) % block_size_bit))
                << (bitsize_src - i - 1));
      }

      size_t bit_per = pos_min * bitsize_per;
      size_t block_per = bit_per / block_size_bit;
      block_start = bit_per % block_size_bit;
      size_t per = 0;
      for (size_t i = 0; i < bitsize_per; i++) {
        per |= (binencdec::getBit(
                    bin_per[block_per + (block_start + i) / block_size_bit],
                    (block_size_bit - 1) - ((block_start + i) % block_size_bit))
                << (bitsize_per - i - 1));
      }

      if (per == 1) {
        s[j] = (char)val;
        break;
      } else {
        pos = val + (pos - pos_prev) % per;
      }
    }
  }
  return s;
}

std::string binencdec::accessBinaryTriplet(const std::string& filename,
                                           std::vector<uInt> positions) {
  std::string s(positions.size(), 'a');
  std::string binfile = filename;
  binfile += ".lzxp";
  std::ifstream fs(binfile, std::ifstream::binary);

  phrase_t phrase_size = 1;
  bitsize_t bitsize_pos = 1;
  bitsize_t bitsize_src = 1;
  bitsize_t bitsize_per = 1;

  fs.read((char*)&phrase_size, sizeof(phrase_t));
  fs.read((char*)&bitsize_pos, sizeof(bitsize_t));
  fs.read((char*)&bitsize_src, sizeof(bitsize_t));
  fs.read((char*)&bitsize_per, sizeof(bitsize_t));

  size_t size_pos = 1 + (phrase_size * bitsize_pos - 1) / block_size_bit;
  size_t size_src = 1 + (phrase_size * bitsize_src - 1) / block_size_bit;
  size_t size_per = 1 + (phrase_size * bitsize_per - 1) / block_size_bit;

  std::vector<uint8_t> bin_pos(size_pos);
  std::vector<uint8_t> bin_src(size_src);
  std::vector<uint8_t> bin_per(size_per);
  fs.read((char*)&bin_pos[0], bin_pos.size());
  fs.read((char*)&bin_src[0], bin_src.size());
  fs.read((char*)&bin_per[0], bin_per.size());
  fs.close();

  for (uInt j = 0; j < positions.size(); j++) {
    uInt pos = positions[j];
    while (true) {
      size_t pos_min = 0;
      size_t pos_max = phrase_size;
      size_t pos_string = 0;
      while (pos_min < pos_max) {
        size_t pos_phrase = (pos_min + pos_max) / 2;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        pos_string = 0;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_string |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
        if (pos >= pos_string) {
          pos_min = pos_phrase + 1;
        } else {
          pos_max = pos_phrase;
        }
      }

      size_t pos_prev = 0;

      size_t pos_phrase = pos_min;
      size_t bit_pos = pos_phrase * bitsize_pos;
      size_t block_start = bit_pos % block_size_bit;
      if (pos_min == 0) {
        pos_prev = 0;
      } else {
        pos_prev = 0;
        pos_phrase = pos_min - 1;
        size_t bit_pos = pos_phrase * bitsize_pos;
        size_t block_pos = bit_pos / block_size_bit;
        size_t block_start = bit_pos % block_size_bit;
        for (size_t i = 0; i < bitsize_pos; i++) {
          pos_prev |=
              (binencdec::getBit(
                   bin_pos[block_pos + (block_start + i) / block_size_bit],
                   (block_size_bit - 1) - ((block_start + i) % block_size_bit))
               << (bitsize_pos - i - 1));
        }
      }

      size_t bit_src = pos_min * bitsize_src;
      size_t block_src = bit_src / block_size_bit;
      block_start = bit_src % block_size_bit;
      size_t val = 0;
      for (size_t i = 0; i < bitsize_src; i++) {
        val |= (binencdec::getBit(
                    bin_src[block_src + (block_start + i) / block_size_bit],
                    (block_size_bit - 1) - ((block_start + i) % block_size_bit))
                << (bitsize_src - i - 1));
      }

      size_t bit_per = pos_min * bitsize_per;
      size_t block_per = bit_per / block_size_bit;
      block_start = bit_per % block_size_bit;
      size_t per = 0;
      for (size_t i = 0; i < bitsize_per; i++) {
        per |= (binencdec::getBit(
                    bin_per[block_per + (block_start + i) / block_size_bit],
                    (block_size_bit - 1) - ((block_start + i) % block_size_bit))
                << (bitsize_per - i - 1));
      }

      if (per == 1) {
        s[j] = (char)val;
        break;
      } else {
        pos = val + (pos - pos_prev) % per;
      }
    }
  }
  return s;
}

void binencdec::setBit(uint8_t& block, uint8_t bit, uint8_t pos) {
  block |= (bit << pos);
}

uint8_t binencdec::getBit(const uint8_t& block, uint8_t pos) {
  return (block >> pos) & 1;
}

void binencdec::encodeBlock(bitsize_t bitsize, std::vector<uint8_t>& bin_blocks,
                            std::vector<uInt>& val_blocks) {
  uInt buff = 0;
  uint8_t block = 0;
  for (uInt i = 0; i < val_blocks.size(); i++) {
    for (uInt j = bitsize; j > 0; j--) {
      binencdec::setBit(block, (val_blocks[i] >> (j - 1)) & 1, 7 - buff);
      buff++;
      if (buff == 8) {
        buff = 0;
        bin_blocks.push_back(block);
        block = 0;
      }
    }
  }
  if (buff != 0) {
    bin_blocks.push_back(block);
  }
}

void binencdec::decodeBlock(std::vector<uint8_t>& bin_blocks,
                            std::vector<uInt>& val_blocks, uint32_t bitsize,
                            uInt tail) {
  uInt val = 0;
  uInt buff = 0;
  for (size_t i = 0; i < bin_blocks.size(); i++) {
    if (i != bin_blocks.size() - 1) {
      for (size_t j = 8; j > 0; j--) {
        val |=
            (binencdec::getBit(bin_blocks[i], j - 1) << (bitsize - buff - 1));
        buff++;
        if (buff == bitsize) {
          val_blocks.push_back(val);
          buff = 0;
          val = 0;
        }
      }
    } else {
      for (size_t j = 8; j > 8 - tail; j--) {
        val |=
            (binencdec::getBit(bin_blocks[i], j - 1) << (bitsize - buff - 1));
        buff++;
        if (buff == bitsize) {
          val_blocks.push_back(val);
          buff = 0;
          val = 0;
        }
      }
    }
  }
}