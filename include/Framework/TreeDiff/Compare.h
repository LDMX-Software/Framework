#ifndef FRAMEWORK_TREEDIFF_COMPARE_H
#define FRAMEWORK_TREEDIFF_COMPARE_H

#include <vector>

#include "TString.h"

namespace framework {

/**
 * @namespace treediff
 *
 * An extension to framework for comparing
 * trees in two separate files that are hypothesized
 * to be identical.
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
 *
 * I don't really know where else to put this information,
 * so I'm putting it here.
 *
 * # ROOT Serialization Primer
 * In order to understand what I'm doing here; first, you need to understand how
 * ROOT serializes a TTree.
 *
 * ### Splitting
 * Each TTree has TBranches created through the TTree::Branch method. If allowed
 * using a non-zero "split level", ROOT will "split" TBranches of complicated
 * objects into several parallel TBranches of less complicated objects. For
 * example, a TBranch of a struct MyObj { int my_int_; float my_float_; }; would
 * be split into two sub-branches: one for my_int_ and one for my_float_. The
 * splitting process is recursive. If a TBranch has a sub-branch that is a
 * complicated object itself, the sub-branch can also split into less
 * complicated sub-branches.
 *
 * Only the lowest-level branches (branches with no sub-branches) follow data
 * and serialize it into the output file. The higher level branches (branches
 * with sub-branches) are only useful for interfacing between our complicated,
 * hierarchical C++ objects and the simple, serialized ones and zeros in the
 * file. This is crucial. For our purposes here, we don't care about the
 * higher-level branches because we only want to look at the simple, serialized
 * data that is easy to compare. Since the splitlevel changes what the
 * lowest-level branches are, we will need to assume that the splitlevel input
 * is the same for branches of the same name.
 *
 * Inside of this namespace, when I say "branch", assume I'm talking about only
 * these lowest-level branches.
 *
 * ### Baskets
 * Branches whose data is actually being serialized into/outof the file often
 * contain large amounts of data that cannot be loaded into memory all at once.
 * In order to get around this difficulty, ROOT "chunks" branches into baskets
 * (TBaskets) that are the objects serialized into the file. The size of these
 * baskets is configurable and is called buffsize at the TTree::Branch level.
 * Since the number of the baskets and which data is in which basket changes
 * depending on the size of these baskets, we will need to assume that the
 * buffsize input is the same for branches of the same name.
 *
 * The TBasket is where the data from its corresponding TBranch is compressed
 * (or decompressed), so getting down the the TBasket level is where we want to
 * be. Note: The TBranch serializes the object before giving the data to the
 * TBasket, so the TBasket doesn't need to know the type of object that TBranch
 * is following.
 *
 * ### Summary
 * In summary, each TTree has several TBranches. Each TBranch may be split into
 * several child TBranches (recursively) depending on the splitlevel input. The
 * bottom TBranches have several TBaskets. Each TBasket has one or more entries
 * in the corresponding TBranch depending on the memory size of the TBranch
 * entries. How the entires in the TBranch are partitioned into TBaskets is
 * controlled by the buffsize input.
 *
 * ### Objects in General
 * Finally, I need to make a comment about how ROOT writes objects to files.
 * This applies to any object that ROOT writes and TBaskets are a special case.
 * ROOT writes objects in two stages. First, ROOT writes a "header" which
 * contains object details such as the name of the object, its class, the size
 * of the object, it's location in the file, and other information we won't use.
 * This "header" is also called a "key" in ROOT terminology; hence, why you see
 * TKeys floating around. The second stage, immediately after this header, is
 * the serialized (usually also compressed) data. At the end of the day, once we
 * have this "key", we can access the serialized data off the file directly.
 * TBasket is actually a specialization of TKey for interfacing with TBranches,
 * so you won't see TKey in the code above; however, you will see me calling
 * TKey methods from the derived class TBasket.
 */
namespace treediff {

/// return status when we failed to run
static const int FAILED_TO_RUN{127};

/// return status for a perfect match
static const int MATCH{0};

/// return status for a successful run but failed match
static const int MISMATCH{1};

/**
 * Isolate comparison function after parsing
 * the command line arguments. This allows us
 * to define different executables sharing the
 * same basic comparison process.
 *
 * @see FAILED_TO_RUN, MATCH, and MISMATCH for
 * the different return statuses.
 *
 * @see framework::treediff::BareTree for how
 * we 'import' the data and compare it across
 * files.
 *
 * This function catches all of our own Execeptions,
 * so it safe to simply return this fuction at the end
 * of your main (after parsing any command-line inputs).
 *
 * ## Known Limitations
 * The reasons for these limitations depend on how ROOT serializes TTrees.
 *
 * @see treediff for an explanation on where these limitations come from.
 *
 * 1. Two event trees need to have the same pass name to be compared well.
 * 2. Two branches need to have the same splitlevel and buffsize. 
 *
 * @param[in] f1 name of first file
 * @param[in] f2 name of second file
 * @param[in] trees name of tress to compare between files
 * @param[in] to_ignore list of substrings of branches to ignore
 * @return exit status for program
 */
int compare(const TString& f1, const TString& f2,
            const std::vector<TString>& trees,
            const std::vector<TString>& to_ignore);

}  // namespace treediff
}  // namespace framework

#endif
