#include <sys/resource.h>
#include <sys/time.h>

#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "binencdec.hpp"
#include "cxxopts.hpp"
#include "lzhb_common.hpp"

// .lz   : naive bitpacked lzhb format
// .lzc  : naive bitpacked lzhb format with char
// .lzx  : naive bitpacked modified lzhb format
// .lzxc : naive bitpacked modified lzhb format with char
// .*p   : naive * format with positions instead of lengths

int main(int argc, char* argv[]) {
  cxxopts::Options options("lzhb_decomp",
                           "decompression accompanying lzhb implementations");
  options.add_options()("f,file", "input file",
                        cxxopts::value<std::string>()->default_value(""))(
      "o,outputfile", "output file",
      cxxopts::value<std::string>()->default_value(""))("h,help",
                                                        "Print usage");
  auto res = options.parse(argc, argv);
  if (res.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  std::string fname = res["file"].as<std::string>();
  std::string ofname = res["outputfile"].as<std::string>();
  if (fname.empty() || ofname.empty()) {
    std::cerr << "file name or output file name is empty" << std::endl;
    exit(1);
  }

  std::string outstr;
  if (fname.size() >= 3 && fname.substr(fname.size() - 3) == ".lz") {
    auto ans = binencdec::decodetoPhrase(fname, false);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 4 && fname.substr(fname.size() - 4) == ".lzp") {
    auto ans = binencdec::decodetoPhrase(fname, true);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 4 && fname.substr(fname.size() - 4) == ".lzc") {
    auto ans = binencdec::decodetoPhraseC(fname, false);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 5 && fname.substr(fname.size() - 5) == ".lzcp") {
    auto ans = binencdec::decodetoPhraseC(fname, true);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 4 && fname.substr(fname.size() - 4) == ".lzx") {
    auto ans = binencdec::decodetoTripletPhrase(fname, false);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 5 && fname.substr(fname.size() - 5) == ".lzxp") {
    auto ans = binencdec::decodetoTripletPhrase(fname, true);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 5 && fname.substr(fname.size() - 5) == ".lzxc") {
    auto ans = binencdec::decodetoTripletPhraseC(fname, false);
    outstr = lzhb::decode(ans);
  } else if (fname.size() >= 6 && fname.substr(fname.size() - 6) == ".lzxcp") {
    auto ans = binencdec::decodetoTripletPhraseC(fname, true);
    outstr = lzhb::decode(ans);
  } else {
    std::cerr << "file name does not have valid extension" << std::endl;
    exit(1);
  }
  std::ofstream fs(ofname, std::ifstream::binary);
  fs.write((char*)outstr.c_str(), outstr.size());
  fs.close();
  return 0;
}