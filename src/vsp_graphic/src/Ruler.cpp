#include "Ruler.h"
#include "TextMgr.h"
#include "OpenGLHeaders.h"
#include "glm/ext.hpp"
#include "Display.h"

namespace VSPGraphic
{
static glm::vec3 _mouseLocInWorld = glm::vec3(0xFFFFFFFF);
static TextMgr _textMgr = TextMgr();

Ruler::Ruler() : Marker()
{
    reset();

    _mouseLocInWorld = glm::vec3(0xFFFFFFFF);
}
Ruler::~Ruler()
{
}

static glm::vec3 calculateOffset( const glm::vec3 &vStart, const glm::vec3 &vEnd, const glm::vec3 &vCursor)
{
    // Project cursor vector onto ruler base vector.
    float magSq = (vEnd.x - vStart.x) * (vEnd.x - vStart.x) + 
        (vEnd.y - vStart.y) * (vEnd.y - vStart.y) + 
        (vEnd.z - vStart.z) * (vEnd.z - vStart.z);

    if(magSq)
    {
        float vDot = glm::dot(vEnd - vStart, vCursor - vStart);
        glm::vec3 vProj = vDot / magSq * (vEnd - vStart);
        glm::vec3 vProjVertex = vStart + vProj;

        return vCursor - vProjVertex;
    }
    return glm::vec3(0);
}

void Ruler::updateMouseLocation( const glm::vec3 &mouseLocInWorld)
{
    _mouseLocInWorld = mouseLocInWorld;
}

void Ruler::placeRuler( const glm::vec3 &v1 )
{
    reset();

    _v1 = v1;
}

void Ruler::placeRuler( const glm::vec3 &v1, const glm::vec3 &v2, const std::string & lbl )
{
    reset();

    _v1 = v1;
    _v2 = v2;
    _label = lbl;
}

void Ruler::placeRuler( const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &offset, const std::string & lbl, int dir )
{
    reset();

    _v1 = v1;
    _v2 = v2;
    _label = lbl;
    _offset = calculateOffset(_v1, _v2, offset);
    _dir = dir;
}

void Ruler::reset()
{
    _label = "Ruler";

    _v1 = glm::vec3(0xFFFFFFFF);
    _v2 = glm::vec3(0xFFFFFFFF);

    _offset = glm::vec3(0xFFFFFFFF);
}

void Ruler::_draw()
{
    glm::vec3 rulerStart;
    glm::vec3 rulerEnd;

    glm::vec3 textLocation = glm::vec3(0);

    glColor3f(1.0, 0.5, 0.0);
    glLineWidth(3.0f * Display::getScreenSizeDiffRatio() );
    glPointSize(12);

    // Last Stage.
    if(_v1 != glm::vec3(0xFFFFFFFF) && _v2 != glm::vec3(0xFFFFFFFF) && _offset != glm::vec3(0xFFFFFFFF))
    {
        rulerStart = _v1 + _offset;
        rulerEnd = _v2 + _offset;

        if ( _dir != 3 ) // 3 means all directions.
        {
            glm::vec3 midpt = ( rulerStart + rulerEnd ) * 0.5f;

            for ( int i = 0; i < 3; i++ )
            {
                if ( i != _dir )
                {
                    rulerStart[i] = midpt[i];
                    rulerEnd[i] = midpt[i];
                }
                else
                {
                    rulerStart[i] = _v1[i];
                    rulerEnd[i] = _v2[i];
                }
            }
        }

        glBegin(GL_POINTS);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glVertex3f(_v2[0], _v2[1], _v2[2]);
        glEnd();

        glBegin(GL_LINE_STRIP);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glVertex3f(rulerStart.x, rulerStart.y, rulerStart.z);
        glVertex3f(rulerEnd.x, rulerEnd.y, rulerEnd.z);
        glVertex3f(_v2[0], _v2[1], _v2[2]);
        glEnd();

        textLocation = (rulerStart + rulerEnd) * 0.5f;
    }
    // Second stage of ruler.
    else if(_v1 != glm::vec3(0xFFFFFFFF) && _v2 != glm::vec3(0xFFFFFFFF))
    {
        if(_mouseLocInWorld != glm::vec3(0xFFFFFFFF))
        {
            glm::vec3 rulerOffset = calculateOffset(_v1, _v2, _mouseLocInWorld);

            rulerStart = _v1 + rulerOffset;
            rulerEnd = _v2 + rulerOffset;
        }
        else
        {
            rulerStart = _v1;
            rulerEnd = _v2;
        }

        glBegin(GL_LINE_STRIP);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glVertex3f(rulerStart.x, rulerStart.y, rulerStart.z);
        glVertex3f(rulerEnd.x, rulerEnd.y, rulerEnd.z);
        glVertex3f(_v2[0], _v2[1], _v2[2]);
        glEnd();

        textLocation = (rulerStart + rulerEnd) * 0.5f;
    }
    // First stage of ruler.
    else if(_v1 != glm::vec3(0xFFFFFFFF) && _v2 == glm::vec3(0xFFFFFFFF))
    {
        glBegin(GL_LINES);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        if(_mouseLocInWorld != glm::vec3(0xFFFFFFFF))
        {
            glVertex3f(_mouseLocInWorld.x, _mouseLocInWorld.y, _mouseLocInWorld.z);
        }
        else
        {
            glVertex3f(_v1[0], _v1[1], _v1[2]);
        }
        glEnd();

        textLocation = (_mouseLocInWorld + _v1) * 0.5f;
    }

    // Render text.
    if(_v1 != glm::vec3(0xFFFFFFFF))
    {
        GLfloat mvArray[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mvArray);
        glm::mat4 mvMat = glm::make_mat4(mvArray);

        GLfloat pArray[16];
        glGetFloatv(GL_PROJECTION_MATRIX, pArray);
        glm::mat4 pMat = glm::make_mat4(pArray);

        glm::vec4 textInNDC = glm::vec4(textLocation, 1);
        textInNDC = pMat * (mvMat * textInNDC);
        glm::mat4 textTMat = glm::translate( glm::mat4( static_cast<float>(1) ), glm::vec3(glm::vec2(textInNDC), 0));

        Color textColor = _getTextColor();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMultMatrixf(&textTMat[0][0]);

        glColor4f(textColor.red, textColor.green, textColor.blue, textColor.alpha);
        glDepthFunc(GL_ALWAYS);
        _textMgr.drawText(_textMgr.loadFont(), _getTextSize(), _label, 0.0f, 0.0f, 0.0f, 0.75f);
        glDepthFunc(GL_LEQUAL);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&pMat[0][0]);

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&mvMat[0][0]);
    }
}
}
