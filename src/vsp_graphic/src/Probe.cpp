#include "Probe.h"
#include "TextMgr.h"
#include "OpenGLHeaders.h"
#include "glm/ext.hpp"
#include "Display.h"

namespace VSPGraphic
{
static glm::vec3 _mouseLocInWorld = glm::vec3(0xFFFFFFFF);
static TextMgr _textMgr = TextMgr();

Probe::Probe() : Marker()
{
    reset();

    _mouseLocInWorld = glm::vec3(0xFFFFFFFF);
}
Probe::~Probe()
{
}

void Probe::updateMouseLocation(const glm::vec3 &mouseLocInWorld)
{
    _mouseLocInWorld = mouseLocInWorld;
}

void Probe::placeProbe( const glm::vec3 &v1, const glm::vec3 &norm, const std::string & lbl )
{
    reset();

    _v1 = v1;

    _norm = norm;

    _label = lbl;
}

void Probe::placeProbe( const glm::vec3 &v1, const glm::vec3 &norm, float len, const std::string & lbl)
{
    reset();

    _v1 = v1;

    _norm = norm;

    _len = len;

    _label = lbl;
}

void Probe::reset()
{
    _label = "Probe";

    _v1 = glm::vec3(0xFFFFFFFF);

    _norm = glm::vec3(0xFFFFFFFF);

    _len = 0xFFFFFFFF;
}

void Probe::_draw()
{
    glm::vec3 probeEnd;

    glm::vec3 textLocation = glm::vec3(0);

    glColor3f(1.0, 0.5, 0.0);
    glLineWidth(3.0f * Display::getScreenSizeDiffRatio() );
    glPointSize(12);

    // Last Stage.
    if(_v1 != glm::vec3(0xFFFFFFFF) && _norm != glm::vec3(0xFFFFFFFF) && _len != 0xFFFFFFFF )
    {
        probeEnd = _v1 + _norm * _len;

        glBegin(GL_POINTS);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glEnd();

        glBegin(GL_LINE_STRIP);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        glVertex3f(probeEnd.x, probeEnd.y, probeEnd.z);
        glEnd();

        textLocation = probeEnd;

    }
    // First stage of ruler.
    else if(_v1 != glm::vec3(0xFFFFFFFF) && _norm != glm::vec3(0xFFFFFFFF) )
    {
        glm::vec3 v2 = _mouseLocInWorld;

        glBegin(GL_LINES);
        glVertex3f(_v1[0], _v1[1], _v1[2]);
        if(_mouseLocInWorld != glm::vec3(0xFFFFFFFF))
        {
            float len = glm::length( _mouseLocInWorld - _v1 );
            v2 = _v1 + _norm * len;
            glVertex3f( v2.x, v2.y, v2.z);
        }
        else
        {
            glVertex3f(_v1[0], _v1[1], _v1[2]);
        }
        glEnd();

        textLocation = v2;
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
