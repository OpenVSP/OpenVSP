# OpenGL .obj Viewer #

A simple OpenGL viewer for .obj files.
I wrote it for OpenGL 2.1 (with VAO and shaders) so that it runs reliably on
lots of machines.
I might extend or fork this to do more interesting things.

## Limitations ##

.obj files must have:
* points
* texture coordinates
* normals

and must be triangulated

MTL files are not supported.


## Instructions ##

If you run the viewer with no parameters it uses a default mesh and texture.

  ./viewer

You can specify:

* .obj file to load

    -o mymesh.obj

* texture to load

    -tex mytexture.png

* shaders to load

    -vs myshader.vert -fs myshader.frag

* scale to apply uniformly (if the mesh is too big to fit in view)

    -sca 0.5

* translation XYZ to apply (to centre mesh in view)

    -tra 0.0 -1.0 0.0

## Keys ##

* F11 - screenshot
* N - toggle visualisation of normals
* P - fill/wireframe/points

## To Do ##

* Support meshes with different components
* Display my .apg format meshes as well
* A version for WebGL
