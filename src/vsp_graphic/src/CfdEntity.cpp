#include "CfdEntity.h"
#include "Lighting.h"
#include "OpenGLHeaders.h"

namespace VSPGraphic
{
CfdEntity::CfdEntity()
{
}
CfdEntity::CfdEntity( Lighting * lights )
{
}
CfdEntity::~CfdEntity()
{
}

void CfdEntity::_draw_Wire_Frame_Solid()
{
    glEnable( GL_CULL_FACE );
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1.f, 1.f );

    _draw_Mesh( 1.f, 1.f, 1.f, 1.f );

    glDisable( GL_POLYGON_OFFSET_FILL );
    glDisable( GL_CULL_FACE );

    _draw_Wire();
}
}