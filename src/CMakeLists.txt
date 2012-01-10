CMAKE_MINIMUM_REQUIRED(VERSION 1.4)

PROJECT (VSP)

SET(CMAKE_PREFIX_PATH
      "C:/Program Files/FLTK"
      ${VSP_SOURCE_DIR}/libxml2-2.7.8.win32
      ${VSP_SOURCE_DIR}/jpeg-8c
      ${VSP_SOURCE_DIR}/jpeg-8c/Release
)

ADD_SUBDIRECTORY( util_code )
ADD_SUBDIRECTORY( triangle )
ADD_SUBDIRECTORY( load_jpg )
ADD_SUBDIRECTORY( xmlvsp )
ADD_SUBDIRECTORY( screens )
ADD_SUBDIRECTORY( sixseries )
ADD_SUBDIRECTORY( vsp )
