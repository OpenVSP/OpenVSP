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
#include "GeomInterface.h"
#include "HingeGeom.h"
#include "GearGeom.h"
#include "AuxiliaryGeom.h"
#include "PropGeom.h"
#include "HumanGeom.h"
#include "MeshGeom.h"
#include "PtCloudGeom.h"
#include "WireGeom.h"
#include "NGonMeshGeom.h"

#include <set>


//==== Clone Geom ====//
class CloneGeom : public Geom, public JointRole, public GearContactRole, public AuxiliaryRole, public RotorRole, public TMeshRole, public HumanVertRole, public PointCloudRole, public WirePtRole, public PGMeshRole
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

    // Whether the subsurface is a copy of the original's (rebuilt every update) rather than
    // one added to this Geom.
    virtual bool IsCopiedSubSurf( const string &id ) const;
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

    // True when the copied shape is not surfaces but one shape placed by one matrix.  Such a
    // Clone has no symmetry copies.
    virtual bool ShowsOnePlacedShape() const;
    virtual int GetSymFlag() const override;

    // Behaves as the original; GetType still returns Clone.
    virtual int GetBehaviorType() const override;
    virtual Geom* GetBehaviorGeom() override;

    //==== Standing in for a landing gear ====//
    // Answered by the original in the gear's own frame; this Geom's matrix places the result.
    virtual void BuildOnePtBasis( const string &cp1, int isymm1, int suspension1, int tire1, double thetabogie, double thetawheel, double thetaroll, Matrix4d &mat, vec3d &p1 ) override;
    virtual void BuildTwoPtBasis( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, Matrix4d &mat, vec3d &p1, vec3d &p2 ) override;
    virtual void BuildThreePtBasis( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, Matrix4d &mat ) override;
    virtual void BuildThreePtOffAxisBasis( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, double mainoffset, Matrix4d &mat ) override;
    virtual bool GetTwoPtPivot( const string &cp1, int isymm1, int suspension1, const string &cp2, int isymm2, int suspension2, vec3d &ptaxis, vec3d &axis ) const override;
    virtual bool GetTwoPtAftAxleAxis( const string &cp1, int isymm1, int suspension1, const string &cp2, int isymm2, int suspension2, double thetabogie, vec3d &ptaxis, vec3d &axis ) const override;
    virtual bool GetTwoPtFwdAxleAxis( const string &cp1, int isymm1, int suspension1, const string &cp2, int isymm2, int suspension2, double thetabogie, vec3d &ptaxis, vec3d &axis ) const override;
    virtual bool GetTwoPtMeanContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2, bool &usepivot, double &mintheta, double &maxtheta ) const override;
    virtual bool GetTwoPtAftContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const override;
    virtual bool GetTwoPtFwdContactPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, double thetabogie, double thetawheel, vec3d &pt, vec3d &normal, vec3d &p1, vec3d &p2 ) const override;
    virtual bool GetTwoPtSideContactPtsNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, vec3d &p1, vec3d &p2, vec3d &normal ) const override;
    virtual bool GetOnePtSideContactPtAxisNormal( const string &cp1, int isymm1, int suspension1, int tire1, double thetabogie, double thetawheel, double thetaroll, vec3d &p1, vec3d &axis, vec3d &normal, int &ysign ) const override;
    virtual bool GetPtNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, vec3d &pt, vec3d &normal ) const override;
    virtual bool GetSteerAngle( const string &cp1, const string &cp2, const string &cp3, int &isteer, double &steerangle ) const override;
    virtual void GetNominalPtNormal( vec3d &pt, vec3d &normal ) const override;
    virtual void GetCG( vec3d &cgnom, vector < vec3d > &cgbounds ) const override;
    virtual bool GetContactPointVecNormal( const string &cp1, int isymm1, int suspension1, int tire1, const string &cp2, int isymm2, int suspension2, int tire2, const string &cp3, int isymm3, int suspension3, int tire3, vector < vec3d > &ptvec, vec3d &normal ) const override;
    virtual Bogie* GetBogie( const string &id ) const override;
    virtual vector < Bogie* > GetBogieVec(  ) override;
    virtual int GetGearModelLenUnits(  ) const override;

    // The original as a gear contact role, if it is one.
    virtual GearContactRole* GetOriginalGearContact() const;

    //==== Standing in for an auxiliary geom ====//
    // Answered by the original, measured against this Geom's own parent gear.
    virtual GearContactRole* GetContactGear() const override;
    virtual int GetAuxiliaryMode() const override;

    // Both roles declare these names; bring both overload sets into scope so neither hides
    // the other.
    using GearContactRole::GetCG;
    using AuxiliaryRole::GetCG;
    using GearContactRole::GetPtNormal;
    using AuxiliaryRole::GetPtNormal;
    using GearContactRole::GetTwoPtSideContactPtsNormal;
    using AuxiliaryRole::GetTwoPtSideContactPtsNormal;
    using GearContactRole::GetContactPointVecNormal;
    using AuxiliaryRole::GetContactPointVecNormal;

    virtual bool GetCGInGear( vec3d &cgnom, vector < vec3d > &cgbounds ) override;
    virtual bool GetPtNormalInGear( vec3d &pt, vec3d &normal ) const override;
    virtual bool GetPtNormalMeanContactPtPivotAxisInGear( vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis, bool &usepivot, double &mintheta, double &maxtheta ) override;
    virtual bool GetSideContactPtRollAxisNormalInGear( vec3d &pt, vec3d &axis, vec3d &normal, int &ysign ) override;
    virtual bool GetPtNormalAftAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis ) override;
    virtual bool GetPtNormalFwdAxleAxisInGear( double thetabogie, vec3d &pt, vec3d &normal, vec3d &ptaxis, vec3d &axis ) override;
    virtual bool GetPtPivotAxisInGear( vec3d &ptaxis, vec3d &axis ) override;
    virtual bool GetTwoPtSideContactPtsNormalInGear( vec3d &p1, vec3d &p2, vec3d &normal ) override;
    virtual bool GetContactPointVecNormalInGear( vector < vec3d > &ptvec, vec3d &normal ) override;
    virtual bool CalculateTurnInGear( vec3d &cor, vec3d &normal, vector<double> &rvec ) override;
    virtual bool GetSpreadTriInSelf( vec3d &pt, vec3d &axis, vector < vec3d > &t, int &flip ) const override;

    // The original as an auxiliary geom, if it is one.
    virtual AuxiliaryRole* GetOriginalAuxiliary() const;

    //==== Standing in for a Geom whose shape is a mesh ====//
    // The original's mesh in its own frame; this Geom's placement positions it.
    virtual vector< TMesh* > CreateTMeshVecInSelf( bool skipnegflipnormal, const int &n_ref ) const override;
    virtual const vector< TMesh* > & GetTMeshVecInSelf() const override;
    virtual Matrix4d GetTMeshTransMat() const override;
    virtual Matrix4d GetTMeshScaleMat() const override;
    virtual const map< vector < int >, int > & GetTMeshSingleTagMap() const override;
    virtual int GetTMeshColorStartDegree() const override;
    virtual vector< TMesh* > CreateTMeshVec( bool skipnegflipnormal, const int &n_ref = 0 ) const override;

    // The original as a mesh, if it is one.
    virtual TMeshRole* GetOriginalTMesh() const;

    //==== Standing in for a Geom built from one vertex set ====//
    // Vertices come from the original; this Geom's symmetry and placement expand them.
    virtual const vector < vec3d > & GetMainVerts() const override;
    virtual HumanVertRole* GetOriginalHumanVert() const;

    // The original's vertices, expanded by this Geom's symmetry and placement.
    virtual void BuildCloneVerts( vector < vector < vec3d > > &verts, vector < bool > &flipnormal ) const;

    virtual void UpdateDrawObj() override;
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec ) override;
    virtual void UpdateBBox() override;

    //==== Standing in for a Geom made of points ====//
    virtual const vector < vec3d > & GetPtsInSelf() const override;
    virtual Matrix4d GetPtsTransMat() const override;
    virtual Matrix4d GetPtsScaleMat() const override;
    virtual PointCloudRole* GetOriginalPointCloud() const;

    //==== Standing in for a Geom made of a point grid ====//
    virtual const vector < vector < vec3d > > & GetMainWirePts() const override;
    virtual Matrix4d GetWireTransMat() const override;
    virtual Matrix4d GetWireScaleMat() const override;
    virtual bool GetWireInvert() const override;
    virtual int GetWireDegenType() const override;
    virtual WirePtRole* GetOriginalWirePts() const;

    //==== Standing in for a Geom made of a polygon mesh ====//
    virtual PGMulti* GetPGMulti() const override;
    virtual Matrix4d GetPGTransMat() const override;
    virtual Matrix4d GetPGScaleMat() const override;
    virtual PGMeshRole* GetOriginalPGMesh() const;

    //==== Standing in for a rotor ====//
    virtual double GetRotorDiameter() const override;
    virtual double GetRotorR0() const override;
    virtual bool GetRotorReverseFlag() const override;
    virtual bool GetRotorHubDiameter( double &hubdia ) const override;

    //==== Standing in for a joint ====//
    // Defined by the original, so a Clone of a hinge moves its children.  The deflection is
    // this Clone's own, so two Clones can sit at different angles.
    virtual double GetJointTranslate() const override
    {
        return m_JointTranslate();
    }
    virtual double GetJointRotate() const override
    {
        return m_JointRotate();
    }
    virtual int GetJointPrimaryDir() const override;

    // The original as a joint, if it is one.
    virtual JointRole* GetOriginalJoint() const;
    virtual Matrix4d BuildJointMatrix( double translate, double rotate, const Matrix4d &model_matrix ) const override;
    virtual void SetJointParmLimits( Parm &translate, Parm &rotate ) override;
    virtual bool GetJointTransMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const override;
    virtual bool GetJointRotMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const override;

    // The original's markers, placed where this Geom is.
    virtual Geom* GetMarkerGeom() override;

    // Picked the same way as the original's.
    virtual bool LoadsMarkersAsMain() override;

    // Whether following the chain of originals from id arrives back here.
    virtual bool IsCloneAncestor( const string &id ) const;

    // The Geom at the end of the chain of originals, or null.  Bounded, since a file may
    // contain a cycle.
    virtual Geom* FollowOriginals() const;

    // The original's scaling, then this Geom's own placement.
    virtual Matrix4d PlaceBorrowedShape( const Matrix4d &scale_mat ) const;

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
    BoolParm m_CloneJoint;
    BoolParm m_AutoName;

    // Used only when the original is a joint.
    Parm m_JointTranslate;
    Parm m_JointRotate;

protected:
    virtual void UpdateSurf() override;
    virtual void UpdateMainTessVec() override;
    virtual void UpdateMainDegenGeomPreview() override;

    // Link to the original if it exists.  Idempotent and run every update, so creation and
    // read order do not matter.
    virtual void ResolveOriginal();

    // Symmetry, placement and attachment are needed before UpdateSurf, so they are copied here.
    virtual void UpdateCopyXFormParms() override;

    // A Blank or a Hinge has no surfaces but still places its children.
    virtual void UpdateSymmAttach() override;

    // Runs after the base class, so copied Parms stay deactivated; their GUI follows the Parm.
    virtual void DeactivateXForms() override;

    // Feature lines and the arc length curve come with the copied surfaces.
    virtual void UpdateFeatureLines() override;
    virtual void UpdateLCurve() override;

    // Copies negative volume and mass properties alongside the surfaces.
    virtual void UpdateCopySurfParms() override;

    // While negative volume is copied, the copied surfaces already carry their CFD types.
    virtual void UpdateFlags() override;

    // Name and colours are not Parms; these run when the original's dirty flag arrives.
    virtual void UpdateCopyAppearance() override;
    virtual void UpdateCopyName() override;

    // Copies of the original's subsurfaces, owned by this Geom so they carry its component ID
    // for the mesher.  Updated in place via m_SubSurfSourceMap so their IDs, which VSPAERO
    // keys control surfaces on, persist.
    virtual void UpdateCopySubSurfs() override;
    virtual SubSurface* FindCopiedSubSurf( const string &source_id, int type );

    // A Clone of a wireframe that is a single row or column of points, drawn as a polyline.
    DrawObj m_WireLineDO;

    string m_OriginalID;

    // Appended to the original's name by automatic naming.  Saved to file.
    string m_NameSuffix;

    // Original subsurface ID -> our copy's ID.  Saved, so the pairing survives a round trip.
    map < string, string > m_SubSurfSourceMap;
};


#endif // !defined(VSPCLONEGEOM__INCLUDED_)
