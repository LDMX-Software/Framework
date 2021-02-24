
#include <iostream>

#include "TFile.h"

#include "Framework/Exception/Exception.h"
#include "Framework/TreeDiff/Compare.h"
#include "Framework/TreeDiff/BareTree.h"

namespace framework {
namespace treediff {

int compare(const TString& f1, const TString& f2,
            const std::vector<TString>& trees,
            const std::vector<TString>& to_ignore) {

  try {
    /**
     * Loading the files causes ROOT to print
     * a lot of 'dictionary not available' warnings.
     *
     * Can we silence them?
     */
    TFile file_1(f1);
    TFile file_2(f2);

    bool mismatch{false};
    for (auto const& tree_name : trees) {
      BareTree tree_1(&file_1,tree_name,to_ignore);
      BareTree tree_2(&file_2,tree_name,to_ignore);
      
      if (tree_1.compare(tree_2)) {
        //success! go to next tree immediately
        continue;
      }

      // match not successful, let's print what was wrong
      mismatch = true;

      std::cout << tree_name << " mismatched between files" << std::endl;
      auto only_in_file_1 = tree_1.getBranchesOnlyHere();
      if (not only_in_file_1.empty()) {
        std::cout << "== Branches Only in '" 
          << file_1.GetName() << "' ==" << std::endl;
        for (const auto& b : only_in_file_1) std::cout << b << std::endl;
      }
  
      auto only_in_file_2 = tree_2.getBranchesOnlyHere();
      if (not only_in_file_2.empty()) {
        std::cout << "== Branches Only in '" 
          << file_2.GetName() << "' ==" << std::endl;
        for (const auto& b : only_in_file_2) std::cout << b << std::endl;
      }
  
      auto diff_branches = tree_1.getBranchesDiffData();
      if (not diff_branches.empty()) {
        std::cout << "== Branches with different content ==" << std::endl;
        for (const auto& b : diff_branches) std::cout << b << std::endl;
      }
      std::cout << std::endl;

    }

    if (mismatch) return MISMATCH;
    else return MATCH;

  } catch (framework::exception::Exception& e) {
    std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
              << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << "\nStack trace: " << std::endl
              << e.stackTrace();
    return FAILED_TO_RUN;
  }
}


}
}
