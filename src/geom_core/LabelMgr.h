#if !defined(VSP_LABELS__INCLUDED_)
#define VSP_LABELS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "Vec2d.h"
#include "Vec3d.h"

#include <string>
#include <vector>

#define STAGE_ZERO "Stage_0"
#define STAGE_ONE "Stage_1"
#define STAGE_TWO "Stage_2"
#define STAGE_COMPLETE "Stage_Complete"

/*!
* Label Info.
*/
class Label : public ParmContainer
{
public:
    /*!
    * Construct a label.
    */
    Label();
    /*!
    * Destructor.
    */
    virtual ~Label();

public:
    /*!
    * Override from ParmContainer.  Update vehicle on change.
    */
    virtual void ParmChanged( Parm* parm_ptr, int type );

public:
    /*!
    * Reset label to default parms.
    */
    virtual void Reset();

    /*!
    * Map xyz to uw.
    * geomId - The Id of geometry where label is attached.
    * xyz - xyz position on geometry.
    */
    static vec2d MapToUW( std::string geomId, vec3d xyz );

    /*!
    * Map uw to xyz.
    * geomId - The Id of geometry where label is attached.
    * uw - uw value.
    */
    static vec3d MapToXYZ( std::string geomId, vec2d uw );

public:
    /*!
    * Current render stage.
    */
    std::string m_Stage;

    /*!
    * Origin's Geometry Id.
    */
    std::string m_OriginGeomID;

    /*!
    * Base Location of Label along U.
    */
    Parm m_OriginU;
    /*!
    * Base Location of Label along W.
    */
    Parm m_OriginW;

    /*!
    * Text color red.
    */
    Parm m_Red;
    /*!
    * Text color green.
    */
    Parm m_Green;
    /*!
    * Text color blue.
    */
    Parm m_Blue;

    /*!
    * Text size.
    */
    Parm m_Size;

    /*!
    * Visibility.
    */
    bool m_Visible;
};

/*!
* Ruler info.  Derived from label.
*/
class Ruler : public Label
{
public:
    /*!
    * Construct a ruler.
    */
    Ruler();
    /*!
    * Destructor.
    */
    virtual ~Ruler();

public:
    /*!
    * Reset properties to default.
    */
    virtual void Reset();

public:

    /*!
    * Id of geometry where end of the ruler is attached.
    */
    std::string m_RulerEndGeomID;

    /*!
    * End Location of Ruler along U.
    */
    Parm m_RulerEndU;
    /*!
    * End Location of Ruler along W.
    */
    Parm m_RulerEndW;

    /*!
    * Offset on x axis.
    */
    Parm m_XOffset;
    /*!
    * Offset on y axis.
    */
    Parm m_YOffset;
    /*!
    * Offset on z axis.
    */
    Parm m_ZOffset;

    /*!
    * Precision of distance.
    */
    Parm m_Precision;
};

/*!
* Keep track a list of Labels.
*/
class LabelMgr
{
public:
    /*!
    * Singleton entry.
    */
    static LabelMgr * getInstance()
    {
        static LabelMgr labelMgr;
        return &labelMgr;
    }

protected:
    /*!
    * Construct a Labels object.
    */
    LabelMgr();
    /*!
    * Destructor.
    */
    virtual ~LabelMgr();

public:
    /*!
    * Create and Push a ruler to list.
    * name - Name of the ruler.
    * return Label's ID.
    */
    virtual std::string CreateAndAddRuler(const string& name);
    /*!
    * Get Label object with its Id.
    */
    Label * Get(std::string ID);
    /*!
    * Get all Labels in a vector.
    */
    std::vector<Label*> GetVec();
    /*!
    * Remove Label with Id.
    */
    void Remove(std::string ID);
    /*!
    * Remove all Labels.
    */
    void Clear();

protected:
    std::vector<Label*> m_Labels;
};
#endif