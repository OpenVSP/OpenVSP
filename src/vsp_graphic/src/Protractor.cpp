#include "Protractor.h"
#include "TextMgr.h"
#include "OpenGLHeaders.h"
#include "glm/ext.hpp"
#include "Display.h"

namespace VSPGraphic
{
static glm::vec3 _mouseLocInWorld = glm::vec3(fNAN);
static TextMgr _textMgr = TextMgr();

Protractor::Protractor() : Marker()
{
    reset();

    _mouseLocInWorld = glm::vec3(fNAN);
}
Protractor::~Protractor()
{
}

static glm::vec3 calculateOffset( const glm::vec3 &vStart, const glm::vec3 &vEnd, const glm::vec3 &vCursor)
{
    // Project cursor vector onto protractor base vector.
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

void Protractor::updateMouseLocation( const glm::vec3 &mouseLocInWorld)
{
    _mouseLocInWorld = mouseLocInWorld;
}

void Protractor::placeProtractor( const glm::vec3 &v1 )
{
    reset();

    _v1 = v1;
}


void Protractor::placeProtractor( const glm::vec3 &v1, const glm::vec3 &v2 )
{
    reset();

    _v1 = v1;
    _v2 = v2;
}

void Protractor::placeProtractor( const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, const glm::vec3 &label_dir, const std::string & lbl )
{
    reset();

    _v1 = v1;
    _v2 = v2;
    _v3 = v3;
    _label_dir = label_dir;
    _label = lbl;
}

void Protractor::placeProtractor( const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, const glm::vec3 &label_dir, const float &offset, const std::string & lbl, int dir )
{
    reset();

    _v1 = v1;
    _v2 = v2;
    _v3 = v3;
    _label_dir = label_dir;
    _label = lbl;
    _offset = offset;
    _dir = dir;
}

void Protractor::reset()
{
    _label = "Protractor";

    _v1 = glm::vec3(fNAN);
    _v2 = glm::vec3(fNAN);
    _v3 = glm::vec3(fNAN);

    _label_dir = glm::vec3(fNAN);
    _offset = (float) fNAN;
}

void Protractor::_draw()
{
    glm::vec3 protractorStart;
    glm::vec3 protractorEnd;

    glm::vec3 textLocation = glm::vec3(0);

    glColor3f(1.0, 0.5, 0.0);
    glLineWidth(3.0f * Display::getScreenSizeDiffRatio() );
    glPointSize(12);

    // Last Stage.
    if( !glm::any(glm::isnan(_v1)) && !glm::any(glm::isnan(_v2)) && !glm::any(glm::isnan(_v3)) && !isnan(_offset) )
    {
        protractorStart = _v1;
        protractorEnd = _v3;

        if ( _dir != 3 ) // 3 means all directions.
        {
            for ( int i = 0; i < 3; i++ )
            {
                if ( i == _dir )
                {
                    protractorStart[i] = _v2[i];
                    protractorEnd[i] = _v2[i];
                }
            }
        }

        glBegin( GL_POINTS );
        glVertex3f( _v1[0], _v1[1], _v1[2] );
        glVertex3f( _v2[0], _v2[1], _v2[2] );
        glVertex3f( _v3[0], _v3[1], _v3[2] );
        glEnd();

        glBegin( GL_LINE_STRIP );
        glVertex3f( _v1[0], _v1[1], _v1[2] );
        glVertex3f( protractorStart.x, protractorStart.y, protractorStart.z );
        glVertex3f( _v2[0], _v2[1], _v2[2] );
        glVertex3f( protractorEnd.x, protractorEnd.y, protractorEnd.z );
        glVertex3f( _v3[0], _v3[1], _v3[2] );
        glEnd();

        textLocation = _v2 + _label_dir * _offset;
    }
    // Third stage of protractor.
    else if( !glm::any(glm::isnan(_v1)) && !glm::any(glm::isnan(_v2)) && !glm::any(glm::isnan(_v3)) && isnan(_offset) )
    {
        glBegin(GL_LINE_STRIP);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glVertex3f(_v2[0], _v2[1], _v2[2]);
        glVertex3f(_v3[0], _v3[1], _v3[2]);
        glEnd();

        if( !glm::any(glm::isnan(_mouseLocInWorld)) )
        {
            textLocation = _v2 + _label_dir * glm::length( _mouseLocInWorld - _v2 );
        }
        else
        {
            textLocation = _v2 + _label_dir * 0.5f * ( glm::length( _v1 - _v2 ) + glm::length( _v3 - _v2 ) );
        }

    }
    // Second stage of protractor.
    else if( !glm::any(glm::isnan(_v1)) && !glm::any(glm::isnan(_v2)) && glm::any(glm::isnan(_v3)) )
    {
        glBegin(GL_LINE_STRIP);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glVertex3f(_v2[0], _v2[1], _v2[2]);
        if( !glm::any(glm::isnan(_mouseLocInWorld)) )
        {
            glVertex3f(_mouseLocInWorld.x, _mouseLocInWorld.y, _mouseLocInWorld.z);
        }
        else
        {
            glVertex3f(_v2[0], _v2[1], _v2[2]);
        }
        glEnd();

        textLocation = (_mouseLocInWorld + _v1) * 0.5f;
    }
    // First stage of protractor.
    else if( !glm::any(glm::isnan(_v1)) && glm::any(glm::isnan(_v2)) )
    {
        glBegin(GL_LINES);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        if( !glm::any(glm::isnan(_mouseLocInWorld)) )
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
    if( !glm::any(glm::isnan(_v1)) )
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
