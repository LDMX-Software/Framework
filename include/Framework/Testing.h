
#include "catch.hpp"

#include <stdlib.h>

namespace ldmx {
namespace test {

/**
 * This is a Catch2 "matcher".
 * We can feed an instance of this matcher
 * to a CHECK_THAT or REQUIRE_THAT call for Catch2
 * to check something for us.
 *
 * In this matcher, we assume the input string
 * is a full path to a configuration script
 * that should be run through the Process.
 *
 * Example:
 *  CHECK_THAT( "/tmp/my_config_test.py" , ldmx::test::fires() );
 */
class fires : public Catch::MatcherBase<std::string> {
 public:
  /// Constructor, does nothing
  fires() {}

  /**
   * Check if the input string does what we want.
   *
   * Checks
   *  1. The input config path can be loaded through python
   *  2. The input config path can be run through the process
   */
  bool match(const std::string& config_path) const override {
    std::string cmd = "fire " + config_path;
    return system(cmd.c_str()) == 0;
  }

  /**
   * Describe what it means to pass this matcher
   */
  virtual std::string describe() const override {
    return "can be loaded through python and run through process.";
  }

};  // fires

}  // namespace test
}  // namespace ldmx
