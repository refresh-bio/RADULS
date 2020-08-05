cmake_minimum_required(VERSION 3.10)

project(RADULS)

IF(CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "build type is release")
  string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE "-O3" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE "-O0" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O1 -march=native")
endif()

### RADULS
add_library(raduls STATIC Raduls/sorting_network.cpp)
target_include_directories(raduls INTERFACE Raduls)
