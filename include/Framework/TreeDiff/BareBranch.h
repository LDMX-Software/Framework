#ifndef FRAMEWORK_TREEDIFF_BAREBRANCH_H
#define FRAMEWORK_TREEDIFF_BAREBRANCH_H

#include "TFile.h"
#include "TBranch.h"

namespace framework {
namespace treediff {

/**
 * Branch *without* access to the data in C++ form.
 *
 * This gives us the ability to access the serialized
 * data from the corresponding branch of the same name
 * in the tree.
 *
 * We assume that this is the "lowest-level" branch,
 * i.e. the branch we wrap here *does not* have any
 * child branches.
 */
class BareBranch {
 public:
  /**
   * Wrap a branch and the file it came from
   * in our class.
   */
  BareBranch(TFile* f, TBranch* b) : file_{f}, branch_{b} {}

  /**
   * Get the name of this branch
   *
   * We use the "full" name because lots of
   * hierachical data is 'split' into sub branches
   * and the full name will give us the name of
   * the sub branch and all its parent branches.
   */
  const TString name() const {
    return branch_->GetFullName();
  }

  /**
   * Do we and the passed BareBranch have the same name?
   */
  bool sameName(const BareBranch& other) const {
    return name().CompareTo(other.name()); 
  }

  /**
   * Do we and the passed BareBranch have the same content?
   */
  bool sameContent(const BareBranch& other) const;

  /**
   * Define equality operator so the symmetric nature
   * of this comparison is made plain.
   */
  bool operator==(const BareBranch& other) const {
    return sameName(other) and sameContent(other);
  }

  /**
   * Get the buffer stored in the passed basket index.
   *
   * @note This returns a newly allocated char array,
   * make sure to clean up after yourself!
   */
  char* getContent(int i_basket, int& len) const;

 private:

  /**
   * The file that we are reading the data from.
   *
   * We have to make this mutable because ROOT is crap
   * and const correctness.
   *
   * We only need this handle to be able to use the
   * TFile::ReadBuffer method which (as the name implies)
   * does not modify the TFile.
   */
  TFile* file_;

  /**
   * A handle to the branch we are reading.
   *
   * We have to make this mutable because ROOT is crap
   * and const correctness.
   */
  TBranch* branch_;
};

}
}

#endif // FRAMEWORK_TREEDIFF_BAREBRANCH_H
