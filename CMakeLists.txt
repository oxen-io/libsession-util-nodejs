cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(VERBOSE ON)

# Detect the number of processors
include(ProcessorCount)
ProcessorCount(N)

# Set a default value in case the detection fails
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
else()
  set(CMAKE_BUILD_PARALLEL_LEVEL 4)  # Fallback to 16 if detection fails
endif()
# set(CMAKE_BUILD_PARALLEL_LEVEL 32)
message(STATUS "Number of processors detected: ${N}")


add_definitions(-DNAPI_VERSION=8)
set(CMAKE_CONFIGURATION_TYPES Release)

project(libsession_util_nodejs LANGUAGES CXX)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_BUILD_TYPE Release)
SET(WITH_TESTS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# when building from a release of libsession on desktop, it complains that ios-cmake is not up to date
# as it is not part of the archive. We actually don't care about it on session-desktop
set(SUBMODULE_CHECK OFF)

file(GLOB SOURCE_FILES src/*.cpp src/groups/*.cpp src/multi_encrypt/*.cpp)

add_subdirectory(libsession-util)


if(MSVC)
  # Windows is horrible
  add_compile_definitions(NOMINMAX)
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} "src/" "node_modules/node-addon-api" "../../node_modules/node-addon-api")

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} ${LIBSESSION_STATIC_BUNDLE_LIBS})

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
