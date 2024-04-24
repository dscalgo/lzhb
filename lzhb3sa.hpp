#ifndef __LZHB3SA_HPP__
#define __LZHB3SA_HPP__
#include <string>
#include <vector>

#include "lzhb_common.hpp"
namespace lzhb3sa {
std::vector<lzhb::Phrase> parse(const std::string& s, uInt height_bound);
std::vector<lzhb::Phrase> parseGreedier(const std::string& s,
                                        uInt height_bound);
std::vector<lzhb::PhraseC> parseC(const std::string& s, uInt height_bound);
std::vector<lzhb::PhraseC> parseGreedierC(const std::string& s,
                                          uInt height_bound);
}  // namespace lzhb3sa
#endif  // __LZHB3SA_HPP__