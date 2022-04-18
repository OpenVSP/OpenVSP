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
        m_OriginGeomID = XmlUtil::FindString( parmcontain_node, "OriginGeomID", "" );
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
        vec3d pt = veh->CompPnt01(m_OriginGeomID, m_OriginIndx(), m_OriginU(), m_OriginW());

        m_X = pt.x();
        m_Y = pt.y();
        m_Z = pt.z();

        vec3d norm = veh->CompNorm01(m_OriginGeomID, m_OriginIndx(), m_OriginU(), m_OriginW());

        m_NX = norm.x();
        m_NY = norm.y();
        m_NZ = norm.z();

        double k1, k2, ka, kg;
        veh->CompCurvature01(m_OriginGeomID, m_OriginIndx(), m_OriginU(), m_OriginW(), k1, k2, ka, kg);

        m_K1 = k1;
        m_K2 = k2;
        m_Ka = ka;
        m_Kg = kg;

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
    m_OriginS.Init( "OriginS", "Measure", this, 0.0, 0.0, 0.5 );
    m_OriginT.Init( "OriginT", "Measure", this, 0.0, 0.0, 1.0 );

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
        m_OriginGeomID = XmlUtil::FindString( parmcontain_node, "OriginGeomID", "" );
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
        vec3d pt = veh->CompPntRST( m_OriginGeomID, m_OriginIndx(), m_OriginR(), m_OriginS(), m_OriginT() );

        m_X = pt.x();
        m_Y = pt.y();
        m_Z = pt.z();

        vec3d norm = veh->CompNorm01( m_OriginGeomID, m_OriginIndx(), m_OriginR(), m_OriginS() );

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
        vec3d origin = veh->CompPnt01(m_OriginGeomID, m_OriginIndx(), m_OriginU(), m_OriginW());
        vec3d end = veh->CompPnt01(m_EndGeomID, m_EndIndx(), m_EndU(), m_EndW());

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
        sprintf( str, "%s%.*f %s", dir, m_Precision(), delta.mag(), LenUnitName( veh->m_MeasureLenUnit() ).c_str() );

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
        m_OriginGeomID = XmlUtil::FindString( parmcontain_node, "OriginGeomID", "" );
        m_EndGeomID = XmlUtil::FindString( parmcontain_node, "EndGeomID", "" );
        m_Stage = STAGE_COMPLETE;
    }

    return parmcontain_node;
}
