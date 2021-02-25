
#include "Framework/TreeDiff/BareTree.h"

#include <set>

#include "Framework/Exception/Exception.h"

namespace framework {
namespace treediff {

BareTree::BareTree(TFile* f, const TString& tree_name,
                   const std::vector<TString>& ignore_substrs)
    : file_{f}, ignore_substrs_{ignore_substrs} {
  // first we get the tree from the file
  tree_ = (TTree*)file_->Get(tree_name);

  // make sure we got a tree
  if (not tree_) {
    EXCEPTION_RAISE("NullTree", ("No tree named '" + tree_name +
                                 "' exists in file '" + file_->GetName() + "'.")
                                    .Data());
  }

  // get the (flattened) list of branches wrapped with BareBranch
  branches_ = flatBranchList(tree_->GetListOfBranches());
}

bool BareTree::compare(const BareTree& other) const {
  // clear the diff-data from any old comparisons
  this->newComparison();
  other.newComparison();

  /**
   * Check size of the two trees.
   * If the size differs, issue a warning that the comparison is ill-formed.
   */
  if (this->tree_->GetEntriesFast() != other.tree_->GetEntriesFast()) {
    std::cerr << "[ BareTree ] WARN : Comparing trees of different sizes.\n"
                 "  This comparison will fail and list all branches as fails!"
              << std::endl;
  }

  // store branch names where we found a name-match
  // between the trees
  std::set<TString> match_found;

  // let's go through these branches
  for (const BareBranch& our_br : branches_) {
    // skip branches that match a pattern in the ignore list
    if (shouldIgnore(our_br)) continue;

    // this branch shouldn't be ignored, so let's try to find
    // it in the other tree
    bool found_name_match{false};
    for (const BareBranch& their_br : other.branches_) {
      // check if current pair of branches have the same name
      if (our_br.sameName(their_br)) {
        found_name_match = true;
        match_found.insert(their_br.name());
        if (not our_br.sameContent(their_br)) {
          // same name branches but not same content
          this->branches_diff_data_.push_back(our_br.name());
          other.branches_diff_data_.push_back(our_br.name());
        }
        break;  // leave after we find a name match
      }         // same name
    }           // loop over their branches

    if (not found_name_match) {
      // we weren't able to find a name match for this branch
      branches_only_here_.push_back(our_br.name());
    }
  }  // loop over our branches

  // check for branches only on the other tree
  for (const BareBranch& their_br : other.branches_) {
    // if branch is should not be ignored AND we couldn't
    // find a match, then this branch is only in the other tree
    if (not shouldIgnore(their_br) and
        match_found.find(their_br.name()) == match_found.end()) {
      other.branches_only_here_.push_back(their_br.name());
    }  // their branch was not found here
  }    // loop over their branches

  // perfect match is only when all three
  // containers of differences are empty
  return (branches_only_here_.empty() and branches_diff_data_.empty() and
          other.branches_only_here_.empty());
}

std::vector<BareBranch> BareTree::flatBranchList(TObjArray* list) const {
  std::vector<BareBranch> flattened_list;
  for (unsigned int i = 0; i < list->GetEntries(); i++) {
    TBranch* sub_branch = (TBranch*)list->At(i);
    if (sub_branch->GetListOfBranches()->GetEntries() > 0) {
      // recursion
      std::vector<BareBranch> sub_list{
          flatBranchList(sub_branch->GetListOfBranches())};
      flattened_list.insert(flattened_list.end(), sub_list.begin(),
                            sub_list.end());
    } else {
      flattened_list.emplace_back(file_, sub_branch);
    }
  }
  return flattened_list;
}

bool BareTree::shouldIgnore(const BareBranch& b) const {
  for (auto const& name_substr : ignore_substrs_) {
    if (b.name().Contains(name_substr)) return true;
  }
  return false;
}

}  // namespace treediff
}  // namespace framework
