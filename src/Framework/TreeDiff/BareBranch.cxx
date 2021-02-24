
#include <cstring>

#include "RZip.h"
#include "TBasket.h"

#include "Framework/Exception/Exception.h"
#include "Framework/TreeDiff/BareBranch.h"

namespace framework {
namespace treediff {

bool BareBranch::sameContent(const BareBranch& other) const {
  // load all of the baskets of this branch into memory
  // ROOT groups data on branches into 'baskets' in order
  // to keep the current amount of memory in use below 
  // a certain threshold, each basket corresponds to a
  // certain number of entries in the branch compressed
  // and saved into the output file
  int num_our_baskets = this->branch_->LoadBaskets();
  int num_their_baskets = other.branch_->LoadBaskets();

  if (num_our_baskets != num_their_baskets) {
    // mismatching number of baskets
    // probably due to change in compression level/algo
    return false;
  }
  
  /**
   * WARN
   * We are assuming that our baskets and their baskets
   * are in the same order!! I don't know if this is a safe
   * assumption.
   */
  bool content_match{true};
  for (int i_basket{0}; i_basket < num_our_baskets; i_basket++) {
    int our_len{0}, their_len{0};
    char* our_buff   = this->getContent(i_basket, our_len);
    char* their_buff = other.getContent(i_basket, their_len);

    content_match = (our_len == their_len) and 
      (memcmp(our_buff, their_buff, our_len) == 0);

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
        ("Received a NULL basket for branch " + name()
        + " and basket index " + std::to_string(i_basket)).Data());
  }

  int compressed_len = basket->GetNbytes() - basket->GetKeylen();
  char *compressed_content = new char[compressed_len];
  if (file_->ReadBuffer(compressed_content,
        basket->GetSeekKey()+basket->GetKeylen(),
        compressed_len)) {
    // return status of 1 is a failure
    delete [] compressed_content;
    EXCEPTION_RAISE("ReadFail",
        ("Failure to read basked " + std::to_string(i_basket)
        + " from branch " + name() + " in file " + file_->GetName()).Data());
  }

  return compressed_content;
  /*
  char *content;
  if (basket->GetObjlen() > compressed_len) {
    //need to de-compress
    len = basket->GetObjlen();
    content = new char[len];

    // IDK what these mean, got to look it up
    int nin, nout{0}, noutot{0}, ncontent;
    while (true) {
      // get information on how content was compressed
      R__unzip_header(&nin, (unsigned char *)content, &ncontent);
      // Actually uncompress some content
      R__unzip(&nin, (unsigned char *)content, &ncontent, (unsigned char *)compressed_content, &nout);
      if (!nout) {
        //all done
        break;
      }
      noutot += nout;
      if (noutot >= len) {
        // all done
        break;
      }
      compressed_content += nin;
      content += nout;
    }

  } else {
    //don't need to de-compress
    len = compressed_len;
    content = new char[len];
    memcpy(content, compressed_content, len);
  }

  delete [] compressed_content;

  return content;
  */
}

}
}
