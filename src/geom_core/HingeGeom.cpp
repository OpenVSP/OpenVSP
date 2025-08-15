//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HingeGeom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"


//==== Constructor ====//
HingeGeom::HingeGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "HingeGeom";
    m_Type.m_Name = "Hinge";
    m_Type.m_Type = HINGE_GEOM_TYPE;

    m_JointTranslate.Init( "JointTranslate", "Hinge", this, 0.0, -1e12, 1e12 );
    m_JointTranslateFlag.Init( "JointTranslateFlag", "Hinge", this, false, 0, 1 );
    m_JointTransMin.Init( "JointTransMin", "Hinge", this, -1000, -1e12, 1e12 );
    m_JointTransMinFlag.Init( "JointTransMinFlag", "Hinge", this, false, 0, 1 );
    m_JointTransMax.Init( "JointTransMax", "Hinge", this, 1000, -1e12, 1e12 );
    m_JointTransMaxFlag.Init( "JointTransMaxFlag", "Hinge", this, false, 0, 1 );
    m_JointRotate.Init( "JointRotate", "Hinge", this, 0.0, -360.0, 360.0 );
    m_JointRotateFlag.Init( "JointRotateFlag", "Hinge", this, true, 0, 1 );
    m_JointRotMin.Init( "JointRotMin", "Hinge", this, -360.0, -360.0, 360.0 );
    m_JointRotMinFlag.Init( "JointRotMinFlag", "Hinge", this, false, 0, 1 );
    m_JointRotMax.Init( "JointRotMax", "Hinge", this, 360.0, -360.0, 360.0 );
    m_JointRotMaxFlag.Init( "JointRotMaxFlag", "Hinge", this, false, 0, 1 );

    m_OrientType.Init( "OrientRotFlag", "Hinge", this, ORIENT_ROT, ORIENT_ROT, ORIENT_NUM_TYPES - 1 );

    m_PrimaryDir.Init( "PrimaryDir", "Hinge", this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_SecondaryDir.Init( "SecondaryDir", "Hinge", this, vsp::Y_DIR, vsp::X_DIR, vsp::Z_DIR );

    m_PrimXVec.Init( "PrimXVec", "Hinge", this, 1.0, -1e12, 1e12 );
    m_PrimYVec.Init( "PrimYVec", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimZVec.Init( "PrimZVec", "Hinge", this, 0.0, -1e12, 1e12 );

    m_PrimXVecRel.Init( "PrimXVecRel", "Hinge", this, 1.0, -1e12, 1e12 );
    m_PrimYVecRel.Init( "PrimYVecRel", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimZVecRel.Init( "PrimZVecRel", "Hinge", this, 0.0, -1e12, 1e12 );

    m_PrimVecAbsRelFlag.Init( "PrimVecAbsRelFlag", "Hinge", this, vsp::REL, vsp::ABS, vsp::REL );

    m_SecVecAbsRelFlag.Init( "SecVecAbsRelFlag", "Hinge", this, vsp::REL, vsp::ABS, vsp::REL );
    m_SecondaryVecDir.Init( "SecondaryVecDir", "Hinge", this, vsp::Y_DIR, vsp::X_DIR, vsp::Z_DIR );

    m_PrimXOff.Init( "PrimXOff", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimYOff.Init( "PrimYOff", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimZOff.Init( "PrimZOff", "Hinge", this, 0.0, -1e12, 1e12 );

    m_PrimXOffRel.Init( "PrimXOffRel", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimYOffRel.Init( "PrimYOffRel", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimZOffRel.Init( "PrimZOffRel", "Hinge", this, 0.0, -1e12, 1e12 );

    m_PrimOffAbsRelFlag.Init( "PrimOffAbsRelFlag", "Hinge", this, vsp::REL, vsp::ABS, vsp::REL );

    m_PrimULoc.Init( "PrimULoc", "Hinge", this, 0, 0, 1 );
    m_PrimWLoc.Init( "PrimWLoc", "Hinge", this, 0, 0, 1 );

    m_PrimaryType.Init( "PrimType", "Hinge", this, VECTOR3D, VECTOR3D, ORIENT_VEC_TYPES - 1 );

    // Disable Parameters that don't make sense for JointGeom
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();

    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    m_PointMass.Deactivate();
    m_CGx.Deactivate();
    m_CGy.Deactivate();
    m_CGz.Deactivate();
    m_Ixx.Deactivate();
    m_Iyy.Deactivate();
    m_Izz.Deactivate();
    m_Ixy.Deactivate();
    m_Ixz.Deactivate();
    m_Iyz.Deactivate();

    m_MainSurfVec.clear();
}

//==== Destructor ====//
HingeGeom::~HingeGeom()
{
}

//==== Scale ====//
void HingeGeom::ApplyScale( double currentScale )
{

    m_JointTranslate *= currentScale;
    m_JointTransMin *= currentScale;
    m_JointTransMax *= currentScale;

    // Scale both sets of XOff, one or the other will override on next UpdateSurf()
    m_PrimXOff *= currentScale;
    m_PrimYOff *= currentScale;
    m_PrimZOff *= currentScale;
    m_PrimXOffRel *= currentScale;
    m_PrimYOffRel *= currentScale;
    m_PrimZOffRel *= currentScale;

}

void HingeGeom::UpdateSurf()
{
    // Should be redundant.
    m_MainSurfVec.clear();
}

void HingeGeom::UpdateXForm()
{
    // Update m_ModelMatrix translations -- ignore rotations for now.
    Geom::UpdateXForm();
    // Evaluate hinge base point.
    vec3d baseOrigin;
    baseOrigin = m_ModelMatrix.xform(vec3d(0.0, 0.0, 0.0 ) );
    m_PrimEndpt = baseOrigin;

    // Ensure secondary and primary directions are different.
    if ( m_PrimaryDir() == m_SecondaryDir() )
    {
        if ( m_PrimaryDir() == vsp::X_DIR )
        {
            m_SecondaryDir = vsp::Y_DIR;
        }
        else if ( m_PrimaryDir() == vsp::Y_DIR )
        {
            m_SecondaryDir = vsp::Z_DIR;
        }
        else if ( m_PrimaryDir() == vsp::Z_DIR )
        {
            m_SecondaryDir = vsp::X_DIR;
        }
    }

    // Find normal direction by clever math.
    int ndir = 3 - ( m_PrimaryDir() + m_SecondaryDir() );

    double tempMat[16];

    Matrix4d attachedRotMat;
    m_AttachMatrix.getMat( tempMat );
    tempMat[12] = tempMat[13] = tempMat[14] = 0;
    attachedRotMat.initMat( tempMat );

    Matrix4d invattachMat;
    invattachMat = m_AttachMatrix;
    invattachMat.affineInverse();

    if ( m_OrientType.Get() == ORIENT_ROT )
    {
        // Use orientation from angles on XForm tab.
    }
    else
    {
        // Calculate based on vectors -- fill in angles.
        vec3d prim = vec3d( 1, 0, 0 );

        if ( m_PrimaryType() == VECTOR3D )
        {
            if ( m_PrimVecAbsRelFlag() == vsp::REL )
            {
                prim.set_xyz( m_PrimXVecRel(), m_PrimYVecRel(), m_PrimZVecRel() );
                prim.normalize();
                prim = attachedRotMat.xform( prim );
            }
            else
            {
                prim.set_xyz( m_PrimXVec(), m_PrimYVec(), m_PrimZVec() );
                prim.normalize();
            }
        }
        else if ( m_PrimaryType() == POINT3D )
        {
            vec3d endpt;
            if ( m_PrimOffAbsRelFlag() == vsp::REL )
            {
                endpt.set_xyz( m_PrimXOffRel(), m_PrimYOffRel(), m_PrimZOffRel() );
                endpt = m_AttachMatrix.xform( endpt );
                m_PrimXOff = endpt.x();
                m_PrimYOff = endpt.y();
                m_PrimZOff = endpt.z();
            }
            else
            {
                endpt.set_xyz( m_PrimXOff(), m_PrimYOff(), m_PrimZOff() );
                vec3d tpt = invattachMat.xform( endpt );
                m_PrimXOffRel = tpt.x();
                m_PrimYOffRel = tpt.y();
                m_PrimZOffRel = tpt.z();
            }
            m_PrimEndpt = endpt;
            prim = endpt - baseOrigin;
            prim.normalize();
        }
        else // All the surface attached types
        {
            Geom* parent = m_Vehicle->FindGeom( GetParentID() );

            if ( parent )
            {
                Matrix4d rotMat;

                Matrix4d parentMat;
                parentMat = parent->getModelMatrix();

                Matrix4d parentRotMat;
                parentMat.getMat( tempMat );
                tempMat[12] = tempMat[13] = tempMat[14] = 0;
                parentRotMat.initMat( tempMat );

                Matrix4d invparentRotMat;
                invparentRotMat = parentRotMat;
                invparentRotMat.affineInverse();

                parent->CompRotCoordSys( 0, m_PrimULoc(), m_PrimWLoc(), rotMat );

                double u = m_PrimULoc();
                const VspSurf* surf = parent->GetMainSurfPtr( 0 );
                if ( surf )
                {
                    u = surf->InvertUMapping( m_PrimULoc() * parent->GetUMapMax( 0 ) ) / parent->GetUMax( 0 );
                }

                vec3d surfpt = parent->CompPnt01( 0, u, m_PrimWLoc());


                if ( m_PrimaryType() == SURFPT )
                {

                    vec3d off;
                    if ( m_PrimOffAbsRelFlag() == vsp::REL )
                    {
                        off.set_xyz( m_PrimXOffRel(), m_PrimYOffRel(), m_PrimZOffRel() );
                        off = parentRotMat.xform( off );
                        m_PrimXOff = off.x();
                        m_PrimYOff = off.y();
                        m_PrimZOff = off.z();
                    }
                    else
                    {
                        off.set_xyz( m_PrimXOff(), m_PrimYOff(), m_PrimZOff() );
                        vec3d invoff = invparentRotMat.xform( off );
                        m_PrimXOffRel = invoff.x();
                        m_PrimYOffRel = invoff.y();
                        m_PrimZOffRel = invoff.z();
                    }

                    m_PrimEndpt = surfpt + off;
                    prim = m_PrimEndpt - baseOrigin;
                    prim.normalize();
                }
                else if ( m_PrimaryType() == UDIR )
                {
                    vec3d dir1, dir2;
                    rotMat.getBasis( prim, dir1, dir2 );
                    m_PrimEndpt = surfpt;
                }
                else if ( m_PrimaryType() == WDIR )
                {
                    vec3d dir1, dir2;
                    rotMat.getBasis( dir1, prim, dir2 );
                    m_PrimEndpt = surfpt;
                }
                else // NDIR
                {
                    vec3d dir1, dir2;
                    rotMat.getBasis( dir1, dir2, prim );
                    m_PrimEndpt = surfpt;
                }
            }
        }

        vec3d sec;

        if ( m_SecVecAbsRelFlag() == vsp::REL )
        {
            sec.v[ m_SecondaryVecDir() ] = 1.0;
            sec = attachedRotMat.xform( sec );
        }
        else
        {
            sec.v[ m_SecondaryVecDir() ] = 1.0;
        }

        // At this point, to calculate the normal direction,
        // prim and sec must be unit vectors in global coordinates.

        vec3d norm = vec3d( 0, 0, 1 );
        if ( std::abs( dot( prim, sec ) ) >= 1.0 ) // Co-linear.  Force minor component.
        {
            sec.set_xyz( 0, 0, 0 );
            sec.v[ prim.minor_comp() ] = 1.0;
        }

        norm = cross( prim, sec );
        norm.normalize();

        sec = cross( norm, prim );
        sec.normalize();


        vector < vec3d > dirs(3);
        dirs[ m_PrimaryDir() ] = prim;
        dirs[ m_SecondaryDir() ] = sec;
        dirs[ ndir ] = norm;

        Matrix4d mat;
        mat.setBasis( dirs[ vsp::X_DIR ], dirs[ vsp::Y_DIR ], dirs[ vsp::Z_DIR ] );

        // Back-fill component orientation angles
        vec3d angles;
        angles = mat.getAngles();

        m_XRot.Set( angles.x() );
        m_YRot.Set( angles.y() );
        m_ZRot.Set( angles.z() );

        // Update Relative Parameters by backing off attach matrix
        Matrix4d relmat = invattachMat;
        relmat.matMult( mat.data() );

        angles = relmat.getAngles();
        m_XRelRot = angles.x();
        m_YRelRot = angles.y();
        m_ZRelRot = angles.z();
    }

    UpdateMotionFlagsLimits();

    // Move joint according to ModelMatrix.
    // Update m_ModelMatrix again -- rotations included this time.
    Geom::UpdateXForm();

    m_JointMatrix = BuildFlippedJointMatrix( m_JointTranslate(), m_JointRotate(), m_ModelMatrix, GetFlipMat() );


    vector < vec3d > dirs(3);
    m_ModelMatrix.getBasis( dirs[ vsp::X_DIR ], dirs[ vsp::Y_DIR ], dirs[ vsp::Z_DIR ] );


    // Back-fill global vector coordinates if input specified relative.
    if ( m_OrientType() == ORIENT_ROT || m_PrimaryType() != VECTOR3D || m_PrimVecAbsRelFlag() == vsp::REL )
    {
        m_PrimXVec = dirs[ m_PrimaryDir() ].x();
        m_PrimYVec = dirs[ m_PrimaryDir() ].y();
        m_PrimZVec = dirs[ m_PrimaryDir() ].z();
    }

    Matrix4d basismat = invattachMat;
    basismat.matMult( m_ModelMatrix.data() );
    basismat.getBasis( dirs[ vsp::X_DIR ], dirs[ vsp::Y_DIR ], dirs[ vsp::Z_DIR ] );

    if ( m_OrientType() == ORIENT_ROT || m_PrimaryType() != VECTOR3D || m_PrimVecAbsRelFlag() == vsp::ABS )
    {
        m_PrimXVecRel = dirs[ m_PrimaryDir() ].x();
        m_PrimYVecRel = dirs[ m_PrimaryDir() ].y();
        m_PrimZVecRel = dirs[ m_PrimaryDir() ].z();
    }

}

void HingeGeom::UpdateMotionFlagsLimits()
{
    SetJointParmLimits( m_JointTranslate, m_JointRotate );
}

void HingeGeom::SetParmLimits( Parm & p, const Parm & pflag, const Parm & pmin, const Parm & pminflag, const Parm & pmax, const Parm & pmaxflag )
{
    if ( pminflag() )
    {
        p.SetLowerLimit( pmin() );
    }
    else
    {
        p.SetLowerLimit( pmin.GetLowerLimit() );
    }

    if ( pmaxflag() )
    {
        p.SetUpperLimit( pmax() );
    }
    else
    {
        p.SetUpperLimit( pmax.GetUpperLimit() );
    }

    if ( !pflag() )
    {
        p = 0.0;
    }
}


void HingeGeom::UpdateDrawObj()
{
    vec3d baseOrigin = m_ModelMatrix.xform(vec3d(0.0, 0.0, 0.0 ) );

    m_HighlightDrawObj.m_PntVec.resize(1);
    m_HighlightDrawObj.m_PntVec[0] = baseOrigin;
    m_HighlightDrawObj.m_PointSize = 10.0;
    m_HighlightDrawObj.m_GeomChanged = true;

    //=== Attach Axis ===//
    if ( m_AxisDrawObj_vec.size() != 3 )
    {
        m_AxisDrawObj_vec.clear();
        m_AxisDrawObj_vec.resize( 3 );
    }
    for ( int i = 0; i < 3; i++ )
    {
        m_AxisDrawObj_vec[i].m_PntVec.clear();
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }

    if ( m_PrimaryType() == UDIR || m_PrimaryType() == WDIR || m_PrimaryType() == NDIR )
    {
        m_HighlightDrawObj.m_PntVec.push_back( m_PrimEndpt );
    }
}

// The joint frame, the frame its children are carried to, the allowed motion, and the primary
// direction line, placed by placer.  A Clone is posed by its own deflection.
void HingeGeom::BuildMarkerDrawObjs( Geom* placer, vector< DrawObj > &marker_vec )
{
    JointRole* joint = Geom::CastTo< JointRole >( placer );
    if ( !joint )
    {
        marker_vec.clear();
        return;
    }

    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    Matrix4d model_matrix = placer->getModelMatrix();
    Matrix4d joint_matrix = joint->GetJointMatrix();

    // Evaluate points for visualization.
    vec3d jointOrigin = joint_matrix.xform(vec3d(0.0, 0.0, 0.0 ) );
    vec3d baseOrigin = model_matrix.xform(vec3d(0.0, 0.0, 0.0 ) );

    vector < vec3d > baseAxis(3);
    vector < vec3d > jointAxis(3);
    for ( int i = 0; i < 3; i++ )
    {
        vec3d pt = vec3d( 0.0, 0.0, 0.0 );
        pt.v[i] = axlen;
        baseAxis[i] = model_matrix.xform(pt );
        jointAxis[i] = joint_matrix.xform(pt );
    }

    if ( marker_vec.size() != NUM_HINGE_MARKERS )
    {
        marker_vec.clear();
        marker_vec.resize( NUM_HINGE_MARKERS );
    }
    for ( int i = 0; i < NUM_HINGE_MARKERS; i++ )
    {
        marker_vec[i].m_PntVec.clear();
        marker_vec[i].m_NormVec.clear();
        marker_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        marker_vec[i].m_GeomChanged = true;
    }

    for ( int i = 0; i < 3; i++ )
    {
        marker_vec[i].m_PntVec.push_back(baseOrigin );
        marker_vec[i].m_PntVec.push_back(baseAxis[i] );
        vec3d c;
        c.v[i] = 1.0;
        marker_vec[i].m_LineColor = c;
    }

    for ( int i = 0; i < 3; i++ )
    {
        int k = i + 3;
        MakeDashedLine(jointOrigin, jointAxis[i], 8, marker_vec[k].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        marker_vec[k].m_LineColor = c;
    }

    for ( int i = 0; i < 6; i++ )
    {
        marker_vec[i].m_GeomID = placer->GetID() + "_Feature_" + std::to_string( i );
        marker_vec[i].m_LineWidth = 2.0;
        marker_vec[i].m_Type = DrawObj::VSP_LINES;
    }

    DrawObj &motion_arrows = marker_vec[ HINGE_MARKER_MOTION_ARROWS ];
    DrawObj &motion_lines = marker_vec[ HINGE_MARKER_MOTION_LINES ];
    DrawObj &primary_line = marker_vec[ HINGE_MARKER_PRIMARY_LINE ];

    if ( m_JointRotateFlag.Get() )
    {
        vec3d u = baseAxis[ m_PrimaryDir() ] - baseOrigin;
        MakeCircleArrow(baseOrigin + 0.6 * u, u, 0.5 * axlen, 0.5 * axlen, motion_lines, motion_arrows );
    }
    if ( m_JointTranslateFlag.Get() )
    {
        MakeArrowhead(baseAxis[ m_PrimaryDir() ], baseAxis[ m_PrimaryDir() ] - baseOrigin, 0.5 * axlen * 0.5, motion_arrows.m_PntVec, motion_arrows.m_NormVec );
    }

    motion_arrows.m_GeomID = placer->GetID() + "MArrows";
    motion_arrows.m_LineWidth = 1.0;
    motion_arrows.m_Type = DrawObj::VSP_SHADED_TRIS;

    for ( int i = 0; i < 4; i++ )
    {
        motion_arrows.m_MaterialInfo.Ambient[i] = 0.2f;
        motion_arrows.m_MaterialInfo.Diffuse[i] = 0.1f;
        motion_arrows.m_MaterialInfo.Specular[i] = 0.7f;
        motion_arrows.m_MaterialInfo.Emission[i] = 0.0f;
    }
    motion_arrows.m_MaterialInfo.Diffuse[3] = 0.5f;
    motion_arrows.m_MaterialInfo.Shininess = 5.0f;

    motion_lines.m_GeomID = placer->GetID() + "MLines";
    motion_lines.m_LineWidth = 2.0;
    motion_lines.m_Type = DrawObj::VSP_LINES;

    primary_line.m_GeomID = placer->GetID() + "PrimLines";
    primary_line.m_LineWidth = 2.0;
    primary_line.m_Type = DrawObj::VSP_LINES;

    if ( m_PrimaryType() == POINT3D || m_PrimaryType() == SURFPT )
    {
        // The direction target, carried from this hinge's frame into placer's.
        Matrix4d to_local = m_ModelMatrix;
        to_local.affineInverse();

        primary_line.m_PntVec.push_back(baseOrigin );
        primary_line.m_PntVec.push_back( model_matrix.xform( to_local.xform( m_PrimEndpt ) ) );
        vec3d c;
        c.v[ m_PrimaryDir() ] = 1.0;
        primary_line.m_LineColor = c;
    }

    // The flip reverses the motion; the frames themselves are not reflected.
    placer->FlipDrawObjs( { &motion_lines, &motion_arrows, &primary_line } );
}

void HingeGeom::SetMarkerVisibility( Geom* placer, vector< DrawObj > &marker_vec )
{
    bool visible = placer->ShowsMarkers();
    for ( int i = 0; i < ( int )marker_vec.size(); i++ )
    {
        marker_vec[i].m_Visible = visible;
    }
}

// The frame the joint turns about is what there is of a Hinge to pick.
void HingeGeom::LoadMainDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    SetMarkerVisibility( this, m_MarkerDrawObj_vec );

    for ( int i = 0; i < 3 && i < ( int )m_MarkerDrawObj_vec.size(); i++ )
    {
        draw_obj_vec.push_back( &m_MarkerDrawObj_vec[i] );
    }
}

void HingeGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    char str[256];

    LoadMainDrawObjs( draw_obj_vec );

    bool isactive = m_Vehicle->IsGeomActive( m_ID );

    snprintf( str, sizeof( str ), "%d",1);
    m_HighlightDrawObj.m_GeomID = m_ID+string(str);
    m_HighlightDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN ) && isactive;

    // Set Render Destination to Main VSP Window.
    m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightDrawObj.m_Type = DrawObj::VSP_POINTS;
    draw_obj_vec.push_back( &m_HighlightDrawObj) ;

    for ( int i = 0; i < m_AxisDrawObj_vec.size(); i++ )
    {
        m_AxisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        snprintf( str, sizeof( str ),  "_%d", i );
        m_AxisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
        m_AxisDrawObj_vec[i].m_Visible = isactive;
        m_AxisDrawObj_vec[i].m_LineWidth = 2.0;
        m_AxisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_AxisDrawObj_vec[i] );
    }

    // The rest of the markers: the joint's own frame and its motion.
    for ( int i = 3; i < ( int )m_MarkerDrawObj_vec.size(); i++ )
    {
        draw_obj_vec.push_back( &m_MarkerDrawObj_vec[i] );
    }
}

Matrix4d HingeGeom::GetJointMatrix() const
{
    return m_JointMatrix;
}

// The deflection is passed in, so a Clone can pose the joint by its own angle.
Matrix4d HingeGeom::BuildJointMatrix( double translate, double rotate, const Matrix4d &model_matrix ) const
{
    Matrix4d joint_matrix;

    vec3d trans;
    trans.v[ m_PrimaryDir() ] = translate;

    joint_matrix.translatev( trans );

    if ( m_PrimaryDir.Get() == vsp::X_DIR )
    {
        joint_matrix.rotateX( rotate );
    }
    else if ( m_PrimaryDir.Get() == vsp::Y_DIR )
    {
        joint_matrix.rotateY( rotate );
    }
    else
    {
        joint_matrix.rotateZ( rotate );
    }

    double mat[16];
    model_matrix.getMat( mat );
    joint_matrix.postMult( mat );

    return joint_matrix;
}

bool HingeGeom::GetJointTransMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const
{
    min_set = m_JointTransMinFlag();
    min_val = m_JointTransMin();
    max_set = m_JointTransMaxFlag();
    max_val = m_JointTransMax();

    return m_JointTranslateFlag();
}

bool HingeGeom::GetJointRotMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const
{
    min_set = m_JointRotMinFlag();
    min_val = m_JointRotMin();
    max_set = m_JointRotMaxFlag();
    max_val = m_JointRotMax();

    return m_JointRotateFlag();
}

void HingeGeom::SetJointParmLimits( Parm &translate, Parm &rotate )
{
    SetParmLimits( translate, m_JointTranslateFlag,
                   m_JointTransMin, m_JointTransMinFlag,
                   m_JointTransMax, m_JointTransMaxFlag );

    SetParmLimits( rotate, m_JointRotateFlag,
                   m_JointRotMin, m_JointRotMinFlag,
                   m_JointRotMax, m_JointRotMaxFlag );
}

// The frame children of this joint hang off.  Built without the flip, which would turn a
// child's shape inside out.
Matrix4d JointRole::GetJointMatrix() const
{
    return BuildJointMatrix( GetJointTranslate(), GetJointRotate(), GetRoleModelMatrix() );
}

// The joint motion seen through the flip: reflect, move, reflect back.  The result is still
// rigid, so the frame stays unreflected; only the motion is flipped.  A plane containing the axis reverses
// the rotation; a plane normal to the axis reverses the translation.
Matrix4d JointRole::BuildFlippedJointMatrix( double translate, double rotate, const Matrix4d &model_matrix, const Matrix4d &flip_mat ) const
{
    Matrix4d frame = model_matrix;
    frame.matMult( flip_mat );

    Matrix4d joint_matrix = BuildJointMatrix( translate, rotate, frame );

    // Each reflection is its own inverse and they commute, so the same matrix undoes it.
    joint_matrix.matMult( flip_mat );

    return joint_matrix;
}

// The direction the joint translates along, in world coordinates, reflected by the flip to
// match BuildFlippedJointMatrix.  Under an odd number of reflections the joint rotates the
// opposite way about this axis.
vec3d JointRole::GetJointAxis() const
{
    Matrix4d mat = GetRoleModelMatrix();
    Matrix4d flip_mat = GetRoleFlipMat();

    vec3d pt( 0.0, 0.0, 0.0 );
    pt.v[ GetJointPrimaryDir() ] = 1.0;

    vec3d local = flip_mat.xform( pt ) - flip_mat.xform( vec3d( 0.0, 0.0, 0.0 ) );

    vec3d origin = mat.xform( vec3d( 0.0, 0.0, 0.0 ) );

    vec3d axis = mat.xform( local ) - origin;
    axis.normalize();

    return axis;
}

// GetJointAxis, reversed under an odd number of reflections, where the joint turns the other
// way about the line it moves along.
vec3d JointRole::GetJointRotationAxis() const
{
    vec3d axis = GetJointAxis();
    if ( GetRoleShapeFlipNormal() )
    {
        axis = axis * -1.0;
    }

    return axis;
}
