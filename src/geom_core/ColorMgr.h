#if !defined(VSP_COLOR_MANAGER__INCLUDE_)
#define VSP_COLOR_MANAGER__INCLUDE_

#include "Color.h"

/*!
* Keeps track color information of a geometry.
*/
class ColorMgr
{
public:
    /*!
    * Construct a ColorMgr.
    */
    ColorMgr();
    /*!
    * Destructor.
    */
    virtual ~ColorMgr();

public:
    /*!
    * Set Wire Color.  rgba are range from 0 to 255.
    */
    void SetWireColor( double r, double g, double b, double a );
    /*!
    * Get Wire Color.
    */
    const Color * GetWireColor() const;

public:
    /*!
    * Encode color info to xml.
    */
    xmlNodePtr EncodeXml( xmlNodePtr & node );
    /*!
    * Decode color info from xml.
    */
    xmlNodePtr DecodeXml( xmlNodePtr & node );

private:
    Color m_WireColor;
};
#endif