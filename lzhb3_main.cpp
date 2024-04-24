#include <sys/resource.h>
#include <sys/time.h>

#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "binencdec.hpp"
#include "cxxopts.hpp"
#include "lzhb3.hpp"
#include "lzhb3sa.hpp"
#include "lzhb_common.hpp"

void run(const std::string& s, const std::string& fname, size_t height_bound,
         bool greedier, bool suffixarray, const std::string& outfn,
         bool verify) {
  auto ttstart = std::chrono::system_clock::now();
  auto ans = greedier ? (suffixarray ? lzhb3sa::parseGreedier(s, height_bound)
                                     : lzhb3::parseGreedier(s, height_bound))
                      : (suffixarray ? lzhb3sa::parse(s, height_bound)
                                     : lzhb3::parse(s, height_bound));
  auto ttend = std::chrono::system_clock::now();
  auto dur = ttend - ttstart;
  double msec =
      std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

  std::time_t t = std::chrono::system_clock::to_time_t(ttstart);
  const std::tm* lt = std::localtime(&t);

  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  auto heights = lzhb::height_array(ans);

  if (verify) {
    std::cerr << "decode check: " << ((s == lzhb::decode(ans)) ? "OK" : "NG")
              << std::endl;
    assert(s == lzhb::decode(ans));
  }

  std::string progname = greedier ? (suffixarray ? "lzhb3SAz" : "lzhb3z")
                                  : (suffixarray ? "lzhb3SA" : "lzhb3");
  // output:
  // "time_stamp,algorithm,file_name,height_bound,file_size,cmp_size,"
  // "max_height,height_average,height_variance,msec,ru_maxrss"
  std::cout << std::put_time(lt, "%c") << "," << progname << "," << fname << ","
            << height_bound << "," << s.size() << "," << ans.size() << ","
            << heights.second << "," << lzhb::average(heights.first) << ","
            << lzhb::variance(heights.first) << "," << msec << ","
            << usage.ru_maxrss << std::endl;
  if (!outfn.empty()) {
    binencdec::encodePhrase(ans, outfn, true, false);
    binencdec::encodePhrase(ans, outfn, true, true);
  }
}

void runC(const std::string& s, const std::string& fname, uInt height_bound,
          bool greedier, bool suffixarray, const std::string& outfn,
          bool verify) {
  auto ttstart = std::chrono::system_clock::now();
  auto ans = greedier ? (suffixarray ? lzhb3sa::parseGreedierC(s, height_bound)
                                     : lzhb3::parseGreedierC(s, height_bound))
                      : (suffixarray ? lzhb3sa::parseC(s, height_bound)
                                     : lzhb3::parseC(s, height_bound));
  auto ttend = std::chrono::system_clock::now();
  auto dur = ttend - ttstart;
  double msec =
      std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

  std::time_t t = std::chrono::system_clock::to_time_t(ttstart);
  const std::tm* lt = std::localtime(&t);

  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  auto heights = lzhb::height_array(ans);

  if (verify) {
    std::cerr << "decode check: " << ((s == lzhb::decode(ans)) ? "OK" : "NG")
              << std::endl;
    assert(s == lzhb::decode(ans));
  }

  std::string progname = greedier ? (suffixarray ? "lzhb3SAaz" : "lzhb3az")
                                  : (suffixarray ? "lzhb3SAa" : "lzhb3a");
  // output:
  // "time_stamp,algorithm,file_name,height_bound,file_size,cmp_size,"
  // "max_height,height_average,height_variance,msec,ru_maxrss"
  std::cout << std::put_time(lt, "%c") << "," << progname << "," << fname << ","
            << height_bound << "," << s.size() << "," << ans.size() << ","
            << heights.second << "," << lzhb::average(heights.first) << ","
            << lzhb::variance(heights.first) << "," << msec << ","
            << usage.ru_maxrss << std::endl;
  if (!outfn.empty()) {
    binencdec::encodePhraseC(ans, outfn, true, false);
    binencdec::encodePhraseC(ans, outfn, true, true);
  }
}

int main(int argc, char* argv[]) {
  std::string s;
  size_t height_bound;
  cxxopts::Options options(
      "lzhb3_main", "height-bounded lz that finds longest valid occurrence");
  options.add_options()("f,file", "input file (use stdin if unspecified)",
                        cxxopts::value<std::string>()->default_value(""))(
      "o,outputfile",
      "output file (no compressed representation if unspecified)",
      cxxopts::value<std::string>()->default_value(""))(
      "a,appendchar", "phrases hold an explicit character at the end",
      cxxopts::value<bool>()->default_value("false"))(
      "s,suffixarray",
      "O(n\\log n) suffix array version (default is to use O(n \\log \\sigma) "
      "suffix tree version)",
      cxxopts::value<bool>()->default_value("false"))(
      "z,optimize", "greedily find best occurrence (minimize maximum height)",
      cxxopts::value<bool>()->default_value("false"))(
      "b,hbound", "maximum allowed height",
      cxxopts::value<size_t>()->default_value("0xffffffffffffffff"))(
      "g,verify", "verify output in various ways",
      cxxopts::value<bool>()->default_value("false"))("h,help", "Print usage");
  auto res = options.parse(argc, argv);
  if (res.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  try {
    height_bound = res["hbound"].as<size_t>();
  } catch (const cxxopts::exceptions::exception& e) {
    height_bound = -1;
  }
  std::string fname = res["file"].as<std::string>();
  std::string ofname = res["outputfile"].as<std::string>();
  if (fname != "") {
    s = lzhb::fileread(fname);
    if (res["appendchar"].as<bool>())
      runC(s, fname, height_bound, res["optimize"].as<bool>(),
           res["suffixarray"].as<bool>(), ofname, res["verify"].as<bool>());
    else
      run(s, fname, height_bound, res["optimize"].as<bool>(),
          res["suffixarray"].as<bool>(), ofname, res["verify"].as<bool>());
  } else {
    while (std::cin >> s) {
      if (res["appendchar"].as<bool>())
        runC(s, fname, height_bound, res["optimize"].as<bool>(),
             res["suffixarray"].as<bool>(), ofname, res["verify"].as<bool>());
      else
        run(s, fname, height_bound, res["optimize"].as<bool>(),
            res["suffixarray"].as<bool>(), ofname, res["verify"].as<bool>());
    }
  }
  return 0;
}
