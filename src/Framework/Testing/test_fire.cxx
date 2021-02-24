
//----------------//
//   C++ StdLib   //
//----------------//
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <set>

#include "TString.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Framework/Exception/Exception.h"
#include "Framework/TreeDiff/Compare.h"
#include "Framework/ConfigurePython.h"
#include "Framework/Process.h"

/**
 * Print the usage of this executable to std::cout
 */
static inline void usage() {
  std::cout 
    << "Use: test-fire [-h,--help] [-i,--ignore class_name]\n"
    << "               {config.py} {output.root}"
    << "-h,--help  Print this help message and exit.\n"
    << "-i,--ignore\n"
    << "           Name of class to ignore. Can specify more than once.\n"
    << "config.py  Configuration script to run.\n"
    << "           Should take the name of the output file as its only argument."
    << "output.root\n"
    << "           Ouput file that config.py should generate."
    << std::endl;
}

/**
 * Helper function for configuration error where
 * an input flag isn't provided its required argument.
 */
static inline void needsArgAfter(const TString& arg) {
  usage();
  std::cout 
    << "** Flag " << arg 
    << " requires an argument after it. **"
    << std::endl;
}

/**
 * The trees in an output file that we should check.
 * These need to match the names of the trees in an
 * output file _exactly_.
 */
static const std::vector<TString> trees_to_check = {
  "LDMX_Events",
  "LDMX_Run"
};

/**
 * test-fire
 *
 * Executable to check that a certain configuration script
 * still produces the same output event file.
 */
int main(int argc, char* argv[]) {
  std::vector<TString> to_ignore;
  std::vector<TString> positional_args;
  for (int arg_i{1}; arg_i < argc; arg_i++) {
    TString arg{argv[arg_i]};
    if (arg.EqualTo("-h") or arg.EqualTo("--help")) {
      usage();
      return 0;
    } else if (arg.EqualTo("-i") or arg.EqualTo("--ignore")) {
      if (arg_i + 1 > argc or argv[arg_i + 1][0] == '-') {
        needsArgAfter(arg);
        return framework::treediff::FAILED_TO_RUN;
      }
      to_ignore.emplace_back(argv[arg_i + 1]);
      // shift arg index to skip name after flag
      arg_i++;
    } else {
      positional_args.emplace_back(arg);
    }
  }

  if (positional_args.size() != 2) {
    usage();
    std::cerr << "** Need to specify two files : "
      << "a config script and the expected output file **" << std::endl;
  }

  TString config = positional_args.at(0);
  TString output_config_should_match = positional_args.at(1);
  TString output_config_generated = output_config_should_match + ".test";

  char *cstr_translation = new char[output_config_generated.Length()+1];
  strcpy(cstr_translation, output_config_generated.Data());
  char *config_args[1] = { cstr_translation };

  // CLI arguments have been parsed, let's run the process

  framework::ProcessHandle p;
  try {
    framework::ConfigurePython cfg(config.Data(),config_args,1);
    delete [] cstr_translation;
    p = cfg.makeProcess();
  } catch (framework::exception::Exception& e) {
    std::cerr << "Configuration Error [" << e.name() << "] : " << e.message()
              << std::endl;
    std::cerr << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << std::endl;
    std::cerr << "Stack trace: " << std::endl << e.stackTrace();
    return framework::treediff::FAILED_TO_RUN;
  }

  // If Ctrl-c is used, immediately exit the application.
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction");
    return framework::treediff::FAILED_TO_RUN;
  }

  try {
    p->run();
  } catch (framework::exception::Exception& e) {
    // Process::run opens up the logging using the parameters passed to it from
    // python
    //  if an Exception is thrown, we haven't gotten to the end of Process::run
    //  where logging is closed, so we can do one more error message and then
    //  close it.
    auto theLog_{framework::logging::makeLogger(
        "test-fire")};  // ldmx_log macro needs this variable to be named 'theLog_'
    ldmx_log(fatal) << "[" << e.name() << "] : " << e.message() << "\n"
                    << "  at " << e.module() << ":" << e.line() << " in "
                    << e.function() << "\nStack trace: " << std::endl
                    << e.stackTrace();
    framework::logging::close();
    return framework::treediff::FAILED_TO_RUN;  // return non-zero error-status
  }

  // get here when we successfully finish running.
  // This means we can move on to comparison

  return framework::treediff::compare(
      output_config_should_match, output_config_generated,
      trees_to_check, to_ignore);
}
