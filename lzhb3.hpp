#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>

#include "lzhb_common.hpp"
#include "suffixtree.hpp"
namespace lzhb3 {

// lzhb3 encoding with self-referencing
std::vector<lzhb::Phrase> parse(const std::string& s, uInt height_bound);
std::vector<lzhb::Phrase> parseGreedier(const std::string& s,
                                        uInt height_bound);

// lzhb3 encoding with self-referencing
std::vector<lzhb::PhraseC> parseC(const std::string& s, uInt height_bound);
std::vector<lzhb::PhraseC> parseGreedierC(const std::string& s,
                                          uInt height_bound);

// lzhb3 encoding with self-referencing (using naive algorithm)
std::vector<lzhb::Phrase> parse_naive(const std::string& s, uInt height_bound);

std::vector<lzhb::Phrase> parseGreedier_naive(const std::string& s,
                                              uInt height_bound);

// lzhb3 encoding with self-referencing (using naive algorithm)
std::vector<lzhb::PhraseC> parseC_naive(const std::string& s,
                                        uInt height_bound);

std::vector<lzhb::PhraseC> parseGreedierC_naive(const std::string& s,
                                                uInt height_bound);
};  // namespace lzhb3
