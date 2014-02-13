#ifdef WIN32
#   include <Windows.h>
#endif

#define GLEW_STATIC
#include <GL/glew.h>

#ifdef __APPLE__
#   include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
