
#include <iostream>
#include <set>

#include "Framework/TreeDiff/Compare.h"
#include "TString.h"

/**
 * Print the usage of this executable to std::cout
 */
static inline void usage() {
  std::cout << "Use: tree-diff [-h,--help] [-i,--ignore s0]\n"
            << "               -t,--tree name0 [-t,--tree name1 ...]\n"
            << "               {file1.root} {file2.root}\n"
            << "-h,--help  Print this help message and exit.\n"
            << "-i,--ignore\n"
            << "           Substrings of branches to ignore. Can specify more "
               "than once.\n"
            << "-t,--tree  Define name(s) of tree(s) to compare. At least one "
               "required.\n";
}

/**
 * Helper function for configuration error where
 * an input flag isn't provided its required argument.
 */
static inline void needsArgAfter(const TString& arg) {
  usage();
  std::cout << "** Flag " << arg << " requires an argument after it. **"
            << std::endl;
}

/**
 * tree-diff
 *
 * Compare the difference between trees
 * in two separate files.
 *
 * @see usage for what the arguments should be
 */
int main(int argc, char* argv[]) {
  std::vector<TString> to_ignore;
  std::vector<TString> tree_names;
  std::vector<TString> file_names;
  for (int arg_i{1}; arg_i < argc; arg_i++) {
    TString arg{argv[arg_i]};
    if (arg.EqualTo("-h") or arg.EqualTo("--help")) {
      usage();
      return 0;
    } else if (arg.EqualTo("-i") or arg.EqualTo("--ignore")) {
      if (arg_i + 1 > argc or argv[arg_i + 1][0] == '-') {
        needsArgAfter(arg);
        return framework::treediff::FAILED_TO_RUN;
      }
      to_ignore.emplace_back(argv[arg_i + 1]);
      // shift arg index to skip name after flag
      arg_i++;
    } else if (arg.EqualTo("-t") or arg.EqualTo("--tree")) {
      if (arg_i + 1 > argc or argv[arg_i + 1][0] == '-') {
        needsArgAfter(arg);
        return framework::treediff::FAILED_TO_RUN;
      }
      tree_names.emplace_back(argv[arg_i + 1]);
      // shift arg index to skip name after flag
      arg_i++;
    } else {
      file_names.push_back(arg);
    }
  }

  if (file_names.size() != 2) {
    usage();
    std::cerr << "Files Given: ";
    for (auto const& f : file_names) std::cerr << f << " ";
    std::cerr << std::endl;
    std::cerr << "** Need to specify exactly two files **" << std::endl;
    return framework::treediff::FAILED_TO_RUN;
  }

  if (tree_names.empty()) {
    usage();
    std::cerr << "** Need to specify at least one tree to compare **"
              << std::endl;
    return framework::treediff::FAILED_TO_RUN;
  }

  return framework::treediff::compare(file_names.at(0), file_names.at(1),
                                      tree_names, to_ignore);
}
