cmake_minimum_required(VERSION 3.25.2)
cmake_policy(SET CMP0141 NEW)
project(stupid_table
LANGUAGES CXX
VERSION 0.1
)

add_executable(testing_program)
target_sources(testing_program PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/StupidTable.hpp)





if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)
