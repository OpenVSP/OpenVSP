CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

INCLUDE_DIRECTORIES(
    ${LIBXML2_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${UTIL_API_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    )

SET(XMLVSP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to XML wrapper header.")

ADD_LIBRARY(xmlvsp
XmlUtil.h
XmlUtil.cpp

)

target_link_libraries( xmlvsp Eigen3::Eigen )
