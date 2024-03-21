//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GeomEngine.h"
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

    m_EngineInModeType.Init( "InletModeType", "EngineModel", this, ENGINE_MODE_TO_LIP, ENGINE_MODE_FLOWTHROUGH, ENGINE_MODE_NUM_TYPES - 1 );
    m_EngineOutModeType.Init( "OutletModeType", "EngineModel", this, ENGINE_MODE_TO_LIP, ENGINE_MODE_TO_LIP, ENGINE_MODE_NUM_TYPES - 1 );

    m_ExtensionDistance.Init( "ExtensionDistance", "EngineModel", this, 10, 0, 1e12 );
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

    m_ExtensionDistance.Deactivate();

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
                m_ExtensionDistance.Activate();
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
                m_ExtensionDistance.Activate();
            }
        }
    }

    if ( m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH || m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG )
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

void GeomEngine::UpdateEngine()
{
    if ( m_EngineGeomIOType() != ENGINE_GEOM_NONE )
    {
        m_OrigSurf = m_MainSurfVec[0];
        m_OrigSurf.BuildFeatureLines( m_ForceXSecFlag );

        BndBox bbox;
        m_OrigSurf.GetBoundingBox( bbox );

        VspSurf surf = m_MainSurfVec[0];
        surf.SetSurfCfdType( vsp::CFD_NORMAL );

        VspSurf surf2;
        bool usesurf2 = false;
        VspSurf surf3;
        bool usesurf3 = false;

        vec3d ptoff( bbox.DiagDist() * 1.0e-4, 0, 0 );
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
            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH )
            {
                // Do Nothing
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG )
            {
                surf.TrimU( outlipu, true );

                // Cap Umax.
                surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
            }
            else if ( m_EngineGeomIOType() >= ENGINE_GEOM_INLET_OUTLET ) // Includes outlet description
            {
                if ( m_EngineOutModeType() == ENGINE_MODE_EXTEND ||
                     m_EngineOutModeType() == ENGINE_MODE_TO_LIP ||
                     m_EngineOutModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.TrimU( outlipu, true );
                }
                else if ( m_EngineOutModeType() == ENGINE_MODE_TO_FACE )
                {
                    surf.TrimU( outfaceu, true );
                }

                if ( m_EngineOutModeType() == ENGINE_MODE_EXTEND )
                {
                    // Handle extension.
                    vector < VspCurve > extcrvs(2);
                    surf.GetUConstCurve( extcrvs[0], outlipu );
                    extcrvs[1] = extcrvs[0];
                    extcrvs[1].OffsetX( m_ExtensionDistance() );

                    vector < double > extparam = { 0, 1.0 };

                    VspSurf ext;
                    ext.SkinC0( extcrvs, extparam, false );
                    ext.InitUMapping( -2 );

                    VspSurf s;
                    s.JoinU( surf, ext );
                    surf = s;
                }

                // Cap Umax.
                surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );

                if ( m_EngineOutModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf2.TrimU( outfaceu, true );
                    surf2.TrimU( outlipu, false );
                    surf2.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, ptoff, false );
                    surf2.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf2.FlipNormal();
                    surf2.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf2 = true;
                }

            }

            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH )
            {
                // Do Nothing
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG )
            {
                surf.TrimU( inlipu, false );

                // Cap Umin.
                surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
            }
            else if ( m_EngineGeomIOType() <= ENGINE_GEOM_INLET_OUTLET ) // Includes inlet description
            {
                if ( m_EngineInModeType() == ENGINE_MODE_EXTEND ||
                          m_EngineInModeType() == ENGINE_MODE_TO_LIP ||
                          m_EngineInModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.TrimU( inlipu, false );
                }
                else if ( m_EngineInModeType() == ENGINE_MODE_TO_FACE )
                {
                    surf.TrimU( infaceu, false );
                }

                if ( m_EngineInModeType() == ENGINE_MODE_EXTEND )
                {
                    // Handle extension.
                    vector < VspCurve > extcrvs(2);
                    surf.GetUConstCurve( extcrvs[1], 0.0 );
                    extcrvs[0] = extcrvs[1];
                    extcrvs[0].OffsetX( -m_ExtensionDistance() );

                    vector < double > extparam = { 0, 1.0 };

                    VspSurf ext;
                    ext.SkinC0( extcrvs, extparam, false );
                    ext.InitUMapping( -2 );

                    VspSurf s;
                    s.JoinU( ext, surf );
                    surf = s;
                }

                // Cap Umin.
                surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );

                if ( m_EngineInModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf3.TrimU( inlipu, true );
                    surf3.TrimU( infaceu, false );
                    surf3.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf3.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, -ptoff, false);
                    surf3.FlipNormal();
                    surf3.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf3 = true;
                }
            }

            if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG )
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
                if ( m_EngineOutModeType() == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( outlipu, true );

                    // Handle extension.
                    vector < VspCurve > extcrvs(2);
                    surf.GetUConstCurve( extcrvs[0], outlipu );
                    extcrvs[1] = extcrvs[0];
                    extcrvs[1].OffsetX( m_ExtensionDistance() );

                    vector < double > extparam = { 0, 1.0 };

                    VspSurf ext;
                    ext.SkinC0( extcrvs, extparam, false );
                    ext.InitUMapping( -2 );

                    VspSurf s;
                    s.JoinU( surf, ext );
                    surf = s;
                }
                else if ( m_EngineOutModeType() == ENGINE_MODE_TO_LIP )
                {
                    surf.TrimU( outlipu, true );
                }
                else if ( m_EngineOutModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.TrimU( outfaceu, true );

                    surf2.TrimU( outfaceu, false );
                    surf2.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf2.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf2 = true;
                }

                // ENGINE_MODE_FLOWTHROUGH_NEG -- does nothing, already closed.
                if ( m_EngineOutModeType() == ENGINE_MODE_TO_LIP ||
                     m_EngineOutModeType() == ENGINE_MODE_EXTEND )
                {
                    surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }
                else // ENGINE_MODE_TO_FACE_NEG
                {
                    surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, ptoff, false );
                }

                if ( m_EngineInModeType() == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( inlipu, false );

                    // Handle extension.
                    vector < VspCurve > extcrvs(2);
                    surf.GetUConstCurve( extcrvs[1], 0.0 );
                    extcrvs[0] = extcrvs[1];
                    extcrvs[0].OffsetX( -m_ExtensionDistance() );

                    vector < double > extparam = { 0, 1.0 };

                    VspSurf ext;
                    ext.SkinC0( extcrvs, extparam, false );
                    ext.InitUMapping( -2 );

                    VspSurf s;
                    s.JoinU( ext, surf );
                    surf = s;
                }
                else if ( m_EngineInModeType() == ENGINE_MODE_TO_LIP )
                {
                    surf.TrimU( inlipu, false );
                }
                else if ( m_EngineInModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.TrimU( infaceu, false );

                    surf3.TrimU( infaceu, true );
                    surf3.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                    surf3.FlipNormal();
                    surf3.SetSurfCfdType( vsp::CFD_NEGATIVE );
                    usesurf3 = true;
                }

                // ENGINE_MODE_FLOWTHROUGH_NEG -- does nothing, already closed.
                if ( m_EngineInModeType() == ENGINE_MODE_TO_LIP ||
                     m_EngineInModeType() == ENGINE_MODE_EXTEND )
                {
                    surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }
                else // ENGINE_MODE_TO_FACE_NEG
                {
                    surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, -ptoff, false );
                }

                if ( m_EngineInModeType() == ENGINE_MODE_FLOWTHROUGH_NEG )
                {
                    surf.SetSurfCfdType( vsp::CFD_NEGATIVE );
                }
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_OUTLET ) // Outlet description
            {
                if ( m_EngineOutModeType() == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( outlipu, true );

                    // Handle extension.
                    vector < VspCurve > extcrvs(2);
                    surf.GetUConstCurve( extcrvs[0], outlipu );
                    extcrvs[1] = extcrvs[0];
                    extcrvs[1].OffsetX( m_ExtensionDistance() );

                    vector < double > extparam = { 0, 1.0 };

                    VspSurf ext;
                    ext.SkinC0( extcrvs, extparam, false );
                    ext.InitUMapping( -2 );

                    VspSurf s;
                    s.JoinU( surf, ext );
                    surf = s;

                    surf.CapUMax( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }

                if ( m_EngineOutModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.SetSurfCfdType( vsp::CFD_NEGATIVE );
                }
            }
            else if ( m_EngineGeomIOType() == ENGINE_GEOM_INLET ) // Inlet description
            {
                if ( m_EngineInModeType() == ENGINE_MODE_EXTEND )
                {
                    surf.TrimU( inlipu, false );

                    // Handle extension.
                    vector < VspCurve > extcrvs(2);
                    surf.GetUConstCurve( extcrvs[1], 0.0 );
                    extcrvs[0] = extcrvs[1];
                    extcrvs[0].OffsetX( -m_ExtensionDistance() );

                    vector < double > extparam = { 0, 1.0 };

                    VspSurf ext;
                    ext.SkinC0( extcrvs, extparam, false );
                    ext.InitUMapping( -2 );

                    VspSurf s;
                    s.JoinU( ext, surf );
                    surf = s;

                    surf.CapUMin( POINT_END_CAP, 1.0, 0.0, 0.5, zero, false );
                }

                if ( m_EngineInModeType() == ENGINE_MODE_TO_FACE_NEG )
                {
                    surf.SetSurfCfdType( vsp::CFD_NEGATIVE );
                }
            }
        }

        m_MainSurfVec.clear();
        m_MainSurfVec.push_back( surf );
        if ( usesurf2 )
        {
            m_MainSurfVec.push_back( surf2 );
        }
        if ( usesurf3 )
        {
            m_MainSurfVec.push_back( surf3 );
        }
    }
}

void GeomEngine::UpdateHighlightDrawObj()
{
    GeomXSec::UpdateHighlightDrawObj();

    m_EngineDrawObj_vec.clear();

    if ( m_EngineGeomIOType() != ENGINE_GEOM_NONE )
    {
        Matrix4d relTrans;

        relTrans = m_AttachMatrix;
        relTrans.affineInverse();
        relTrans.matMult( m_ModelMatrix.data() );
        relTrans.postMult( m_AttachMatrix.data() );

        double tol = 1e-2;

        double eng_loc[ vsp::ENGINE_LOC_NUM ] = { m_EngineInLipU(), m_EngineInFaceU(), m_EngineOutLipU(), m_EngineOutFaceU() };
        int eng_color[ vsp::ENGINE_LOC_NUM ] = { DrawObj::CYAN, DrawObj::MAGENTA, DrawObj::YELLOW, DrawObj::LIME };

        double umax = m_OrigSurf.GetUMax();

        m_EngineDrawObj_vec.resize( vsp::ENGINE_LOC_NUM );
        for ( int i = 0; i < vsp::ENGINE_LOC_NUM; i++ )
        {
            if ( m_engine_spec[i] )
            {
                char str[256];
                snprintf( str, sizeof( str ), "_%d", i );

                m_OrigSurf.TessULine( eng_loc[i] * umax, m_EngineDrawObj_vec[i].m_PntVec, tol );
                relTrans.xformvec( m_EngineDrawObj_vec[i].m_PntVec );

                m_EngineDrawObj_vec[i].m_LineWidth = 5;
                m_EngineDrawObj_vec[i].m_LineColor = DrawObj::Color( eng_color[i] );
                m_EngineDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
                m_EngineDrawObj_vec[i].m_GeomID = "ENG_" + m_ID + str;
                m_EngineDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
                m_EngineDrawObj_vec[i].m_GeomChanged = true;
            }
        }
    }
}

void GeomEngine::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    GeomXSec::LoadDrawObjs( draw_obj_vec );

    for ( int i = 0; i < m_EngineDrawObj_vec.size(); i++ )
    {
        if ( m_engine_spec[i] )
        {
            m_EngineDrawObj_vec[i].m_Visible = ( m_EngineGeomIOType() != ENGINE_GEOM_NONE ) && m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN );
            draw_obj_vec.push_back( &m_EngineDrawObj_vec[i] );
        }
    }
}
