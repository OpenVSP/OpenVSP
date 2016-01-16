#ifndef _VSP_GRAPHIC_GLSL_COMPILER_H
#define _VSP_GRAPHIC_GLSL_COMPILER_H

namespace VSPGraphic
{
/*!
* Shader program.
*/
class Shader
{
public:
    /*!
    * Constructor.
    *
    * vShaderSrc - Vertex Shader Source Code.
    * fShaderSrc - Fragment Shader Source Code.
    */
    Shader( const char * vShaderSrc, const char * fShaderSrc );
    /*!
    * Destructor.
    */
    virtual ~Shader();

public:
    /*!
    * Bind this shader program to current render pipeline.
    */
    void bind();
    /*!
    * Bind fixed render pipeline.
    */
    static void unbind();

public:
    /*
    * Set Uniform Variable.
    *
    * name - Variable Name.
    * v - value int.
    */
    void setUniform1i( const char * name, int v );
    /*
    * Set Uniform Variable.
    *
    * name - Variable Name.
    * v - value float.
    */
    void setUniform1f( const char * name, float v );
    /*
    * Set Uniform Variable set.
    *
    * name - Variable Name.
    * v0 - value set one.
    * v1 - value set two.
    */
    void setUniform2f( const char * name, float v0, float v1 );
    /*
    * Set Uniform Variable array.
    *
    * name - Variable Name.
    * vec1 - int array.
    */
    void setUniform1iv( const char * name, int size, int * vec1 );
    /*
    * Set Uniform Variable array.
    *
    * name - Variable Name.
    * size - number of 1D vec in array.
    * vec1 - float array.
    */
    void setUniform1fv( const char * name, int size, float * vec1 );
    /*
    * Set Uniform Variable array.
    *
    * name - Variable Name.
    * size - number of 2D vec in array.
    * vec2 - float 2D vector array.
    */
    void setUniform2fv( const char * name, int size, float * vec2 );
    /*
    * Set Uniform Variable array.
    *
    * name - Variable Name.
    * size - number of 4D vec in array.
    * vec4 - float 4D vector array.
    */
    void setUniform4fv( const char * name, int size, float * vec4 );
    /*
    * Set Uniform Variable Matrix.
    *
    * name - Variable Name.
    * size - number of matrices in array.
    * mat4x4 - four by four matrix array.
    */
    void setUniformMatrix4fv( const char * name, int size, float * mat4x4 );

private:
    void _create( const char * vShaderSrc, const char * fShaderSrc );

private:
    bool _supported;

private:
    int _id;
    int _vShaderId;
    int _fShaderId;
};
}
#endif
