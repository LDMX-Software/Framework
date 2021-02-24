
#include <iostream>
#include <set>

#include "Framework/Exception/Exception.h"
#include "Framework/TreeDiff/BareTree.h"
#include "TString.h"

/**
 * Print the usage of this executable to std::cout
 */
static inline void usage() {
  std::cout << "Use: tree-diff [-h,--help] [-i,--ignore class_name]"
            << std::endl;
  std::cout << "               -t,--tree tree_name" << std::endl;
  std::cout << "               {file1.root} {file2.root}" << std::endl;
  std::cout << "  Display the objects that are different between the two input "
               "root files."
            << std::endl;
  std::cout << "-h,--help  Print this help message and exit." << std::endl;
  std::cout << "-i,--ignore" << std::endl;
  std::cout << "           Name of class to ignore. Can specify more than once."
            << std::endl;
  std::cout << "-t,--tree  Define name of tree to compare. REQUIRED."
            << std::endl;
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

/// return status when executable failed to run
static const int FAILED_TO_RUN{127};

/// return status for a perfect match
static const int MATCH{0};

/// return status for a successful run but failed match
static const int MISMATCH{1};

/**
 * tree-diff
 *
 * Compare the difference between the same tree
 * in two separate files.
 *
 * @see usage for what the arguments should be
 */
int main(int argc, char* argv[]) {
  std::vector<TString> to_ignore;
  TString tree_name;
  std::vector<TString> file_names;
  for (int arg_i{1}; arg_i < argc; arg_i++) {
    TString arg{argv[arg_i]};
    if (arg.EqualTo("-h") or arg.EqualTo("--help")) {
      usage();
      return 0;
    } else if (arg.EqualTo("-i") or arg.EqualTo("--ignore")) {
      if (arg_i + 1 > argc or argv[arg_i + 1][0] == '-') {
        needsArgAfter(arg);
        return FAILED_TO_RUN;
      }
      to_ignore.emplace_back(argv[arg_i + 1]);
      // shift arg index to skip name after flag
      arg_i++;
    } else if (arg.EqualTo("-t") or arg.EqualTo("--tree")) {
      if (arg_i + 1 > argc or argv[arg_i + 1][0] == '-') {
        needsArgAfter(arg);
        return FAILED_TO_RUN;
      }
      tree_name = argv[arg_i + 1];
      // shift arg index to skip name after flag
      arg_i++;
    } else {
      file_names.push_back(arg);
    }
  }

  if (file_names.size() != 2) {
    usage();
    std::cerr << "** Need to specify exactly two files **" << std::endl;
  }

  if (tree_name.IsNull()) {
    usage();
    std::cerr << "** Need to specify name of TTree to compare **" << std::endl;
  }

  try {
    /**
     * Loading the files causes ROOT to print
     * a lot of 'dictionary not available' warnings.
     *
     * Can we silence them?
     */
    TFile f1(file_names.at(0));
    TFile f2(file_names.at(1));

    framework::treediff::BareTree t1(&f1, tree_name, to_ignore);
    framework::treediff::BareTree t2(&f2, tree_name, to_ignore);

    if (t1.compare(t2)) {
      // match successful, leave early
      return MATCH;
    }

    // match not successful, let's print what was wrong

    auto only_in_file_1 = t1.getBranchesOnlyHere();
    if (not only_in_file_1.empty()) {
      std::cout << "== Branches Only in '" << file_names.at(0)
                << "'==" << std::endl;
      for (const auto& b : only_in_file_1) std::cout << b << std::endl;
    }

    auto only_in_file_2 = t2.getBranchesOnlyHere();
    if (not only_in_file_2.empty()) {
      std::cout << "== Branches Only in '" << file_names.at(1)
                << "'==" << std::endl;
      for (const auto& b : only_in_file_2) std::cout << b << std::endl;
    }

    auto diff_branches = t1.getBranchesDiffData();
    if (not diff_branches.empty()) {
      std::cout << "== Branches with different content ==" << std::endl;
      for (const auto& b : diff_branches) std::cout << b << std::endl;
      // optionally make histograms comparing these branches?
    }

    return MISMATCH;

  } catch (framework::exception::Exception& e) {
    std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
              << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << "\nStack trace: " << std::endl
              << e.stackTrace();
    return FAILED_TO_RUN;
  }
}
