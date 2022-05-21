cmake_minimum_required (VERSION 3.16)
# -------------------------------------------------------------------
# project
# -------------------------------------------------------------------
project(space-event-fusion)

# options
OPTION(ENABLE_TESTING  "Enable code testing support"   OFF)

# properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# -------------------------------------------------------------------
# entry points
# -------------------------------------------------------------------
# space library
add_library(event-fusion
	./src/fusion.cpp
	./src/resources/timer.cpp
	./src/resources/stream/remote.cpp
	./src/resources/message/local.cpp
)
target_include_directories(event-fusion
PUBLIC
	./include
)
# space testing 
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

# -------------------------------------------------------------------
# summary
# -------------------------------------------------------------------
message(STATUS)
message(STATUS "${PROJECT_NAME} configuration:")
message(STATUS "  CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(STATUS "  ENABLE_TESTING   = ${ENABLE_TESTING}")
message(STATUS)

# -------------------------------------------------------------------
# end
# -------------------------------------------------------------------