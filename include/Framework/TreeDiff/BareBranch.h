#ifndef FRAMEWORK_TREEDIFF_BAREBRANCH_H
#define FRAMEWORK_TREEDIFF_BAREBRANCH_H

#include "TBranch.h"
#include "TFile.h"

namespace framework {
namespace treediff {

/**
 * Branch *without* access to the data in C++ form.
 *
 * This gives us the ability to access the serialized data from the 
 * corresponding branch of the same name in the tree.
 *
 * We assume that this is the "lowest-level" branch, 
 * i.e. the branch we wrap here *does not* have any child branches.
 */
class BareBranch {
 public:
  /**
   * Wrap a branch and the file it came from in our class.
   */
  BareBranch(TFile* f, TBranch* b) : file_{f}, branch_{b} {}

  /**
   * Get the name of this branch
   *
   * We use the "full" name because lots of
   * hierachical data is 'split' into sub branches
   * and the full name will give us the name of
   * the sub branch and all its parent branches.
   *
   * This allows us to ignore any potential conflicts
   * when two different branches may have the same
   * sub-branch name e.g. two branches of the same type.
   *
   * @note We could think about implementing a method of
   * removing the pass name from the full branch name here.
   * I'm opposed to this, but that would make it easier to
   * compare two files whose only difference is the pass name.
   */
  const TString name() const { return branch_->GetFullName(); }

  /**
   * Do we and the passed BareBranch have the same name?
   *
   * @param[in] other another BareBranch to compare names with
   * @returns true if our name and their name match exactly
   */
  bool sameName(const BareBranch& other) const {
    return name().EqualTo(other.name());
  }

  /**
   * Do we and the passed BareBranch have the same content?
   *
   * We don't do smart things like checking if the two branches
   * share any content. It is a simple manner of checking if
   * *any* of the serialized data differs. This includes the case
   * where one branch just happens to have more entries. Even
   * if one branch is a perfect subset of another, since they
   * are different sizes this comparison will fail.
   *
   * This is the heavy duty part, so pay attention.
   *
   * 1. Load the baskets of both our branch and their branch into memory.
   *    This lets ROOT know that we will *not* be reading this file
   *    sequentially and instead looking at this branch entirely.
   * 2. Make sure the number of baskets is the same.
   *    Since the splitting and compression of data is completely deterministic
   *    in ROOT, *I think* this will only fail if the splitlevel or buffsize of our
   *    branches change or if the configuration of ROOT's memory changes.
   * 3. Compare the decompressed buffers of each of the baskets in sequence.
   *    If any of the baskets don't match in length (amount of data)
   *    or content (the data itself), we fail the comparison.
   * 4. Clean up after ourselves by deleting buffers and dropping baskets.
   *
   * @note I assume that our baskets and their baskets are in the same order.
   * I don't know if this is a safe assumption, but it seems to work.
   * It also makes intuitive sense that the baskets would be in the same order
   * since the data was generalized and serialized in a deterministic
   * fashion.
   *
   * @note This method of comparison assumes that the splitlevel
   * and buffsize input into the TTree::Branch method when the branches
   * were being created are the same. In other words, it is likely
   * that two branches with the same data but different splitlevel
   * and/or buffsize will fail the comparison.
   *
   * @see getContent for how we get the data from a basket
   * @throws Exceptions if reading buffers for this branch fails.
   *
   * @param[in] other another BareBranch to check against
   * @returns true if our content matches other's perfectly
   */
  bool sameContent(const BareBranch& other) const;

 private:
  /**
   * Get the buffer stored in the passed basket index.
   *
   * We do this in two steps.
   *
   * 1. Read the serialized data from the file.
   * 2. Decompress the data (if need-be).
   *
   * @see TFile::ReadBuffer for how we read in the data from the file.
   * @see R__unzip_header and R__unzip for how we decompress our buffer.
   *
   * @note This returns a newly allocated char array,
   * make sure to clean up after yourself!
   *
   * @throws Exception if we can't access the basket for the passed index.
   * @throws Exception if we can't read the basket from the file we have.
   * @throws Exception if we aren't able to decompress the buffer.
   *
   * Fun Fact: Since a 'char' is 1 byte, the "length" of the buffer
   * array is _also_ the "size" of the data in bytes. Sometimes, I
   * use the word "length" becuase I'm thinking of the buffer as an
   * array of char and sometimes I use "size" because I'm thinking
   * of the data that is being stored.
   *
   * @param[in] i_basket Index of basket to read in
   * @param[out] len Length of buffer read in
   * @returns newly allocated buffer of length len
   */
  char* getContent(int i_basket, int& len) const;

 private:
  /**
   * The file that we are reading the data from.
   *
   * We only need this handle to be able to use the
   * TFile::ReadBuffer method which (as the name implies)
   * does not modify the TFile.
   */
  TFile* file_;

  /**
   * A handle to the branch we are reading.
   *
   * Used for getting the baskets of data.
   */
  TBranch* branch_;
};

}  // namespace treediff
}  // namespace framework

#endif  // FRAMEWORK_TREEDIFF_BAREBRANCH_H
