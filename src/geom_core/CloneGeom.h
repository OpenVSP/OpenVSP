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


    // Meshing resamples the surface, so these defer to the original's tessellation scheme.
    virtual void GetUWTess( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen,
                            vector< double > &utess, vector< double > &vtess, const int & n_ref = 0 ) const override;
    virtual void UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen,
                                  vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms,
                                  vector< vector< vec3d > > &uw_pnts, const int & n_ref = 0 ) const override;
    virtual void UpdateSplitTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess,
                                       vector< vector< vector< vec3d > > > &pnts,
                                       vector< vector< vector< vec3d > > > &norms ) const override;

    // The copied surfaces are already capped; take the original's results.
    virtual void UpdateEndCaps( int ncap = -1 ) override;

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node ) override;
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node ) override;

    // False if the Geom cannot be copied or would form a cycle of Clones.
    virtual bool SetOriginalID( const string &id );
    virtual string GetOriginalID() const
    {
        return m_OriginalID;
    }

    // Suffix that automatic naming appends to the original's name.
    virtual string GetNameSuffix() const
    {
        return m_NameSuffix;
    }
    virtual void SetNameSuffix( const string &suffix );

    // True while automatic naming is on, even before an original is found, since the next
    // update writes the name.  Losing the original turns automatic naming off.
    virtual bool NameIsAutomatic() const override;

    // The original's textures while appearance is copied.  Asked of the original, so a Clone
    // of a Clone reaches the end of the chain.
    virtual TextureMgr* GetDrawTextureMgr() override;

    // The Parms each switch governs, copied from the original.
    static void CopySymParms( Geom* from, Geom* to );
    static void CopyXFormParms( Geom* from, Geom* to );
    static void CopyAttachParms( Geom* from, Geom* to );
    static void CopyMassPropParms( Geom* from, Geom* to );
    static void CopyNegativeVolumeParm( Geom* from, Geom* to );
    static void CopySetFlags( Geom* from, Geom* to );
    static void CopyAppearance( Geom* from, Geom* to );

    virtual Geom* GetOriginalGeom() const;

    // The original's boxes, laid out by this Clone's symmetry and placement.  Whether the origin
    // belongs in the placed box is also the original's answer.
    virtual void UpdateMainBBox() override;
    virtual bool PlacedBBoxIncludesOrigin() const override;

    // Whether following the chain of originals from id arrives back here.
    virtual bool IsCloneAncestor( const string &id ) const;

    // This Clone's flip planes, combined with the original's while symmetry is copied.
    virtual int GetFlipFlag() const override;

    // What is copied from the original.  All on by default except the transformation.
    BoolParm m_CloneSets;
    BoolParm m_CloneSym;
    BoolParm m_CloneXForm;
    BoolParm m_CloneAttach;
    BoolParm m_CloneAppearance;
    BoolParm m_CloneNegativeVolume;
    BoolParm m_CloneMassProps;
    BoolParm m_CloneSubSurfs;
    BoolParm m_AutoName;

protected:
    virtual void UpdateSurf() override;
    virtual void UpdateMainTessVec() override;
    virtual void UpdateMainDegenGeomPreview() override;

    // Link to the original if it exists.  Idempotent and run every update, so creation and
    // read order do not matter.
    virtual void ResolveOriginal();

    // Symmetry, placement and attachment are needed before UpdateSurf, so they are copied here.
    virtual void UpdateCopyXFormParms() override;

    // Runs after the base class, so copied Parms stay deactivated; their GUI follows the Parm.
    virtual void DeactivateXForms() override;

    // Copies negative volume and mass properties alongside the surfaces.
    virtual void UpdateCopySurfParms() override;

    // While negative volume is copied, the copied surfaces already carry their CFD types.
    virtual void UpdateFlags() override;

    // Name and colours are not Parms; these run when the original's dirty flag arrives.
    virtual void UpdateCopyAppearance() override;
    virtual void UpdateCopyName() override;

    string m_OriginalID;

    // Appended to the original's name by automatic naming.  Saved to file.
    string m_NameSuffix;
};


#endif // !defined(VSPCLONEGEOM__INCLUDED_)
