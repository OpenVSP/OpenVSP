//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HingeGeom.h"
#include "Vehicle.h"


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

    m_PrimVecAbsRelFlag.Init( "PrimVecAbsRelFlag", "Hinge", this, RELATIVE_XFORM, ABSOLUTE_XFORM, RELATIVE_XFORM );

    m_SecVecAbsRelFlag.Init( "SecVecAbsRelFlag", "Hinge", this, RELATIVE_XFORM, ABSOLUTE_XFORM, RELATIVE_XFORM );
    m_SecondaryVecDir.Init( "SecondaryVecDir", "Hinge", this, vsp::Y_DIR, vsp::X_DIR, vsp::Z_DIR );

    m_PrimXOff.Init( "PrimXOff", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimYOff.Init( "PrimYOff", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimZOff.Init( "PrimZOff", "Hinge", this, 0.0, -1e12, 1e12 );

    m_PrimXOffRel.Init( "PrimXOffRel", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimYOffRel.Init( "PrimYOffRel", "Hinge", this, 0.0, -1e12, 1e12 );
    m_PrimZOffRel.Init( "PrimZOffRel", "Hinge", this, 0.0, -1e12, 1e12 );

    m_PrimOffAbsRelFlag.Init( "PrimOffAbsRelFlag", "Hinge", this, RELATIVE_XFORM, ABSOLUTE_XFORM, RELATIVE_XFORM );

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

    Update();
}

//==== Destructor ====//
HingeGeom::~HingeGeom()
{
}

void HingeGeom::UpdateSurf()
{
    // Update m_ModelMatrix translations -- ignore rotations for now.
    GeomXForm::Update();
    // Evaluate hinge base point.
    m_BaseOrigin = m_ModelMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );
    m_PrimEndpt = m_BaseOrigin;

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
    Matrix4d attachedMat = ComposeAttachMatrix();

    Matrix4d attachedRotMat;
    attachedMat.getMat( tempMat );
    tempMat[12] = tempMat[13] = tempMat[14] = 0;
    attachedRotMat.initMat( tempMat );

    Matrix4d invattachMat;
    invattachMat = attachedMat;
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
            if ( m_PrimVecAbsRelFlag() == RELATIVE_XFORM )
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
            if ( m_PrimOffAbsRelFlag() == RELATIVE_XFORM )
            {
                endpt.set_xyz( m_PrimXOffRel(), m_PrimYOffRel(), m_PrimZOffRel() );
                endpt = attachedMat.xform( endpt );
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
            prim = endpt - m_BaseOrigin;
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

                parent->CompRotCoordSys( m_PrimULoc(), m_PrimWLoc(), rotMat );

                vec3d surfpt = parent->GetUWPt( m_PrimULoc(), m_PrimWLoc() );


                if ( m_PrimaryType() == SURFPT )
                {

                    vec3d off;
                    if ( m_PrimOffAbsRelFlag() == RELATIVE_XFORM )
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
                    prim = m_PrimEndpt - m_BaseOrigin;
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


    // Initialize the joint matrix to identity.
    m_JointMatrix.loadIdentity();

    // Do everything to build joint motion.
    vec3d trans;
    trans.v[ m_PrimaryDir() ] = m_JointTranslate();

    m_JointMatrix.translatev( trans );

    if ( m_PrimaryDir.Get() == vsp::X_DIR )
    {
        m_JointMatrix.rotateX( m_JointRotate() );
    }
    else if ( m_PrimaryDir.Get() == vsp::Y_DIR )
    {
        m_JointMatrix.rotateY( m_JointRotate() );
    }
    else
    {
        m_JointMatrix.rotateZ( m_JointRotate() );
    }


    // Move joint according to ModelMatrix.
    // Update m_ModelMatrix again -- rotations included this time.
    GeomXForm::Update();
    double mat[16];
    m_ModelMatrix.getMat( mat );
    m_JointMatrix.postMult( mat );


    vector < vec3d > dirs(3);
    m_ModelMatrix.getBasis( dirs[ vsp::X_DIR ], dirs[ vsp::Y_DIR ], dirs[ vsp::Z_DIR ] );


    // Back-fill global vector coordinates if input specified relative.
    if ( m_OrientType() == ORIENT_ROT || m_PrimaryType() != VECTOR3D || m_PrimVecAbsRelFlag() == RELATIVE_XFORM )
    {
        m_PrimXVec = dirs[ m_PrimaryDir() ].x();
        m_PrimYVec = dirs[ m_PrimaryDir() ].y();
        m_PrimZVec = dirs[ m_PrimaryDir() ].z();
    }

    Matrix4d basismat = invattachMat;
    basismat.matMult( m_ModelMatrix.data() );
    basismat.getBasis( dirs[ vsp::X_DIR ], dirs[ vsp::Y_DIR ], dirs[ vsp::Z_DIR ] );

    if ( m_OrientType() == ORIENT_ROT || m_PrimaryType() != VECTOR3D || m_PrimVecAbsRelFlag() == ABSOLUTE_XFORM )
    {
        m_PrimXVecRel = dirs[ m_PrimaryDir() ].x();
        m_PrimYVecRel = dirs[ m_PrimaryDir() ].y();
        m_PrimZVecRel = dirs[ m_PrimaryDir() ].z();
    }


    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    // Evaluate points for visualization.
    m_JointOrigin = m_JointMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );

    m_BaseAxis.clear();
    m_BaseAxis.resize( 3 );
    m_JointAxis.clear();
    m_JointAxis.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        vec3d pt = vec3d( 0.0, 0.0, 0.0 );
        pt.v[i] = axlen;
        m_BaseAxis[i] = m_ModelMatrix.xform( pt );
        m_JointAxis[i] = m_JointMatrix.xform( pt );
    }
}

void HingeGeom::UpdateMotionFlagsLimits()
{
    SetParmLimits( m_JointTranslate, m_JointTranslateFlag,
                   m_JointTransMin, m_JointTransMinFlag,
                   m_JointTransMax, m_JointTransMaxFlag );

    SetParmLimits( m_JointRotate, m_JointRotateFlag,
                   m_JointRotMin, m_JointRotMinFlag,
                   m_JointRotMax, m_JointRotMaxFlag );
}

void HingeGeom::SetParmLimits( Parm & p, Parm & pflag, Parm & pmin, Parm & pminflag, Parm & pmax, Parm & pmaxflag )
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
    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    m_HighlightDrawObj.m_PntVec.resize(1);
    m_HighlightDrawObj.m_PntVec[0] = m_BaseOrigin;
    m_HighlightDrawObj.m_PointSize = 10.0;

    m_FeatureDrawObj_vec.clear();
    m_FeatureDrawObj_vec.resize( 6 );
    for ( int i = 0; i < 3; i++ )
    {
        m_FeatureDrawObj_vec[i].m_PntVec.push_back( m_BaseOrigin );
        m_FeatureDrawObj_vec[i].m_PntVec.push_back( m_BaseAxis[i] );
        vec3d c;
        c.v[i] = 1.0;
        m_FeatureDrawObj_vec[i].m_LineColor = c;
        m_FeatureDrawObj_vec[i].m_GeomChanged = true;
    }

    //=== Attach Axis ===//
    m_AxisDrawObj_vec.clear();
    m_AxisDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }

    for ( int i = 0; i < 3; i++ )
    {
        int k = i + 3;
        MakeDashedLine( m_JointOrigin, m_JointAxis[i], 8, m_FeatureDrawObj_vec[k].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_FeatureDrawObj_vec[k].m_LineColor = c;
        m_FeatureDrawObj_vec[k].m_GeomChanged = true;
    }

    m_MotionLinesDO.m_PntVec.clear();
    m_MotionArrowsDO.m_PntVec.clear();

    if ( m_JointRotateFlag.Get() )
    {
        vec3d u = m_BaseAxis[ m_PrimaryDir() ] - m_BaseOrigin;
        MakeCircleArrow( m_BaseOrigin + 0.6 * u, u, 0.5 * axlen, m_MotionLinesDO, m_MotionArrowsDO );
    }
    if ( m_JointTranslateFlag.Get() )
    {
        MakeArrowhead( m_BaseAxis[ m_PrimaryDir() ], m_BaseAxis[ m_PrimaryDir() ] - m_BaseOrigin, 0.5 * axlen * 0.5, m_MotionArrowsDO.m_PntVec );
    }

    m_PrimaryLineDO.m_PntVec.clear();
    m_PrimaryLineDO.m_GeomChanged = true;

    if ( m_PrimaryType() == POINT3D || m_PrimaryType() == SURFPT )
    {
        m_PrimaryLineDO.m_PntVec.push_back( m_BaseOrigin );
        m_PrimaryLineDO.m_PntVec.push_back( m_PrimEndpt );
        m_PrimaryLineDO.m_LineWidth = 2.0;
        m_PrimaryLineDO.m_Type = DrawObj::VSP_LINES;
        vec3d c;
        c.v[ m_PrimaryDir() ] = 1.0;
        m_PrimaryLineDO.m_LineColor = c;
    }
    else if ( m_PrimaryType() == UDIR || m_PrimaryType() == WDIR || m_PrimaryType() == NDIR )
    {
        m_HighlightDrawObj.m_PntVec.push_back( m_PrimEndpt );
    }
}

void HingeGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    char str[256];

    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        sprintf(str,"%d",1);
        m_HighlightDrawObj.m_GeomID = m_ID+string(str);
        m_HighlightDrawObj.m_Visible = !m_GuiDraw.GetNoShowFlag();

        // Set Render Destination to Main VSP Window.
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_Type = DrawObj::VSP_POINTS;
        draw_obj_vec.push_back( &m_HighlightDrawObj) ;

        for ( int i = 0; i < m_AxisDrawObj_vec.size(); i++ )
        {
            m_AxisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            sprintf( str, "_%d", i );
            m_AxisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
            m_AxisDrawObj_vec[i].m_LineWidth = 2.0;
            m_AxisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_AxisDrawObj_vec[i] );
        }
    }

    if ( ( m_GuiDraw.GetDispFeatureFlag() && !m_GuiDraw.GetNoShowFlag() ) || m_Vehicle->IsGeomActive( m_ID ))
    {
        for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
        {
            m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            sprintf( str, "_%d", i );
            m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;
            m_FeatureDrawObj_vec[i].m_LineWidth = 2.0;
            m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_FeatureDrawObj_vec[i] );
        }

        m_MotionArrowsDO.m_GeomID = m_ID + "MArrows";
        m_MotionArrowsDO.m_LineWidth = 1.0;
        m_MotionArrowsDO.m_Type = DrawObj::VSP_SHADED_TRIS;
        m_MotionArrowsDO.m_NormVec = vector <vec3d> ( m_MotionArrowsDO.m_PntVec.size() );

        for ( int i = 0; i < 4; i++ )
        {
            m_MotionArrowsDO.m_MaterialInfo.Ambient[i] = 0.2f;
            m_MotionArrowsDO.m_MaterialInfo.Diffuse[i] = 0.1f;
            m_MotionArrowsDO.m_MaterialInfo.Specular[i] = 0.7f;
            m_MotionArrowsDO.m_MaterialInfo.Emission[i] = 0.0f;
        }
        m_MotionArrowsDO.m_MaterialInfo.Diffuse[3] = 0.5f;
        m_MotionArrowsDO.m_MaterialInfo.Shininess = 5.0f;


        m_MotionLinesDO.m_GeomID = m_ID + "MLines";
        m_MotionLinesDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_MotionLinesDO.m_LineWidth = 2.0;
        m_MotionLinesDO.m_Type = DrawObj::VSP_LINES;

        m_PrimaryLineDO.m_GeomID = m_ID + "PrimLines";
        m_PrimaryLineDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_PrimaryLineDO.m_LineWidth = 2.0;
        m_PrimaryLineDO.m_Type = DrawObj::VSP_LINES;

        draw_obj_vec.push_back( &m_MotionArrowsDO );
        draw_obj_vec.push_back( &m_MotionLinesDO );
        draw_obj_vec.push_back( &m_PrimaryLineDO );
    }
}

Matrix4d HingeGeom::GetJointMatrix()
{
    return m_JointMatrix;
}
