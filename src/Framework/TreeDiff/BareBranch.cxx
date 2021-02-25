
#include "Framework/TreeDiff/BareBranch.h"

#include <cstring>
#include <iostream>

#include "Framework/Exception/Exception.h"
#include "RZip.h"
#include "TBasket.h"

namespace framework {
namespace treediff {

bool BareBranch::sameContent(const BareBranch& other) const {
  // load all of the baskets of this branch into memory
  // ROOT groups data on branches into 'baskets' in order
  // to keep the current amount of memory in use below
  // a certain threshold, each basket corresponds to a
  // certain number of entries in the branch compressed
  // and saved into the output file
  int num_our_baskets   = this->branch_->LoadBaskets();
  int num_their_baskets = other.branch_->LoadBaskets();

  if (num_our_baskets != num_their_baskets) {
    // mismatching number of baskets
    // probably due to change in compression level/algo
    // (if the files are supposed to be the same)
    
    // make sure to drop all our baskets
    // so that ROOT knows we don't care about those objects
    // anymore
    this->branch_->DropBaskets("all");
    other.branch_->DropBaskets("all");
    return false;
  }

  /**
   * WARN
   * We are assuming that our baskets and their baskets
   * are in the same order!! I don't know if this is a safe
   * assumption. I think it is, but I haven't found any
   * documentation assuring me of this fact.
   */
  bool content_match{true};
  for (int i_basket{0}; i_basket < num_our_baskets; i_basket++) {
    int our_len{0}, their_len{0};
    char* our_buff   = this->getContent(i_basket, our_len  );
    char* their_buff = other.getContent(i_basket, their_len);

    /**
     * The contents of these two baskets match if
     *  (1) they are the same size (or equivalently, length) AND
     *  (2) the data stored is equivalent bit-by-bit
     */
    content_match =
        (our_len == their_len) and (memcmp(our_buff, their_buff, our_len) == 0);

    delete[] our_buff;
    delete[] their_buff;

    // leave on first failure
    if (not content_match) break;
  }

  // make sure to drop all our baskets
  // so that ROOT knows we don't care about those objects
  // anymore
  this->branch_->DropBaskets("all");
  other.branch_->DropBaskets("all");

  // content match will stay true only
  // if all baskets have the same size
  // and content bytes
  return content_match;
}

char* BareBranch::getContent(int i_basket, int& len) const {
  TBasket* basket = branch_->GetBasket(i_basket);
  if (not basket) {
    EXCEPTION_RAISE("NullBasket",
                    ("Received a NULL basket for branch " + name() +
                     " and basket index " + std::to_string(i_basket))
                        .Data());
  }

  int compressed_len = basket->GetNbytes() - basket->GetKeylen();
  char* compressed_content = new char[compressed_len];
  if (file_->ReadBuffer(compressed_content,
                        basket->GetSeekKey() + basket->GetKeylen(),
                        compressed_len)) {
    // non-zero return status is a failure
    delete[] compressed_content;
    EXCEPTION_RAISE("ReadFail",
                    ("Failure to read basked " + std::to_string(i_basket) +
                     " from branch " + name() + " in file " + file_->GetName())
                        .Data());
  }

  /* don't try to uncompress
   * helpful if decompression is being dramatic
  len = compressed_len;
  return compressed_content;
  */

  char* content;
  if (basket->GetObjlen() > compressed_len) {
    // need to de-compress
    len = basket->GetObjlen();
    content = new char[len];

    /**
     * unzipped_len should end up being equal to decompressed_len
     * but they should be separate so that we can check if there was
     * an error.
     *
     * Both of these should also be equal to the length of the object
     * (len == basket->GetObjlen()), but again, we want to be safe
     * and make sure there isn't any funny business.
     */
    int unzipped_len, decompressed_len;
    // get information on how content was compressed
    int rc = R__unzip_header(
        &compressed_len, (unsigned char*)compressed_content, &decompressed_len);
    if (rc != 0) {
      EXCEPTION_RAISE("UnzipFail",
                      ("Failed to de-compress basket " +
                       std::to_string(i_basket) + " from branch " + name())
                          .Data());
    }
    // Actually uncompress some content
    R__unzip(&compressed_len, (unsigned char*)compressed_content,
             &decompressed_len, (unsigned char*)content, &unzipped_len);
    if (unzipped_len == 0) {
      EXCEPTION_RAISE("UnzipFail",
                      ("Failed to de-compress basket " +
                       std::to_string(i_basket) + " from branch " + name())
                          .Data());
    }

  } else {
    // don't need to de-compress
    len = compressed_len;
    content = new char[len];
    memcpy(content, compressed_content, len);
  }

  delete[] compressed_content;

  return content;
}

}  // namespace treediff
}  // namespace framework
