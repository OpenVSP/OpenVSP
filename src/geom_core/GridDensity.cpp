//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GridDensity
//
//////////////////////////////////////////////////////////////////////

#include "GridDensity.h"
#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

BaseSource::BaseSource() : ParmContainer()
{
    m_Name = "Source_Name";
    m_Type = -1;
    m_GroupName = "Source";

    m_Len.Init( "SrcLen", m_GroupName, this, 0.1, 1.0e-8, 1.0e12 );
    m_Len.SetDescript( "Source edge length" );

    m_Rad.Init( "SrcRad", m_GroupName, this, 1.0, 1.0e-8, 1.0e12 );
    m_Rad.SetDescript( "Source influence radius" );

    m_MainSurfIndx = -1;
    m_SurfIndx = -1;
}

void BaseSource::ParmChanged( Parm* parm_ptr, int type )
{
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

//==== Encode XML ====//
xmlNodePtr BaseSource::EncodeXml(  xmlNodePtr & node  )
{
    xmlNodePtr src_node = xmlNewChild( node, NULL, BAD_CAST "Source", NULL );
    if ( src_node )
    {
        XmlUtil::AddIntNode( src_node, "Type", m_Type );

        ParmContainer::EncodeXml( src_node );
    }
    return src_node;
}

void BaseSource::AdjustLen( double val )
{
    m_Len = m_Len() * val;
}

void BaseSource::AdjustRad( double val )
{
    m_Rad = m_Rad() * val;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

PointSource::PointSource()
{
    SetLoc( vec3d() );

    m_ULoc.Init( "U_Loc", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_ULoc.SetDescript( "Source U location " );

    m_WLoc.Init( "W_Loc", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_WLoc.SetDescript( "Source W location" );

    m_Type = POINT_SOURCE;
    m_Name = "Point_Name";

    m_PointDO.m_GeomID = GetID();
    m_PointDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_PointDO.m_Type = DrawObj::VSP_LINE_LOOP;
    m_PointDO.m_LineWidth = 1.0;
    m_PointDO.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
}

void PointSource::SetNamedVal( string name, double val )
{
    if ( name == "U1" )
    {
        m_ULoc = val;
    }
    else if ( name == "W1" )
    {
        m_WLoc = val;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

LineSource::LineSource()
{
    SetEndPnts( vec3d(), vec3d() );

    m_Len2.Init( "SrcLen2", m_GroupName, this, 0.1, 1.0e-8, 1.0e12 );
    m_Len2.SetDescript( "Source edge length" );

    m_Rad2.Init( "SrcRad2", m_GroupName, this, 1.0, 1.0e-8, 1.0e12 );
    m_Rad2.SetDescript( "Source influence radius" );

    m_ULoc1.Init( "U_Loc1", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_ULoc1.SetDescript( "Source U1 location " );

    m_WLoc1.Init( "W_Loc1", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_WLoc1.SetDescript( "Source W1 location" );

    m_ULoc2.Init( "U_Loc2", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_ULoc2.SetDescript( "Source U2 location " );

    m_WLoc2.Init( "W_Loc2", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_WLoc2.SetDescript( "Source W2 location" );

    m_Type = LINE_SOURCE;
    m_Name = "Line_Name";

    m_LineDO1.m_GeomID = GetID() + "1";
    m_LineDO1.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LineDO1.m_Type = DrawObj::VSP_LINE_LOOP;
    m_LineDO1.m_LineWidth = 1.0;
    m_LineDO1.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_LineDO2.m_GeomID = GetID() + "2";
    m_LineDO2.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LineDO2.m_Type = DrawObj::VSP_LINE_LOOP;
    m_LineDO2.m_LineWidth = 1.0;
    m_LineDO2.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_LineDO3.m_GeomID = GetID() + "3";
    m_LineDO3.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LineDO3.m_Type = DrawObj::VSP_LINES;
    m_LineDO3.m_LineWidth = 1.0;
    m_LineDO3.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
}

void LineSource::SetNamedVal( string name, double val )
{
    if ( name == "U1" )
    {
        m_ULoc1 = val;
    }
    else if ( name == "W1" )
    {
        m_WLoc1 = val;
    }
    else if ( name == "U2" )
    {
        m_ULoc2 = val;
    }
    else if ( name == "W2" )
    {
        m_WLoc2 = val;
    }
    else if ( name == "Length2" )
    {
        m_Len2 = val;
    }
    else if ( name == "Radius2" )
    {
        m_Rad2 = val;
    }
}

void LineSource::UpdateBBox()
{
    m_Box.Update( vec3d( m_Pnt1[0] + m_Rad(), m_Pnt1[1] + m_Rad(),  m_Pnt1[2] + m_Rad() ) );
    m_Box.Update( vec3d( m_Pnt1[0] - m_Rad(), m_Pnt1[1] - m_Rad(),  m_Pnt1[2] - m_Rad() ) );
    m_Box.Update( vec3d( m_Pnt2[0] + m_Rad2(), m_Pnt2[1] + m_Rad2(),  m_Pnt2[2] + m_Rad2() ) );
    m_Box.Update( vec3d( m_Pnt2[0] - m_Rad2(), m_Pnt2[1] - m_Rad2(),  m_Pnt2[2] - m_Rad2() ) );
}

void LineSource::SetEndPnts( const vec3d & pnt1, const vec3d & pnt2 )
{
    m_Pnt1 = pnt1;
    m_Pnt2 = pnt2;
    m_Line = pnt2 - pnt1;
    m_DotLine = max( 0.0000001, dot( m_Line, m_Line ) );
    UpdateBBox();
}

void LineSource::AdjustLen( double val )
{
    m_Len = m_Len() * val;
    m_Len2 = m_Len2() * val;
}

void LineSource::AdjustRad( double val )
{
    m_Rad = m_Rad() * val;
    m_Rad2 = m_Rad2() * val;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

BoxSource::BoxSource()
{
    m_Type = BOX_SOURCE;
    m_Rad = 0;
    m_Name = "Box_Name";

    m_ULoc1.Init( "U_Loc1", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_ULoc1.SetDescript( "Source U1 location " );

    m_WLoc1.Init( "W_Loc1", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_WLoc1.SetDescript( "Source W1 location" );

    m_ULoc2.Init( "U_Loc2", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_ULoc2.SetDescript( "Source U2 location " );

    m_WLoc2.Init( "W_Loc2", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_WLoc2.SetDescript( "Source W2 location" );

    m_BoxDO1.m_GeomID = GetID() + "1";
    m_BoxDO1.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_BoxDO1.m_Type = DrawObj::VSP_LINE_LOOP;
    m_BoxDO1.m_LineWidth = 1.0;
    m_BoxDO1.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_BoxDO2.m_GeomID = GetID() + "2";
    m_BoxDO2.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_BoxDO2.m_Type = DrawObj::VSP_LINE_LOOP;
    m_BoxDO2.m_LineWidth = 1.0;
    m_BoxDO2.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_BoxDO3.m_GeomID = GetID() + "3";
    m_BoxDO3.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_BoxDO3.m_Type = DrawObj::VSP_LINES;
    m_BoxDO3.m_LineWidth = 1.0;
    m_BoxDO3.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
}

void BoxSource::ComputeCullPnts()
{
    m_CullMinPnt = m_MinPnt - vec3d( m_Rad(), m_Rad(), m_Rad() );
    m_CullMaxPnt = m_MaxPnt + vec3d( m_Rad(), m_Rad(), m_Rad() );
}

void BoxSource::SetMinMaxPnts( const vec3d & min_pnt, const vec3d & max_pnt )
{
    m_MinPnt = min_pnt;
    m_MaxPnt = max_pnt;
    ComputeCullPnts();
    m_Box.Update( m_CullMinPnt );
    m_Box.Update( m_CullMaxPnt );
}

void BoxSource::SetRad( double rad )
{
    m_Rad = rad;
    ComputeCullPnts();
    m_Box.Update( m_CullMinPnt );
    m_Box.Update( m_CullMaxPnt );
}

void BoxSource::SetNamedVal( string name, double val )
{
    if ( name == "U1" )
    {
        m_ULoc1 = val;
    }
    else if ( name == "W1" )
    {
        m_WLoc1 = val;
    }
    else if ( name == "U2" )
    {
        m_ULoc2 = val;
    }
    else if ( name == "W2" )
    {
        m_WLoc2 = val;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

GridDensity::GridDensity() : ParmContainer()
{
    m_GroupName = "NONE";
}

void GridDensity::InitParms()
{
    m_RigorLimit.Init( "RigorLimit", m_GroupName, this, false, 0, 1 );

    m_BaseLen.Init( "BaseLen", m_GroupName, this, 0.5, 1.0e-8, 1.0e12 );
    m_BaseLen.SetDescript( "Maximum mesh edge length" );

    m_MinLen.Init( "MinLen", m_GroupName, this, 0.1, 1.0e-8, 1.0e12 );
    m_MinLen.SetDescript( "Minimum mesh edge length" );

    m_MaxGap.Init( "MaxGap", m_GroupName, this, 0.005, 1.0e-8, 1.0e12 );
    m_MaxGap.SetDescript( "Maximum mesh edge gap" );

    m_NCircSeg.Init( "NCircSeg", m_GroupName, this, 16, 1.0e-8, 1.0e12 );
    m_NCircSeg.SetDescript( "Number of edge segments to resolve circle" );

    m_GrowRatio.Init( "GrowRatio", m_GroupName, this, 1.3, 1.0, 10.0 );
    m_GrowRatio.SetDescript( "Maximum allowed edge growth ratio" );

    m_FarMaxLen.Init( "MaxFar", m_GroupName, this, 2.0, 1.0e-8, 1.0e12 );
    m_FarMaxLen.SetDescript( "Maximum far field mesh edge length" );

    m_FarMaxGap.Init( "MaxFarGap", m_GroupName, this, 0.02, 1.0e-8, 1.0e12 );
    m_FarMaxGap.SetDescript( "Maximum far field mesh edge gap" );

    m_FarNCircSeg.Init( "FarNCircSeg", m_GroupName, this, 16, 1.0e-8, 1.0e12 );
    m_FarNCircSeg.SetDescript( "Number of far field edge segments to resolve circle" );

}

GridDensity::~GridDensity()
{
    ClearSources();
}


xmlNodePtr GridDensity::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr gridnode = xmlNewChild( node, NULL, BAD_CAST m_Name.c_str(), NULL );

    ParmContainer::EncodeXml( gridnode );

    return gridnode;
}

xmlNodePtr GridDensity::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr gridnode = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( gridnode )
    {
        ParmContainer::DecodeXml( gridnode );
    }

    return gridnode;
}

void GridDensity::ParmChanged( Parm* parm_ptr, int type )
{
    VehicleMgr.GetVehicle()->UpdateGui();
//    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

double GridDensity::GetRadFrac( bool farflag )
{
    if ( !farflag )
    {
        double radFrac;

        if( m_NCircSeg() > 2.0 )
        {
            radFrac = 2.0 * sin( PI / m_NCircSeg() );
        }
        else  // Switch to 4/n behavior below well defined range.
        {
            radFrac = 4.0 / m_NCircSeg();
        }

        return radFrac;
    }
    else
    {
        return GetFarRadFrac();
    }
}

double GridDensity::GetFarRadFrac()
{
    double radFrac;

    if( m_FarNCircSeg() > 2.0 )
    {
        radFrac = 2.0 * sin( PI / m_FarNCircSeg() );
    }
    else  // Switch to 4/n behavior below well defined range.
    {
        radFrac = 4.0 / m_FarNCircSeg();
    }

    return radFrac;
}

void GridDensity::RemoveSource( BaseSource* s )
{
    vector< BaseSource* > sVec;
    for ( int i = 0 ; i < ( int )m_Sources.size() ; i++ )
    {
        if ( m_Sources[i] != s )
        {
            sVec.push_back( m_Sources[i] );
        }
    }
    m_Sources = sVec;
}

double GridDensity::GetTargetLen( vec3d& pos, bool farFlag )
{
    double target_len;

    if( !farFlag )
    {
        target_len = m_BaseLen();
    }
    else
    {
        target_len = m_FarMaxLen();
    }

    for ( int i = 0 ; i < ( int )m_Sources.size() ; i++ )
    {
        double len = m_Sources[i]->GetTargetLen( target_len, pos );
        if ( len < target_len )
        {
            target_len = len;
        }
    }
    return target_len;
}

void GridDensity::ScaleAllSources( double scale )
{
    for ( int i = 0 ; i < ( int )m_Sources.size() ; i++ )
    {
        m_Sources[i]->AdjustLen( scale );
    }
}


void GridDensity::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0; i < ( int )m_Sources.size(); i++ )
    {
        m_Sources[i]->LoadDrawObjs( draw_obj_vec );
    }
}

void GridDensity::Show( bool flag )
{
    for ( int i = 0; i < ( int )m_Sources.size(); i++ )
    {
        m_Sources[i]->Show( flag );
    }
}

void GridDensity::Highlight( BaseSource * source )
{
    for ( int i = 0; i < ( int )m_Sources.size(); i++ )
    {
        if( m_Sources[i] == source )
        {
            m_Sources[i]->Highlight( true );
        }
        else
        {
            m_Sources[i]->Highlight( false );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

CfdGridDensity::CfdGridDensity() : GridDensity()
{
    m_Name = "CFDGridDensity";
    m_GroupName = "CFDGridDensity";
    InitParms();
}

FeaGridDensity::FeaGridDensity() : GridDensity()
{
    m_Name = "FEAGridDensity";
    m_GroupName = "FEAGridDensity";
    InitParms();
}
