#ifndef _VSP_GRAPHIC_RENDER_OPERATION_BASE_H
#define _VSP_GRAPHIC_RENDER_OPERATION_BASE_H

#include "Common.h"
#include "SceneObject.h"

namespace VSPGraphic
{
class VertexBuffer;
class ColorBuffer;
class ElementBuffer;

/*!
* Renderable class.
* This class is the base class for all renderable objects.
*/
class Renderable : public SceneObject
{
public:
    /*!
    * Constructor.
    */
    Renderable();
    /*!
    * Destructor.
    */
    virtual ~Renderable();

public:
    /*!
    * Push a block of data from memory to the back of Vertex Buffer.
    * mem_ptr points to the beginning of memory block, mem_size defines the size of the data.
    */
    virtual void appendVBuffer( void * mem_ptr, unsigned int mem_size );
    /*!
    * Reset Vertex Buffer append location to start of the buffer.
    */
    virtual void emptyVBuffer();

public:
    /*!
    * Push a block of data from memory to the back of Element Buffer.
    * mem_ptr points to the beginning of memory block, mem_size defines the size of the data.
    */
    virtual void appendEBuffer( void * mem_ptr, unsigned int mem_size );
    /*!
    * Reset Element Buffer append location to start of the buffer.
    */
    virtual void emptyEBuffer();
    /*!
    * Enable or Disable Element Buffer usage.  Enable this will activate Element Indexing.
    * Disabled by default.
    */
    void enableEBuffer( bool enable );

public:
    /*!
    * Push a block of data from memory to the back of Color Buffer.
    * mem_ptr points to the beginning of memory block, mem_size defines the size of the data.
    */
    virtual void appendCBuffer( void * mem_ptr, unsigned int mem_size );
    /*!
    * Reset Color Buffer append location to start of the buffer.
    */
    virtual void emptyCBuffer();
    /*!
    * Enable or Disable Color Buffer usage.  Enable this will render color base on Color Buffer.
    * Disabled by default.
    */
    void enableCBuffer( bool enable );

public:
    /*!
    * Set mesh color.
    */
    void setMeshColor( float r, float g, float b, float a = 1.f );
    /*!
    * Set line color.
    */
    void setLineColor( float r, float g, float b, float a = 1.f );
    /*!
    * Set point color.
    */
    void setPointColor( float r, float g, float b, float a = 1.f );
    /*!
    * Set text color.
    */
    void setTextColor( float r, float g, float b, float a = 1.f );

public:
    /*!
    * Set line thickness.
    */
    void setLineWidth( float width );
    /*!
    * Set point size.
    */
    void setPointSize( float size );
    /*!
    * Set text size.
    */
    void setTextSize( float size );

public:
    /*!
    * Set Geometry Primitive Type.
    * VSP_TRIANGLES, VSP_QUADS, VSP_LINES, VSP_POINTS, VSP_LINE_LOOP, VSP_LINE_STRIP.
    */
    void setPrimType( Common::VSPenum type );
    /*!
    * Get primitive type.
    */
    Common::VSPenum getPrimType();

public:
    /*!
    * Set render style.
    * VSP_DRAW_MESH_SHADED, VSP_DRAW_MESH_TEXTURED,
    * VSP_DRAW_WIRE_FRAME, VSP_DRAW_WIRE_FRAME_SOLID.
    */
    void setRenderStyle( Common::VSPenum style );
    /*!
    * Get render style.
    */
    Common::VSPenum getRenderStyle();

public:
    /*!
    * Get Vertex Buffer Pointer.
    */
    VertexBuffer * getVBuffer();

    /*!
    * Get Element Buffer Pointer.
    */
    ElementBuffer * getEBuffer();
    /*!
    * Get Element Buffer Flag.  Return true if Element Buffer is enabled.
    * Else false.
    */
    bool getEBufferFlag();

protected:
    struct Color;

protected:
    bool _getCBufferFlag()
    {
        return _cBufferFlag;
    }

    // Color
    Color _getMeshColor()
    {
        return _meshColor;
    }
    Color _getLineColor()
    {
        return _lineColor;
    }
    Color _getPointColor()
    {
        return _pointColor;
    }
    Color _getTextColor()
    {
        return _textColor;
    }

    // Size
    float _getLineWidth()
    {
        return _lineWidth;
    }
    float _getPointSize()
    {
        return _pointSize;
    }
    float _getTextSize()
    {
        return _textSize;
    }

protected:
    struct Color
    {
        float red;
        float green;
        float blue;
        float alpha;
    };

protected:
    VertexBuffer * _vBuffer;
    ColorBuffer * _cBuffer;
    ElementBuffer * _eBuffer;

protected:
    Common::VSPenum _type, _style;

    Color _meshColor, _lineColor, _pointColor, _textColor;
    float _lineWidth, _pointSize, _textSize;

    bool _eBufferFlag, _cBufferFlag;
};
}
#endif