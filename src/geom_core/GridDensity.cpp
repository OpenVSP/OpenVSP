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

//  m_ReflSource = NULL;
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

void BaseSource::CheckCorrectRad( double base_len )
{
    double len = m_Len();
    double sum_len = 0;
    while ( len < base_len )
    {
        sum_len += len;
        len *= 1.20;            // Only Allow 20% Increase in Length
    }

    if ( m_Rad() < sum_len )
    {
        m_Rad = sum_len;
    }
}

void BaseSource::AdjustLen( double val )
{
    m_Len = m_Len() * val;
//  if ( m_ReflSource )
//      m_ReflSource->SetLen( m_Len );
}
void BaseSource::AdjustRad( double val )
{
    m_Rad = m_Rad() * val;
//  if ( m_ReflSource )
//      m_ReflSource->SetRad( m_Rad );
}

vector< vec3d > BaseSource::CreateSphere( double rad, const vec3d& loc )
{
    int i, j;
    int num_lats = 8;
    int num_longs = 8;

    vector< vec3d > sphere;

    for ( i = 0; i < num_lats; i++ )
    {
        double lat = PI * ( -0.5 + ( double )i / num_lats );
        double z  = rad * sin( lat );
        double zr = rad * cos( lat );

        for ( j = 0; j < num_longs; j++ )
        {
            double lng = 2 * PI * ( double )j / num_longs;
            double x = cos( lng ) * zr;
            double y = sin( lng ) * zr;

            sphere.push_back( vec3d( x + loc[0], y + loc[1], z + loc[2] ) );
        }
    }
    return sphere;
}

/*
void BaseSource::DrawSphere( double rad, const vec3d& loc )
{
    int i, j;
    int num_lats = 8;
    int num_longs = 8;

    for ( i = 0 ; i < num_lats ; i++ )
    {
        glBegin( GL_LINE_LOOP );
        double lat = PI * (-0.5 + (double)i/num_lats);
        double z  = rad*sin(lat);
        double zr = rad*cos(lat);

        for ( j = 0 ; j < num_longs ; j++ )
        {
            double lng = 2 * PI * (double)j/num_longs;
            double x = cos(lng)*zr;
            double y = sin(lng)*zr;
            glVertex3d(x + loc[0], y + loc[1], z + loc[2]);
        }
        glEnd();
    }

}
*/

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
    m_PointDO.m_Type = DrawObj::VSP_LINE_LOOP;
    m_PointDO.m_LineWidth = 1.0;
    m_PointDO.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
}

void PointSource::Copy( BaseSource* s )
{
    m_Name = s->GetName();
    m_Type = s->GetType();
    m_Len = s->m_Len();
    m_Rad = s->m_Rad();
    m_Box = s->GetBBox();

    m_ULoc = ( ( PointSource* )s )->m_ULoc();
    m_WLoc = ( ( PointSource* )s )->m_WLoc();
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

double PointSource::GetTargetLen( double base_len, vec3d &  pos )
{
    double dist2 = dist_squared( pos, m_Loc );

    double radSquared = m_Rad() * m_Rad();

    if ( dist2 > radSquared )
    {
        return base_len;
    }

    double fract = dist2 / radSquared;

    return ( m_Len() + fract * ( base_len - m_Len()  ) );
}

bool PointSource::ReadData( char* buff )
{
    char name[256];
    float x, y, z, rad, len;
    sscanf( buff, "%s %f %f %f  %f  %f", name, &x, &y, &z, &rad, &len );
    vec3d loc = vec3d( x, y, z );
    SetLoc( loc );
    m_Rad = rad;
    m_Len = len;

    m_Box.Update( vec3d( x + rad, y, z ) );
    m_Box.Update( vec3d( x - rad, y, z ) );
    m_Box.Update( vec3d( x , y + rad, z ) );
    m_Box.Update( vec3d( x, y - rad, z ) );
    m_Box.Update( vec3d( x, y, z + rad ) );
    m_Box.Update( vec3d( x, y, z - rad ) );

    return true;
}

void PointSource::Update( Geom* geomPtr )
{
    vec3d p = geomPtr->GetUWPt( m_ULoc(), m_WLoc() );

    SetLoc( p );

//  if ( geomPtr->getSymCode() == NO_SYM )
//  {
//      if ( m_ReflSource )
//      {
//          delete m_ReflSource;
//          m_ReflSource = NULL;
//      }
//  }
//  else
//  {
//      if ( !m_ReflSource )
//          m_ReflSource = new PointSource();
//
//      PointSource* ps = (PointSource*)m_ReflSource;
//
//      ps->SetGeomPtr( geomPtr );
//      ps->SetLen( m_Len );
//      ps->SetRad( m_Rad );
//      ps->SetName( m_Name );
//
//      vec3d symVec = geomPtr->getSymVec();
//      symVec = m_Loc*symVec;
//      ps->SetLoc( symVec );
//  }

}

/*
void PointSource::Draw()
{
    glPushMatrix();

    DrawSphere( m_Rad, m_Loc );
    //glTranslated( m_Loc[0], m_Loc[1], m_Loc[2] );
    //gluSphere(m_Quadric, m_Rad, 6, 6);

    glPopMatrix();
}
*/

void PointSource::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    m_PointDO.m_PntVec = CreateSphere( m_Rad.Get(), m_Loc );
    draw_obj_vec.push_back( &m_PointDO );
}

void PointSource::Show( bool flag )
{
    m_PointDO.m_Visible = flag;
}

void PointSource::Highlight( bool flag )
{
    if( flag )
    {
        m_PointDO.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
    }
    else
    {
        m_PointDO.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
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
    m_LineDO1.m_Type = DrawObj::VSP_LINE_LOOP;
    m_LineDO1.m_LineWidth = 1.0;
    m_LineDO1.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_LineDO2.m_GeomID = GetID() + "2";
    m_LineDO2.m_Type = DrawObj::VSP_LINE_LOOP;
    m_LineDO2.m_LineWidth = 1.0;
    m_LineDO2.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_LineDO3.m_GeomID = GetID() + "3";
    m_LineDO3.m_Type = DrawObj::VSP_LINES;
    m_LineDO3.m_LineWidth = 1.0;
    m_LineDO3.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
}

void LineSource::Copy( BaseSource* s )
{
    m_Name = s->GetName();
    m_Type = s->GetType();
    m_Len = s->m_Len();
    m_Rad = s->m_Rad();
    m_Box = s->GetBBox();

    m_Len = ( ( LineSource* )s )->m_Len();
    m_Len2 = ( ( LineSource* )s )->m_Len2();
    m_Rad = ( ( LineSource* )s )->m_Rad();
    m_Rad2 = ( ( LineSource* )s )->m_Rad2();

    m_ULoc1 = ( ( LineSource* )s )->m_ULoc1();
    m_WLoc1 = ( ( LineSource* )s )->m_WLoc1();

    m_ULoc2 = ( ( LineSource* )s )->m_ULoc2();
    m_WLoc2 = ( ( LineSource* )s )->m_WLoc2();
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
//  if ( m_ReflSource )
//  {
//      ((LineSource*)m_ReflSource)->m_Len = m_Len1());
//      ((LineSource*)m_ReflSource)->m_Len2 = m_Len2();
//  }
}

void LineSource::AdjustRad( double val )
{
    m_Rad = m_Rad() * val;
    m_Rad2 = m_Rad2() * val;
//  if ( m_ReflSource )
//  {
//      ((LineSource*)m_ReflSource)->m_Rad = m_Rad1();
//      ((LineSource*)m_ReflSource)->m_Rad2 = m_Rad2();
//  }
}

double LineSource::GetTargetLen( double base_len, vec3d &  pos )
{

    double retlen = base_len;
    if ( !m_Box.CheckPnt( pos[0], pos[1], pos[2] ) )
    {
        return retlen;
    }

    vec3d origVec = pos - m_Pnt1;
    double numer = dot( m_Line, origVec );

    double dist2;
    double t = numer / m_DotLine;

    if ( t <= 0 )
    {
        dist2 = dist_squared( m_Pnt1, pos );
        if ( dist2 > m_RadSquared1 )
        {
            return base_len;
        }
        double fract = dist2 / m_RadSquared1;
        retlen = m_Len() + fract * ( base_len - m_Len()  );
    }
    else if ( t >= 1 )
    {
        dist2 = dist_squared( m_Pnt2, pos );
        if ( dist2 > m_RadSquared2 )
        {
            return base_len;
        }
        double fract = dist2 / m_RadSquared2;
        retlen = m_Len2() + fract * ( base_len - m_Len2()  );
    }
    else
    {
        vec3d proj = m_Pnt1 + m_Line * t;
        dist2 = dist_squared( proj, pos );

        double fract_rad = m_Rad() + ( m_Rad2() - m_Rad() ) * t;
        double fract_rad_sqr = fract_rad * fract_rad;

        if ( dist2 > fract_rad_sqr )
        {
            return base_len;
        }

        double fract_sqr = dist2 / fract_rad_sqr;
        double fract_len = m_Len() + ( m_Len2() - m_Len() ) * t;

        retlen = fract_len + fract_sqr * ( base_len - fract_len  );
    }

    if ( retlen > base_len )
    {
        printf( "Ret Len\n" );
    }
    else if ( retlen < m_Len() && retlen < m_Len2() )
    {
        printf( "Ret Len\n" );
    }


    return retlen;
}

void LineSource::Update( Geom* geomPtr )
{
    m_Pnt1 = geomPtr->GetUWPt( m_ULoc1(), m_WLoc1() );
    m_Pnt2 = geomPtr->GetUWPt( m_ULoc2(), m_WLoc2() );
    SetEndPnts( m_Pnt1, m_Pnt2 );
}

/*
void LineSource::Draw()
{
    vec3d p;

    DrawSphere( m_Rad1, m_Pnt1 );
    DrawSphere( m_Rad2, m_Pnt2 );
    //glPushMatrix();
    //glTranslated( m_Pnt1[0], m_Pnt1[1], m_Pnt1[2] );
    //gluSphere(m_Quadric, m_Rad, 6, 6);
    //glPopMatrix();
    //glPushMatrix();
    //glTranslated( m_Pnt2[0], m_Pnt2[1], m_Pnt2[2] );
    //gluSphere(m_Quadric, m_Rad, 6, 6);
    //glPopMatrix();

    glBegin( GL_LINES );
        glVertex3d( m_Pnt1[0], m_Pnt1[1], m_Pnt1[2]+m_Rad1);
        glVertex3d( m_Pnt2[0], m_Pnt2[1], m_Pnt2[2]+m_Rad2);
        glVertex3d( m_Pnt1[0], m_Pnt1[1], m_Pnt1[2]-m_Rad1 );
        glVertex3d( m_Pnt2[0], m_Pnt2[1], m_Pnt2[2]-m_Rad2 );
        glVertex3d( m_Pnt1[0], m_Pnt1[1]+m_Rad1, m_Pnt1[2] );
        glVertex3d( m_Pnt2[0], m_Pnt2[1]+m_Rad2, m_Pnt2[2] );
        glVertex3d( m_Pnt1[0], m_Pnt1[1]-m_Rad1, m_Pnt1[2] );
        glVertex3d( m_Pnt2[0], m_Pnt2[1]-m_Rad2, m_Pnt2[2] );
        glVertex3d( m_Pnt1[0]+m_Rad1, m_Pnt1[1], m_Pnt1[2] );
        glVertex3d( m_Pnt2[0]+m_Rad2, m_Pnt2[1], m_Pnt2[2] );
        glVertex3d( m_Pnt1[0]-m_Rad1, m_Pnt1[1], m_Pnt1[2] );
        glVertex3d( m_Pnt2[0]-m_Rad2, m_Pnt2[1], m_Pnt2[2] );
    glEnd();
}
*/

void LineSource::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    m_LineDO1.m_PntVec = CreateSphere( m_Rad.Get(), m_Pnt1 );

    m_LineDO2.m_PntVec = CreateSphere( m_Rad2.Get(), m_Pnt2 );

    vector< vec3d > lines;
    lines.push_back( vec3d( m_Pnt1[0], m_Pnt1[1], m_Pnt1[2] + m_Rad.Get() ) );
    lines.push_back( vec3d( m_Pnt2[0], m_Pnt2[1], m_Pnt2[2] + m_Rad2.Get() ) );
    lines.push_back( vec3d( m_Pnt1[0], m_Pnt1[1], m_Pnt1[2] - m_Rad.Get() ) );
    lines.push_back( vec3d( m_Pnt2[0], m_Pnt2[1], m_Pnt2[2] - m_Rad2.Get() ) );
    lines.push_back( vec3d( m_Pnt1[0], m_Pnt1[1] + m_Rad.Get(), m_Pnt1[2] ) );
    lines.push_back( vec3d( m_Pnt2[0], m_Pnt2[1] + m_Rad2.Get(), m_Pnt2[2] ) );
    lines.push_back( vec3d( m_Pnt1[0], m_Pnt1[1] - m_Rad.Get(), m_Pnt1[2] ) );
    lines.push_back( vec3d( m_Pnt2[0], m_Pnt2[1] - m_Rad2.Get(), m_Pnt2[2] ) );
    lines.push_back( vec3d( m_Pnt1[0] + m_Rad.Get(), m_Pnt1[1], m_Pnt1[2] ) );
    lines.push_back( vec3d( m_Pnt2[0] + m_Rad2.Get(), m_Pnt2[1], m_Pnt2[2] ) );
    lines.push_back( vec3d( m_Pnt1[0] - m_Rad.Get(), m_Pnt1[1], m_Pnt1[2] ) );
    lines.push_back( vec3d( m_Pnt2[0] - m_Rad2.Get(), m_Pnt2[1], m_Pnt2[2] ) );
    m_LineDO3.m_PntVec = lines;

    draw_obj_vec.push_back( &m_LineDO1 );
    draw_obj_vec.push_back( &m_LineDO2 );
    draw_obj_vec.push_back( &m_LineDO3 );
}

void LineSource::Show( bool flag )
{
    m_LineDO1.m_Visible = flag;
    m_LineDO2.m_Visible = flag;
    m_LineDO3.m_Visible = flag;
}

void LineSource::Highlight( bool flag )
{
    if( flag )
    {
        m_LineDO1.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
        m_LineDO2.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
        m_LineDO3.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
    }
    else
    {
        m_LineDO1.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
        m_LineDO2.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
        m_LineDO3.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
    }
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
    m_BoxDO1.m_Type = DrawObj::VSP_LINE_LOOP;
    m_BoxDO1.m_LineWidth = 1.0;
    m_BoxDO1.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_BoxDO2.m_GeomID = GetID() + "2";
    m_BoxDO2.m_Type = DrawObj::VSP_LINE_LOOP;
    m_BoxDO2.m_LineWidth = 1.0;
    m_BoxDO2.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );

    m_BoxDO3.m_GeomID = GetID() + "3";
    m_BoxDO3.m_Type = DrawObj::VSP_LINES;
    m_BoxDO3.m_LineWidth = 1.0;
    m_BoxDO3.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
}

void BoxSource::ComputeCullPnts()
{
    m_CullMinPnt = m_MinPnt - vec3d( m_Rad(), m_Rad(), m_Rad() );
    m_CullMaxPnt = m_MaxPnt + vec3d( m_Rad(), m_Rad(), m_Rad() );
}


void BoxSource::Copy( BaseSource* s )
{
    m_Name = s->GetName();
    m_Type = s->GetType();
    m_Len = s->m_Len();
    m_Rad = s->m_Rad();
    m_Box = s->GetBBox();

    m_ULoc1 = ( ( BoxSource* )s )->m_ULoc1();
    m_WLoc2 = ( ( BoxSource* )s )->m_WLoc2();
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

double BoxSource::GetTargetLen( double base_len, vec3d &  pos )
{
    if ( pos[0] <= m_CullMinPnt[0] )
    {
        return base_len;
    }
    if ( pos[1] <= m_CullMinPnt[1] )
    {
        return base_len;
    }
    if ( pos[2] <= m_CullMinPnt[2] )
    {
        return base_len;
    }
    if ( pos[0] >= m_CullMaxPnt[0] )
    {
        return base_len;
    }
    if ( pos[1] >= m_CullMaxPnt[1] )
    {
        return base_len;
    }
    if ( pos[2] >= m_CullMaxPnt[2] )
    {
        return base_len;
    }

    if ( pos[0] > m_MinPnt[0] && pos[0] < m_MaxPnt[0] &&
            pos[1] > m_MinPnt[1] && pos[1] < m_MaxPnt[1] &&
            pos[2] > m_MinPnt[2] && pos[2] < m_MaxPnt[2] )
    {
        return m_Len();
    }

    double fract[3];

    for ( int i = 0 ; i < 3 ; i++ )
    {
        fract[i] = 0;
        if ( pos[i] < m_MinPnt[i] )
        {
            fract[i] = ( m_MinPnt[i] - pos[i] ) / m_Rad();
        }
        else if ( pos[i] > m_MaxPnt[i] )
        {
            fract[i] = ( pos[i] - m_MaxPnt[i] ) / m_Rad();
        }
    }

    double max_fract = max( max( fract[0], fract[1] ), fract[2] );
    return ( m_Len() + max_fract * ( base_len - m_Len()  ) );
}

bool BoxSource::ReadData( char* buff )
{
    char name[256];
    float x, y, z, xx, yy, zz, rad, len;
    sscanf( buff, "%s %f %f %f  %f %f %f  %f  %f", name, &x, &y, &z,  &xx, &yy, &zz, &rad, &len );
    vec3d p0 = vec3d( x, y, z );
    vec3d p1 = vec3d( xx, yy, zz );
    SetMinMaxPnts( p0, p1 );
    m_Rad = rad;
    m_Len = len;

    m_Box.Update( vec3d( x - rad, y - rad, z - rad ) );
    m_Box.Update( vec3d( x + rad, y + rad, z + rad ) );

    ComputeCullPnts();

    return true;
}

void BoxSource::Update( Geom* geomPtr )
{
    BndBox box;
    int num_segs = 8;
    vector< vec3d > pVec;
    for ( int i = 0 ; i < num_segs ; i++ )
    {
        double fu = ( double )i / ( double )( num_segs - 1 );
        double u = m_ULoc1() + fu * ( m_ULoc2() - m_ULoc1() );
        for ( int j = 0 ; j < num_segs ; j++ )
        {
            double fw = ( double )j / ( double )( num_segs - 1 );
            double w = m_WLoc1() + fu * ( m_WLoc2() - m_WLoc1() );
            vec3d p = geomPtr->GetUWPt( u, w );
            pVec.push_back( p );
            box.Update( p );
        }
    }
    m_Box = box;
    vec3d p0 = box.GetCornerPnt( 0 );
    vec3d p1 = box.GetCornerPnt( 7 );
    SetMinMaxPnts( p0, p1 );

}

/*
void BoxSource::Draw()
{
    glBegin( GL_LINE_LOOP );
        glVertex3dv( m_Box.get_pnt(0).data() );
        glVertex3dv( m_Box.get_pnt(1).data() );
        glVertex3dv( m_Box.get_pnt(3).data() );
        glVertex3dv( m_Box.get_pnt(2).data() );
    glEnd();

    glBegin( GL_LINE_LOOP );
        glVertex3dv( m_Box.get_pnt(4).data() );
        glVertex3dv( m_Box.get_pnt(5).data() );
        glVertex3dv( m_Box.get_pnt(7).data() );
        glVertex3dv( m_Box.get_pnt(6).data() );
    glEnd();

    glBegin( GL_LINES );
        glVertex3dv( m_Box.get_pnt(0).data() );
        glVertex3dv( m_Box.get_pnt(4).data() );
        glVertex3dv( m_Box.get_pnt(1).data() );
        glVertex3dv( m_Box.get_pnt(5).data() );
        glVertex3dv( m_Box.get_pnt(3).data() );
        glVertex3dv( m_Box.get_pnt(7).data() );
        glVertex3dv( m_Box.get_pnt(2).data() );
        glVertex3dv( m_Box.get_pnt(6).data() );
    glEnd();

}
*/

void BoxSource::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    vector< vec3d > loop1 , loop2, lines;

    loop1.push_back( m_Box.GetCornerPnt( 0 ) );
    loop1.push_back( m_Box.GetCornerPnt( 1 ) );
    loop1.push_back( m_Box.GetCornerPnt( 3 ) );
    loop1.push_back( m_Box.GetCornerPnt( 2 ) );
    m_BoxDO1.m_PntVec = loop1;

    loop2.push_back( m_Box.GetCornerPnt( 4 ) );
    loop2.push_back( m_Box.GetCornerPnt( 5 ) );
    loop2.push_back( m_Box.GetCornerPnt( 7 ) );
    loop2.push_back( m_Box.GetCornerPnt( 6 ) );
    m_BoxDO2.m_PntVec = loop2;

    lines.push_back( m_Box.GetCornerPnt( 0 ) );
    lines.push_back( m_Box.GetCornerPnt( 4 ) );
    lines.push_back( m_Box.GetCornerPnt( 1 ) );
    lines.push_back( m_Box.GetCornerPnt( 5 ) );
    lines.push_back( m_Box.GetCornerPnt( 3 ) );
    lines.push_back( m_Box.GetCornerPnt( 7 ) );
    lines.push_back( m_Box.GetCornerPnt( 2 ) );
    lines.push_back( m_Box.GetCornerPnt( 6 ) );
    m_BoxDO3.m_PntVec = lines;

    draw_obj_vec.push_back( &m_BoxDO1 );
    draw_obj_vec.push_back( &m_BoxDO2 );
    draw_obj_vec.push_back( &m_BoxDO3 );
}

void BoxSource::Show( bool flag )
{
    m_BoxDO1.m_Visible = flag;
    m_BoxDO2.m_Visible = flag;
    m_BoxDO3.m_Visible = flag;
}

void BoxSource::Highlight( bool flag )
{
    if( flag )
    {
        m_BoxDO1.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
        m_BoxDO2.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
        m_BoxDO3.m_LineColor = vec3d( 1, 100.0 / 255, 0 );
    }
    else
    {
        m_BoxDO1.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
        m_BoxDO2.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
        m_BoxDO3.m_LineColor = vec3d( 100.0 / 255, 100.0 / 255, 100.0 / 255 );
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

void GridDensity::ParmChanged( Parm* parm_ptr, int type )
{
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
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

/*
void GridDensity::Draw(BaseSource* curr_source )
{
    for ( int i = 0 ; i < (int)m_Sources.size() ; i++ )
    {
        glColor4ub( 100, 100, 100, 255 );
        if ( curr_source == m_Sources[i] )
            glColor4ub( 255, 100, 0, 255 );

        m_Sources[i]->Draw();
    }
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

CfdGridDensity::CfdGridDensity() : GridDensity()
{
    m_GroupName = "CFDGridDensity";
    InitParms();
}

FeaGridDensity::FeaGridDensity() : GridDensity()
{
    m_GroupName = "FEAGridDensity";
    InitParms();
}
