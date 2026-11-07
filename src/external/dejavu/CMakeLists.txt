CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(DEJAVU_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to the embedded DejaVu fonts")

ADD_LIBRARY( dejavu
DejaVuSans.cpp
DejaVuSansOblique.cpp
DejaVuSansBold.cpp
DejaVuSansBoldOblique.cpp
DejaVuFonts.h
)
