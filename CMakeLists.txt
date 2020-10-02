cmake_minimum_required (VERSION 3.13)

# project
project(space-event-fusion)

# space target
add_library(event-fusion
	./src/fusion.cpp
	./src/resources/base.cpp
	./src/resources/timer.cpp
)

# includes
target_include_directories(event-fusion
PUBLIC
	./include
)

# test
enable_testing()
add_subdirectory(test)