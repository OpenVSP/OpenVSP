#include <assert.h>
#include <string>

#include "Shader.h"

#include "OpenGLHeaders.h"

namespace VSPGraphic
{
static bool _checkGLSLSupport();

Shader::Shader( const char * vShaderSrc, const char * fShaderSrc )
{
    _supported = _checkGLSLSupport();

    if( _supported )
    {
        _id = -1;

        _vShaderId = -1;
        _fShaderId = -1;

        _create( vShaderSrc, fShaderSrc );
    }
}
Shader::~Shader()
{
    if( _supported )
    {
        if( _id != -1 )
        {
            glUseProgram( _id );

            glDetachShader( _id, _vShaderId );
            glDetachShader( _id, _fShaderId );

            glDeleteShader( _vShaderId );
            glDeleteShader( _fShaderId );

            glDeleteProgram( _id );
        }
    }
}

void Shader::bind()
{
    if( _supported && _id != -1 )
    {
        glUseProgram( _id );
    }
}

void Shader::unbind()
{
    if( _checkGLSLSupport() )
    {
        glUseProgram( 0 );
    }
}

void Shader::setUniform1i( const char * name, int v )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform1i( uniformLoc, v );
    }
}

void Shader::setUniform1f( const char * name, float v )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform1f( uniformLoc, v );
    }
}

void Shader::setUniform2f( const char * name, float v0, float v1 )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform2f( uniformLoc, v0, v1 );
    }
}

void Shader::setUniform1iv( const char * name, int size, int * vec1 )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform1iv( uniformLoc, size, vec1 );
    }
}

void Shader::setUniform1fv( const char * name, int size, float * vec1 )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform1fv( uniformLoc, size, vec1 );
    }
}

void Shader::setUniform2fv( const char * name, int size, float * vec2 )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform2fv( uniformLoc, size, vec2 );
    }
}

void Shader::setUniform4fv( const char * name, int size, float * vec4 )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniform4fv( uniformLoc, size, vec4 );
    }
}

void Shader::setUniformMatrix4fv( const char * name, int size, float * mat4x4 )
{
    if( _supported )
    {
        bind();
        GLint uniformLoc = glGetUniformLocation( _id, name );

        assert( uniformLoc != -1 );
        glUniformMatrix4fv( uniformLoc, size, GL_FALSE, mat4x4 );
    }
}

void Shader::_create( const char * vShaderSrc, const char * fShaderSrc )
{
    int compileOK, logLength, tempHolder;
    char * errorMsg;

    // Compile Vertex Shader.
    this->_vShaderId = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( this->_vShaderId, 1, &vShaderSrc, 0 );
    glCompileShader( this->_vShaderId );
    glGetShaderiv( this->_vShaderId, GL_COMPILE_STATUS, &compileOK );
    if( !compileOK )
    {
        glGetShaderiv( this->_vShaderId, GL_INFO_LOG_LENGTH, &logLength );
        errorMsg = ( char * )malloc( logLength );
        glGetShaderInfoLog( this->_vShaderId, logLength, &tempHolder, errorMsg );
        std::printf( "\nVertex Shader Compile Error: \n %s", errorMsg );
        free( errorMsg );
    }

    // Compile Fragment Shader.
    this->_fShaderId = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource(  this->_fShaderId, 1, &fShaderSrc, 0 );
    glCompileShader(  this->_fShaderId );
    glGetShaderiv(  this->_fShaderId, GL_COMPILE_STATUS, &compileOK );
    if( !compileOK )
    {
        glGetShaderiv(  this->_fShaderId, GL_INFO_LOG_LENGTH, &logLength );
        errorMsg = ( char * )malloc( logLength );
        glGetShaderInfoLog(  this->_fShaderId, logLength, &tempHolder, errorMsg );
        std::printf( "\nFragment Shader Compile Error: \n %s", errorMsg );
        free( errorMsg );
    }

    // Build Shader Program
    _id = glCreateProgram();

    glAttachShader( _id, this->_vShaderId );
    glAttachShader( _id,  this->_fShaderId );
    glLinkProgram( _id );
    glGetProgramiv( _id, GL_LINK_STATUS, &compileOK );
    if( !compileOK )
    {
        glGetProgramiv( _id, GL_INFO_LOG_LENGTH, &logLength );
        errorMsg = ( char * )malloc( logLength );
        glGetProgramInfoLog( _id, 100, &tempHolder, errorMsg );
        std::printf( "\nShader Linking Error: \n %s", errorMsg );
        free( errorMsg );
    }
}

static bool _checkGLSLSupport()
{
    static GLboolean supported = glewIsSupported( "GL_VERSION_2_0" );
    if( supported )
    {
        return true;
    }
    else
    {
        return false;
    }
}
}
