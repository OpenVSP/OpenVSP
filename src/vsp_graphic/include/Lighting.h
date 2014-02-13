#ifndef _VSP_LIGHTING_H
#define _VSP_LIGHTING_H

#include <vector>

namespace VSPGraphic
{
/*!
* LightSource class.
* This class represent a single light source.
*/
class LightSource
{
public:
    /*!
    * Constructor.
    */
    LightSource( unsigned int id );
    /*!
    * Destructor.
    */
    virtual ~LightSource();

public:
    /*!
    * Update position, ambient, diffuse and specular value.
    */
    void update();
    /*!
    * Enable light source.
    */
    void enable();
    /*!
    * Disable light source.
    */
    void disable();
    /*!
    * Get if this light source is enabled.
    */
    bool isEnabled();

public:
    /*!
    * Position of the light source.
    * Acceptable parameter are float array[4] or four float values.
    */
    void position( float * posArray );
    void position( float x, float y, float z, float w );

    /*!
    * Ambient light of the light source.
    * Acceptable parameter are float array[4] or four float values.
    */
    void ambient( float * ambArray );
    void ambient( float r, float g, float b, float a );

    /*!
    * Diffuse light of the light source.
    * Acceptable parameter are float array[4] or four float values.
    */
    void diffuse( float * diffArray );
    void diffuse( float r, float g, float b, float a );

    /*!
    * Specular light of the light source.
    * Acceptable parameter are float array[4] or four float values.
    */
    void specular( float * specArray );
    void specular( float r, float g, float b, float a );

private:
    unsigned int _id;
    bool _enabled;

    float _pos[4];
    float _amb[4];
    float _diff[4];
    float _spec[4];
};

/*!
* Lighting class.
* This class manages set of light sources.
*/
class Lighting
{
public:
    /*!
    * Constructor.
    */
    Lighting();
    /*!
    * Destructor.
    */
    virtual ~Lighting();

public:
    /*!
    * Update light sources if needed.
    */
    void update();
    /*!
    * Get lights' enable status.
    */
    std::vector<bool> getLightEnableStatus();

public:
    /*
    * Light sources.
    * Only support eight light sources at the moment.
    */
    LightSource * light0;
    LightSource * light1;
    LightSource * light2;
    LightSource * light3;
    LightSource * light4;
    LightSource * light5;
    LightSource * light6;
    LightSource * light7;

    /*
    * Get Light source at index.  Return null if index is invalid.
    */
    LightSource * getLightSource( unsigned int index );

private:
    std::vector<LightSource *> _lightSources;
    bool _enabled;
    bool _isChanged;
};
}
#endif