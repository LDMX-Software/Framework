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