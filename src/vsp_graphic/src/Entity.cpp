#include <assert.h>

#include "OpenGLHeaders.h"

#include "Entity.h"

#include "Lighting.h"
#include "VertexBuffer.h"
#include "ColorBuffer.h"
#include "ElementBuffer.h"

#include "Display.h"

namespace VSPGraphic
{
Entity::Entity() : Renderable()
{
    _lighting = NULL;
}
Entity::Entity( Lighting * lights ) : Renderable()
{
    _lighting = lights;
    _textureMgr.setLighting( _lighting );
}
Entity::~Entity()
{
}

void Entity::setLighting( Lighting * lighting )
{
    _lighting = lighting;
    _textureMgr.setLighting( _lighting );
}

void Entity::setMaterial( Material * material )
{
    assert( material );
    setMaterial( material->ambient, material->diffuse, material->specular, material->emission, material->shininess );
}

void Entity::setMaterial( float ambi[], float diff[], float spec[], float emis[], float shin )
{
    _material.ambient[0] = ambi[0]; _material.ambient[1] = ambi[1]; _material.ambient[2] = ambi[2];
    _material.ambient[3] = ambi[3];

    _material.diffuse[0] = diff[0]; _material.diffuse[1] = diff[1]; _material.diffuse[2] = diff[2];
    _material.diffuse[3] = diff[3];

    _material.specular[0] = spec[0]; _material.specular[1] = spec[1]; _material.specular[2] = spec[2];
    _material.specular[3] = spec[3];

    _material.emission[0] = emis[0]; _material.emission[1] = emis[1]; _material.emission[2] = emis[2];
    _material.emission[3] = emis[3];

    _material.shininess = shin;
}

bool Entity::isTransparent()
{
    return _material.diffuse[3] < 1.0;
}

void Entity::_predraw()
{
    switch( getRenderStyle() )
    {
    case Common::VSP_DRAW_MESH_SHADED:
        _draw_Mesh( 0.f, 0.f, 0.f, 0.f );
        break;

    case Common::VSP_DRAW_WIRE_FRAME_SOLID:
        _draw_Mesh( 0.f, 0.f, 0.f, 0.f );
        break;

    case Common::VSP_DRAW_WIRE_FRAME_TRANSPARENT_BACK:
        _draw_Mesh( 0.f, 0.f, 0.f, 0.f );
        break;

    case Common::VSP_DRAW_MESH_TEXTURED:
        _draw_Mesh( 0.f, 0.f, 0.f, 0.f );
        break;

    default:
        break;
    }
}

void Entity::_draw()
{
    switch( getRenderStyle() )
    {
    case Common::VSP_DRAW_MESH_SHADED:
        _draw_Mesh_Shaded();
        break;

    case Common::VSP_DRAW_WIRE_FRAME:
        _draw_Wire_Frame();
        break;

    case Common::VSP_DRAW_WIRE_FRAME_SOLID:
        _draw_Wire_Frame_Solid();
        break;

    case Common::VSP_DRAW_WIRE_FRAME_TRANSPARENT_BACK:
        _draw_Wire_Frame_Transparent_Back();
        break;

    case Common::VSP_DRAW_MESH_TEXTURED:
        _draw_Mesh_Textured();
        break;

    default:
        break;
    }
}

void Entity::_draw_Mesh_Shaded()
{
    if( _lighting )
    {
        glEnable( GL_LIGHTING );
        _material.bind();
    }

    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1.f, 1.f );

    _draw_Mesh();

    glDisable( GL_POLYGON_OFFSET_FILL );

    glDisable( GL_LIGHTING );
}

void Entity::_draw_Mesh_Textured()
{
    if( _lighting )
    {
        glEnable( GL_LIGHTING );
        _material.bind();
    }

    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1.f, 1.f );

    _textureMgr.bind();
    _draw_Mesh();
    _textureMgr.unbind();

    glDisable( GL_POLYGON_OFFSET_FILL );

    glDisable( GL_LIGHTING );
}

void Entity::_draw_Wire_Frame()
{
    glDepthFunc( GL_LESS );

    _draw_Wire();

    glDepthFunc( GL_LEQUAL );
}

void Entity::_draw_Wire_Frame_Solid()
{
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1.f, 1.f );

    _draw_Mesh( 1.f, 1.f, 1.f, 1.f );

    glDisable( GL_POLYGON_OFFSET_FILL );

    _draw_Wire();
}

void Entity::_draw_Wire_Frame_Transparent_Back()
{
    if( _lighting )
    {
        glEnable( GL_LIGHTING );
        _material.bind();
    }

    glEnable( GL_CULL_FACE );
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1.f, 1.f );

    _draw_Mesh();

    glDisable( GL_POLYGON_OFFSET_FILL );
    glDisable( GL_CULL_FACE );

    glDisable( GL_LIGHTING );

    _draw_Wire();
}

void Entity::_draw_Mesh()
{
    bool cBufferEnabled = _getCBufferFlag();

    if( cBufferEnabled )
    {
        _cBuffer->bind();
    }

    Color color = _getMeshColor();
    _draw_Mesh( color.red, color.green, color.blue, color.alpha );

    if( cBufferEnabled )
    {
        _cBuffer->unbind();
    }
}

void Entity::_draw_Mesh( float r, float g, float b, float a )
{
    bool eBufferEnabled = getEBufferFlag();

    glColor4f( r, g, b, a );

    if( eBufferEnabled )
    {
        _draw_Mesh_EBuffer();
    }
    else
    {
        _draw_Mesh_VBuffer();
    }
}

void Entity::_draw_Wire()
{
    bool cBufferEnabled = _getCBufferFlag();

    if( cBufferEnabled )
    {
        _cBuffer->bind();
    }

    float lineWidth = _getLineWidth();
    Color color = _getLineColor();
    _draw_Wire( color.red, color.green, color.blue, color.alpha, lineWidth );

    if( cBufferEnabled )
    {
        _cBuffer->unbind();
    }
}

void Entity::_draw_Wire( float r, float g, float b, float a, float lineWidth )
{
    bool eBufferEnabled = getEBufferFlag();

    glColor4f( r, g, b, a );

    // If linewidth is not set, set it to default width.
    if( lineWidth == 0.f )
    {
        glLineWidth( _getLineWidth() * Display::getScreenSizeDiffRatio() );
    }
    else
    {
        glLineWidth( lineWidth * Display::getScreenSizeDiffRatio() );
    }

    if( eBufferEnabled )
    {
        _draw_Wire_EBuffer();
    }
    else
    {
        _draw_Wire_VBuffer();
    }
}

void Entity::_draw_Mesh_VBuffer()
{
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    _draw_VBuffer();
}

void Entity::_draw_Wire_VBuffer()
{
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    _draw_VBuffer();
}

void Entity::_draw_Mesh_EBuffer()
{
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    _draw_EBuffer();
}

void Entity::_draw_Wire_EBuffer()
{
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    _draw_EBuffer();
}

void Entity::_draw_VBuffer()
{
    if( _getFacingCWFlag() )
    {
        glFrontFace(GL_CW);
    }
    else
    {
        glFrontFace(GL_CCW);
    }

    switch( getPrimType() )
    {
    case Common::VSP_TRIANGLES:
        _vBuffer->draw( GL_TRIANGLES );
        break;

    case Common::VSP_QUADS:
        _vBuffer->draw( GL_QUADS );
        break;

    default:
        break;
    }

    glFrontFace(GL_CCW);
}

void Entity::_draw_EBuffer()
{
    if( _getFacingCWFlag() )
    {
        glFrontFace(GL_CW);
    }
    else
    {
        glFrontFace(GL_CCW);
    }

    switch( getPrimType() )
    {
    case Common::VSP_TRIANGLES:
        _eBuffer->bind();
        _vBuffer->drawElem( GL_TRIANGLES, _eBuffer->getElemSize(), ( void* )0 );
        _eBuffer->unbind();
        break;

    case Common::VSP_QUADS:
        _eBuffer->bind();
        _vBuffer->drawElem( GL_QUADS, _eBuffer->getElemSize(), ( void* )0 );
        _eBuffer->unbind();
        break;

    default:
        break;
    }

    glFrontFace(GL_CCW);
}
}
