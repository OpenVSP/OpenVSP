CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to STB library")

ADD_LIBRARY(stb_image
dummy.cpp
stb_image.h
stb_image_write.h
)
