#ifndef FRAMEWORK_EVENTFILEFACTORY_H_
#define FRAMEWORK_EVENTFILEFACTORY_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventFile.h"

namespace framework {

class EventFileFactory {

public:
  /// Get the instance to this factory
  static EventFileFactory *getInstance();

  /// Default constructor
  ~EventFileFactory();

  /**
   * Create an instance of the EventFile of the given type.
   *
   * @parser[in] type String type of the EventFile that needs to be created.
   */
  EventFile *createEventFile(const std::string &name,
                             ldmx::Parameters &parameters);

private:
  /// Static instance of this class
  static EventFileFactory *instance_;

  /// Default constructor
  EventFileFactory();

  /**
   * Register the EventFile with this factory.
   *
   * This is used to map the name of the EventFIle to the function used to
   * create it.
   *
   * @param[in] name Name of the EventFile being registered.
   * @param[in] create Function used to create this function.
   */
  void registerEventFile(const std::string &name, createFunc create);

  // Mapping between an EventFile type and its create function
  std::map<std::string, createFunc> parser_map;

}; // EventFileFactory
} // namespace framework
#endif // FRAMEWORK_EVENTFILEFACTORY_H_
