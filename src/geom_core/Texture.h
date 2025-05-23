#if !defined(VSP_TEXTURE__INCLUDED_)
#define VSP_TEXTURE__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

/*!
* Information needed to build a single texture.
*/
class Texture : public ParmContainer
{
public:
    /*!
    * Construct a texture.
    */
    Texture( std::string fileName );
    /*!
    * Destructor.
    */
    virtual ~Texture();

public:
    /*!
    * Override ParmContainer.
    */
    xmlNodePtr EncodeXml( xmlNodePtr &node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

public:
    /*!
    * File path + file name.
    */
    std::string m_FileName;

    /*!
    * Translate on U coordinate.
    */
    Parm m_U;
    /*!
    * Translate on W coordinate.
    */
    Parm m_W;

    /*!
    * Scale on U coordinate.
    */
    Parm m_UScale;
    /*!
    * Scale on W coordinate.
    */
    Parm m_WScale;

    /*!
    * Texture Alpha.
    */
    Parm m_Transparency;

    /*!
    * Flag to flip U coordinate.
    */
    BoolParm m_FlipU;
    /*!
    * Flag to flip W coordinate. 
    */
    BoolParm m_FlipW;
};
#endif