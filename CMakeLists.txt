cmake_minimum_required (VERSION 2.6)
project (gulan)

file(GLOB SOURCES "src/*.c")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pg -O2 -Wall -Werror -Wextra -pthread")
#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -O2 -Wall -Werror -Wextra -pthread -DNDEBUG")
add_executable(gulan ${SOURCES})
add_subdirectory(tests)
