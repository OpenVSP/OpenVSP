//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPCLONEGEOM__INCLUDED_)
#define VSPCLONEGEOM__INCLUDED_

#include "Geom.h"

#include <set>


//==== Clone Geom ====//
class CloneGeom : public Geom
{
public:
    CloneGeom( Vehicle* vehicle_ptr );
    virtual ~CloneGeom();

    virtual void UpdateSets() override;

    virtual void ComputeCenter() override;
    virtual void AddDefaultSources( double base_len = 1.0 ) override;


    virtual xmlNodePtr EncodeXml( xmlNodePtr & node ) override;
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node ) override;

    // False if the Geom cannot be copied or would form a cycle of Clones.
    virtual bool SetOriginalID( const string &id );
    virtual string GetOriginalID() const
    {
        return m_OriginalID;
    }

    virtual Geom* GetOriginalGeom() const;

    // The original's boxes, laid out by this Clone's symmetry and placement.  Whether the origin
    // belongs in the placed box is also the original's answer.
    virtual void UpdateMainBBox() override;
    virtual bool PlacedBBoxIncludesOrigin() const override;

    // Whether following the chain of originals from id arrives back here.
    virtual bool IsCloneAncestor( const string &id ) const;

protected:
    virtual void UpdateSurf() override;
    virtual void UpdateMainTessVec() override;
    virtual void UpdateMainDegenGeomPreview() override;

    // Link to the original if it exists.  Idempotent and run every update, so creation and
    // read order do not matter.
    virtual void ResolveOriginal();

    string m_OriginalID;
};


#endif // !defined(VSPCLONEGEOM__INCLUDED_)
