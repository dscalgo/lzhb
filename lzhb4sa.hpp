#ifndef __LZHB4SA_HPP__
#define __LZHB4SA_HPP__
#include <string>
#include <tuple>
#include <vector>

#include "lzhb_common.hpp"
namespace lzhb4sa {

std::vector<lzhb::PhraseMod> parse(const std::string& s, uInt height_bound);
std::vector<lzhb::PhraseModC> parseC(const std::string& s, uInt height_bound);
std::vector<lzhb::PhraseMod> parseGreedier(const std::string& s,
                                           uInt height_bound);
std::vector<lzhb::PhraseModC> parseGreedierC(const std::string& s,
                                             uInt height_bound);

}  // namespace lzhb4sa
#endif  // __LZHB4SA_HPP__
