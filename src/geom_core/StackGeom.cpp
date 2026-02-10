//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "StackGeom.h"
#include "Vehicle.h"

using namespace vsp;

//==== Constructor ====//
StackGeom::StackGeom( Vehicle* vehicle_ptr ) : GeomEngine( vehicle_ptr )
{
    m_Name = "StackGeom";
    m_Type.m_Name = "Stack";
    m_Type.m_Type = STACK_GEOM_TYPE;

    m_Closed = false;

    m_XSecSurf.SetParentContainer( GetID() );

    m_XSecSurf.SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    m_OrderPolicy.Init( "OrderPolicy", "Design", this, STACK_FREE, STACK_FREE, NUM_STACK_POLICY - 1 );
    m_OrderPolicy.SetDescript( "XSec ordering policy for stack" );

    //==== rename capping controls for stack specific terminology ====//
    m_CapUMinOption.SetDescript("Type of End Cap on Stack Nose");
    m_CapUMinOption.Parm::Set(NO_END_CAP);
    m_CapUMinTess.SetDescript("Number of tessellated curves on Stack Nose and Tail");
    m_CapUMaxOption.SetDescript("Type of End Cap on Stack Tail");
    m_CapUMaxOption.Parm::Set(NO_END_CAP);

    InitParms( STACK_PRESET_DEFAULT );
}

void StackGeom::InitParms( int stack_type )
{
    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 17;
    m_TessW.SetMultShift( 8, 1 );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_STACK );
    m_XSecSurf.DeleteAllXSecs();

    vector < StackXSec* > stack_xs;

    if ( stack_type == STACK_PRESET_DEFAULT )
    {
        // Default StackXSecs
        StackXSec default_xs0 = StackXSec( new PointXSec() );
        StackXSec default_xs1 = StackXSec( new EllipseXSec() );
        StackXSec default_xs2 = StackXSec( new EllipseXSec() );
        StackXSec default_xs3 = StackXSec( new EllipseXSec() );
        StackXSec default_xs4 = StackXSec( new PointXSec() );

        default_xs0.m_XDelta = 0.0;
        default_xs0.m_TopRAngle = 45.0;
        default_xs0.m_TopRStrength = 0.75;
        default_xs0.m_RightRAngle = 45.0;
        default_xs0.m_RightRStrength = 0.75;

        default_xs1.m_XDelta = 1.0;
        default_xs1.GetXSecCurve()->SetWidthHeight( 3.0, 2.5 );

        default_xs2.m_XDelta = 2.0;
        default_xs2.GetXSecCurve()->SetWidthHeight( 3.0, 2.5 );

        default_xs3.m_XDelta = 1.0;
        default_xs3.GetXSecCurve()->SetWidthHeight( 3.0, 2.5 );

        default_xs4.m_XDelta = 0.5;
        default_xs4.m_TopLAngle = -45.0;
        default_xs4.m_TopLStrength = 0.75;
        default_xs4.m_RightLAngle = -45.0;
        default_xs4.m_RightLStrength = 0.75;

        m_OrderPolicy.Set( STACK_FREE );
        m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
        m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
        stack_xs = { &(default_xs0),
                        &(default_xs1),
                        &(default_xs2),
                        &(default_xs3),
                        &(default_xs4), };
        InitXSecs( stack_xs );

        m_EngineInLipIndex   = 0;
        m_EngineInFaceIndex  = 0;
        m_EngineOutFaceIndex = 0;
        m_EngineOutLipIndex  = 0;

        m_EngineGeomIOType = ENGINE_GEOM_NONE;
        m_EngineGeomInType = ENGINE_GEOM_FLOWTHROUGH;
        m_EngineGeomOutType = ENGINE_GEOM_TO_LIP;
        m_EngineInModeType = ENGINE_MODE_TO_LIP;
        m_EngineOutModeType = ENGINE_MODE_TO_LIP;
    }
    else if ( stack_type >= STACK_PRESET_CYLINDER_ENDPTS &&  stack_type <= STACK_PRESET_CYLINDER_ENDCAPS )
    {
        // Cylinder StackXSec
        StackXSec cyl_face_a = StackXSec( new CircleXSec() );
        StackXSec cyl_face_b = StackXSec( new CircleXSec() );
        StackXSec end_point = StackXSec( new PointXSec() );

        end_point.m_TopLAngle = -90.0;
        end_point.m_RightLAngle = -90.0;
        end_point.m_TopRAngle = 90.0;
        end_point.m_RightRAngle = 90.0;

        cyl_face_a.ClearSkinning();
        cyl_face_a.GetXSecCurve()->SetWidthHeight( 2.5, 2.5 );

        cyl_face_b.CopyFrom( &(cyl_face_a) );
        cyl_face_b.m_XDelta = 3.0;

        switch ( stack_type )
        {
            case STACK_PRESET_CYLINDER_ENDPTS:
            {
                end_point.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(cyl_face_a),
                            &(cyl_face_b),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 0;
                m_EngineInFaceIndex  = 0;
                m_EngineOutFaceIndex = 0;
                m_EngineOutLipIndex  = 0;

                m_EngineGeomIOType = ENGINE_GEOM_NONE;
                m_EngineGeomInType = ENGINE_GEOM_FLOWTHROUGH;
                m_EngineGeomOutType = ENGINE_GEOM_TO_LIP;
                m_EngineInModeType = ENGINE_MODE_TO_LIP;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_CYLINDER_ENDCAPS:
            {
                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::FLAT_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::FLAT_END_CAP );
                stack_xs = { &(cyl_face_a),
                            &(cyl_face_b),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 0;
                m_EngineInFaceIndex  = 0;
                m_EngineOutFaceIndex = 0;
                m_EngineOutLipIndex  = 0;

                m_EngineGeomIOType = ENGINE_GEOM_NONE;
                m_EngineGeomInType = ENGINE_GEOM_FLOWTHROUGH;
                m_EngineGeomOutType = ENGINE_GEOM_TO_LIP;
                m_EngineInModeType = ENGINE_MODE_TO_LIP;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
        }
    }
    else
    {
        // Engine Flowpath StackXSecs
        StackXSec end_point = StackXSec( new PointXSec() );
        StackXSec nacelle_outlet_lip = StackXSec( new CircleXSec() );
        StackXSec nacelle_outlet_face = StackXSec( new CircleXSec() );
        StackXSec nacelle_midplane = StackXSec( new CircleXSec() );
        StackXSec nacelle_inlet_face = StackXSec( new CircleXSec() );
        StackXSec nacelle_inlet_lip = StackXSec( new CircleXSec() );
        StackXSec nacelle_outer = StackXSec( new CircleXSec() );

        end_point.m_TopLAngle = -90.0;
        end_point.m_RightLAngle = -90.0;
        end_point.m_TopRAngle = 90.0;
        end_point.m_RightRAngle = 90.0;

        nacelle_outlet_lip.m_XDelta = 3.0;
        nacelle_outlet_lip.m_AllSymFlag = 1;
        nacelle_outlet_lip.m_TopLAngle = -15.0;
        nacelle_outlet_lip.m_TopLStrength = 0.25;
        nacelle_outlet_lip.m_TopLRAngleEq = false;
        nacelle_outlet_lip.m_TopRAngle = -165.0;
        nacelle_outlet_lip.GetXSecCurve()->SetWidthHeight( 3.0, 3.0 );

        nacelle_outlet_face.m_XDelta = -2.5;
        nacelle_outlet_face.m_AllSymFlag = 1;
        nacelle_outlet_face.m_TopLAngle = -180.0;
        nacelle_outlet_face.GetXSecCurve()->SetWidthHeight( 2.0, 2.0 );

        nacelle_midplane.m_XDelta = -0.75;
        nacelle_midplane.m_AllSymFlag = 1;
        nacelle_midplane.m_TopLAngle = -180.0;
        nacelle_midplane.GetXSecCurve()->SetWidthHeight( 2.0, 2.0 );

        nacelle_inlet_face.m_XDelta = -0.75;
        nacelle_inlet_face.m_AllSymFlag = 1;
        nacelle_inlet_face.m_TopLAngle = -180.0;
        nacelle_inlet_face.GetXSecCurve()->SetWidthHeight( 2.0, 2.0 );

        nacelle_inlet_lip.m_XDelta = -2.0;
        nacelle_inlet_lip.m_AllSymFlag = 1;
        nacelle_inlet_lip.m_TopLRStrengthEq = true;
        nacelle_inlet_lip.m_TopLAngle = 90.0;
        nacelle_inlet_lip.m_TopLStrength = 0.55;
        nacelle_inlet_lip.m_TopRAngle = 90.0;
        nacelle_inlet_lip.m_TopRStrength = 0.55;
        nacelle_inlet_lip.GetXSecCurve()->SetWidthHeight( 3.0, 3.0 );

        nacelle_outer.m_XDelta = 3.0;
        nacelle_outer.GetXSecCurve()->SetWidthHeight( 4.0, 4.0 );

        switch ( stack_type )
        {
            case STACK_PRESET_FLOWTHRU_OUTLIP_ORIG:
            {
                m_OrderPolicy.Set( STACK_LOOP );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(nacelle_outlet_lip),
                            &(nacelle_outlet_face),
                            &(nacelle_midplane),
                            &(nacelle_inlet_face),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 4;
                m_EngineInFaceIndex  = 3;
                m_EngineOutFaceIndex = 1;
                m_EngineOutLipIndex  = 0;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_FLOWTHROUGH;
                m_EngineInModeType = ENGINE_MODE_FLOWTHROUGH_NEG;

                break;
            }
            case STACK_PRESET_FLOWTHRU_INLIP_ORIG:
            {
                m_OrderPolicy.Set( STACK_LOOP );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(nacelle_outlet_face),
                            &(nacelle_midplane),
                            &(nacelle_inlet_face),
                            &(nacelle_inlet_lip),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 0;
                m_EngineInFaceIndex  = 5;
                m_EngineOutFaceIndex = 3;
                m_EngineOutLipIndex  = 2;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_FLOWTHROUGH;
                m_EngineInModeType = ENGINE_MODE_FLOWTHROUGH_NEG;

                break;
            }
            case STACK_PRESET_FLOWTHRU_MID_ORIG:
            {
                m_OrderPolicy.Set( STACK_LOOP );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(nacelle_midplane),
                            &(nacelle_inlet_face),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(nacelle_outlet_face),
                            &(nacelle_midplane),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 2;
                m_EngineInFaceIndex  = 1;
                m_EngineOutFaceIndex = 5;
                m_EngineOutLipIndex  = 4;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_FLOWTHROUGH;
                m_EngineInModeType = ENGINE_MODE_FLOWTHROUGH_NEG;

                break;
            }
            case STACK_PRESET_BOTHFACEFACE:
            {
                // Modify ref xsecs delta, skinning
                nacelle_inlet_face.m_XDelta = 0.0;

                nacelle_inlet_face.ClearSkinning();
                nacelle_outlet_face.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_face),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(nacelle_outlet_face),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 2;
                m_EngineInFaceIndex  = 1;
                m_EngineOutFaceIndex = 5;
                m_EngineOutLipIndex  = 4;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_TO_FACE;
                m_EngineGeomOutType = ENGINE_GEOM_TO_FACE;
                m_EngineInModeType = ENGINE_MODE_TO_FACE;
                m_EngineOutModeType = ENGINE_MODE_TO_FACE;

                break;
            }
            case STACK_PRESET_BOTHLIPFACE:
            {
                // Modify ref xsecs delta, skinning
                nacelle_inlet_lip.m_XDelta = 0.0;

                nacelle_outlet_face.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(nacelle_outlet_face),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 1;
                m_EngineOutFaceIndex = 4;
                m_EngineOutLipIndex  = 3;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_TO_LIP;
                m_EngineGeomOutType = ENGINE_GEOM_TO_FACE;
                m_EngineInModeType = ENGINE_MODE_TO_LIP;
                m_EngineOutModeType = ENGINE_MODE_TO_FACE;

                break;
            }
            case STACK_PRESET_BOTHFACELIP:
            {
                // Modify ref xsecs delta, skinning
                nacelle_inlet_face.m_XDelta = 0.0;

                nacelle_inlet_face.ClearSkinning();
                nacelle_outlet_lip.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_face),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 2;
                m_EngineInFaceIndex  = 1;
                m_EngineOutLipIndex  = 4;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_TO_FACE;
                m_EngineGeomOutType = ENGINE_GEOM_TO_LIP;
                m_EngineInModeType = ENGINE_MODE_TO_FACE;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_BOTHLIPLIP:
            {
                // Modify ref xsecs delta, skinning
                nacelle_inlet_lip.m_XDelta = 0.0;

                nacelle_outlet_lip.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 1;
                m_EngineOutLipIndex  = 3;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_TO_LIP;
                m_EngineGeomOutType = ENGINE_GEOM_TO_LIP;
                m_EngineInModeType = ENGINE_MODE_TO_LIP;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_OUTFACE:
            {
                // Modify ref xsecs delta, skinning
                nacelle_inlet_lip.m_XDelta = 0.0;

                nacelle_outlet_face.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(nacelle_outlet_face),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineOutFaceIndex = 3;
                m_EngineOutLipIndex  = 2;

                m_EngineGeomIOType = ENGINE_GEOM_OUTLET;
                m_EngineGeomOutType = ENGINE_GEOM_TO_FACE;
                m_EngineOutModeType = ENGINE_MODE_TO_FACE;

                break;
            }
            case STACK_PRESET_OUTLIP:
            {
                // Modify ref xsecs delta, skinning
                nacelle_outlet_face.m_AllSymFlag = 1;

                nacelle_outlet_lip.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_outer),
                            &(nacelle_outlet_lip),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineOutLipIndex  = 2;

                m_EngineGeomIOType = ENGINE_GEOM_OUTLET;
                m_EngineGeomOutType = ENGINE_GEOM_TO_LIP;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_INFACE:
            {
                // Modify ref xsecs delta, skinning
                end_point.m_XDelta = 3.0;
                nacelle_inlet_face.m_XDelta = 0.0;

                nacelle_inlet_face.ClearSkinning();

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_face),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex  = 2;
                m_EngineInFaceIndex = 1;

                m_EngineGeomIOType = ENGINE_GEOM_INLET;
                m_EngineGeomInType = ENGINE_GEOM_TO_FACE;
                m_EngineInModeType = ENGINE_MODE_TO_FACE;

                break;
            }
            case STACK_PRESET_INLIP:
            {
                // Modify ref xsecs delta, skinning
                end_point.m_XDelta = 3.0;
                nacelle_inlet_lip.m_XDelta = 0.0;

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_lip),
                            &(nacelle_outer),
                            &(end_point),};
                InitXSecs( stack_xs );

                m_EngineInLipIndex = 1;

                m_EngineGeomIOType = ENGINE_GEOM_INLET;
                m_EngineGeomInType = ENGINE_GEOM_INLET;
                m_EngineInModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_FLOWPATH_BOTH:
            {
                // reverse order of xsecs means an inverse offset of XDeltas
                nacelle_outlet_lip.m_XDelta = -nacelle_outlet_face.m_XDelta();
                nacelle_outlet_face.m_XDelta = -nacelle_midplane.m_XDelta();
                nacelle_midplane.m_XDelta = -nacelle_inlet_face.m_XDelta();
                nacelle_inlet_face.m_XDelta = -nacelle_inlet_lip.m_XDelta();
                nacelle_inlet_lip.m_XDelta = 0.0;

                // flip skinning due to reversal of usual xsec order
                nacelle_inlet_lip.m_TopLRAngleEq = false;
                nacelle_inlet_lip.m_RightLRAngleEq = false;
                nacelle_inlet_lip.m_TopRAngle = -nacelle_inlet_lip.m_TopRAngle();
                nacelle_inlet_lip.m_RightRAngle = -nacelle_inlet_lip.m_RightRAngle();

                nacelle_inlet_face.m_TopLAngle = 0.0;
                nacelle_midplane.m_TopLAngle = 0.0;
                nacelle_outlet_face.m_TopLAngle = 0.0;

                nacelle_outlet_lip.m_TopLAngle = 15.0;
                nacelle_outlet_lip.m_TopRAngle = -90.0;

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_lip),
                            &(nacelle_inlet_face),
                            &(nacelle_midplane),
                            &(nacelle_outlet_face),
                            &(nacelle_outlet_lip),
                            &(end_point),};

                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 1;
                m_EngineInFaceIndex  = 2;
                m_EngineOutFaceIndex = 4;
                m_EngineOutLipIndex  = 5;

                m_EngineGeomIOType = ENGINE_GEOM_INLET_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_FLOWPATH;
                m_EngineGeomOutType = ENGINE_GEOM_TO_FACE;
                m_EngineInModeType = ENGINE_MODE_FLOWTHROUGH_NEG;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_FLOWPATH_IN:
            {
                // reverse order of xsecs means an inverse offset of XDeltas
                nacelle_inlet_face.m_XDelta = -nacelle_inlet_lip.m_XDelta();
                nacelle_inlet_lip.m_XDelta = 0.0;

                // flip skinning due to reversal of usual xsec order
                nacelle_inlet_lip.m_TopLRAngleEq = false;
                nacelle_inlet_lip.m_TopRAngle = -nacelle_inlet_lip.m_TopRAngle();

                nacelle_inlet_face.m_TopLRAngleEq = false;
                nacelle_inlet_face.m_TopLAngle = 0.0;
                nacelle_inlet_face.m_TopRAngle = -90.0;

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_inlet_lip),
                            &(nacelle_inlet_face),
                            &(end_point),};

                InitXSecs( stack_xs );

                m_EngineInLipIndex   = 1;
                m_EngineInFaceIndex  = 2;

                m_EngineGeomIOType = ENGINE_GEOM_INLET;
                m_EngineGeomInType = ENGINE_GEOM_FLOWPATH;
                m_EngineGeomOutType = ENGINE_GEOM_TO_FACE;
                m_EngineInModeType = ENGINE_MODE_FLOWTHROUGH_NEG;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
            case STACK_PRESET_FLOWPATH_OUT:
            {
                // reverse order of xsecs means an inverse offset of XDeltas
                nacelle_outlet_lip.m_XDelta = -nacelle_outlet_face.m_XDelta();
                nacelle_outlet_face.m_XDelta = 0.0;

                // flip skinning due to reversal of usual xsec order
                nacelle_outlet_face.m_TopLRAngleEq = false;
                nacelle_outlet_face.m_TopLAngle = 90.0;
                nacelle_outlet_face.m_TopRAngle = 0.0;

                nacelle_outlet_lip.m_TopLAngle = 15.0;
                nacelle_outlet_lip.m_TopRAngle = -90.0;

                m_OrderPolicy.Set( STACK_FREE );
                m_CapUMinOption.Set( CAP_TYPE::NO_END_CAP );
                m_CapUMaxOption.Set( CAP_TYPE::NO_END_CAP );
                stack_xs = { &(end_point),
                            &(nacelle_outlet_face),
                            &(nacelle_outlet_lip),
                            &(end_point),};

                InitXSecs( stack_xs );

                m_EngineOutFaceIndex = 1;
                m_EngineOutLipIndex  = 2;

                m_EngineGeomIOType = ENGINE_GEOM_OUTLET;
                m_EngineGeomInType = ENGINE_GEOM_FLOWPATH;
                m_EngineGeomOutType = ENGINE_GEOM_TO_FACE;
                m_EngineInModeType = ENGINE_MODE_FLOWTHROUGH_NEG;
                m_EngineOutModeType = ENGINE_MODE_TO_LIP;

                break;
            }
        }
    }
    Update();
}

void StackGeom::InitXSecs( vector < StackXSec* > stack_xs )
{
    for ( int i = 0; i < stack_xs.size() ; i++ )
    {
        m_XSecSurf.AddXSecCopy( stack_xs[i] );
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );
        xs->SetGroupDisplaySuffix( i );

        if ( i == 0 )
        {
            xs->FlipLRSkinning();
        }
    }
    // ValidateParms to update index parm upper limits
    ValidateParms();
}

//==== Destructor ====//
StackGeom::~StackGeom()
{

}

void StackGeom::ChangeID( const string &id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

void StackGeom::UpdatePreTess()
{
    m_TessUVec.clear();
    m_FwdClusterVec.clear();
    m_AftClusterVec.clear();

    unsigned int nxsec = m_XSecSurf.NumXSec();

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            if ( i > 0 )
            {
                m_TessUVec.push_back( xs->m_SectTessU() );
                m_FwdClusterVec.push_back( xs->m_FwdCluster() );
                m_AftClusterVec.push_back( xs->m_AftCluster() );
            }
        }
    }
}

//==== Update Fuselage And Cross Section Placement ====//
void StackGeom::UpdateSurf()
{
    unsigned int nxsec = m_XSecSurf.NumXSec();

    if ( m_OrderPolicy() == STACK_LOOP )
    {
        StackXSec* first_xs = (StackXSec*) m_XSecSurf.FindXSec( 0 );
        StackXSec* last_xs = (StackXSec*) m_XSecSurf.FindXSec( nxsec - 1 );

        if ( first_xs && last_xs )
        {
            if ( last_xs->GetXSecCurve()->GetType() != first_xs->GetXSecCurve()->GetType() )
            {
                m_XSecSurf.ChangeXSecShape( nxsec - 1, first_xs->GetXSecCurve()->GetType() );
                last_xs = (StackXSec*) m_XSecSurf.FindXSec( nxsec - 1 );
            }

            if( last_xs )
            {
                last_xs->GetXSecCurve()->CopyFrom( first_xs->GetXSecCurve() );
            }
        }
    }

    //==== Cross Section Curves & joint info ====//
    vector< rib_data_type > rib_vec;
    rib_vec.resize( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            EnforceOrder( xs, i, m_OrderPolicy() );

            bool first = false;
            bool last = false;

            if( i == 0 ) first = true;
            else if( i == (nxsec-1) ) last = true;

            //==== Reset Group Names ====//
            xs->SetGroupDisplaySuffix( i );

            rib_vec[i] = xs->GetRib( first, last );
        }
    }


    m_MainSurfVec[0].SkinRibs( rib_vec, false );
    m_MainSurfVec[0].SetMagicVParm( false );

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetUnsetParms( i, m_MainSurfVec[0] );
        }
    }

    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }
}

void StackGeom::GetUWTess( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< double > &utess, vector< double > &vtess, const int & n_ref ) const
{
    vector < int > tessvec;
    vector < double > fwdc;
    vector < double > aftc;

    if (m_CapUMinOption()!=NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        fwdc.push_back( 1.0 );
        aftc.push_back( 1.0 );
    }

    for ( int i = 0; i < m_TessUVec.size(); i++ )
    {
        tessvec.push_back( m_TessUVec[i] );
        fwdc.push_back( m_FwdClusterVec[i] );
        aftc.push_back( m_AftClusterVec[i] );
    }

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        fwdc.push_back( 1.0 );
        aftc.push_back( 1.0 );
    }

    std::vector<int> umerge = std::vector<int>();
    surf.SetRootTipClustering( fwdc, aftc );
    surf.GetUWTess( utess, vtess, tessvec, m_TessW(), m_CapUMinTess(), m_TessU(), degen, umerge, n_ref );
}

void StackGeom::UpdateTesselate( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess, bool degen, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, const int & n_ref ) const
{
    vector < int > tessvec;
    vector < double > fwdc;
    vector < double > aftc;

    if (m_CapUMinOption()!=NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        fwdc.push_back( 1.0 );
        aftc.push_back( 1.0 );
    }

    for ( int i = 0; i < m_TessUVec.size(); i++ )
    {
        tessvec.push_back( m_TessUVec[i] );
        fwdc.push_back( m_FwdClusterVec[i] );
        aftc.push_back( m_AftClusterVec[i] );
    }

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        fwdc.push_back( 1.0 );
        aftc.push_back( 1.0 );
    }

    std::vector<int> umerge = std::vector<int>();
    surf.SetRootTipClustering( fwdc, aftc );
    surf.Tesselate( tessvec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), m_TessU(), degen, umerge, n_ref );
}

void StackGeom::UpdateSplitTesselate( const VspSurf &surf, bool
                                      capUMinSuccess, bool capUMaxSuccess, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms ) const
{
    vector < int > tessvec;
    vector < double > fwdc;
    vector < double > aftc;

    if (m_CapUMinOption()!=NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        fwdc.push_back( 1.0 );
        aftc.push_back( 1.0 );
    }

    for ( int i = 0; i < m_TessUVec.size(); i++ )
    {
        tessvec.push_back( m_TessUVec[i] );
        fwdc.push_back( m_FwdClusterVec[i] );
        aftc.push_back( m_AftClusterVec[i] );
    }

    if (m_CapUMaxOption()!=NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
        fwdc.push_back( 1.0 );
        aftc.push_back( 1.0 );
    }

    surf.SetRootTipClustering( fwdc, aftc );
    surf.SplitTesselate( tessvec, m_TessW(), pnts, norms, m_CapUMinTess(), m_TessU() );
}


//==== Compute Rotation Center ====//
void StackGeom::ComputeCenter()
{
    m_Center.set_x( 0.0 );
    m_Center.set_y( 0.0 );
    m_Center.set_z( 0.0 );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr StackGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr fuselage_node = xmlNewChild( node, nullptr, BAD_CAST "FuselageGeom", nullptr );
    if ( fuselage_node )
    {
        m_XSecSurf.EncodeXml( fuselage_node );
    }
    return fuselage_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr StackGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr fuselage_node = XmlUtil::GetNode( node, "FuselageGeom", 0 );
    if ( fuselage_node )
    {
        m_XSecSurf.DecodeXml( fuselage_node );
    }

    return fuselage_node;
}

//==== Override Geom Cut/Copy/Paste/Insert ====//
void StackGeom::CutXSec( int index )
{
    m_XSecSurf.CutXSec( index );
    // Set up flag so Update() knows to regenerate surface.
    // Insert / split cases don't need this because Parms are added,
    // which implicitly triggers this flag.
    // However, cut deletes Parms - requiring an explicit flag.
    m_SurfDirty = true;
    Update();
}
void StackGeom::CopyXSec( int index )
{
    m_XSecSurf.CopyXSec( index );
}
void StackGeom::PasteXSec( int index )
{
    m_XSecSurf.PasteXSec( index );
    Update();
}
void StackGeom::InsertXSec( int index, int type )
{
    m_ActiveXSec = index;
    InsertXSec( type );
}


//==== Cut Active XSec ====//
void StackGeom::CutActiveXSec()
{
    CutXSec( m_ActiveXSec() );
    //m_XSecSurf.CutXSec( m_ActiveXSec );
    //SetActiveXSecIndex( GetActiveXSecIndex() );
    //m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    //Update();
}

//==== Copy Active XSec ====//
void StackGeom::CopyActiveXSec()
{
    CopyXSec( m_ActiveXSec() );
//    m_XSecSurf.CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void StackGeom::PasteActiveXSec()
{
    PasteXSec( m_ActiveXSec() );
    //m_XSecSurf.PasteXSec( m_ActiveXSec );
    //m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    //Update();
}

//==== Insert XSec ====//
void StackGeom::InsertXSec( )
{
    int xsec_lim = NumXSec() - 1; // STACK_LOOP
    if ( m_OrderPolicy() == STACK_FREE )
    {
        xsec_lim = NumXSec();
    }

    if ( m_ActiveXSec() >= xsec_lim || m_ActiveXSec() < 0 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec() );
    if ( xs )
    {
        InsertXSec( xs->GetXSecCurve()->GetType() );
    }
}

//==== Insert XSec ====//
void StackGeom::InsertXSec( int type )
{
    int xsec_lim = NumXSec() - 1; // STACK_LOOP
    if ( m_OrderPolicy() == STACK_FREE )
    {
        xsec_lim = NumXSec();
    }

    if ( m_ActiveXSec() >= xsec_lim || m_ActiveXSec() < 0 )
    {
        return;
    }

    StackXSec* xs = ( StackXSec* ) GetXSec( m_ActiveXSec() );

    m_XSecSurf.InsertXSec( type, m_ActiveXSec() );
    m_ActiveXSec = m_ActiveXSec() + 1;

    StackXSec* inserted_xs = ( StackXSec* ) GetXSec( m_ActiveXSec() );

    if ( inserted_xs )
    {
        inserted_xs->CopyFrom( xs );

        if ( std::abs(inserted_xs->m_XDelta()) < 0.0000001 )
            inserted_xs->m_XDelta = 1.0;

        inserted_xs->SetLateUpdateFlag( true );
    }
    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void StackGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void StackGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetScale( currentScale );
        }
    }

    if ( !m_AutoExtensionFlag() )
    {
        m_ExtensionDistance.Set( m_ExtensionDistance() * currentScale );
    }

    m_LastScale = m_Scale();
}

void StackGeom::AddDefaultSources( double base_len )
{
    switch ( m_OrderPolicy() )
    {
    case STACK_FREE:
    {

        StackXSec* lastxs = (StackXSec*) m_XSecSurf.FindXSec( m_XSecSurf.NumXSec() - 1);
        if( lastxs )
        {
            Matrix4d prevxform;
            prevxform.loadIdentity();

            prevxform.matMult( lastxs->GetTransform()->data() );

            prevxform.affineInverse();
            vec3d offset = prevxform.xform( vec3d( 0.0, 0.0, 0.0 ) );

            double len = offset.mag();

            AddDefaultSourcesXSec( base_len, len, 0 );
            AddDefaultSourcesXSec( base_len, len, m_XSecSurf.NumXSec() - 1 );
        }


        break;
    }
    case STACK_LOOP:
    {
        int iback = -1;
        double dfront = -1.0;

        for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
        {
            StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );
            if ( xs )
            {
                Matrix4d prevxform;
                prevxform.loadIdentity();

                prevxform.matMult( xs->GetTransform()->data() );

                prevxform.affineInverse();
                vec3d offset = prevxform.xform( vec3d( 0.0, 0.0, 0.0 ) );

                double len = offset.mag();

                if ( len > dfront )
                {
                    dfront = len;
                    iback = i;
                }

            }
        }

        AddDefaultSourcesXSec( base_len, dfront, 0 );
        if ( iback >= 0 )
        {
            AddDefaultSourcesXSec( base_len, dfront, iback );
        }

        break;
    }
    }
}

bool StackGeom::IsClosed() const
{
    return m_Closed;
}


void StackGeom::EnforceOrder( StackXSec* xs, int indx, int policy )
{
    int nxsec = m_XSecSurf.NumXSec();

    bool first = false;
    bool last = false;
    bool nextlast = false;

    if( indx == 0 ) first = true;
    else if( indx == (nxsec-1) ) last = true;
    else if( indx == (nxsec-2) ) nextlast = true;

    // STACK_FREE implicit.
    if ( first )
    {
        xs->m_XDelta.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_YDelta.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_ZDelta.SetLowerUpperLimits( 0.0, 0.0 );

        xs->m_XRotate.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_YRotate.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_ZRotate.SetLowerUpperLimits( 0.0, 0.0 );
    }
    else
    {
        xs->m_XDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );
        xs->m_YDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );
        xs->m_ZDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );

        xs->m_XRotate.SetLowerUpperLimits( -180.0, 180.0 );
        xs->m_YRotate.SetLowerUpperLimits( -180.0, 180.0 );
        xs->m_ZRotate.SetLowerUpperLimits( -180.0, 180.0 );
    }

    if( policy == STACK_LOOP )
    {
        if ( last )
        {
            StackXSec* prevxs = (StackXSec*) m_XSecSurf.FindXSec( indx - 1);
            if( prevxs )
            {
                Matrix4d prevxform;
                prevxform.loadIdentity();

                prevxform.matMult( prevxs->GetTransform()->data() );

                prevxform.affineInverse();
                vec3d offset = prevxform.xform( vec3d( 0.0, 0.0, 0.0 ) );

                xs->m_XDelta.SetLowerUpperLimits( offset.x(), offset.x() );
                xs->m_YDelta.SetLowerUpperLimits( offset.y(), offset.y() );
                xs->m_ZDelta.SetLowerUpperLimits( offset.z(), offset.z() );

                xs->m_XDelta.Set( offset.x() );
                xs->m_YDelta.Set( offset.y() );
                xs->m_ZDelta.Set( offset.z() );

                vec3d angle = prevxform.getAngles();
                xs->m_XRotate.SetLowerUpperLimits( angle.x(), angle.x() );
                xs->m_YRotate.SetLowerUpperLimits( angle.y(), angle.y() );
                xs->m_ZRotate.SetLowerUpperLimits( angle.z(), angle.z() );

                xs->m_XRotate.Set( angle.x() );
                xs->m_YRotate.Set( angle.y() );
                xs->m_ZRotate.Set( angle.z() );
            }
        }
    }
}
