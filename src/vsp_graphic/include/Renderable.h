#ifndef _VSP_GRAPHIC_RENDER_OPERATION_BASE_H
#define _VSP_GRAPHIC_RENDER_OPERATION_BASE_H

#include "Common.h"

namespace VSPGraphic
{
class VertexBuffer;
class ColorBuffer;
class ElementBuffer;

/*!
* Renderable class.
* This class is the base class for all renderable objects.
*/
class Renderable
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

public:
    /*!
    * Set line thickness.
    */
    void setLineWidth( float width );
    /*!
    * Set point size.
    */
    void setPointSize( float size );

public:
    /*!
    * Set Geometry Type.
    */
    void setGeomType( Common::VSPenum type );
    /*!
    * Set render style.
    */
    void setRenderStyle( Common::VSPenum style );
    /*!
    * Set visibility.
    */
    void setVisibility( bool isVisible );
    /*!
    * Get visibility.
    */
    bool getVisibility()
    {
        return _visible;
    }

public:
    /*!
    * Perform preprocessing rendering.
    */
    void predraw();
    /*!
    * Render object.
    */
    void draw();
    /*!
    * Peform postprocessing.
    */
    void postdraw();

public:
    /*!
    * Enable / disable multi-pass rendering.
    */
    void enablePredraw( bool enable );
    /*!
    * Enable / disable post processing.
    */
    void enablePostdraw( bool enable );

protected:
    /*!
    * Protected.  Perform preprocessing.
    * All pre-render / render passes goes here.  Must implement.
    */
    virtual void _predraw() = 0;
    /*!
    * Protected.  Draw object.
    * All actual rendering code goes here.  Must implement.
    */
    virtual void _draw() = 0;
    /*!
    * Protected.  Perform postprocessing.
    * All postprocessing goes here.  Must implement.
    */
    virtual void _postdraw() = 0;

protected:
    struct Color;

protected:
    bool _getPreDrawFlag()
    {
        return _predrawFlag;
    }
    bool _getPostDrawFlag()
    {
        return _postdrawFlag;
    }

    bool _getEBufferFlag()
    {
        return _eBufferFlag;
    }
    bool _getCBufferFlag()
    {
        return _cBufferFlag;
    }

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

    float _getLineWidth()
    {
        return _lineWidth;
    }
    float _getPointSize()
    {
        return _pointSize;
    }

    Common::VSPenum _getGeomType()
    {
        return _type;
    }
    Common::VSPenum _getRenderStyle()
    {
        return _style;
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

    Color _meshColor, _lineColor, _pointColor;
    float _lineWidth, _pointSize;

    bool _visible;
    bool _eBufferFlag, _cBufferFlag;
    bool _predrawFlag, _postdrawFlag;
};
}
#endif