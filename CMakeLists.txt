# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  fire
  VERSION 1.0.0
  DESCRIPTION "Framework for sImulation and Reconstruction of Events"
  LANGUAGES CXX)

setup_library(module Framework name Exception)
setup_library(module Framework name Configure interface)

# Search for the Python3 library
find_package(Python3 COMPONENTS Interpreter Development)

# Search for Boost
find_package(
  Boost REQUIRED
  COMPONENTS system
             log
             filesystem
             thread
             chrono
             atomic
             regex)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Execute the command to extract the SHA1 hash of the current git tag. The
# variable GIT_SHA1 will be set to contain the hash.
execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE GIT_SHA1)

# Remove the newline character
string(REGEX REPLACE "\n$" "" GIT_SHA1 "${GIT_SHA1}")

# Copies the file 'Version.h.in', substitutes the value of GIT_SHA1 and writes
# it out to Version.h.
configure_file(${PROJECT_SOURCE_DIR}/include/Framework/Version.h.in
               ${PROJECT_SOURCE_DIR}/include/Framework/Version.h)
install(FILES ${PROJECT_SOURCE_DIR}/include/Framework/Version.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Framework/)

# Setup the library
setup_library(module Framework 
  dependencies Python3::Python
               ROOT::Core
               ROOT::Hist
               ROOT::TreePlayer
               Boost::log
               Boost::atomic
               Boost::regex
               Framework::Exception
               Framework::Configure)

# Compiling the Framework library requires features introduced by the cpp 17
# standard.
set_target_properties(
  Framework
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS NO)

# Add the fire executable
add_executable(fire ${PROJECT_SOURCE_DIR}/src/Framework/fire.cxx)

# Link to the Framework library
target_link_libraries(fire PRIVATE Framework::Framework)

# Install the fire executable
install(TARGETS fire DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Setup the test
setup_test(dependencies Framework::Framework)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Framework)
