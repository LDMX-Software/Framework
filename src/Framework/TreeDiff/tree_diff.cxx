
#include <iostream>
#include <set>

#include "TString.h"

#include "Framework/Exception/Exception.h"
#include "Framework/TreeDiff/BareTree.h"

static inline void usage() {
  std::cout << "Use: root-diff [-h,--help] [-i,--ignore class_name]" << std::endl;
  std::cout << "               -t,--tree tree_name" << std::endl;
  std::cout << "               {file1.root} {file2.root}" << std::endl;
  std::cout << "  Display the objects that are different between the two input root files." << std::endl;
  std::cout << "-h,--help  Print this help message and exit." << std::endl;
  std::cout << "-i,--ignore" << std::endl;
  std::cout << "           Name of class to ignore. Can specify more than once." << std::endl;
  std::cout << "-t,--tree  Define name of tree to compare. REQUIRED." << std::endl;
}

static inline void needsArgAfter(const TString &arg) {
  usage();
  std::cout << "** Flag " << arg
    << " requires an argument after it. **" << std::endl;
}

static const int FAILED_TO_RUN{127};
static const int MATCH{0};
static const int MISMATCH{1};

/**
 * tree-diff
 *
 * Compare the difference between the same tree
 * in two separate files.
 */
int main(int argc, char *argv[]) {
  std::set<TString> ignore_branches;
  TString tree_name;
  std::vector<TString> file_names;
  for (int arg_i{1}; arg_i < argc; arg_i++) {
    TString arg{argv[arg_i]};
    if (arg.EqualTo("-h") or arg.EqualTo("--help")) {
      usage();
      return 0;
    } else if (arg.EqualTo("-i") or arg.EqualTo("--ignore")) {
      if (arg_i+1 > argc) {
        needsArgAfter(arg);
        return FAILED_TO_RUN;
      }
      ignore_branches.insert(argv[arg_i+1]);
      //shift arg index to skip name after flag
      arg_i++;
    } else if (arg.EqualTo("-t") or arg.EqualTo("--tree")) {
      if (arg_i+1 > argc) {
        needsArgAfter(arg);
        return FAILED_TO_RUN;
      }
      tree_name = argv[arg_i+1];
      //shift arg index to skip name after flag
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
    TFile f1(file_names.at(0));
    TFile f2(file_names.at(1));

    framework::treediff::BareTree t1(&f1,tree_name);
    framework::treediff::BareTree t2(&f2,tree_name);

    if (t1.compare(t2)) {
      //match successful, leave early
      return MATCH;
    }

    // match not successful, let's print what was wrong

    // there are some branches that are in one file and not the other
    //   //      what are these branches?
    auto only_in_file_1 = t1.getBranchesOnlyHere();
    if (not only_in_file_1.empty()) {
      std::cout << "== Branches Only in '" << file_names.at(0)
                << "'==" << std::endl;
      for (const auto& b : only_in_file_1) std::cout << b << std::endl;
    }

    auto only_in_file_2 = t2.getBranchesOnlyHere();
    if (not only_in_file_2.empty()) {
      std::cout << "== Objects Only in '" << file_names.at(1)
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
