#include "Routing.h"
#include "TextMgr.h"
#include "OpenGLHeaders.h"
#include "glm/ext.hpp"
#include "Display.h"
#include "VertexBuffer.h"

namespace VSPGraphic
{
static glm::vec3 _mouseLocInWorld = glm::vec3(0xFFFFFFFF);
static TextMgr _textMgr = TextMgr();

Routing::Routing() : Marker()
{
    reset();

    _mouseLocInWorld = glm::vec3(0xFFFFFFFF);
    _liveIndex = -1;
}
Routing::~Routing()
{
}

void Routing::updateMouseLocation( const glm::vec3 &mouseLocInWorld )
{
    _mouseLocInWorld = mouseLocInWorld;
}

void Routing::reset()
{
    _liveIndex = -1;
}

void Routing::setLiveIndex( int li )
{
    _liveIndex = li;
}

void Routing::_draw()
{
    glColor3f(1.0, 0.5, 0.0);
    glLineWidth(3.0f * Display::getScreenSizeDiffRatio() );
    glPointSize(12);

    glBegin(GL_LINE_STRIP);
    {
        int npt = _v.size();
        unsigned int i = 0;
        for( ; i < _liveIndex && i < npt; i++)
        {
            glm::vec3 v = _v[i];
            glVertex3f(v[0], v[1], v[2]);
        }

        if(_mouseLocInWorld != glm::vec3(0xFFFFFFFF))
        {
            glVertex3f(_mouseLocInWorld.x, _mouseLocInWorld.y, _mouseLocInWorld.z);
        }
        i++;

        for( ; i < npt; i++)
        {
            glm::vec3 v = _v[i];
            glVertex3f(v[0], v[1], v[2]);
        }
    }
    glEnd();
    }

}
