#ifndef _VSP_GRAPHIC_SHADER_REPO_H
#define _VSP_GRAPHIC_SHADER_REPO_H

#include <vector>

namespace VSPGraphic
{
enum VSPShaderEnum
{
    SHADER_TEXTURE01,
    SHADER_TEXTURE01_NO_LIGHTING,
    SHADER_MULTITEXTURE02_BLEND,
    SHADER_MULTITEXTURE02_LAYERED,
    SHADER_MULTITEXTURE03_BLEND,
    SHADER_MULTITEXTURE03_LAYERED,
    SHADER_MULTITEXTURE04_BLEND,
    SHADER_MULTITEXTURE04_LAYERED,
    SHADER_MULTITEXTURE05_BLEND,
    SHADER_MULTITEXTURE05_LAYERED,
    SHADER_MULTITEXTURE06_BLEND,
    SHADER_MULTITEXTURE06_LAYERED,
    SHADER_MULTITEXTURE07_BLEND,
    SHADER_MULTITEXTURE07_LAYERED,
    SHADER_MULTITEXTURE08_BLEND,
    SHADER_MULTITEXTURE08_LAYERED,
    NUM_OF_SHADERS,
};

class Shader;

/*!
* Shader Program Repository.
*/
class ShaderRepo
{
public:
    /*!
    * Constructor.
    */
    ShaderRepo();
    /*!
    * Destructor.
    */
    virtual ~ShaderRepo();

public:
    /*!
    * Get Shader Program.
    *
    * shaderType - enum for shader type.
    */
    Shader * getShader( VSPShaderEnum shaderType );

private:
    struct ShaderInfo;
    void _build( ShaderInfo * sInfo );

private:
    struct ShaderInfo
    {
        Shader * shaderPtr;
        const char * vShader;
        const char * fShader;
    };
    std::vector<ShaderInfo> _shaderRepo;
};
}

#endif
