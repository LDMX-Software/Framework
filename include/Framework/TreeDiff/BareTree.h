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
 * 3. Comparing buffers is safer and less prone to bugs
 *    than writing custom comparison operators for all
 *    our objects.
 *
 * In the process of checking the equality of bare trees,
 * we can modify some member variables allowing us to look into
 * the form of an in-equality much better.
 *
 * Similar to regular `diff` or `git diff`,
 * this method of comparison is really only helpful
 * if the trees being compared are (in some sense) "close"
 * to being identical. 
 *
 * For example, if the two trees only
 * differ by their number of entries (say one tree has one
 * more event than the other), all of the branches will be
 * listed as having "different content".
 */
class BareTree {
 public:
  /**
   * Generate the list of branches that will need to be compared.
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
   * @throws Exception if reading buffers in branches fails.
   *
   * We warn the user if the two trees being compared are different sizes.
   * This is because the comparison *will* fail and list *all* branches
   * as having different content.
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
   * need a list of them. Moreover, sometimes this "splitting"
   * is done recursively when a top-level class has another
   * "splitt-able" class as a member.
   *
   * This function is recursive because sometimes ROOT is recursive.
   *
   * @param[in] branch_list List of branches 
   * retrieved from TBranch::GetListOfBranches()
   * @returns list of flattened branches wrapped in our BareBranch
   */
  std::vector<BareBranch> flatBranchList(TObjArray* branch_list) const;

  /**
   * Reset comparison objects
   *
   * Currently, this is not needed since both executables that
   * use this comparison method only execute one comparison;
   * nevertheless, I could forsee the addition of multiple
   * comparisons in one run e.g. to do a comparison between
   * all pairs of three files.
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
