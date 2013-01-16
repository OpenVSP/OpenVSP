//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GridDensity
//
//////////////////////////////////////////////////////////////////////

#include "GridDensity.h"
#include "geom.h"

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

BaseSource::BaseSource()
{
	m_Name = Stringc("Source_Name");
	m_ReflSource = NULL;
}


void BaseSource::CheckCorrectRad( double base_len )
{
	double len = m_Len;
	double sum_len = 0;
	while ( len < base_len )
	{
		sum_len += len;
		len *= 1.20;			// Only Allow 20% Increase in Length
	}

	if ( m_Rad < sum_len )
	{
		SetRad( sum_len );
	}
}

void BaseSource::AdjustLen(double val)
{
	SetLen(  m_Len*val );
	if ( m_ReflSource )
		m_ReflSource->SetLen( m_Len );
}
void BaseSource::AdjustRad(double val)
{
	SetRad( m_Rad*val );
	if ( m_ReflSource )
		m_ReflSource->SetRad( m_Rad );
}

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

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

void PointSource::Copy( BaseSource* s )
{
	m_Name = s->GetName();
	m_GeomPtr = s->GetGeomPtr();
	m_Type = s->GetType();
	m_Len = s->GetLen();
	m_Rad = s->GetRad();
	m_RadSquared = m_Rad*m_Rad;
	m_Box = s->GetBBox();

	m_UWLoc = ((PointSource*)s)->GetUWLoc();
}

void PointSource::SetNamedVal( Stringc name, double val )
{
	if ( name == "U1" )				m_UWLoc[0] = val;
	else if ( name == "W1" )		m_UWLoc[1] = val;	 
}

double PointSource::GetTargetLen( double base_len, vec3d &  pos )
{
	double dist2 = dist_squared( pos, m_Loc );

	if ( dist2 > m_RadSquared )
		return base_len;

	double fract = dist2/m_RadSquared;

	return (m_Len + fract*( base_len - m_Len  ));
}

bool PointSource::ReadData( char* buff )
{
	char name[256];
	float x, y, z, rad, len;
	sscanf( buff, "%s %f %f %f  %f  %f", name, &x, &y, &z, &rad, &len );
	vec3d loc = vec3d(x,y,z);
	SetLoc( loc );
	SetRad( rad );
	SetLen( len );

	m_Box.update( vec3d( x + rad, y, z ) );			m_Box.update( vec3d( x - rad, y, z ) );
	m_Box.update( vec3d( x , y + rad, z ) );		m_Box.update( vec3d( x, y - rad, z ) );
	m_Box.update( vec3d( x, y, z + rad ) );			m_Box.update( vec3d( x, y, z - rad ) );

	return true;
}

void PointSource::Update( Geom* geomPtr )
{
	vec3d p = geomPtr->getAttachUVPos( m_UWLoc[0], m_UWLoc[1] );
	SetLoc( p );

	if ( geomPtr->getSymCode() == NO_SYM )
	{
		if ( m_ReflSource )
		{
			delete m_ReflSource;
			m_ReflSource = NULL;
		}
	}
	else
	{
		if ( !m_ReflSource )
			m_ReflSource = new PointSource();

		PointSource* ps = (PointSource*)m_ReflSource;

		ps->SetGeomPtr( geomPtr );
		ps->SetLen( m_Len );
		ps->SetRad( m_Rad );
		ps->SetName( m_Name );

		vec3d symVec = geomPtr->getSymVec();
		symVec = m_Loc*symVec;
		ps->SetLoc( symVec );
	}

}


void PointSource::Draw()
{
	glPushMatrix();

	DrawSphere( m_Rad, m_Loc );
	//glTranslated( m_Loc[0], m_Loc[1], m_Loc[2] );
	//gluSphere(m_Quadric, m_Rad, 6, 6);

	glPopMatrix();
}

void PointSource::WriteParms( xmlNodePtr root )
{
	xmlAddStringNode( root, "Name", GetName() );
	xmlAddIntNode( root, "Type", m_Type );

	xmlAddDoubleNode( root, "Rad", m_Rad );
	xmlAddDoubleNode( root, "Len", m_Len );

	xmlAddDoubleNode( root, "U", m_UWLoc.x() );
	xmlAddDoubleNode( root, "W", m_UWLoc.y() );
}

void PointSource::ReadParms( xmlNodePtr root )
{
	SetName( xmlFindString( root, "Name", GetName() ) );

	SetLen( xmlFindDouble( root, "Len", m_Len ) );
	SetRad( xmlFindDouble( root, "Rad", m_Rad ) );

	double u = xmlFindDouble( root, "U", m_UWLoc[0] );
	double w = xmlFindDouble( root, "W", m_UWLoc[1] );
	vec2d loc = vec2d(u, w);
	SetUWLoc( loc );
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
	SetRad(0.0);
	SetRad2(0.0);
	SetLen(0.0);
	SetLen2(0.0);
	m_Type = LINE_SOURCE; 
	m_Name = Stringc("Line_Name"); 
}

LineSource::LineSource( double len1, double len2, double r1, double r2, vec3d & pnt1, vec3d & pnt2 )
{
	m_Name = Stringc("Line_Name");
	m_Type = LINE_SOURCE;
	SetEndPnts( pnt1, pnt2 );
	SetRad(r1);
	SetRad2(r2);
	SetLen(len1);
	SetLen2(len2);
}

void LineSource::Copy( BaseSource* s )
{
	m_Name = s->GetName();
	m_GeomPtr = s->GetGeomPtr();
	m_Type = s->GetType();
	m_Len = s->GetLen();
	m_Rad = s->GetRad();
	m_RadSquared = m_Rad*m_Rad;
	m_Box = s->GetBBox();

	SetLen(((LineSource*)s)->GetLen());
	SetLen2(((LineSource*)s)->GetLen2());
	SetRad(((LineSource*)s)->GetRad());
	SetRad2(((LineSource*)s)->GetRad2());

	m_UWLoc1 = ((LineSource*)s)->GetUWLoc1();
	m_UWLoc2 = ((LineSource*)s)->GetUWLoc2();
}

void LineSource::SetNamedVal( Stringc name, double val )
{
	if ( name == "U1" )				m_UWLoc1[0] = val;
	else if ( name == "W1" )		m_UWLoc1[1] = val;	 
	else if ( name == "U2" )		m_UWLoc2[0] = val;	 
	else if ( name == "W2" )		m_UWLoc2[1] = val;
	else if ( name == "Length2" )	SetLen2( val );
	else if ( name == "Radius2" )	SetRad2( val );
}

void LineSource::UpdateBBox()
{
	m_Box.update( vec3d( m_Pnt1[0] + m_Rad1, m_Pnt1[1] + m_Rad1,  m_Pnt1[2] + m_Rad1 ) );			
	m_Box.update( vec3d( m_Pnt1[0] - m_Rad1, m_Pnt1[1] - m_Rad1,  m_Pnt1[2] - m_Rad1 ) );
	m_Box.update( vec3d( m_Pnt2[0] + m_Rad2, m_Pnt2[1] + m_Rad2,  m_Pnt2[2] + m_Rad2 ) );			
	m_Box.update( vec3d( m_Pnt2[0] - m_Rad2, m_Pnt2[1] - m_Rad2,  m_Pnt2[2] - m_Rad2 ) );
}

void LineSource::SetEndPnts( const vec3d & pnt1, const vec3d & pnt2 )
{
	m_Pnt1 = pnt1;
	m_Pnt2 = pnt2;
	m_Line = pnt2 - pnt1;
	m_DotLine = max( 0.0000001, dot( m_Line, m_Line ));
	UpdateBBox();
}

void LineSource::SetRad( double r1 )
{
	m_Rad1 = r1; 
	m_RadSquared1 = r1*r1;
	UpdateBBox();
}

void LineSource::SetRad2( double r2 )
{
	m_Rad2 = r2; 
	m_RadSquared2 = r2*r2;
	UpdateBBox();
}

void LineSource::SetLen( double len1 )
{
	m_Len1 = len1;
}

void LineSource::SetLen2( double len2 )
{
	m_Len2 = len2;
}
void LineSource::AdjustLen(double val)
{
	SetLen(  m_Len1*val );
	SetLen2(  m_Len2*val );
	if ( m_ReflSource )
	{
		((LineSource*)m_ReflSource)->SetLen( m_Len1 );
		((LineSource*)m_ReflSource)->SetLen2( m_Len2 );
	}
}
void LineSource::AdjustRad(double val)
{
	SetRad( m_Rad1*val );
	SetRad2( m_Rad2*val );
	if ( m_ReflSource )
	{
		((LineSource*)m_ReflSource)->SetRad( m_Rad1 );
		((LineSource*)m_ReflSource)->SetRad2( m_Rad2 );
	}
}

double LineSource::GetTargetLen( double base_len, vec3d &  pos )
{

	double retlen = base_len;
	if ( !m_Box.check_pnt( pos[0], pos[1], pos[2] ) )
		return retlen;

	vec3d origVec = pos - m_Pnt1;
	double numer = dot( m_Line, origVec );

	double dist2;
	double t = numer/m_DotLine;

	if ( t <= 0 )
	{
		dist2 = dist_squared( m_Pnt1, pos );
		if ( dist2 > m_RadSquared1 )
			return base_len;
		double fract = dist2/m_RadSquared1;
		retlen = m_Len1 + fract*( base_len - m_Len1  );
//		return (m_Len1 + fract*( base_len - m_Len1  ));
	}
	else if ( t >= 1 )
	{
		dist2 = dist_squared( m_Pnt2, pos );
		if ( dist2 > m_RadSquared2 )
			return base_len;
		double fract = dist2/m_RadSquared2;
		retlen = m_Len2 + fract*( base_len - m_Len2  );
//		return (m_Len2 + fract*( base_len - m_Len2  ));
	}
	else
	{
		vec3d proj = m_Pnt1 + m_Line*t;
		dist2 = dist_squared( proj, pos );

		double fract_rad = m_Rad1 + (m_Rad2 - m_Rad1)*t;
		double fract_rad_sqr = fract_rad*fract_rad;

		if ( dist2 > fract_rad_sqr )
			return base_len;

		double fract_sqr = dist2/fract_rad_sqr;
		double fract_len = m_Len1 + (m_Len2 - m_Len1)*t;

		retlen = fract_len + fract_sqr*( base_len - fract_len  );
	}

	if ( retlen > base_len )
		printf("Ret Len\n");
	else if ( retlen < m_Len1 && retlen < m_Len2 )
		printf("Ret Len\n");


	return retlen;
}


//double LineSource::GetTargetLen( double base_len, vec3d &  pos )
//{
//	if ( !m_Box.check_pnt( pos[0], pos[1], pos[2] ) )
//		return base_len;
//
//	vec3d origVec = pos - m_Pnt1;
//	double numer = dot( m_Line, origVec );
//
//	double dist2;
//	double t = numer/m_DotLine;
//
//	if ( t <= 0 )
//		dist2 = dist_squared( m_Pnt1, pos );
//	else if ( t >= 1 )
//		dist2 = dist_squared( m_Pnt2, pos );
//	else
//	{
//		vec3d proj = m_Pnt1 + m_Line*t;
//		dist2 = dist_squared( proj, pos );
//	}
//
//	if ( dist2 > m_RadSquared1 )
//		return base_len;
//
//	double fract = dist2/m_RadSquared1;
//
//	return (m_Len1 + fract*( base_len - m_Len1  ));
//}
//






//bool LineSource::ReadData( char* buff )
//{
//	char name[256];
//	float x, y, z, xx, yy, zz, rad, len;
//	sscanf( buff, "%s %f %f %f  %f %f %f  %f  %f", name, &x, &y, &z,  &xx, &yy, &zz, &rad, &len );
//	vec3d ep0 = vec3d( x, y, z );
//	vec3d ep1 = vec3d( xx, yy, zz );
//	SetEndPnts( ep0, ep1 );
//	SetRad( rad );
//	SetLen( len );
//
//	return true;
//}

void LineSource::Update( Geom* geomPtr )
{
	m_Pnt1 = geomPtr->getAttachUVPos( m_UWLoc1[0], m_UWLoc1[1] );
	m_Pnt2 = geomPtr->getAttachUVPos( m_UWLoc2[0], m_UWLoc2[1] );
	SetEndPnts( m_Pnt1, m_Pnt2 );

	if ( geomPtr->getSymCode() == NO_SYM  )
	{
		if ( m_ReflSource )
		{
			delete m_ReflSource;
			m_ReflSource = NULL;
		}
	}
	else  
	{
		if ( !m_ReflSource )
			m_ReflSource = new LineSource();

		LineSource* ls = (LineSource*)m_ReflSource;

		ls->SetGeomPtr( geomPtr );
		ls->SetLen( m_Len1 );
		ls->SetLen2( m_Len2 );
		ls->SetRad( m_Rad1 );
		ls->SetRad2( m_Rad2 );
		ls->SetName( m_Name );

		vec3d symVec = geomPtr->getSymVec();
		vec3d rp1 = m_Pnt1*symVec;
		vec3d rp2 = m_Pnt2*symVec;
		ls->SetEndPnts(rp1, rp2);
	}

}

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

void LineSource::WriteParms( xmlNodePtr root )
{
	xmlAddStringNode( root, "Name", GetName() );
	xmlAddIntNode( root, "Type", m_Type );

	xmlAddDoubleNode( root, "Rad1", m_Rad1 );
	xmlAddDoubleNode( root, "Rad2", m_Rad2 );
	xmlAddDoubleNode( root, "Len1", m_Len1 );
	xmlAddDoubleNode( root, "Len2", m_Len2 );

	xmlAddDoubleNode( root, "U1", m_UWLoc1.x() );
	xmlAddDoubleNode( root, "W1", m_UWLoc1.y() );
	xmlAddDoubleNode( root, "U2", m_UWLoc2.x() );
	xmlAddDoubleNode( root, "W2", m_UWLoc2.y() );
}

void LineSource::ReadParms( xmlNodePtr root )
{
	SetName( xmlFindString( root, "Name", GetName() ) );

	SetLen( xmlFindDouble( root, "Len1", m_Len1 ) );
	SetLen2( xmlFindDouble( root, "Len2", m_Len2 ) );
	SetRad( xmlFindDouble( root, "Rad1", m_Rad1 ) );
	SetRad2( xmlFindDouble( root, "Rad2", m_Rad2 ) );

	double u1 = xmlFindDouble( root, "U1", m_UWLoc1[0] );
	double w1 = xmlFindDouble( root, "W1", m_UWLoc1[1] );
	vec2d loc = vec2d(u1, w1);
	SetUWLoc1( loc );
	double u2 = xmlFindDouble( root, "U2", m_UWLoc2[0] );
	double w2 = xmlFindDouble( root, "W2", m_UWLoc2[1] );
	loc = vec2d(u2, w2);
	SetUWLoc2( loc );
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

void BoxSource::ComputeCullPnts()
{
	m_CullMinPnt = m_MinPnt - vec3d( m_Rad, m_Rad, m_Rad );
	m_CullMaxPnt = m_MaxPnt + vec3d( m_Rad, m_Rad, m_Rad );
}


void BoxSource::Copy( BaseSource* s )
{
	m_Name = s->GetName();
	m_GeomPtr = s->GetGeomPtr();
	m_Type = s->GetType();
	m_Len = s->GetLen();
	m_Rad = s->GetRad();
	m_RadSquared = m_Rad*m_Rad;
	m_Box = s->GetBBox();

	m_UWLoc1 = ((BoxSource*)s)->GetUWLoc1();
	m_UWLoc2 = ((BoxSource*)s)->GetUWLoc2();
}

void BoxSource::SetMinMaxPnts( const vec3d & min_pnt, const vec3d & max_pnt )
{
	m_MinPnt = min_pnt;
	m_MaxPnt = max_pnt;
	ComputeCullPnts();
	m_Box.update( m_CullMinPnt );
	m_Box.update( m_CullMaxPnt );
}

void BoxSource::SetRad( double rad )
{
	m_Rad = rad; 
	m_RadSquared = rad*rad;
	ComputeCullPnts();
	m_Box.update( m_CullMinPnt );
	m_Box.update( m_CullMaxPnt );
}

void BoxSource::SetNamedVal( Stringc name, double val )
{
	if ( name == "U1" )				m_UWLoc1[0] = val;
	else if ( name == "W1" )		m_UWLoc1[1] = val;	 
	else if ( name == "U2" )		m_UWLoc2[0] = val;	 
	else if ( name == "W2" )		m_UWLoc2[1] = val;	 
}

double BoxSource::GetTargetLen( double base_len, vec3d &  pos )
{
	if ( pos[0] <= m_CullMinPnt[0] )		return base_len;
	if ( pos[1] <= m_CullMinPnt[1] )		return base_len;
	if ( pos[2] <= m_CullMinPnt[2] )		return base_len;
	if ( pos[0] >= m_CullMaxPnt[0] )		return base_len;
	if ( pos[1] >= m_CullMaxPnt[1] )		return base_len;
	if ( pos[2] >= m_CullMaxPnt[2] )		return base_len;

	if ( pos[0] > m_MinPnt[0] && pos[0] < m_MaxPnt[0] &&
		 pos[1] > m_MinPnt[1] && pos[1] < m_MaxPnt[1] && 
		 pos[2] > m_MinPnt[2] && pos[2] < m_MaxPnt[2] )
		return m_Len;

	double fract[3];
	
	for ( int i = 0 ; i < 3 ; i++ )
	{
		fract[i] = 0;
		if ( pos[i] < m_MinPnt[i] )			fract[i] = (m_MinPnt[i] - pos[i])/m_Rad;
		else if ( pos[i] > m_MaxPnt[i] )	fract[i] = (pos[i] - m_MaxPnt[i])/m_Rad;
	}

	double max_fract = max( max( fract[0], fract[1] ), fract[2] );
	return (m_Len + max_fract*( base_len - m_Len  ));
}

bool BoxSource::ReadData( char* buff )
{
	char name[256];
	float x, y, z, xx, yy, zz, rad, len;
	sscanf( buff, "%s %f %f %f  %f %f %f  %f  %f", name, &x, &y, &z,  &xx, &yy, &zz, &rad, &len );
	vec3d p0 = vec3d( x, y, z );
	vec3d p1 = vec3d( xx, yy, zz );
	SetMinMaxPnts( p0, p1 );
	SetRad( rad );
	SetLen( len );

	m_Box.update( vec3d( x - rad, y - rad, z - rad ) );
	m_Box.update( vec3d( x + rad, y + rad, z + rad ) );

	ComputeCullPnts();

	return true;
}

void BoxSource::Update( Geom* geomPtr )
{
	bbox box;
	int num_segs = 8;
	vector< vec3d > pVec;
	for ( int i = 0 ; i < num_segs ; i++ )
	{
		double fu = (double)i/(double)(num_segs-1);
		double u = m_UWLoc1[0] + fu*(m_UWLoc2[0] - m_UWLoc1[0]);
		for ( int j = 0 ; j < num_segs ; j++ )
		{
			double fw = (double)j/(double)(num_segs-1);
			double w = m_UWLoc1[1] + fu*(m_UWLoc2[1] - m_UWLoc1[1]);
			vec3d p = geomPtr->getAttachUVPos( u, w );
			pVec.push_back( p );
			box.update(p);
		}
	}
	m_Box = box;
	vec3d p0 = box.get_pnt(0);
	vec3d p1 = box.get_pnt(7);
	SetMinMaxPnts( p0, p1 ); 

	if ( geomPtr->getSymCode() == NO_SYM  )
	{
		if ( m_ReflSource )
		{
			delete m_ReflSource;
			m_ReflSource = NULL;
		}
	}
	else  
	{
		if ( !m_ReflSource )
			m_ReflSource = new BoxSource();

		BoxSource* bs = (BoxSource*)m_ReflSource;

		bs->SetGeomPtr( geomPtr );
		bs->SetLen( m_Len );
		bs->SetRad( m_Rad );
		bs->SetName( m_Name );

		vec3d symVec = geomPtr->getSymVec();

		bbox rbox;
		for ( int i = 0 ; i < 8 ; i++ )
			rbox.update( box.get_pnt(i)*symVec );

		bs->SetBBox( rbox );
		vec3d p0 = rbox.get_pnt(0);
		vec3d p7 = rbox.get_pnt(7);
		bs->SetMinMaxPnts( p0, p7  ); 

	}



}

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

void BoxSource::WriteParms( xmlNodePtr root )
{
	xmlAddStringNode( root, "Name", GetName() );
	xmlAddIntNode( root, "Type", m_Type );

	xmlAddDoubleNode( root, "Rad", m_Rad );
	xmlAddDoubleNode( root, "Len", m_Len );

	xmlAddDoubleNode( root, "U1", m_UWLoc1.x() );
	xmlAddDoubleNode( root, "W1", m_UWLoc1.y() );
	xmlAddDoubleNode( root, "U2", m_UWLoc2.x() );
	xmlAddDoubleNode( root, "W2", m_UWLoc2.y() );
}

void BoxSource::ReadParms( xmlNodePtr root )
{
	SetName( xmlFindString( root, "Name", GetName() ) );

	SetLen( xmlFindDouble( root, "Len", m_Len ) );
	SetRad( xmlFindDouble( root, "Rad", m_Rad ) );

	double u1 = xmlFindDouble( root, "U1", m_UWLoc1[0] );
	double w1 = xmlFindDouble( root, "W1", m_UWLoc1[1] );
	vec2d loc = vec2d(u1, w1);
	SetUWLoc1( loc );
	double u2 = xmlFindDouble( root, "U2", m_UWLoc2[0] );
	double w2 = xmlFindDouble( root, "W2", m_UWLoc2[1] );
	loc = vec2d(u2, w2);
	SetUWLoc2( loc );
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

GridDensity::GridDensity()
{
	m_BaseLen = 0.5;
	m_MinLen = 0.1;
	SetNCircSeg(16.0);
	m_MaxGap = 0.005;
	m_GrowRatio = 1.3;
}

GridDensity::~GridDensity()
{
	ClearSources();
}

void GridDensity::SetNCircSeg( double v )
{
	m_NCircSeg = v;
	if(v > 2.0)
		m_RadFrac = 2.0*sin(PI/v);
	else  // Switch to 4/n behavior below well defined range.
		m_RadFrac = 4.0/v;
}

void GridDensity::RemoveSource( BaseSource* s )
{
	vector< BaseSource* > sVec;
	for ( int i = 0 ; i < (int)m_Sources.size() ; i++ )
	{
		if ( m_Sources[i] != s )
			sVec.push_back( m_Sources[i] );
	}
	m_Sources = sVec;
}
	
////===== Read Surf From File  =====//
//void GridDensity::ReadFile( const char* file_name )
//{
//	FILE* file_id = fopen(file_name, "r");
//
//	if ( file_id )
//	{
//		char buff[512];
//		char name[256];
//		fgets( buff, 512, file_id );
//		sscanf( buff, "%s %lf", name, &m_BaseLen );
//
//		while ( fgets( buff, 512, file_id ) )
//		{
//			strcpy( name, "Nada" );
//			if ( sscanf( buff, "%s", name ) )
//			{
//				if ( string(name) == string("Point") )
//				{
//					PointSource* ps = new PointSource;
//					if ( ps->ReadData( buff ) )
//						m_Sources.push_back( ps );
//					else
//						delete ps;
//				}
//				else if ( string(name) == string("Line") )
//				{
//					LineSource* ls = new LineSource;
//					if ( ls->ReadData( buff ) )
//						m_Sources.push_back( ls );
//					else
//						delete ls;
//				}
//				else if ( string(name) == string("Box") )
//				{
//					BoxSource* bs = new BoxSource;
//					if ( bs->ReadData( buff ) )
//						m_Sources.push_back( bs );
//					else
//						delete bs;
//				}
//			}
//		}
//		fclose( file_id );
//	}
//
//	for ( int i = 0 ; i < (int)m_Sources.size() ; i++ )
//	{
//		m_Sources[i]->CheckCorrectRad( m_BaseLen );
//	}
//}


double GridDensity::GetTargetLen( vec3d& pos )
{
	double target_len = m_BaseLen;

	for ( int i = 0 ; i < (int)m_Sources.size() ; i++ )
	{
		double len = m_Sources[i]->GetTargetLen( m_BaseLen, pos );
		if ( len < target_len )
			target_len = len;
	}
	return target_len;
}

void GridDensity::ScaleAllSources( double scale )
{
	for ( int i = 0 ; i < (int)m_Sources.size() ; i++ )
	{
		m_Sources[i]->AdjustLen( scale );
	}
}

	

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





