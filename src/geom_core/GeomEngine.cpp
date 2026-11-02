//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GeomEngine.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
using namespace vsp;

//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GeomEngine::GeomEngine( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_EngineGeomIOType.Init( "GeomIOType", "EngineModel", this, ENGINE_GEOM_NONE, ENGINE_GEOM_NONE, ENGINE_GEOM_IO_NUM_TYPES - 1 );
    m_EngineGeomInType.Init( "GeomInType", "EngineModel", this, ENGINE_GEOM_FLOWTHROUGH, ENGINE_GEOM_FLOWTHROUGH, ENGINE_GEOM_NUM_TYPES - 1 );
    m_EngineGeomOutType.Init( "GeomOutType", "EngineModel", this, ENGINE_GEOM_TO_LIP, ENGINE_GEOM_FLOWTHROUGH, ENGINE_GEOM_NUM_TYPES - 1 );

    m_EngineInFaceMode.Init( "InletFaceMode", "EngineModel", this, ENGINE_LOC_INDEX, ENGINE_LOC_INDEX, ENGINE_LOC_U );
    m_EngineInLipMode.Init( "InletLipMode", "EngineModel", this, ENGINE_LOC_INDEX, ENGINE_LOC_INDEX, ENGINE_LOC_U );
    m_EngineOutFaceMode.Init( "OutletFaceMode", "EngineModel", this, ENGINE_LOC_INDEX, ENGINE_LOC_INDEX, ENGINE_LOC_U );
    m_EngineOutLipMode.Init( "OutletLipMode", "EngineModel", this, ENGINE_LOC_INDEX, ENGINE_LOC_INDEX, ENGINE_LOC_U );

    m_EngineInFaceIndex.Init( "InletFaceIndex", "EngineModel", this, 0, 0, 1e6 );
    m_EngineInLipIndex.Init( "InletLipIndex", "EngineModel", this, 0, 0, 1e6 );
    m_EngineInFaceU.Init( "InletFaceU", "EngineModel", this, 0, 0, 1.0 );
    m_EngineInLipU.Init( "InletLipU", "EngineModel", this, 0, 0, 1.0 );

    m_EngineOutFaceIndex.Init( "OutletFaceIndex", "EngineModel", this, 0, 0, 1e6 );
    m_EngineOutLipIndex.Init( "OutletLipIndex", "EngineModel", this, 0, 0, 1e6 );
    m_EngineOutFaceU.Init( "OutletFaceU", "EngineModel", this, 0, 0, 1.0 );
    m_EngineOutLipU.Init( "OutletLipU", "EngineModel", this, 0, 0, 1.0 );

    m_EngineShowStationsFlag.Init( "EngineShowStationsFlag", "EngineModel", this, true, false, true );

    m_EngineInModeType.Init( "InletModeType", "EngineModel", this, ENGINE_MODE_TO_LIP, ENGINE_MODE_FLOWTHROUGH, ENGINE_MODE_NUM_TYPES - 1 );
    m_EngineOutModeType.Init( "OutletModeType", "EngineModel", this, ENGINE_MODE_TO_LIP, ENGINE_MODE_TO_LIP, ENGINE_MODE_NUM_TYPES - 1 );

    m_RotExtensionFlag.Init( "RotExtensionFlag", "EngineModel", this, false, false, true );
    m_ExtensionDistance.Init( "ExtensionDistance", "EngineModel", this, 10, 0, 1e12 );
    m_AutoExtensionSet.Init( "AutoExtensionSet", "EngineModel", this, DEFAULT_SET, 0, vsp::MAX_NUM_SETS );
    m_AutoExtensionFlag.Init( "AutoExtensionFlag", "EngineModel", this, false, false, true );

}

//==== Destructor ====//
GeomEngine::~GeomEngine()
{

}

void GeomEngine::ValidateParms()
{
    for ( int i = 0; i < ENGINE_LOC_NUM; i++ )
    {
        m_engine_spec[ i ] = false;
    }

    m_EngineGeomInType.Deactivate();
    m_EngineGeomOutType.Deactivate();

    m_EngineInFaceMode.Deactivate();
    m_EngineInLipMode.Deactivate();
    m_EngineInFaceIndex.Deactivate();
    m_EngineInLipIndex.Deactivate();
    m_EngineInFaceU.Deactivate();
    m_EngineInLipU.Deactivate();

    m_EngineOutFaceMode.Deactivate();
    m_EngineOutLipMode.Deactivate();
    m_EngineOutFaceIndex.Deactivate();
    m_EngineOutLipIndex.Deactivate();
    m_EngineOutFaceU.Deactivate();
    m_EngineOutLipU.Deactivate();

    m_EngineInModeType.Deactivate();
    m_EngineOutModeType.Deactivate();

    m_RotExtensionFlag.Deactivate();
    m_ExtensionDistance.Deactivate();
    m_AutoExtensionSet.Deactivate();
    m_AutoExtensionFlag.Deactivate();

    if ( m_EngineGeomIOType() != ENGINE_GEOM_NONE )
    {
        if ( m_EngineGeomIOType() <= ENGINE_GEOM_INLET_OUTLET ) // Includes inlet description
        {
            m_EngineGeomInType.Activate();
            m_EngineInModeType.Activate();

            // All need to enable lip variables
            m_engine_spec[ ENGINE_LOC_INLET_LIP ] = true;
            m_EngineInLipMode.Activate();

            if ( m_EngineInLipMode() == ENGINE_LOC_INDEX )
            {
                m_EngineInLipIndex.Activate();
            }
            else
            {
                m_EngineInLipU.Activate();
            }

            if ( m_EngineGeomInType() != ENGINE_GEOM_TO_LIP ) // Need to enable face variables
            {
                m_engine_spec[ ENGINE_LOC_INLET_FACE ] = true;
                m_EngineInFaceMode.Activate();

                if ( m_EngineInFaceMode() == ENGINE_LOC_INDEX )
                {
                    m_EngineInFaceIndex.Activate();
                }
                else
                {
                    m_EngineInFaceU.Activate();
                }
            }

            if ( m_EngineInModeType() == ENGINE_MODE_EXTEND )
            {
                m_RotExtensionFlag.Activate();
                m_AutoExtensionFlag.Activate();

                if ( m_AutoExtensionFlag() )
                {
                    m_AutoExtensionSet.Activate();
                }
                else
                {
                    m_ExtensionDistance.Activate();
                }
            }
        }

        if ( m_EngineGeomIOType() >= ENGINE_GEOM_INLET_OUTLET ) // Includes outlet description
        {
            if ( !( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && ( m_EngineGeomInType() == ENGINE_GEOM_FLOWTHROUGH || m_EngineGeomInType() == ENGINE_GEOM_FLOWPATH ) ) )
            {
                m_EngineGeomOutType.Activate();
            }
            m_EngineOutModeType.Activate();

            // All need to enable lip variables
            m_engine_spec[ ENGINE_LOC_OUTLET_LIP ] = true;
            m_EngineOutLipMode.Activate();

            if ( m_EngineOutLipMode() == ENGINE_LOC_INDEX )
            {
                m_EngineOutLipIndex.Activate();
            }
            else
            {
                m_EngineOutLipU.Activate();
            }

            if ( m_EngineGeomOutType() != ENGINE_GEOM_TO_LIP || // Need to enable face variables
                 ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineGeomInType() == ENGINE_GEOM_FLOWTHROUGH ) )
            {
                m_engine_spec[ ENGINE_LOC_OUTLET_FACE ] = true;
                m_EngineOutFaceMode.Activate();

                if ( m_EngineOutFaceMode() == ENGINE_LOC_INDEX )
                {
                    m_EngineOutFaceIndex.Activate();
                }
                else
                {
                    m_EngineOutFaceU.Activate();
                }
            }

            if ( m_EngineOutModeType() == ENGINE_MODE_EXTEND )
            {
                m_RotExtensionFlag.Activate();
                m_AutoExtensionFlag.Activate();

                if ( m_AutoExtensionFlag() )
                {
                    m_AutoExtensionSet.Activate();
                }
                else
                {
                    m_ExtensionDistance.Activate();
                }
            }
        }
    }

    // A flowthrough inlet mode sets the outlet too, but not on a flowpath.
    if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET &&
         m_EngineGeomInType() != ENGINE_GEOM_FLOWPATH && m_EngineGeomOutType() != ENGINE_GEOM_FLOWPATH &&
         ( m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH || m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG || m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY ) )
    {
        m_EngineOutModeType.Deactivate();
    }

    m_EngineInFaceIndex.SetUpperLimit( m_XSecSurf.NumXSec() - 1 );
    if ( m_EngineInFaceMode() == ENGINE_LOC_INDEX )
    {
        m_EngineInFaceU = ( ( double ) m_EngineInFaceIndex() ) / ( ( double ) ( m_XSecSurf.NumXSec() - 1.0 ) );
    }

    m_EngineInLipIndex.SetUpperLimit( m_XSecSurf.NumXSec() - 1 );
    if ( m_EngineInLipMode() == ENGINE_LOC_INDEX )
    {
        m_EngineInLipU = ( ( double ) m_EngineInLipIndex() ) / ( ( double ) ( m_XSecSurf.NumXSec() - 1.0 ) );
    }

    m_EngineOutFaceIndex.SetUpperLimit( m_XSecSurf.NumXSec() - 1 );
    if ( m_EngineOutFaceMode() == ENGINE_LOC_INDEX )
    {
        m_EngineOutFaceU = ( ( double ) m_EngineOutFaceIndex() ) / ( ( double ) ( m_XSecSurf.NumXSec() - 1.0 ) );
    }

    m_EngineOutLipIndex.SetUpperLimit( m_XSecSurf.NumXSec() - 1 );
    if ( m_EngineOutLipMode() == ENGINE_LOC_INDEX )
    {
        m_EngineOutLipU = ( ( double ) m_EngineOutLipIndex() ) / ( ( double ) ( m_XSecSurf.NumXSec() - 1.0 ) );
    }
}

void GeomEngine::Update( bool fullupdate )
{
    ValidateParms();

    GeomXSec::Update( fullupdate );
}

void GeomEngine::UpdateFlags()
{
    // Engine settings not used, update flags as normal to reflect m_NegativeVolumeFlag.
    if ( m_EngineGeomIOType() == ENGINE_GEOM_NONE )
    {
        Geom::UpdateFlags();
    }
    // Engine settings used, ignore m_NegativeVolumeFlag and use engine definitions.
}

double GeomEngine::roll_t( const double &torig, const double &troll, const double &tmin, const double &tmax )
{
    double t = torig - troll;
    if ( t < tmin )
    {
        t = t + ( tmax - tmin );
    }
    return t;
}

double GeomEngine::unroll_t( const double &t, const double &troll, const double &tmin, const double &tmax )
{
    double torig = t + troll;
    if ( torig > tmax )
    {
        torig = torig - ( tmax - tmin );
    }
    return torig;
}

void GeomEngine::Extend( VspSurf &surf, const double & u, bool extbefore )
{
    // Handle extension.
    vector < VspCurve > extcrvs(2);
    surf.GetUConstCurve( extcrvs[0], u );
    extcrvs[1] = extcrvs[0];

    vec3d xdir = vec3d( 1., 0. ,0. );

    if ( m_RotExtensionFlag() )
    {
        Matrix4d model_matrix;
        model_matrix = GetShapeMatrix();
        model_matrix.affineInverse();
        vec3d ydir, zdir;
        model_matrix.getBasis( xdir, ydir, zdir );

        // A flip that turns the shape round in x turns the face round with it.
        if ( GetFlipMat().xformnorm( vec3d( 1.0, 0.0, 0.0 ) ).x() < 0.0 )
        {
            xdir = -1.0 * xdir;
        }
    }

    if ( extbefore )
    {
        extcrvs[0].Offset( -m_ExtensionDistance() * xdir );
    }
    else
    {
        extcrvs[1].Offset( m_ExtensionDistance() * xdir );
    }

    vector < double > extparam = { 0, 1.0 };

    VspSurf ext;
    ext.SkinC0Uniform( extcrvs, extparam, false );
    ext.InitUMapping( -2 );

    VspSurf s;
    if ( extbefore )
    {
        s.JoinU( ext, surf );
    }
    else
    {
        s.JoinU( surf, ext );
    }
    surf = s;
}

void GeomEngine::UpdateMainBBox()
{
    Geom::UpdateMainBBox();

    // The extension is sized from the model box, so only the surface it was grown from counts.
    m_ScaleIndependentMainBBox = m_UnextendedMainBBox;
}

void GeomEngine::UpdateXForm()
{
    GeomXSec::UpdateXForm();

    // m_XFormDirty implied because we're in UpdateXForms()
    if ( m_EngineGeomIOType() != ENGINE_GEOM_NONE
        && ( ( m_EngineGeomIOType() <= ENGINE_GEOM_INLET_OUTLET // Includes inlet description
            && m_EngineInModeType() == ENGINE_MODE_EXTEND )
        ||   ( m_EngineGeomIOType() >= ENGINE_GEOM_INLET_OUTLET  // Includes outlet description
            && m_EngineOutModeType() == ENGINE_MODE_EXTEND ) )
        && m_RotExtensionFlag() )
    {
        m_SurfDirty = true;
    }
}

void GeomEngine::UpdateEngine()
{
    // Only the caps the engine adds below are cut apart for the CFD mesh.
    m_MainSurfVec[0].ClearCFDCaps();

    m_UnextendedMainBBox.Reset();
    m_MainSurfVec[0].GetBoundingBox( m_UnextendedMainBBox );

    if ( m_EngineGeomIOType() != ENGINE_GEOM_NONE )
    {

        if ( m_AutoExtensionFlag() )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();
            if ( veh )
            {
                BndBox setBox;
                if ( veh->GetScaleIndependentBBoxSet( m_AutoExtensionSet(), setBox ) )
                {
                    m_ExtensionDistance = setBox.GetMax( 0 ) - setBox.GetMin( 0 );
                }
            }
        }

        // The trim, roll, cap, and extend operations below carry this mapping along, so the
        // tessellation can relate each section of the engine surface to a section of the original.
        m_MainSurfVec[0].InitUMapping();

        m_OrigSurf = m_MainSurfVec[0];
        m_OrigSurf.BuildFeatureLines( m_ForceXSecFlag );

        // A mode the GUI greys out for an end is taken as the one it offers there.  A flowthrough
        // mode on an inlet and outlet modeled to the lip or the face asks each end for the mode to
        // the face it implies.  An end modeled to its lip has no face, so a mode to the face closes
        // it at the lip, and one that keeps only the negative recess keeps nothing.
        int inmode = m_EngineInModeType();
        int outmode = m_EngineOutModeType();

        bool inend = m_EngineGeomIOType() <= ENGINE_GEOM_INLET_OUTLET;
        bool outend = m_EngineGeomIOType() >= ENGINE_GEOM_INLET_OUTLET;
        bool inlip = inend && m_EngineGeomInType() == ENGINE_GEOM_TO_LIP;
        bool outlip = outend && m_EngineGeomOutType() == ENGINE_GEOM_TO_LIP &&
                      !( inend && m_EngineGeomInType() == ENGINE_GEOM_FLOWTHROUGH );

        if ( inend && outend &&
             ( m_EngineGeomInType() == ENGINE_GEOM_TO_LIP || m_EngineGeomInType() == ENGINE_GEOM_TO_FACE ) &&
             ( m_EngineGeomOutType() == ENGINE_GEOM_TO_LIP || m_EngineGeomOutType() == ENGINE_GEOM_TO_FACE ) &&
             ( inmode == ENGINE_MODE_FLOWTHROUGH || inmode == ENGINE_MODE_FLOWTHROUGH_NEG ||
               inmode == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY ) )
        {
            outmode = ENGINE_MODE_TO_FACE;
            if ( inmode == ENGINE_MODE_FLOWTHROUGH_NEG )
            {
                outmode = ENGINE_MODE_TO_FACE_NEG;
            }
            else if ( inmode == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY )
            {
                outmode = ENGINE_MODE_TO_FACE_NEG_ONLY;
            }
            inmode = outmode;
        }

        bool nopositive = ( inlip && inmode == ENGINE_MODE_TO_FACE_NEG_ONLY ) ||
                          ( outlip && outmode == ENGINE_MODE_TO_FACE_NEG_ONLY );

        if ( inlip && ( inmode == ENGINE_MODE_TO_FACE || inmode == ENGINE_MODE_TO_FACE_NEG ||
                        inmode == ENGINE_MODE_TO_FACE_NEG_ONLY ) )
        {
            inmode = ENGINE_MODE_TO_LIP;
        }
        if ( outlip && ( outmode == ENGINE_MODE_TO_FACE || outmode == ENGINE_MODE_TO_FACE_NEG ||
                         outmode == ENGINE_MODE_TO_FACE_NEG_ONLY ) )
        {
            outmode = ENGINE_MODE_TO_LIP;
        }

        VspSurf surf = m_MainSurfVec[0];
        bool usesurf = true;
        surf.SetSurfCfdType( vsp::CFD_NORMAL );

        VspSurf surf2;
        bool usesurf2 = false;
        VspSurf surf3;
        bool usesurf3 = false;

        vec3d ptoff( m_UnextendedMainBBox.DiagDist() * 1.0e-4, 0, 0 );
        vec3d zero( 0.0, 0.0, 0.0 );

        double umax = surf.GetUMax();
        double umin = 0.0; // m_SurfVec[0].GetUMin();  umin is assumed zero always.
        double uroll = 0;

        double infaceu = m_EngineInFaceU() * umax;
        double inlipu = m_EngineInLipU() * umax;
        double outfaceu = m_EngineOutFaceU() * umax;
        double outlipu = m_EngineOutLipU() * umax;

        if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineGeomInType() == ENGINE_GEOM_FLOWTHROUGH )
        {
            uroll = infaceu;
            surf.RollU( uroll );

            infaceu = roll_t( infaceu, uroll, umin, umax );
            inlipu = roll_t( inlipu, uroll, umin, umax );
            outfaceu = roll_t( outfaceu, uroll, umin, umax );
            outlipu = roll_t( outlipu, uroll, umin, umax );
        }

        // Initialize alternate surfaces as copy of first post-roll if required.
        surf2 = surf;
        surf3 = surf;

        if ( m_EngineGeomInType() != ENGINE_GEOM_FLOWPATH && m_EngineGeomOutType() != ENGINE_GEOM_FLOWPATH )
        {
            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && inmode == ENGINE_MODE_FLOWTHROUGH )
            {
                // Do Nothing
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && inmode == ENGINE_MODE_FLOWTHROUGH_NEG )
            {
                surf.TrimU( outlipu, true );

                // Cap Umax.
                surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && inmode == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY )
            {
                usesurf = false;
            }
            else if ( m_EngineGeomIOType() >= ENGINE_GEOM_INLET_OUTLET ) // Includes outlet description
            {
                if ( outmode == ENGINE_MODE_EXTEND ||
                     outmode == ENGINE_MODE_TO_LIP ||
                     outmode == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.TrimU( outlipu, true );
                }
                else if ( outmode == ENGINE_MODE_TO_FACE )
                {
                    surf.TrimU( outfaceu, true );
                }

                if ( outmode == ENGINE_MODE_EXTEND )
                {
                    // Handle extension.
                    Extend( surf, outlipu, false );
                }

                // Cap Umax.
                surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );

                if ( outmode == ENGINE_MODE_TO_FACE_NEG || outmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                {
                    if ( outmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                    {
                        usesurf = false;
                    }
                    surf2.TrimU( outfaceu, true );
                    surf2.TrimU( outlipu, false );
                    surf2.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, ptoff, false );
                    surf2.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf2.FlipNormal();
                    surf2.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf2 = true;
                }
            }

            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && inmode == ENGINE_MODE_FLOWTHROUGH )
            {
                // Do Nothing
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && inmode == ENGINE_MODE_FLOWTHROUGH_NEG )
            {
                surf.TrimU( inlipu, false );

                // Cap Umin.
                surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && inmode == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY )
            {
                usesurf = false;
            }
            else if ( m_EngineGeomIOType() <= ENGINE_GEOM_INLET_OUTLET ) // Includes inlet description
            {
                if ( inmode == ENGINE_MODE_EXTEND ||
                          inmode == ENGINE_MODE_TO_LIP ||
                          inmode == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.TrimU( inlipu, false );
                }
                else if ( inmode == ENGINE_MODE_TO_FACE )
                {
                    surf.TrimU( infaceu, false );
                }

                if ( inmode == ENGINE_MODE_EXTEND )
                {
                    // Handle extension.
                    Extend( surf, 0.0, true );
                }

                // Cap Umin.
                surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );

                if ( inmode == ENGINE_MODE_TO_FACE_NEG || inmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                {
                    if ( inmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                    {
                        usesurf = false;
                    }
                    surf3.TrimU( inlipu, true );
                    surf3.TrimU( infaceu, false );
                    surf3.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf3.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, -ptoff, false);
                    surf3.FlipNormal();
                    surf3.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf3 = true;
                }
            }

            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && ( inmode == ENGINE_MODE_FLOWTHROUGH_NEG || inmode == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY ) )
            {
                uroll = inlipu;
                surf2.RollU( uroll );

                infaceu = roll_t( infaceu, uroll, umin, umax );
                inlipu = roll_t( inlipu, uroll, umin, umax );
                outfaceu = roll_t( outfaceu, uroll, umin, umax );
                outlipu = roll_t( outlipu, uroll, umin, umax );

                surf2.TrimU( outlipu, false );
                surf2.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, ptoff, false );
                surf2.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, -ptoff, false );
                surf2.FlipNormal();
                surf2.SetSurfCfdType( vsp::CFD_NEGATIVE );
                usesurf2 = true;
            }
        }
        else // Inlet or outlet geometry representation given as flowpath.
        {
            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET ) // Both
            {
                if ( outmode == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( outlipu, true );

                    // Handle extension.
                    Extend( surf, outlipu, false );
                }
                else if ( outmode == ENGINE_MODE_TO_LIP )
                {
                    surf.TrimU( outlipu, true );
                }
                else if ( outmode == ENGINE_MODE_TO_FACE_NEG || outmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                {
                    if ( outmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                    {
                        usesurf = false;
                    }
                    surf.TrimU( outfaceu, true );

                    surf2.TrimU( outfaceu, false );
                    surf2.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf2.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf2 = true;
                }

                // ENGINE_MODE_FLOWTHROUGH_NEG -- does nothing, already closed.
                if ( outmode == ENGINE_MODE_TO_LIP ||
                     outmode == ENGINE_MODE_EXTEND )
                {
                    surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }
                else // ENGINE_MODE_TO_FACE_NEG
                {
                    surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, ptoff, false );
                }

                if ( inmode == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( inlipu, false );

                    // Handle extension.
                    Extend( surf, 0.0, true );
                }
                else if ( inmode == ENGINE_MODE_TO_LIP )
                {
                    surf.TrimU( inlipu, false );
                }
                else if ( inmode == ENGINE_MODE_TO_FACE_NEG || inmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                {
                    if ( inmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                    {
                        usesurf = false;
                    }
                    surf.TrimU( infaceu, false );

                    surf3.TrimU( infaceu, true );
                    surf3.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf3.FlipNormal();
                    surf3.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf3 = true;
                }

                // ENGINE_MODE_FLOWTHROUGH_NEG -- does nothing, already closed.
                if ( inmode == ENGINE_MODE_TO_LIP ||
                     inmode == ENGINE_MODE_EXTEND )
                {
                    surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }
                else // ENGINE_MODE_TO_FACE_NEG
                {
                    surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, -ptoff, false );
                }

                // The flowpath is the only surface here, so both negative modes make it negative.
                if ( inmode == ENGINE_MODE_FLOWTHROUGH_NEG ||
                     inmode == ENGINE_MODE_FLOWTHROUGH_NEG_ONLY )
                {
                    surf.SetSurfCfdType( vsp::CFD_NEGATIVE );
                }
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_OUTLET ) // Outlet description
            {
                if ( outmode == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( outlipu, true );

                    // Handle extension.
                    Extend( surf, outlipu, false );

                    surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }

                if ( outmode == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.SetSurfCfdType( vsp::CFD_NEGATIVE );
                }

                if ( outmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                {
                    usesurf = false;
                }
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET ) // Inlet description
            {
                if ( inmode == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( inlipu, false );

                    // Handle extension.
                    Extend( surf, 0.0, true );

                    surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }

                if ( inmode == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.SetSurfCfdType( vsp::CFD_NEGATIVE );
                }

                if ( inmode == ENGINE_MODE_TO_FACE_NEG_ONLY )
                {
                    usesurf = false;
                }
            }
        }

        // With only the flowpath represented, or only a recess there is no face for, the
        // positive surface goes whatever the ends do.
        if ( nopositive )
        {
            usesurf = false;
        }

        // Assign into the existing surfaces (resize + index) rather than clear + push_back, so
        // the surface storage is reused across updates -- matching the m_MainSurfVec pattern
        // used elsewhere.
        int ns = 0;
        if ( usesurf )
        {
            ns++;
        }
        if ( usesurf2 )
        {
            ns++;
        }
        if ( usesurf3 )
        {
            ns++;
        }
        m_MainSurfVec.resize( ns );

        int isurf = 0;
        if ( usesurf )
        {
            m_MainSurfVec[ isurf ] = surf;
            isurf++;
        }
        if ( usesurf2 )
        {
            m_MainSurfVec[ isurf ] = surf2;
            isurf++;
        }
        if ( usesurf3 )
        {
            m_MainSurfVec[ isurf ] = surf3;
            isurf++;
        }

        // One end cap flag per engine surface.  Only the main engine surface is cut from the
        // surface UpdateEndCaps capped, so it keeps that surface's flags; the flowpath surfaces
        // carry no Geom end caps.
        bool capumin = false;
        bool capumax = false;
        if ( usesurf && !m_CapUMinSuccess.empty() && !m_CapUMaxSuccess.empty() )
        {
            capumin = m_CapUMinSuccess[0];
            capumax = m_CapUMaxSuccess[0];
        }
        m_CapUMinSuccess.assign( ns, false );
        m_CapUMaxSuccess.assign( ns, false );
        if ( usesurf )
        {
            m_CapUMinSuccess[0] = capumin;
            m_CapUMaxSuccess[0] = capumax;
        }

        // Where a positive and a negative surface both remain, they meet at faces that lie one
        // on the other, and each face must be a surface of its own for the CFD mesh to pair it
        // with the other.  The CFD mesh alone cuts apart the caps the engine closed them with.
        bool positive = false;
        bool negative = false;
        for ( int i = 0 ; i < ns ; i++ )
        {
            if ( m_MainSurfVec[i].GetSurfCfdType() == vsp::CFD_NEGATIVE )
            {
                negative = true;
            }
            else
            {
                positive = true;
            }
        }
        for ( int i = 0 ; i < ns ; i++ )
        {
            m_MainSurfVec[i].SetCFDCutCaps( positive && negative );
        }
    }
}

// UpdateEngine has already built the U mapping of each engine surface, so only the arc length
// curve is built here.  InitUMapping would replace that mapping with one of the engine surface itself.
void GeomEngine::UpdateLCurve()
{
    if ( m_EngineGeomIOType() == ENGINE_GEOM_NONE )
    {
        Geom::UpdateLCurve();
        return;
    }

    for ( int i = 0; i < m_MainSurfVec.size(); i++ )
    {
        m_MainSurfVec[i].BuildLCurve();
    }
}

// The inlet and outlet lip and face stations on the main surface, placed by placer.
void GeomEngine::BuildMarkerDrawObjs( Geom* placer, vector< DrawObj > &marker_vec )
{
    marker_vec.clear();

    vector< Matrix4d > trans_vec = placer->GetTransMatVec();
    if ( m_EngineGeomIOType() == ENGINE_GEOM_NONE || trans_vec.empty() )
    {
        return;
    }

    Matrix4d relTrans = trans_vec[ 0 ];

    double tol = 1e-2;

    double eng_loc[ vsp::ENGINE_LOC_NUM ] = { m_EngineInLipU(), m_EngineInFaceU(), m_EngineOutLipU(), m_EngineOutFaceU() };
    int eng_color[ vsp::ENGINE_LOC_NUM ] = { DrawObj::CYAN, DrawObj::MAGENTA, DrawObj::YELLOW, DrawObj::LIME };

    double umax = m_OrigSurf.GetUMax();

    for ( int i = 0; i < vsp::ENGINE_LOC_NUM; i++ )
    {
        if ( m_engine_spec[i] )
        {
            char str[256];
            snprintf( str, sizeof( str ), "_%d", i );

            marker_vec.push_back( DrawObj() );
            DrawObj &dobj = marker_vec.back();

            m_OrigSurf.TessULine( eng_loc[i] * umax, dobj.m_PntVec, tol );
            relTrans.xformvec( dobj.m_PntVec );

            dobj.m_LineWidth = 7;
            dobj.m_LineColor = DrawObj::Color( eng_color[i] );
            dobj.m_Type = DrawObj::VSP_LINE_STRIP;
            dobj.m_GeomID = "ENG_" + placer->GetID() + str;
            dobj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
            dobj.m_GeomChanged = true;

            dobj.m_StippleFactor = 20;
            dobj.m_StipplePattern = 0xAAAA;
            dobj.m_StippleFlag = true;
        }
    }
}

void GeomEngine::SetMarkerVisibility( Geom* placer, vector< DrawObj > &marker_vec )
{
    bool visible = placer->m_GuiDraw.GetDispFeatureFlag() &&
                   m_EngineShowStationsFlag() &&
                   placer->GetSetFlag( vsp::SET_SHOWN );

    for ( int i = 0; i < ( int )marker_vec.size(); i++ )
    {
        marker_vec[i].m_Visible = visible;
    }
}

bool GeomEngine::IsModelScaleSensitive()
{
    // If we're auto-calculating the extension distance, then this model is sensitive to the overall
    // scale of the model.
    return m_AutoExtensionFlag();
}