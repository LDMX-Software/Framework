#ifndef FRAMEWORK_TREEDIFF_BARETREE_H
#define FRAMEWORK_TREEDIFF_BARETREE_H

#include <vector>

#include "Framework/TreeDiff/BareBranch.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"

namespace framework {
namespace treediff {

/**
 * Wrapper for TTrees where we only access the serialized
 * form of the data on the branches. This is convenient
 * for us because
 *
 * 1. It is faster than creating the necessary objects.
 * 2. We don't have to import that ROOT dictionary here.
 *
 * In the process of checking the equality of bare trees,
 * we can modify some member variables allowing us to look into
 * the form of an in-equality much better.
 */
class BareTree {
 public:
  /**
   * Generate the list of branches that will need to
   * be compared.
   *
   * @param[in] f handle to file we are reading from
   * @param[in] tree_name name of tree in file to wrap
   * @param[in] ignore_substrs list of sub-strings of branch names to ignore in
   * any future comparison
   */
  BareTree(TFile* f, const TString& tree_name,
           const std::vector<TString>& ignore_substrs = {});

  /**
   * Do the comparison between two BareTrees.
   *
   * We don't modify the actual list of the branches
   * but we do modify the list of branches that are only
   * here and that differ in their data.
   *
   * @see BareBranch::sameName and BareBranch::sameContent for
   * how we compare individual branches.
   *
   * @param[in] other Another BareTree to compare ourselves to
   * @return true if we have the same structure and content as other
   */
  bool compare(const BareTree& other) const;

  /**
   * Get the list of branches that are only in this map.
   *
   * @note This is only filled during a comparison!
   */
  const std::vector<TString>& getBranchesOnlyHere() const {
    return branches_only_here_;
  }

  /**
   * Get the list of branches that differ in data between
   * this map and the one most recently compared to it.
   *
   * @note This is only filled during a comparison!
   */
  const std::vector<TString>& getBranchesDiffData() const {
    return branches_diff_data_;
  }

 private:
  /**
   * Flatten the hierarchical branch list into lowest level branches
   * and wrap them in our bare class.
   *
   * When ROOT knows how to, it saves space and time by "splitting"
   * higher level branches into sub-branches. The sub-branches are
   * actually where all the baskets and data are stored, so we
   * need a list of them.
   */
  std::vector<BareBranch> flatBranchList(TObjArray* branch_list) const;

  /**
   * Reset comparison objects
   */
  void newComparison() const {
    branches_only_here_.clear();
    branches_diff_data_.clear();
  }

  /**
   * Check if the input branch should be ignored.
   *
   * @return true if we should skip the branch
   */
  bool shouldIgnore(const BareBranch& b) const;

 private:
  /**
   * The file that we are reading the data from.
   */
  TFile* file_;

  /**
   * A handle to the tree we are reading.
   *
   * @note Not sure if this is needed,
   * only used to get the list of branches.
   */
  TTree* tree_;

  /**
   * The list of branches that have no sub-branches.
   */
  std::vector<BareBranch> branches_;

  /**
   * List of branch name sub-strings to ignore.
   *
   * The reason we use sub-strings is to avoid having
   * to specify the pass name which we encode into the branch name.
   */
  std::vector<TString> ignore_substrs_;

  /**
   * Branches only in this tree map after a comparison is made.
   */
  mutable std::vector<TString> branches_only_here_;

  /**
   * Branches that have different data in this tree map and
   * the other after a comparison is made.
   */
  mutable std::vector<TString> branches_diff_data_;
};

}  // namespace treediff
}  // namespace framework

#endif  // FRAMEWORK_TREEDIFF_BARETREE_H
