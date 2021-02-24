
#include "Framework/TreeDiff/BareTree.h"
#include "Framework/Exception/Exception.h"

#include <set>

namespace framework {
namespace treediff {

BareTree::BareTree(TFile* f, const TString &tree_name) : file_{f} {

  // first we get the tree from the file
  // and get the (flattened) branch list
  tree_     = (TTree*)file_->Get(tree_name);
  
  if (not tree_) {
    EXCEPTION_RAISE("NullTree",
        ("No tree named '" + tree_name + "' exists in file '"
        + file_->GetName() + "'.").Data());
  }
  
  branches_ = flatBranchList(tree_->GetListOfBranches());
}

bool BareTree::compare(const BareTree& other) const {
  this->newComparison();
  other.newComparison();

  // store branch names where we found a name-match
  // between the trees
  std::set<TString> match_found;

  // let's go through these branches
  for (const BareBranch& our_br : branches_) {
    bool found_name_match{false};
    for (const BareBranch& their_br : other.branches_) {
      if (our_br.sameName(their_br)) {
        found_name_match = true;
        match_found.insert(their_br.name());
        if (not our_br.sameContent(their_br)) {
          this->branches_diff_data_.push_back(our_br.name());
          other.branches_diff_data_.push_back(our_br.name());
        }
        break;
      }
    }

    if (not found_name_match) {
      branches_only_here_.push_back(our_br.name());
    }
  } // loop over branches

  // check for branches only on the other tree
  for (const BareBranch& their_br : other.branches_) {
    if (match_found.find(their_br.name()) == match_found.end()) {
      other.branches_only_here_.push_back(their_br.name());
    }
  }

  return (branches_only_here_.empty() 
          and branches_diff_data_.empty() 
          and other.branches_only_here_.empty());
}

std::vector<BareBranch> BareTree::flatBranchList(TObjArray* list) const {
  std::vector<BareBranch> flattened_list;
  for (unsigned int i=0; i < list->GetEntries(); i++) {
    TBranch* sub_branch = (TBranch*)list->At(i);
    if (sub_branch->GetListOfBranches()->GetEntries() > 0) {
      //recursion
      std::vector<BareBranch> sub_list{flatBranchList(sub_branch->GetListOfBranches())};
      flattened_list.insert( flattened_list.end(), sub_list.begin(), sub_list.end());
    } else {
      flattened_list.emplace_back(file_,sub_branch);
    }
  }
  return flattened_list;
}

}
}
