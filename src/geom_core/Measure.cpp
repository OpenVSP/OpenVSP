//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "Defines.h"

#include "Measure.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VSP_Geom_API.h"
#include "UnitConversion.h"
#include "VspUtil.h"

Probe::Probe() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_OriginU.Init( "OriginU", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginW.Init( "OriginW", "Measure", this, 0.0, 0.0, 1.0 );

    m_OriginIndx.Init( "OriginIndx", "Measure", this, 0, 0, 1e6 );

    m_Precision.Init( "Precision", "Measure", this, 3, 0, 20 );

    m_Visible.Init( "Visible", "Measure", this, 1, 0, 1 );

    m_X.Init( "X", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_Y.Init( "Y", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_Z.Init( "Z", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_NX.Init( "NX", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_NY.Init( "NY", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_NZ.Init( "NZ", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_K1.Init( "Kmax", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_K2.Init( "Kmin", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_Ka.Init( "Kmean", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_Kg.Init( "KGaussian", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_Len.Init( "Len", "Measure", this, 1.0, 0.0, 1.0e12 );

    m_LabelDO.m_GeomID = GenerateRandomID( 4 ) + "_Probe";
    m_LabelDO.m_Type = DrawObj::VSP_PROBE;
    m_LabelDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LabelDO.m_Probe.Step = DrawObj::VSP_PROBE_STEP_ZERO;
    m_LabelDO.m_TextColor = vec3d( 0, 0, 0 );
    m_LabelDO.m_TextSize = 0;
    m_LabelDO.m_Visible = true;


    LinkMgr.RegisterContainer( GetID() );
}
Probe::~Probe()
{
    LinkMgr.UnRegisterContainer( GetID() );
}

xmlNodePtr Probe::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "OriginGeomID", m_OriginGeomID );
    }

    return parmcontain_node;
}

xmlNodePtr Probe::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_OriginGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "OriginGeomID", m_OriginGeomID ) );
        m_Stage = STAGE_COMPLETE;
    }

    return parmcontain_node;
}

void Probe::Reset()
{
    m_Stage = STAGE_ZERO;

    m_OriginU = 0.0;
    m_OriginW = 0.0;

    m_Len = 1.0;

    m_Precision = 3;
}

void Probe::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_LabelDO );
}

void Probe::Update()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        vec3d pt, norm;
        double k1 = 0;
        double k2 = 0;
        double ka = 0;
        double kg = 0;
        Geom *geom = veh->FindGeom( m_OriginGeomID );

        if ( geom )
        {
            VspSurf *surf = geom->GetSurfPtr( m_OriginIndx() );
            if ( surf )
            {
                double umapmax = surf->GetUMapMax();
                double umax = surf->GetUMax();
                double u = surf->InvertUMapping( m_OriginU() * umapmax ) / umax;
                if ( u < 0 )
                {
                    u = m_OriginU();
                }

                pt = surf->CompPnt01( u, m_OriginW() );
                norm = surf->CompNorm01( u, m_OriginW() );
                surf->CompCurvature01( u, m_OriginW(), k1, k2, ka, kg );
            }
        }

        m_X = pt.x();
        m_Y = pt.y();
        m_Z = pt.z();

        if ( norm.mag() < 1e-6 )
        {
            norm.set_xyz( 0, 0, 1 );
        }

        m_NX = norm.x();
        m_NY = norm.y();
        m_NZ = norm.z();

        m_K1 = isfinite( k1 ) ? k1 : 0.0;
        m_K2 = isfinite( k2 ) ? k2 : 0.0;
        m_Ka = isfinite( ka ) ? ka : 0.0;
        m_Kg = isfinite( kg ) ? kg : 0.0;

        m_LabelDO.m_Probe.Pt = pt;
        m_LabelDO.m_Probe.Norm = norm;
        m_LabelDO.m_Probe.Len = m_Len();
        m_LabelDO.m_Probe.Label = GetName();
        m_LabelDO.m_GeomChanged = true;
        m_LabelDO.m_Visible = m_Visible();

        m_LabelDO.m_TextSize = veh->m_TextSize();
    }
}

bool Probe::Valid()
{
    if ( m_Stage == STAGE_ZERO )
    {
        return true;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* geom = veh->FindGeom( m_OriginGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_OriginIndx() < 0 || m_OriginIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }
    }

    return true;
}

void Probe::SetLenFromPlacement( const vec3d &placement )
{

    vec3d pt = vec3d( m_X(), m_Y(), m_Z() );
    vec3d v = ( placement - pt );

    m_Len = v.mag();
}


RSTProbe::RSTProbe() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_OriginR.Init( "OriginR", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginS.Init( "OriginS", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginT.Init( "OriginT", "Measure", this, 0.0, 0.0, 1.0 );

    m_LMNFlag.Init( "LMNFlag", "Measure", this, false, false, true );

    m_OriginL.Init( "OriginL", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginM.Init( "OriginM", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginN.Init( "OriginN", "Measure", this, 0.0, 0.0, 1.0 );

    m_OriginIndx.Init( "OriginIndx", "Measure", this, 0, 0, 1e6 );

    m_Precision.Init( "Precision", "Measure", this, 3, 0, 20 );

    m_Visible.Init( "Visible", "Measure", this, 1, 0, 1 );

    m_X.Init( "X", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_Y.Init( "Y", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_Z.Init( "Z", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_Len.Init( "Len", "Measure", this, 1.0, 0.0, 1.0e12 );

    m_LabelDO.m_GeomID = GenerateRandomID( 4 ) + "_Probe";
    m_LabelDO.m_Type = DrawObj::VSP_PROBE;
    m_LabelDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LabelDO.m_Probe.Step = DrawObj::VSP_PROBE_STEP_ZERO;
    m_LabelDO.m_TextColor = vec3d( 0, 0, 0 );
    m_LabelDO.m_TextSize = 0;
    m_LabelDO.m_Visible = true;

    LinkMgr.RegisterContainer( GetID() );
}
RSTProbe::~RSTProbe()
{
    LinkMgr.UnRegisterContainer( GetID() );
}

xmlNodePtr RSTProbe::EncodeXml(xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "OriginGeomID", m_OriginGeomID );
    }

    return parmcontain_node;
}

xmlNodePtr RSTProbe::DecodeXml(xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_OriginGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "OriginGeomID", m_OriginGeomID ) );
        m_Stage = STAGE_COMPLETE;
    }

    return parmcontain_node;
}

void RSTProbe::Reset()
{
    m_Stage = STAGE_ZERO;

    m_OriginR = 0.0;
    m_OriginS = 0.0;
    m_OriginT = 0.0;

    m_LMNFlag = false;

    m_OriginL = 0.0;
    m_OriginM = 0.0;
    m_OriginN = 0.0;

    m_Len = 1.0;

    m_Precision = 3;
}

void RSTProbe::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_LabelDO );
}

void RSTProbe::Update()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        vec3d pt, norm;
        Geom *geom = veh->FindGeom( m_OriginGeomID );
        if ( geom )
        {
            VspSurf *surf = geom->GetSurfPtr( m_OriginIndx() );
            if ( surf )
            {
                if ( m_LMNFlag() )
                {
                    double r, s, t;
                    surf->ConvertLMNtoRST( m_OriginL(), m_OriginM(), m_OriginN(), r, s, t );
                    m_OriginR = r;
                    m_OriginS = s;
                    m_OriginT = t;
                }
                else
                {
                    double l, m, n;
                    surf->ConvertRSTtoLMN( m_OriginR(), m_OriginS(), m_OriginT(), l, m, n );
                    m_OriginL = l;
                    m_OriginM = m;
                    m_OriginN = n;
                }

                double umapmax = surf->GetUMapMax();
                double umax = surf->GetUMax();
                double r = surf->InvertUMapping( m_OriginR() * umapmax ) / umax;
                if ( r < 0 )
                {
                    r = m_OriginR();
                }

                pt = surf->CompPntRST( r, m_OriginS(), m_OriginT() );
                norm = surf->CompNorm01( r, 0.5 * m_OriginS() );
            }
        }

        m_X = pt.x();
        m_Y = pt.y();
        m_Z = pt.z();

        m_LabelDO.m_Probe.Pt = pt;
        m_LabelDO.m_Probe.Norm = norm;
        m_LabelDO.m_Probe.Len = m_Len();
        m_LabelDO.m_Probe.Label = GetName();
        m_LabelDO.m_GeomChanged = true;
        m_LabelDO.m_Visible = m_Visible();

        m_LabelDO.m_TextSize = veh->m_TextSize();
    }
}

bool RSTProbe::Valid()
{
    if ( m_Stage == STAGE_ZERO )
    {
        return true;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* geom = veh->FindGeom( m_OriginGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_OriginIndx() < 0 || m_OriginIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }
    }

    return true;
}

void RSTProbe::SetLenFromPlacement(const vec3d &placement )
{

    vec3d pt = vec3d( m_X(), m_Y(), m_Z() );
    vec3d v = ( placement - pt );

    m_Len = v.mag();
}

Ruler::Ruler() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_OriginU.Init( "OriginU", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginW.Init( "OriginW", "Measure", this, 0.0, 0.0, 1.0 );

    m_OriginIndx.Init( "OriginIndx", "Measure", this, 0, 0, 1e6 );

    m_EndU.Init( "EndU", "Measure", this, 0.0, 0.0, 1.0 );
    m_EndW.Init( "EndW", "Measure", this, 0.0, 0.0, 1.0 );

    m_EndIndx.Init( "EndIndx", "Measure", this, 0, 0, 1e6 );

    m_XOffset.Init( "X_Offset", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_YOffset.Init( "Y_Offset", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_ZOffset.Init( "Z_Offset", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_Precision.Init( "Precision", "Measure", this, 3, 0, 20 );

    m_Visible.Init( "Visible", "Measure", this, 1, 0, 1 );

    m_DeltaX.Init( "DeltaX", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_DeltaY.Init( "DeltaY", "Measure", this, 0.0, -1.0e12, 1.0e12 );
    m_DeltaZ.Init( "DeltaZ", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_Distance.Init( "Distance", "Measure", this, 0.0, -1.0e12, 1.0e12 );

    m_Component.Init( "Component", "Measure", this, vsp::ALL_DIR, vsp::X_DIR, vsp::ALL_DIR );

    m_LabelDO.m_GeomID = GenerateRandomID( 4 ) + "_Ruler";
    m_LabelDO.m_Type = DrawObj::VSP_RULER;
    m_LabelDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LabelDO.m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;
    m_LabelDO.m_TextColor = vec3d( 0, 0, 0 );
    m_LabelDO.m_TextSize = 0;
    m_LabelDO.m_Visible = true;


    LinkMgr.RegisterContainer( GetID() );

}

Ruler::~Ruler()
{
    LinkMgr.UnRegisterContainer( GetID() );
}

void Ruler::Reset()
{
    m_Stage = STAGE_ZERO;

    m_OriginU = 0.0;
    m_OriginW = 0.0;

    m_EndU = 0.0;
    m_EndW = 0.0;

    m_XOffset = 0.0;
    m_YOffset = 0.0;
    m_ZOffset = 0.0;

    m_Precision = 3;

    m_Component = vsp::ALL_DIR;
}

void Ruler::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_LabelDO );
}

void Ruler::Update()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        vec3d origin;
        Geom *ogeom = veh->FindGeom( m_OriginGeomID );
        if ( ogeom )
        {
            VspSurf *osurf = ogeom->GetSurfPtr( m_OriginIndx() );
            if ( osurf )
            {
                double umapmax = osurf->GetUMapMax();
                double umax = osurf->GetUMax();
                double u = osurf->InvertUMapping( m_OriginU() * umapmax ) / umax;
                if ( u < 0 )
                {
                    u = m_OriginU();
                }
                origin = osurf->CompPnt01( u, m_OriginW() );
            }
        }

        vec3d end;
        Geom *egeom = veh->FindGeom( m_EndGeomID );
        if ( egeom )
        {
            VspSurf *esurf = egeom->GetSurfPtr( m_EndIndx() );
            if ( esurf )
            {
                double umapmax = esurf->GetUMapMax();
                double umax = esurf->GetUMax();
                double u = esurf->InvertUMapping( m_EndU() * umapmax ) / umax;
                if ( u < 0 )
                {
                    u = m_EndU();
                }
                end = esurf->CompPnt01( u, m_EndW() );
            }
        }

        vec3d delta = end - origin;

        char dir[4] = {'\0',':',' ','\0'}; // set up string with first char null to skip
        if ( m_Component() != vsp::ALL_DIR )
        {
            vec3d mask;
            mask.v[ m_Component() ] = 1;
            for ( int i = 0; i < 3; i ++ )
            {
                delta.v[i] *= mask.v[i];
            }
            dir[0] = 88 + m_Component(); // Set first char to X, Y, Z
        }

        m_DeltaX = delta.x();
        m_DeltaY = delta.y();
        m_DeltaZ = delta.z();

        m_Distance = delta.mag();

        char str[255];
        snprintf( str, sizeof( str ), "%s%.*f %s", dir, m_Precision(), delta.mag(), LenUnitName( veh->m_MeasureLenUnit() ).c_str() );

        m_LabelDO.m_Ruler.Start = origin;
        m_LabelDO.m_Ruler.End = end;
        m_LabelDO.m_Ruler.Label = string( str );
        m_LabelDO.m_Ruler.Dir = m_Component();
        m_LabelDO.m_GeomChanged = true;
        m_LabelDO.m_Visible = m_Visible();

        m_LabelDO.m_TextSize = veh->m_TextSize();
    }
}

bool Ruler::Valid()
{
    if ( m_Stage == STAGE_ZERO )
    {
        return true;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* geom = veh->FindGeom( m_OriginGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_OriginIndx() < 0 || m_OriginIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }

        if ( m_Stage == STAGE_ONE )
        {
            return true;
        }

        geom = veh->FindGeom( m_EndGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_EndIndx() < 0 || m_EndIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }
    }

    return true;
}

xmlNodePtr Ruler::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "OriginGeomID", m_OriginGeomID );
        XmlUtil::AddStringNode( parmcontain_node, "EndGeomID", m_EndGeomID );
    }

    return parmcontain_node;
}

xmlNodePtr Ruler::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_OriginGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "OriginGeomID", m_OriginGeomID ) );
        m_EndGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "EndGeomID", m_EndGeomID ) );
        m_Stage = STAGE_COMPLETE;
    }

    return parmcontain_node;
}

Protractor::Protractor() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_OriginU.Init( "OriginU", "Measure", this, 0.0, 0.0, 1.0 );
    m_OriginW.Init( "OriginW", "Measure", this, 0.0, 0.0, 1.0 );

    m_OriginIndx.Init( "OriginIndx", "Measure", this, 0, 0, 1e6 );

    m_MidU.Init( "MidU", "Measure", this, 0.0, 0.0, 1.0 );
    m_MidW.Init( "MidW", "Measure", this, 0.0, 0.0, 1.0 );

    m_MidIndx.Init( "MidIndx", "Measure", this, 0, 0, 1e6 );

    m_EndU.Init( "EndU", "Measure", this, 0.0, 0.0, 1.0 );
    m_EndW.Init( "EndW", "Measure", this, 0.0, 0.0, 1.0 );

    m_EndIndx.Init( "EndIndx", "Measure", this, 0, 0, 1e6 );

    m_Offset.Init( "Offset", "Measure", this, 0.0, 0, 1.0e12 );

    m_Precision.Init( "Precision", "Measure", this, 1, 0, 20 );

    m_Visible.Init( "Visible", "Measure", this, 1, 0, 1 );

    m_ThetaX.Init( "ThetaX", "Measure", this, 0.0, -360, 360 );
    m_ThetaY.Init( "ThetaY", "Measure", this, 0.0, -360, 360 );
    m_ThetaZ.Init( "ThetaZ", "Measure", this, 0.0, -360, 360 );

    m_Theta.Init( "Theta", "Measure", this, 0.0, -360, 360 );

    m_Component.Init( "Component", "Measure", this, vsp::ALL_DIR, vsp::X_DIR, vsp::ALL_DIR );

    string tmpID = GenerateRandomID( 4 );
    m_LabelDO.m_GeomID = tmpID + "_Protractor";
    m_LabelDO.m_Type = DrawObj::VSP_PROTRACTOR;
    m_LabelDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LabelDO.m_Protractor.Step = DrawObj::VSP_PROTRACTOR_STEP_ZERO;
    m_LabelDO.m_TextColor = vec3d( 0, 0, 0 );
    m_LabelDO.m_TextSize = 0;
    m_LabelDO.m_Visible = true;

    m_ArcDO.m_GeomID = tmpID + "_Arc";
    m_ArcDO.m_Type = DrawObj::VSP_LINE_STRIP;
    m_ArcDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_ArcDO.m_TextColor = vec3d( 0, 0, 0 );
    m_ArcDO.m_LineColor = vec3d( 1.0, 0.5, 0.0);
    m_ArcDO.m_LineWidth = 3.0;
    m_ArcDO.m_TextSize = 0;
    m_ArcDO.m_Visible = true;

    LinkMgr.RegisterContainer( GetID() );

}

Protractor::~Protractor()
{
    LinkMgr.UnRegisterContainer( GetID() );
}

void Protractor::Reset()
{
    m_Stage = STAGE_ZERO;

    m_OriginU = 0.0;
    m_OriginW = 0.0;

    m_MidU = 0.0;
    m_MidW = 0.0;

    m_EndU = 0.0;
    m_EndW = 0.0;

    m_Offset = 0.0;

    m_Precision = 1;

    m_Component = vsp::ALL_DIR;
}

void Protractor::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_LabelDO );
    draw_obj_vec.push_back( &m_ArcDO );
}

void Protractor::Update()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        vec3d origin;
        Geom *ogeom = veh->FindGeom( m_OriginGeomID );
        if ( ogeom )
        {
            VspSurf *osurf = ogeom->GetSurfPtr( m_OriginIndx() );
            if ( osurf )
            {
                double umapmax = osurf->GetUMapMax();
                double umax = osurf->GetUMax();
                double u = osurf->InvertUMapping( m_OriginU() * umapmax ) / umax;
                if ( u < 0 )
                {
                    u = m_OriginU();
                }
                origin = osurf->CompPnt01( u, m_OriginW() );
            }
        }

        vec3d mid = GetMidPt();

        vec3d end;
        Geom *egeom = veh->FindGeom( m_EndGeomID );
        if ( egeom )
        {
            VspSurf *esurf = egeom->GetSurfPtr( m_EndIndx() );
            if ( esurf )
            {
                double umapmax = esurf->GetUMapMax();
                double umax = esurf->GetUMax();
                double u = esurf->InvertUMapping( m_EndU() * umapmax ) / umax;
                if ( u < 0 )
                {
                    u = m_EndU();
                }
                end = esurf->CompPnt01( u, m_EndW() );
            }
        }

        vec3d u = origin - mid;
        vec3d v = end - mid;

        vec3d thetas;

        char dir[4] = {'\0',':',' ','\0'}; // set up string with first char null to skip
        if ( m_Component() == vsp::ALL_DIR )
        {
            for ( int i = 0; i < 3; i++ )
            {
                vec3d uprime = u;
                vec3d vprime = v;
                uprime[i] = 0;
                vprime[i] = 0;
                thetas[i] = angle( uprime, vprime );
            }
        }
        else
        {
            u[m_Component()] = 0;
            v[m_Component()] = 0;

            thetas[m_Component()] = angle( u, v );
            dir[0] = 88 + m_Component(); // Set first char to X, Y, Z
        }

        vec3d w = slerp( u, v, 0.5 );
        w.normalize();

        int npts = 11;
        vector < vec3d > arcpts;
        arcpts.push_back( mid );
        for ( int i = 0; i < npts; i++ )
        {
            vec3d rvec = slerp( u, v, (double) i / (double) ( npts - 1 ) );
            arcpts.push_back( mid + rvec * m_Offset() );
        }
        arcpts.push_back( mid );

        m_Theta = angle( u, v ) * 180.0 / PI;

        m_ThetaX = thetas.x() * 180.0 / PI;
        m_ThetaY = thetas.y() * 180.0 / PI;
        m_ThetaZ = thetas.z() * 180.0 / PI;

        char str[255];
        snprintf( str, sizeof( str ), "%s%.*f deg", dir, m_Precision(), m_Theta() );

        m_LabelDO.m_Protractor.Start = origin;
        m_LabelDO.m_Protractor.Mid = mid;
        m_LabelDO.m_Protractor.End = end;
        m_LabelDO.m_Protractor.Label_Dir = w;
        m_LabelDO.m_Protractor.Offset = m_Offset();
        m_LabelDO.m_Protractor.Label = string( str );
        m_LabelDO.m_Protractor.Dir = m_Component();
        m_LabelDO.m_GeomChanged = true;
        m_LabelDO.m_Visible = m_Visible();

        m_LabelDO.m_TextSize = veh->m_TextSize();

        m_ArcDO.m_PntVec = arcpts;
        m_ArcDO.m_GeomChanged = true;
        m_ArcDO.m_Visible = m_Visible();
    }
}

vec3d Protractor::GetMidPt()
{
    vec3d mid;

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom *mgeom = veh->FindGeom( m_MidGeomID );
        if ( mgeom )
        {
            VspSurf *msurf = mgeom->GetSurfPtr( m_MidIndx());
            if ( msurf )
            {
                double umapmax = msurf->GetUMapMax();
                double umax = msurf->GetUMax();
                double u = msurf->InvertUMapping( m_MidU() * umapmax ) / umax;
                if ( u < 0 )
                {
                    u = m_MidU();
                }
                mid = msurf->CompPnt01( u, m_MidW());
            }
        }
    }
    return mid;
}

bool Protractor::Valid()
{
    if ( m_Stage == STAGE_ZERO )
    {
        return true;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* geom = veh->FindGeom( m_OriginGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_OriginIndx() < 0 || m_OriginIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }

        if ( m_Stage == STAGE_ONE )
        {
            return true;
        }

        geom = veh->FindGeom( m_MidGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_MidIndx() < 0 || m_MidIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }

        if ( m_Stage == STAGE_TWO )
        {
            return true;
        }

        geom = veh->FindGeom( m_EndGeomID );
        if ( !geom )
        {
            return false;
        }
        if ( m_EndIndx() < 0 || m_EndIndx() >= geom->GetNumTotalSurfs() )
        {
            return false;
        }
    }

    return true;
}

xmlNodePtr Protractor::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "OriginGeomID", m_OriginGeomID );
        XmlUtil::AddStringNode( parmcontain_node, "MidGeomID", m_MidGeomID );
        XmlUtil::AddStringNode( parmcontain_node, "EndGeomID", m_EndGeomID );
    }

    return parmcontain_node;
}

xmlNodePtr Protractor::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_OriginGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "OriginGeomID", m_OriginGeomID ) );
        m_MidGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "MidGeomID", m_MidGeomID ) );
        m_EndGeomID = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "EndGeomID", m_EndGeomID ) );
        m_Stage = STAGE_COMPLETE;
    }

    return parmcontain_node;
}
