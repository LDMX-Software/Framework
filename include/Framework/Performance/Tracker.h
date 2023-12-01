#ifndef FRAMEWORK_PERFORMANCE_TRACKER
#define FRAMEWORK_PERFORMANCE_TRACKER

#include <map>

#include <TTree.h>
#include <TDirectory.h>

#include "Framework/Performance/Timer.h"
#include "Framework/Performance/Callback.h"

namespace framework::performance {

/**
 * Class to interface between framework::Process and various Measurements
 * that can eventually be written into the output histogram file.
 *
 * @see Measurement for what measurements are taken and how they are taken.
 */
class Tracker {
 public:
  /**
   * Create the tracker with a specific destination for writing information
   *
   * @param[in] storage_directory directory in-which to write data when closing
   * @param[in] names sequence of processor names we will be tracking
   */
  Tracker(TDirectory *storage_directory, const std::vector<std::string>& names);
  /**
   * Close up tracking and write all of the data collected to the storage directory
   */
  ~Tracker();
  /* begin list of callbacks for various points in Process::run */
  /// literally first line of Process::run
  void absolute_start();
  /// literally last line of Process::run (if run compeletes without error)
  void absolute_end();
  /// start the timer for a specific callback and specific processor
  void start(Callback cb, std::size_t i_proc);
  /// end the timer for a specific callback and specific processor
  void end(Callback cb, std::size_t i_proc);
  /// inform us that we finished an event (and whether it was completed or not)
  void end_event(bool completed);

 private:
  /**
   * Special name representing "all" processors in the sequence.
   * For measurements related to beginning, this is before all
   * processors and for measurements related to ending, this is
   * after all processors.
   */
  static const std::string ALL;
  /// handle to the destination for the data
  TDirectory *storage_directory_;
  /// event-by-event perf info
  TTree *event_data_;
  /// buffer for time measurements within each event
  std::vector<double> event_times_;
  /// buffer for bool flag on if event completed
  bool event_completed_;

  Timer absolute_{false};
  std::vector<std::vector<Timer>> processor_timers_;
  std::vector<std::string> names_;
};
}

#endif
