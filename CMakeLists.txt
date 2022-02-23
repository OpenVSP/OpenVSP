CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT( VSP_TOP )

# Trivial top-level CMake file to force all the project files to appear in
# some IDEs like Eclipse.

# Required here to make tests show up as top-level target if project starts here.
INCLUDE( CTest )

ADD_SUBDIRECTORY( src )
