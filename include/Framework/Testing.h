
#include "catch.hpp"

#include "Framework/Process.h"
#include "Framework/ConfigurePython.h"

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
  ///Constructor, does nothing
  fires() { }

  /**
   * Check if the input string does what we want.
   *
   * Checks
   *  1. The input config path can be loaded through python
   *  2. The input config path can be run through the process
   */
  bool match(const std::string& config_path) const override {
    
    try {
      char** args;
      ConfigurePython cfg(config_path,args,0);
      ProcessHandle p = cfg.makeProcess();
      p->run();
    } catch(...) { return false; }

    return true;
  }

  /**
   * Describe what it means to pass this matcher
   */
  virtual std::string describe() const override {
    return "can be loaded through python and run through process.";
  }

}; //fires

} //test
} //ldmx
