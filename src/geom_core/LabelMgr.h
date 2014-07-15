#if !defined(VSP_LABELS_MANAGER__INCLUDED_)
#define VSP_LABELS_MANAGER__INCLUDED_

#include "Label.h"

#include <vector>

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
    * return Label's ID.
    */
    virtual std::string CreateAndAddRuler();
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

public:
    /*!
    * Encode Label Info to XML.
    */
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    /*!
    * Decode Label Info from XML.
    */
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

protected:
    std::vector<Label*> m_Labels;
};
#endif