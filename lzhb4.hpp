#ifndef __LZHB4_HPP__
#define __LZHB4_HPP__
#include <string>
#include <tuple>
#include <vector>

#include "lzhb_common.hpp"
namespace lzhb4 {

// compute greedy/greedier LZHB encoding using efficient algorithm
std::vector<lzhb::PhraseMod> parse(const std::string& s, uInt height_bound);
std::vector<lzhb::PhraseMod> parseGreedier(const std::string& s,
                                           uInt height_bound);

// adds extra char to each phrase
std::vector<lzhb::PhraseModC> parseC(const std::string& s, uInt height_bound);
std::vector<lzhb::PhraseModC> parseGreedierC(const std::string& s,
                                             uInt height_bound);

// naive versions for verification
std::vector<lzhb::PhraseMod> parse_naive(const std::string& s,
                                         uInt height_bound);
std::vector<lzhb::PhraseMod> parseGreedier_naive(const std::string& s,
                                                 uInt height_bound);
std::vector<lzhb::PhraseModC> parseC_naive(const std::string& s,
                                           uInt height_bound);
std::vector<lzhb::PhraseModC> parseGreedierC_naive(const std::string& s,
                                                   uInt height_bound);

}  // namespace lzhb4
#endif  // __LZHB4_HPP__