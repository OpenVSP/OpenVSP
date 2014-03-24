#if !defined(VSP_LABELS__INCLUDED_)
#define VSP_LABELS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

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

public:
    /*!
    * Current render stage.
    */
    std::string m_Stage;

    /*!
    * Starting Geometry's Id.
    */
    std::string m_StartGeomID;
    /*!
    * Starting Geometry's buffer index.
    */
    Parm m_StartIndex;

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
    * Ending Geometry's Id.
    */
    std::string m_EndGeomID;
    /*!
    * Ending Geometry's buffer index.
    */
    Parm m_EndIndex;

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
class Labels
{
public:
    /*!
    * Construct a Labels object.
    */
    Labels();
    /*!
    * Destructor.
    */
    virtual ~Labels();

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