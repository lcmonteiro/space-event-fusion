cmake_minimum_required (VERSION 3.13)

# project
project(space-event-fusion)

set(CMAKE_CXX_STANDARD 17)

# space target
add_library(event-fusion
	./src/fusion.cpp
	./src/resources/timer.cpp
	#./src/resources/stream/remote.cpp
	#./src/resources/message/local.cpp
)

add_executable(fusion-test
	./test.cpp
)

# includes
target_include_directories(event-fusion
PUBLIC
	./include
)

# test
enable_testing()
add_subdirectory(test)