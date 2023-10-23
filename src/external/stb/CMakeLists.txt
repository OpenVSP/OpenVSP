CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to STB library")

ADD_LIBRARY(stb_image
dummy.cpp
stb_image.h
stb_image_write.h
)

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    TARGET_COMPILE_OPTIONS( stb_image PUBLIC -Wno-deprecated-declarations)
endif()
