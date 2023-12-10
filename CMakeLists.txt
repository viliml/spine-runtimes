cmake_minimum_required(VERSION 3.5)
project(spine)

set(CMAKE_CXX_STANDARD, 17)

if(MSVC)
	message("MSCV detected")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused-value -Wno-c++11-long-long -Wno-variadic-macros -Werror -Wextra -pedantic -Wnonportable-include-path -Wshadow -std=c11")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wunused-value -Wno-c++11-long-long -Wno-variadic-macros -Werror -Wextra -Wnon-virtual-dtor -pedantic -Wnonportable-include-path -Wshadow -std=c++17 -fno-exceptions -fno-rtti")
endif()

set(CMAKE_INSTALL_PREFIX "./")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(spine-cpp)
add_subdirectory(spine-sfml/cpp)
