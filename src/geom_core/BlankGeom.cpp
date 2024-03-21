//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BlankGeom.h"
#include "Vehicle.h"


//==== Constructor ====//
BlankGeom::BlankGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "BlankGeom";
    m_Type.m_Name = "Blank";
    m_Type.m_Type = BLANK_GEOM_TYPE;

    // Point Mass Parms
    m_BlankPointMassFlag.Init( "Point_Mass_Flag", "Mass", this, false, 0, 1 );
    m_BlankPointMassFlag.SetDescript( "Deprecated" );

    m_BlankPointMass.Init( "Point_Mass", "Mass", this, 0, 0, 1e12 );
    m_BlankPointMass.SetDescript( "Deprecated" );

    // Disable Parameters that don't make sense for BlankGeom
    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    Update();
}

//==== Destructor ====//
BlankGeom::~BlankGeom()
{
}

void BlankGeom::UpdateSurf()
{
    if ( m_BlankPointMassFlag.Get() )
    {
        m_PointMass = m_BlankPointMass();
        m_BlankPointMassFlag = false;
        m_BlankPointMass = 0;
    }
}

void BlankGeom::UpdateDrawObj()
{
    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    m_FeatureDrawObj_vec.clear();
    m_FeatureDrawObj_vec.resize( 3 );

    for ( int i = 0; i < 3; i++ )
    {
        vec3d c;
        c.v[ i ] = 1.0;
        m_FeatureDrawObj_vec[ i ].m_LineColor = c;
        m_FeatureDrawObj_vec[ i ].m_GeomChanged = true;
    }

    m_HighlightDrawObj.m_PntVec.clear();
    m_HighlightDrawObj.m_PointSize = 10.0;
    m_HighlightDrawObj.m_GeomChanged = true;

    m_PtMassCGDrawObj.m_PntVec.clear();
    m_PtMassCGDrawObj.m_GeomChanged = true;

    for ( int j = 0; j < m_TransMatVec.size(); j++ )
    {
        vec3d blankOrigin = m_TransMatVec[ j ].getTranslation();

        m_HighlightDrawObj.m_PntVec.push_back( blankOrigin );

        if ( m_PointMass() > 0.0 )
        {
            vec3d cg = m_TransMatVec[ j ].xform( vec3d( m_CGx(), m_CGy(), m_CGz() ) );
            m_PtMassCGDrawObj.m_PntVec.push_back( cg );
        }

        for ( int i = 0; i < 3; i++ )
        {
            vec3d pt = vec3d( 0.0, 0.0, 0.0 );
            pt.v[ i ] = axlen;

            m_FeatureDrawObj_vec[ i ].m_PntVec.push_back( blankOrigin );
            m_FeatureDrawObj_vec[ i ].m_PntVec.push_back( m_TransMatVec[ j ].xform( pt ) );
        }
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
}

void BlankGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    char str[256];

    bool isactive = m_Vehicle->IsGeomActive( m_ID );

        snprintf( str, sizeof( str ), "%d",1);
        m_HighlightDrawObj.m_GeomID = m_ID+string(str);
        m_HighlightDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN ) && isactive;

        // Set Render Destination to Main VSP Window.
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_Type = DrawObj::VSP_POINTS;
        draw_obj_vec.push_back( &m_HighlightDrawObj) ;

        m_PtMassCGDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_PtMassCGDrawObj.m_GeomID = m_ID + string( "PtMassCG" );
        m_PtMassCGDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN ) && isactive;
        m_PtMassCGDrawObj.m_PointSize = 10.0;
        m_PtMassCGDrawObj.m_PointColor = vec3d( 0, 0, 1 );
        m_PtMassCGDrawObj.m_Type = DrawObj::VSP_POINTS;
        draw_obj_vec.push_back( &m_PtMassCGDrawObj );

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


        for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
        {
            m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            snprintf( str, sizeof( str ),  "_%d", i );
            m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;
            m_FeatureDrawObj_vec[i].m_Visible = ( m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN ) ) || m_Vehicle->IsGeomActive( m_ID );
            m_FeatureDrawObj_vec[i].m_LineWidth = 2.0;
            m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_FeatureDrawObj_vec[i] );
        }
}

// This is substantially similar to Geom::UpdateSymmAttach() and could probably be combined in a meaningful way.
void BlankGeom::UpdateSymmAttach()
{
    int num_main = 1;                 // Currently hard-coded to 1.  Some of below is over-complex for this case.
    unsigned int num_surf = GetNumSymmCopies() * num_main;

    m_TransMatVec.resize( num_surf, Matrix4d() );
    // Compute Relative Translation Matrix
    Matrix4d symmOriginMat;
    Matrix4d relTrans;
    if ( m_SymAncestOriginFlag() )
    {
        symmOriginMat = GetAncestorAttachMatrix( m_SymAncestor() - 1 );
    }
    else
    {
        symmOriginMat = GetAncestorModelMatrix( m_SymAncestor() - 1 );
    }
    relTrans = symmOriginMat;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );

    for ( int i = 0 ; i < ( int )m_TransMatVec.size() ; i++ )
    {
        m_TransMatVec[i].initMat( relTrans.data() );
    }

    // Copy main surfs
    int symFlag = GetSymFlag();
    if ( symFlag != 0 )
    {
        int numShifts = -1;
        Matrix4d Ref; // Reflection Matrix
        Matrix4d Ref_Orig; // Original Reflection Matrix
        Matrix4d Rel; // Relative Transformation matrix with Reflection applied ( this is for the main surfaces )

        double angle = ( 360 ) / ( double )m_SymRotN();
        int currentIndex = num_main;
        bool radial = false;

        for ( int i = 0 ; i < GetNumSymFlags() ; i ++ ) // Loop through each of the set sym flags
        {
            // Find next set sym flag
            while ( true )
            {
                numShifts++;
                if ( ( ( symFlag >> numShifts ) & ( 1 << 0 ) ) || numShifts > vsp::SYM_NUM_TYPES )
                {
                    break;
                }
            }

            // Create Reflection Matrix
            if ( ( 1 << numShifts ) == vsp::SYM_XY )
            {
                Ref.loadXYRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_XZ )
            {
                Ref.loadXZRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_YZ )
            {
                Ref.loadYZRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_X )
            {
                Ref.loadIdentity();
                Ref.rotateX( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_Y )
            {
                Ref.loadIdentity();
                Ref.rotateY( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_Z )
            {
                Ref.loadIdentity();
                Ref.rotateZ( angle );
                Ref_Orig = Ref;
                radial = true;
            }

            // number of additional surfaces for a single reflection ( for rotational reflections it is m_SymRotN-1 times this number
            int numAddSurfs = currentIndex;
            int addIndex = 0;

            for ( int j = currentIndex ; j < currentIndex + numAddSurfs ; j++ )
            {
                if ( radial ) // rotational reflection
                {
                    for ( int k = 0 ; k < m_SymRotN() - 1 ; k++ )
                    {
                        m_TransMatVec[j + k * numAddSurfs].initMat( m_TransMatVec[j - currentIndex].data() );
                        m_TransMatVec[j + k * numAddSurfs].postMult( Ref.data() ); // Apply Reflection

                        // Increment rotation by the angle
                        Ref.postMult( Ref_Orig.data() );
                        addIndex++;
                    }
                    // Reset reflection matrices to the beginning angle
                    Ref = Ref_Orig;
                }
                else
                {
                    m_TransMatVec[j].initMat( m_TransMatVec[j - currentIndex].data() );
                    m_TransMatVec[j].postMult( Ref.data() ); // Apply Reflection
                    addIndex++;
                }
            }

            currentIndex += addIndex;
            radial = false;
        }
    }

    Matrix4d retrun_relTrans = relTrans;
    retrun_relTrans.affineInverse();

    //==== Save Transformation Matrix and Apply Transformations ====//
    for ( int i = 0 ; i < num_surf ; i++ )
    {
        m_TransMatVec[i].postMult( symmOriginMat.data() );
    }

}

void BlankGeom::ReadV2File( xmlNodePtr &root )
{
    xmlNodePtr node;

    //===== Read General Parameters =====//
    node = XmlUtil::GetNode( root, "General_Parms", 0 );
    if ( node )
    {
        Geom::ReadV2File( node );
    }

    //===== Read Blank Parameters =====//
    xmlNodePtr blank_node = XmlUtil::GetNode( root, "Blank_Parms", 0 );
    if ( blank_node )
    {
        m_PointMass = XmlUtil::FindDouble( blank_node, "PointMass", m_PointMass() );
    }
}
