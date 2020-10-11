
#include "Framework/EventFileFactory.h"

namespace framework {
EventFileFactory *EventFileFactory::instance_ = nullptr;

EventFileFactory *EventFileFactory::getInstance() {
  if (!instance_)
    instance_ = new EventFileFactory;
  return instance_;
}

EventFileFactory::EventFileFactory() {}

void EventFileFactory::registerEventFile(const std::string &name,
                                         createFunc create) {
  parser_map[name] = create;
}

EventFile *EventFileFactory::createEventFile(const std::string &name,
                                             ldmx::Parameters &parameters) {
  auto it{parser_map.find(name)};
  if (it != parser_map.end())
    return it->second(parameters);
  return nullptr;
}
} // namespace framework
