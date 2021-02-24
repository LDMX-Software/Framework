#ifndef FRAMEWORK_TREEDIFF_COMPARE_H
#define FRAMEWORK_TREEDIFF_COMPARE_H

#include <vector>

#include "TString.h"

namespace framework {
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
 * @param[in] f1 name of first file
 * @param[in] f2 name of second file
 * @param[in] trees name of tress to compare between files
 * @param[in] to_ignore list of substrings of branches to ignore
 * @return exit status for program
 */
int compare(const TString& f1, const TString& f2,
            const std::vector<TString>& trees,
            const std::vector<TString>& to_ignore);

}
}

#endif
