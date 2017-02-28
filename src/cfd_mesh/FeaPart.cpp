//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//   FEA Part Class
//   J.R. Gloudemans - 2/14/09
//******************************************************************************

#ifdef WIN32
#include <windows.h>
#endif

// #include <FL/Fl.H>

#include "FeaPart.h"
#include "FeaMeshMgr.h"


//============================================================================//
//============================================================================//
void FeaNode::AddTag( int type, int id )
{
    //==== Check For Duplicate Tags ====//
    for ( int i = 0 ; i < ( int )m_Tags.size() ; i++ )
    {
        if ( m_Tags[i].m_ID == id && m_Tags[i].m_Type == type )
        {
            return;
        }
    }

    FeaNodeTag tag;
    tag.m_Type = type;
    tag.m_ID   = id;
    m_Tags.push_back( tag );
}

bool FeaNode::HasTag( int type, int id )
{
    for ( int i = 0 ; i < ( int )m_Tags.size() ; i++ )
    {
        if ( m_Tags[i].m_ID == id && m_Tags[i].m_Type == type )
        {
            return true;
        }
    }
    return false;
}

bool FeaNode::HasTag( int type )
{
    for ( int i = 0 ; i < ( int )m_Tags.size() ; i++ )
    {
        if ( m_Tags[i].m_Type == type )
        {
            return true;
        }
    }
    return false;
}

bool FeaNode::HasOnlyType( int type )
{
    for ( int i = 0 ; i < ( int )m_Tags.size() ; i++ )
    {
        if ( m_Tags[i].m_Type != type )
        {
            return false;
        }
    }
    return true;
}

int FeaNode::GetIndex()
{
    return m_Index;
}

void FeaNode::WriteNASTRAN( FILE* fp )
{
    fprintf( fp, "GRID,%d,,", m_Index );

    double x = m_Pnt.x();
    double y = m_Pnt.y();
    double z = m_Pnt.z();

    if ( std::abs( x ) < 10.0 )
    {
        fprintf( fp, "%8.5f,", x );
    }
    else if ( std::abs( x ) < 100.0 )
    {
        fprintf( fp, "%8.4f,", x );
    }
    else
    {
        fprintf( fp, "%8.3f,", x );
    }

    if ( std::abs( y ) < 10.0 )
    {
        fprintf( fp, "%8.5f,", y );
    }
    else if ( std::abs( y ) < 100.0 )
    {
        fprintf( fp, "%8.4f,", y );
    }
    else
    {
        fprintf( fp, "%8.3f,", y );
    }

    if ( std::abs( z ) < 10.0 )
    {
        fprintf( fp, "%8.5f\n", z );
    }
    else if ( std::abs( z ) < 100.0 )
    {
        fprintf( fp, "%8.4f\n", z );
    }
    else
    {
        fprintf( fp, "%8.3f\n", z );
    }

}

void FeaNode::WriteCalculix( FILE* fp )
{
    fprintf( fp, "%d,%f,%f,%f\n", m_Index, m_Pnt.x(), m_Pnt.y(), m_Pnt.z() );
}


//============================================================================//
//============================================================================//
void FeaElement::DeleteAllNodes()
{
    int i;
    for ( i = 0 ; i < ( int )m_Corners.size() ; i++ )
    {
        delete m_Corners[i];
    }
    for ( i = 0 ; i < ( int )m_Mids.size() ; i++ )
    {
        delete m_Mids[i];
    }

    m_Corners.clear();
    m_Mids.clear();
}
void FeaElement::LoadNodes( vector< FeaNode* > & node_vec )
{
    int i;
    for ( i = 0 ; i < ( int )m_Corners.size() ; i++ )
    {
        node_vec.push_back( m_Corners[i] );
    }
    for ( i = 0 ; i < ( int )m_Mids.size() ; i++ )
    {
        node_vec.push_back( m_Mids[i] );
    }
}

//void FeaElement::DrawPoly()
//{
//  int i;
//  for ( i = 0 ; i < (int)m_Corners.size() ; i++ )
//      glVertex3dv( m_Corners[i]->m_Pnt.data() );
//}


//============================================================================//
//============================================================================//
void FeaTri::Create( vec3d & p0, vec3d & p1, vec3d & p2 )
{
    m_Type = FEA_TRI_6;
    DeleteAllNodes();
    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );
    m_Corners.push_back( new FeaNode( p2 ) );

    vec3d p01 = ( p0 + p1 ) * 0.5;
    vec3d p12 = ( p1 + p2 ) * 0.5;
    vec3d p20 = ( p2 + p0 ) * 0.5;

    m_Mids.push_back( new FeaNode( p01 ) );
    m_Mids.push_back( new FeaNode( p12 ) );
    m_Mids.push_back( new FeaNode( p20 ) );
}

void FeaTri::WriteCalculix( FILE* fp, int id )
{
    fprintf( fp, "%d,%d,%d,%d,%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex() );
}

void FeaTri::WriteNASTRAN( FILE* fp, int id )
{
    fprintf( fp, "CTRIA6,%d,1,%d,%d,%d,%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(),
             m_Mids[0]->GetIndex(),    m_Mids[1]->GetIndex(),    m_Mids[2]->GetIndex() );
}

double FeaTri::ComputeMass( double density )
{
    double mass = 0.0;
    if ( m_Corners.size() < 3 )
    {
        return mass;
    }

    double a = area( m_Corners[0]->m_Pnt,  m_Corners[1]->m_Pnt,  m_Corners[2]->m_Pnt );
    double avg_t = ( m_Corners[0]->m_Thick + m_Corners[1]->m_Thick + m_Corners[2]->m_Thick ) / 3.0;

    mass = a * avg_t * density;
    return mass;
}

//============================================================================//
//============================================================================//
void FeaQuad::Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & p3 )
{
    m_Type = FEA_QUAD_8;
    DeleteAllNodes();
    m_Corners.push_back( new FeaNode( p0 ) );
    m_Corners.push_back( new FeaNode( p1 ) );
    m_Corners.push_back( new FeaNode( p2 ) );
    m_Corners.push_back( new FeaNode( p3 ) );

    vec3d p01 = ( p0 + p1 ) * 0.5;
    vec3d p12 = ( p1 + p2 ) * 0.5;
    vec3d p23 = ( p2 + p3 ) * 0.5;
    vec3d p30 = ( p3 + p0 ) * 0.5;

    m_Mids.push_back( new FeaNode( p01 ) );
    m_Mids.push_back( new FeaNode( p12 ) );
    m_Mids.push_back( new FeaNode( p23 ) );
    m_Mids.push_back( new FeaNode( p30 ) );
}

void FeaQuad::WriteCalculix( FILE* fp, int id )
{
    fprintf( fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(), m_Corners[3]->GetIndex(),
             m_Mids[0]->GetIndex(), m_Mids[1]->GetIndex(), m_Mids[2]->GetIndex(), m_Mids[3]->GetIndex() );
}
void FeaQuad::WriteNASTRAN( FILE* fp, int id )
{
    fprintf( fp, "CQUAD8,%d,1,%d,%d,%d,%d,%d,%d,+\n+,%d,%d\n", id,
             m_Corners[0]->GetIndex(), m_Corners[1]->GetIndex(), m_Corners[2]->GetIndex(), m_Corners[3]->GetIndex(),
             m_Mids[0]->GetIndex(),    m_Mids[1]->GetIndex(),    m_Mids[2]->GetIndex(), m_Mids[3]->GetIndex() );
}

double FeaQuad::ComputeMass( double density )
{
    double mass = 0.0;
    if ( m_Corners.size() < 4 )
    {
        return mass;
    }

    double a012 = area( m_Corners[0]->m_Pnt,  m_Corners[1]->m_Pnt,  m_Corners[2]->m_Pnt );
    double a023 = area( m_Corners[0]->m_Pnt,  m_Corners[2]->m_Pnt,  m_Corners[3]->m_Pnt );
    double a = a012 + a023;

    double avg_t = ( m_Corners[0]->m_Thick + m_Corners[1]->m_Thick +
                     m_Corners[2]->m_Thick + m_Corners[3]->m_Thick ) / 4.0;

    mass = a * avg_t * density;
    return mass;
}


//============================================================================//
//============================================================================//
bool SpliceCompare( FeaSplice* a,  FeaSplice* b )
{
    return ( a->m_Pos() < b->m_Pos() );
}

FeaSplice::FeaSplice()
{
    m_Pos   = 0.5;
    m_Thick = 0.1;
    m_FixedFlag = false;
}


FeaSpliceLine::FeaSpliceLine( double default_thick )
{
    m_SectID = 0;
    m_PerSpan = 0.5;
    m_DefaultThick = default_thick;
    m_EditSplice = NULL;
    m_HighlightSplice = NULL;
    m_Mode = NORMAL_MODE;
    m_WinXScale = 0.98;

    //==== Add Begin/End Splices ====//
    FeaSplice* s0 = new FeaSplice();
    s0->m_Pos = 0.0;
    s0->m_Thick = m_DefaultThick;
    s0->m_FixedFlag = true;
    m_SpliceVec.push_back( s0 );

    FeaSplice* s1 = new FeaSplice();
    s1->m_Pos = 1.0;
    s1->m_Thick = m_DefaultThick;
    s1->m_FixedFlag = true;
    m_SpliceVec.push_back( s1 );

}

FeaSpliceLine::~FeaSpliceLine()
{
    ClearSpliceVec();
}


void FeaSpliceLine::ClearSpliceVec()
{
    for ( int i = 0 ; i < ( int )m_SpliceVec.size() ; i++ )
    {
        delete  m_SpliceVec[i];
    }
    m_SpliceVec.clear();
}

double FeaSpliceLine::FindMaxThick()
{
    double max_t = 0.0;
    for ( int i = 0 ; i < ( int )m_SpliceVec.size() ; i++ )
    {
        if ( m_SpliceVec[i]->m_Thick() > max_t )
        {
            max_t = m_SpliceVec[i]->m_Thick();
        }
    }
    return max_t;
}

void FeaSpliceLine::SetMode( int mode )
{
    m_Mode = mode;
}

void FeaSpliceLine::DelEditSplice()
{
    if ( !m_EditSplice || m_EditSplice->m_FixedFlag )           // Dont Delete
    {
        return;
    }

    vector< FeaSplice* > tmpVec;
    for ( int i = 0 ; i < ( int )m_SpliceVec.size() ; i++ )
    {
        if ( m_SpliceVec[i] == m_EditSplice )
        {
            delete m_SpliceVec[i];
        }
        else
        {
            tmpVec.push_back( m_SpliceVec[i] );
        }
    }
    m_SpliceVec = tmpVec;
    m_EditSplice = NULL;
    m_HighlightSplice = NULL;
}

void FeaSpliceLine::ChangeEditSplice( int inc )
{
    int id = -1;
    for ( int i = 0 ; i < ( int )m_SpliceVec.size() ; i++ )
    {
        if ( m_SpliceVec[i] == m_EditSplice )
        {
            id = i;
        }
    }
    if ( id >= 0 )
    {
        id += inc;
    }

    if ( id < 0 )
    {
        id = 0;
    }
    else if ( id >= ( int )m_SpliceVec.size() )
    {
        id = ( int )m_SpliceVec.size() - 1;
    }

    m_EditSplice = m_SpliceVec[id];
}

void FeaSpliceLine::SetEditSpliceLoc( double xc )
{
    if ( !m_EditSplice || m_EditSplice->m_FixedFlag )           // Dont Move
    {
        return;
    }

    int id = -1;
    for ( int i = 1 ; i < ( int )m_SpliceVec.size() - 1 ; i++ )
    {
        if ( m_SpliceVec[i] == m_EditSplice )
        {
            id = i;
        }
    }

    double lower_xc = 0.000001;
    double upper_xc = 0.999999;
    if ( id > 0 && id < ( int )m_SpliceVec.size() - 1 )
    {
        lower_xc = m_SpliceVec[id - 1]->m_Pos() + 0.000001;
        upper_xc = m_SpliceVec[id + 1]->m_Pos() - 0.000001;
    }

    if ( xc < lower_xc )
    {
        xc = lower_xc;
    }
    if ( xc > upper_xc )
    {
        xc = upper_xc;
    }

    m_EditSplice->m_Pos = xc;

}

double FeaSpliceLine::ComputeThick( double per_chord )
{
    if ( ( int )m_SpliceVec.size() < 2 )
    {
        return m_DefaultThick();
    }

    if ( per_chord <= m_SpliceVec[0]->m_Pos() )
    {
        return m_SpliceVec[0]->m_Thick();
    }
    if ( per_chord >= m_SpliceVec.back()->m_Pos() )
    {
        return m_SpliceVec.back()->m_Thick();
    }

    //==== Assume Splices Are Consistent and Sorted By StartPos ====//
    for ( int i = 1 ; i < ( int )m_SpliceVec.size() ; i++ )
    {
        if ( per_chord >= m_SpliceVec[i - 1]->m_Pos() && per_chord <= m_SpliceVec[i]->m_Pos() )
        {
            double fract = 0.0;
            double denom = m_SpliceVec[i]->m_Pos() - m_SpliceVec[i - 1]->m_Pos();
            if ( denom > DBL_EPSILON )
            {
                fract = ( per_chord - m_SpliceVec[i - 1]->m_Pos() ) / denom;
            }

            double thick = m_SpliceVec[i - 1]->m_Thick() +
                           fract * ( m_SpliceVec[i]->m_Thick() - m_SpliceVec[i - 1]->m_Thick() );

            return thick;
        }
    }
    return m_DefaultThick();
}

void FeaSpliceLine::SetEndPoints( vec3d & e0, vec3d & e1 )
{
    m_EndPnts[0] = e0;
    m_EndPnts[1] = e1;
}

int FeaSpliceLine::processDragEvent()
{
    //===== Get Mouse Position ====//
//  int x = Fl::event_x();
//  int y = Fl::event_y();
    int x = 0;
    int y = 0;

    m_MouseX = ( double )x / winWidth;
    m_MouseY = ( double )y / winHeight;

    if ( m_Mode == NORMAL_MODE &&  m_EditSplice )
    {
        double refxc = MapMouseToChord( m_RefMouseX );
        double xc = MapMouseToChord( m_MouseX );
        double delxc = xc - refxc;
        SetEditSpliceLoc( m_RefPos + delxc );
//      FeaMeshMgr.UpdateGUI();
//      FeaMeshMgr.AircraftDraw();
    }

    return 1;
}

int FeaSpliceLine::processMoveEvent()
{
    //===== Get Mouse Position ====//
//  int x = Fl::event_x();
//  int y = Fl::event_y();
    int x = 0;
    int y = 0;

    m_MouseX = ( double )x / winWidth;
    m_MouseY = ( double )y / winHeight;

    if ( m_Mode == NORMAL_MODE )
    {
        //==== Find Closest Splice ====//
        m_HighlightSplice = FindClosestSplice( m_MouseX, m_MouseY );
    }



    return 1;
}

double FeaSpliceLine::MapMouseToChord( double x )
{
    double chord = x - 0.5 * ( 1.0 - m_WinXScale );
    chord = chord / m_WinXScale;

    if ( chord < 0.0001 )
    {
        chord = 0.0001;
    }
    else if ( chord > 0.9999 )
    {
        chord = 0.9999;
    }

    return chord;
}

FeaSplice* FeaSpliceLine::FindClosestSplice( double x, double y )
{
    FeaSplice* close_splice = NULL;
    double close_dist = 1.0e12;
    double xc = MapMouseToChord( x );
    for ( int i = 0 ; i < ( int )m_SpliceVec.size() ; i++ )
    {
        double d = std::abs( xc - m_SpliceVec[i]->m_Pos() );
        if ( d < close_dist )
        {
            close_dist = d;
            close_splice = m_SpliceVec[i];
        }
    }

    return close_splice;
}

int FeaSpliceLine::processPushEvent()
{
    if ( m_Mode == ADD_MODE )
    {
        FeaSplice* s = new FeaSplice();
        s->m_Pos = MapMouseToChord( m_MouseX );
        s->m_Thick = m_DefaultThick;
        m_SpliceVec.push_back( s );
        m_EditSplice = s;
        m_HighlightSplice = s;
        m_RefMouseX = m_MouseX;
        m_RefPos = m_EditSplice->m_Pos();
        sort( m_SpliceVec.begin(), m_SpliceVec.end(), SpliceCompare );
        m_Mode = NORMAL_MODE;
//      FeaMeshMgr.UpdateGUI();
//      FeaMeshMgr.AircraftDraw();
    }
    else if ( m_Mode == NORMAL_MODE )
    {
        //==== Find Closest Splice ====//
        m_EditSplice = m_HighlightSplice;
        m_RefMouseX = m_MouseX;

        if ( m_EditSplice )
        {
            m_RefPos = m_EditSplice->m_Pos();
        }

//      FeaMeshMgr.UpdateGUI();
//      FeaMeshMgr.AircraftDraw();
    }

    return 1;
}

int FeaSpliceLine::processReleaseEvent()
{
    return 1;
}

//void FeaSpliceLine::DrawMain()
//{
//  glBegin( GL_LINES );
//      glVertex3dv( m_EndPnts[0].data() );
//      glVertex3dv( m_EndPnts[1].data() );
//  glEnd();
//
//
//  glPointSize(8.0);
//  glBegin( GL_POINTS );
//  for ( int i = 0 ; i < (int)m_SpliceVec.size() ; i++ )
//  {
//      vec3d p = m_EndPnts[0] + (m_EndPnts[1] - m_EndPnts[0])*m_SpliceVec[i]->m_Pos;
//      glVertex3dv( p.data() );
//  }
//  glEnd();
//
//}
//
//void FeaSpliceLine::draw()
//{
//  int i;
//  //==== Draw Grid ====//
//  float gridSize = 0.1f;
//
//  glLineWidth(1.0);
//  glColor3f(0.8f, 0.8f, 0.8f);
//  glBegin( GL_LINES );
//  for ( i = 0 ; i < 41 ; i++ )
//  {
//      if ( i == 20 )
//          glColor3f(0.8f, 0.8f, 0.8f);
//      else
//          glColor3f(0.9f, 0.9f, 0.9f);
//
//      glVertex2f( gridSize*(float)i - 20.0f*gridSize, -20.0f*gridSize );
//      glVertex2f( gridSize*(float)i - 20.0f*gridSize,  20.0f*gridSize );
//      glVertex2f( -20.0f*gridSize, gridSize*(float)i - 20.0f*gridSize );
//      glVertex2f(  20.0f*gridSize, gridSize*(float)i - 20.0f*gridSize );
//  }
//  glEnd();
//
//  //==== Draw Thick Dist ====//
//  double max_thick = FindMaxThick();
//  double ar = (double)winHeight/(double)winWidth;
//
//  //==== Draw Control Points =====//
//  glLineWidth(1.0);
//  glPointSize(5.0);
//  for ( i = 0 ; i < (int)m_SpliceVec.size() ; i++ )
//  {
//      double ht = 0.35;
//      FeaSplice* sp = m_SpliceVec[i];
//      if ( sp == m_EditSplice )
//      {
//          glColor3f( 1.0f, 0.0f, 0.0f );
//          ht = 0.45;
//      }
//      else if ( sp == m_HighlightSplice && m_Mode == NORMAL_MODE )
//      {
//          glColor3f( 0.3f, 0.3f, 0.3f);
//          ht = 0.40;
//      }
//      else
//      {
//          glColor3f( 0.6f, 0.6f, 0.6f );
//      }
//
//      glBegin( GL_LINES );
//          glVertex2d( m_WinXScale*sp->m_Pos - m_WinXScale*0.5,  ar*ht );
//          glVertex2d( m_WinXScale*sp->m_Pos - m_WinXScale*0.5, -ar*ht );
//      glEnd();
//
//      glBegin( GL_POINTS );
//              glVertex2d( m_WinXScale*sp->m_Pos - m_WinXScale*0.5,  ar*ht );
//      glEnd();
//
//  }
//  glColor4f( 0.0f, 0.0f, 1.0f, 1.0f );
//  glLineWidth(2.0);
//  glEnable( GL_LINE_SMOOTH );
//
//  //==== Draw Thick Dist ====//
//  for ( i = 0 ; i < (int)m_SpliceVec.size()-1 ; i++ )
//  {
//      FeaSplice* sp0 = m_SpliceVec[i];
//      double t0 = (m_SpliceVec[i]->m_Thick/max_thick)*ar*0.5;
//      FeaSplice* sp1 = m_SpliceVec[i+1];
//      double t1 = (m_SpliceVec[i+1]->m_Thick/max_thick)*ar*0.5;
//      glBegin( GL_LINE_LOOP );
//          glVertex2d( m_WinXScale*sp0->m_Pos - m_WinXScale*0.5,  t0*0.5 );
//          glVertex2d( m_WinXScale*sp0->m_Pos - m_WinXScale*0.5, -t0*0.5 );
//          glVertex2d( m_WinXScale*sp1->m_Pos - m_WinXScale*0.5, -t1*0.5 );
//          glVertex2d( m_WinXScale*sp1->m_Pos - m_WinXScale*0.5,  t1*0.5 );
//      glEnd();
//  }
//  glDisable( GL_LINE_SMOOTH );
//
//
//
//}


//============================================================================//
//============================================================================//
FeaPart::FeaPart()
{
    m_Density = 1.0;
}

FeaPart::~FeaPart()
{

}
void FeaPart::LoadNodes( vector< FeaNode* > & node_vec )
{
    int i;
    for ( i = 0 ; i < ( int )m_Elements.size() ; i++ )
    {
        m_Elements[i]->LoadNodes( node_vec );
    }
}

double FeaPart::ComputeMass()
{
    double mass = 0;
    for ( int i = 0 ; i < ( int )m_Elements.size() ; i++ )
    {
        mass += m_Elements[i]->ComputeMass( m_Density() );
    }

    return mass;
}

//============================================================================//
//============================================================================//
FeaSlice::FeaSlice()
{
    m_Surf = new Surf();
    m_Surf->SetCompID( COMP_ID );
    m_NumDivisions = 1;
}

FeaSlice::~FeaSlice()
{
    delete m_Surf;
}

void FeaSlice::Clean()
{
    m_UpperPnts.clear();
    m_LowerPnts.clear();

    for ( int i = 0 ; i < ( int )m_Elements.size() ; i++ )
    {
        delete m_Elements[i];
    }
    m_Elements.clear();
}

void FeaSlice::SetEndPoints( vec2d & uwA, vec2d uwB )
{
    //==== Adjust UWs - Splitting at Seams has Problems ====//
    if ( std::abs( uwA[0] - uwB[0] ) < 0.00001 )
    {
        uwA[0] += 0.00001;
        uwB[0] += 0.00001;
    }
    if ( std::abs( uwA[1] - uwB[1] ) < 0.00001 )
    {
        uwA[1] += 0.00001;
        uwB[1] += 0.00001;
    }

    //==== Find Starting Section ====//
    int sectID = ( int )uwA[0];
    vec3d norm = FeaMeshMgr.GetNormal( sectID );
    m_Normal = norm;

    //==== Find Bounding Box ====//
    BndBox box;
    vec3d lpnt1 = FeaMeshMgr.ComputePoint( uwA, true );
    vec3d lpnt2 = lpnt1 + norm;
    for ( int i = 0 ; i < 11 ; i++ )
    {
        vec2d uw = uwA + ( uwB - uwA ) * ( double )i * 0.1;
        vec3d pnt = FeaMeshMgr.ComputePoint( uw, true );
        box.Update( proj_pnt_on_line( lpnt1, lpnt2, pnt ) );
        pnt = FeaMeshMgr.ComputePoint( uw, false );
        box.Update( proj_pnt_on_line( lpnt1, lpnt2, pnt ) );
    }

    double scale = 1.1 * box.DiagDist();

    m_UpperEndPnts[0] = lpnt1;
    m_UpperEndPnts[1] = FeaMeshMgr.ComputePoint( uwB, true );

    //==== Find Corner Points & Slightly Expand====//
    vec3d pA = FeaMeshMgr.ComputePoint( uwA, true );
    vec3d pB = FeaMeshMgr.ComputePoint( uwB, true );
    vec3d off = pA - pB;
//  off.normalize();
    vec3d upA  = pA + norm * scale;
    vec3d lpA  = pA - norm * scale;
    vec3d upB  = pB + norm * scale;
    vec3d lpB  = pB - norm * scale;

    if ( uwA[0] < 0.0001 || uwA[1] < 0.0001 )
    {
        upA = upA + off * 0.05;
        lpA = lpA + off * 0.05;
    }
    double maxu = ( double )FeaMeshMgr.GetNumSections() - 0.0001;
    if ( uwB[0] > maxu || uwB[1] > 0.9999 )
    {
        upB = upB - off * 0.05;
        lpB = lpB - off * 0.05;
    }

    //==== Start Point for Chains ====//
    m_UpperStartChainPnt = FeaMeshMgr.ComputePoint( uwB, true );
    m_LowerStartChainPnt = FeaMeshMgr.ComputePoint( uwB, false );
    m_UpperEndChainPnt = FeaMeshMgr.ComputePoint( uwA, true );
    m_LowerEndChainPnt = FeaMeshMgr.ComputePoint( uwA, false );

    threed_point_type pt0, pt1, pt2, pt3;

    upA.get_pnt( pt0 );
    upB.get_pnt( pt1 );
    lpA.get_pnt( pt2 );
    lpB.get_pnt( pt3 );

    m_Surf->GetSurfCore()->MakePlaneSurf( pt0, pt1, pt2, pt3 );
    m_Surf->GetSurfCore()->BuildPatches( m_Surf );
}
void FeaSlice::SetUpperCapSurfs( Surf* s0, Surf* s1 )
{
    m_CapUpperSurf[0] = s0;
    m_CapUpperSurf[1] = s1;
}

void FeaSlice::SetLowerCapSurfs( Surf* s0, Surf* s1 )
{
    m_CapLowerSurf[0] = s0;
    m_CapLowerSurf[1] = s1;
}

void FeaSlice::FindUpperLowerPoints()
{
    int num_sections = FeaMeshMgr.GetNumSections();
    list< ISegChain* > upper_chain_list;
    list< ISegChain* > lower_chain_list;
    if ( IsCap() )
    {
        if ( m_CapUpperSurf[0] != m_CapUpperSurf[1] )   // On Boundry of Two Surfaces
        {
            FeaMeshMgr.LoadCapChains( m_CapUpperSurf[0], m_CapUpperSurf[1], upper_chain_list );
            FeaMeshMgr.LoadCapChains( m_CapLowerSurf[0], m_CapLowerSurf[1], lower_chain_list );
        }
        else if ( m_CapInFlag )     // Inside Edge
        {
            FeaMeshMgr.LoadCapChains( m_CapUpperSurf[0], 0.0, upper_chain_list );
            FeaMeshMgr.LoadCapChains( m_CapLowerSurf[0], 0.0, lower_chain_list );
        }
        else                        // Outside Edge
        {
            FeaMeshMgr.LoadCapChains( m_CapUpperSurf[0], 1.0, upper_chain_list );
            FeaMeshMgr.LoadCapChains( m_CapLowerSurf[0], 1.0, lower_chain_list );
        }
    }
    else
    {
        for ( int i = 0 ; i < num_sections ; i++ )
        {
            FeaMeshMgr.LoadChains( m_Surf, true, i, upper_chain_list );
            FeaMeshMgr.LoadChains( m_Surf, false, i, lower_chain_list );
        }
    }

    vector< vec3d > upper_pnts;
    MergeChains( upper_chain_list, m_UpperStartChainPnt, upper_pnts );

    //==== Find Lower Points ====//
    vector< vec3d > lower_pnts;
    FindLowerPnts(  upper_pnts, lower_chain_list, lower_pnts );

    MatchUpperLowerPnts( upper_pnts, lower_pnts );
}

// This function does not account for the curvature based meshing as incorporated
// in the surface mesh density map.  This does not appear to cause any problems,
// but may be an issue down the road.
int FeaSlice::ComputeNumDivisions()
{
    //==== Find Max Number of Elements Needed ====//
    GridDensity* grid_density = FeaMeshMgr.GetGridDensityPtr();
    int max_num = 0;
    for ( int i = 0 ; i < ( int )m_UpperPnts.size() ; i++ )
    {
        double d = dist( m_UpperPnts[i], m_LowerPnts[i] );
        double esize = grid_density->GetTargetLen( m_UpperPnts[i] );

        if ( esize > 0.0 )
        {
            int num = ( int )( d / esize ) + 1;
            if ( num > max_num )
            {
                max_num = num;
            }
        }
    }
    return max_num;
}

void FeaSlice::BuildMesh()
{
    //==== Create Elements ====//
    double duptol = 0.00001;
    for ( int i = 0 ; i < ( int )m_UpperPnts.size() - 1 ; i++ )
    {
        vec3d puA = m_UpperPnts[i];
        vec3d plA = m_LowerPnts[i];
        vec3d puB = m_UpperPnts[i + 1];
        vec3d plB = m_LowerPnts[i + 1];

        for ( int j = 0 ; j < m_NumDivisions ; j++ )
        {
            double f0 = ( double )j / ( double )m_NumDivisions;
            double f1 = ( double )( j + 1 ) / ( double )m_NumDivisions;
            vec3d p0 = puA + ( plA - puA ) * f0;
            vec3d p1 = puA + ( plA - puA ) * f1;
            vec3d p2 = puB + ( plB - puB ) * f1;
            vec3d p3 = puB + ( plB - puB ) * f0;

            double d01 = dist( p0, p1 );
            double d12 = dist( p1, p2 );
            double d23 = dist( p2, p3 );
            double d30 = dist( p3, p0 );
            if ( d01 > duptol && d12 > duptol && d23 > duptol && d30 > duptol )
            {
                FeaQuad* quad = new FeaQuad;
                quad->Create( p0, p1, p2, p3 );
                m_Elements.push_back( quad );
            }
            else if ( d01 > duptol && d12 > duptol && d23 > duptol )
            {
                FeaTri* tri = new FeaTri;
                tri->Create( p0, p1, p2 );
                m_Elements.push_back( tri );
            }
            else if ( d01 > duptol && d12 > duptol && d30 > duptol )
            {
                FeaTri* tri = new FeaTri;
                tri->Create( p0, p1, p2 );
                m_Elements.push_back( tri );
            }
            else if ( d12 > duptol && d23 > duptol && d30 > duptol )
            {
                FeaTri* tri = new FeaTri;
                tri->Create( p0, p2, p3 );
                m_Elements.push_back( tri );
            }
            else if ( d01 > duptol && d23 > duptol && d30 > duptol )
            {
                FeaTri* tri = new FeaTri;
                tri->Create( p0, p2, p3 );
                m_Elements.push_back( tri );
            }
        }
    }
}

void FeaSlice::MergeChains( list < ISegChain* > & chain_list, vec3d & start_pnt, vector< vec3d > & pnt_vec )
{
    //==== Look For First Upper Point ====//
    pnt_vec.push_back( start_pnt );

    double tol = 0.01;
    bool keepGoing = true;
    while ( keepGoing )
    {
        keepGoing = false;
        bool close_flip = false;
        ISegChain* close_chain = NULL;
        double close_dist = 1.0e12;

        list< ISegChain* >::iterator c;
        for ( c = chain_list.begin() ; c != chain_list.end(); c++ )
        {
            double d = dist( pnt_vec.back(), ( *c )->m_TessVec[0]->m_Pnt );
            if ( d < close_dist )
            {
                close_flip = false;
                close_chain = ( *c );
                close_dist  = d;
            }
            d = dist( pnt_vec.back(), ( *c )->m_TessVec.back()->m_Pnt );
            if ( d < close_dist )
            {
                close_flip = true;
                close_chain = ( *c );
                close_dist  = d;
            }
        }

        if ( ( close_dist < tol || ( pnt_vec.size() == 1 && close_dist < 1.0e6 ) ) && close_chain )
        {
            keepGoing = true;
            if ( close_flip )
            {
                if ( pnt_vec.size() == 1 )                              // Replace Starting Pnt
                {
                    pnt_vec[0] = close_chain->m_TessVec.back()->m_Pnt;
                }
                for ( int i = close_chain->m_TessVec.size() - 2 ; i >= 0 ; i-- )
                {
                    pnt_vec.push_back( close_chain->m_TessVec[i]->m_Pnt );
                }
            }
            else
            {
                if ( pnt_vec.size() == 1 )                              // Replace Starting Pnt
                {
                    pnt_vec[0] = close_chain->m_TessVec[0]->m_Pnt;
                }
                for ( int i = 1 ; i < ( int )close_chain->m_TessVec.size() ; i++ )
                {
                    pnt_vec.push_back( close_chain->m_TessVec[i]->m_Pnt );
                }
            }
            chain_list.remove( close_chain );
        }
    }
}

void FeaSlice::FindLowerPnts(  vector< vec3d > & upper_pnts, list< ISegChain* > & chain_list,
                               vector< vec3d > & lower_pnts )
{
    //==== Find Closest Chain Point to StartChain =====//
    vector< vec3d > chain_end_pnts;
    list< ISegChain* >::iterator c;
    for ( c = chain_list.begin() ; c != chain_list.end(); c++ )
    {
        chain_end_pnts.push_back( ( *c )->m_TessVec.front()->m_Pnt );
        chain_end_pnts.push_back( ( *c )->m_TessVec.back()->m_Pnt );
    }

    int index = FindClosestPnt( m_LowerStartChainPnt, chain_end_pnts );
    vec3d start_pnt = chain_end_pnts[index];

    MergeChains( chain_list, start_pnt, lower_pnts );

}

void FeaSlice::MatchUpperLowerPnts( vector< vec3d > & upper_pnts, vector< vec3d > & lower_pnts )
{
    vector< int > upper_cnt;
    upper_cnt.resize( upper_pnts.size() );
    for ( int i = 0 ; i < ( int )upper_cnt.size() ; i++ )
    {
        upper_cnt[i] = 0;
    }

    vector< int > lower_cnt;
    lower_cnt.resize( lower_pnts.size() );
    for ( int i = 0 ; i < ( int )lower_cnt.size() ; i++ )
    {
        lower_cnt[i] = 0;
    }

    for ( int i = 0 ; i < ( int )upper_pnts.size() ; i++ )
    {
        int ind = FindClosestPnt( upper_pnts[i], m_Normal, lower_pnts );
        lower_cnt[ind]++;
    }
    for ( int i = 0 ; i < ( int )lower_pnts.size() ; i++ )
    {
        int ind = FindClosestPnt( lower_pnts[i], m_Normal, upper_pnts );
        upper_cnt[ind]++;
    }

    //==== Handle Unequal Counts ====//
    int total_upper = 0;
    for ( int i = 0 ; i < ( int )upper_cnt.size() ; i++ )
    {
        if ( upper_cnt[i] > 1 )
        {
            total_upper += upper_cnt[i];
        }
        else
        {
            total_upper++;
        }
    }
    int total_lower = 0;
    for ( int i = 0 ; i < ( int )lower_cnt.size() ; i++ )
    {
        if ( lower_cnt[i] > 1 )
        {
            total_lower += lower_cnt[i];
        }
        else
        {
            total_lower++;
        }
    }

    for ( int i = 0 ; i < ( int )upper_pnts.size() ; i++ )
    {
        if ( ( total_upper > total_lower ) && upper_cnt[i] == 0 )
        {
            int ind = FindClosestPnt( upper_pnts[i], m_Normal, lower_pnts );
            lower_cnt[ind]++;
            total_lower++;
        }
    }

    for ( int i = 0 ; i < ( int )lower_pnts.size() ; i++ )
    {
        if ( ( total_lower > total_upper ) && lower_cnt[i] == 0 )
        {
            int ind = FindClosestPnt( lower_pnts[i], m_Normal, upper_pnts );
            upper_cnt[ind]++;
            total_upper++;
        }
    }

    m_UpperPnts.clear();
    for ( int i = 0 ; i < ( int )upper_pnts.size() ; i++ )
    {
        m_UpperPnts.push_back( upper_pnts[i] );
        for ( int j = 1 ; j < upper_cnt[i] ; j++ )
        {
            m_UpperPnts.push_back( upper_pnts[i] );
        }
    }
    m_LowerPnts.clear();
    for ( int i = 0 ; i < ( int )lower_pnts.size() ; i++ )
    {
        m_LowerPnts.push_back( lower_pnts[i] );
        for ( int j = 1 ; j < lower_cnt[i] ; j++ )
        {
            m_LowerPnts.push_back( lower_pnts[i] );
        }
    }

    assert( m_UpperPnts.size() == m_LowerPnts.size() );
}

int FeaSlice::FindClosestPnt( vec3d & pnt, vec3d & dir, vector< vec3d > & target_vec )
{
    int close_ind = -1;
    double close_dist = 1.0e12;
    for ( int i = 0 ; i < ( int )target_vec.size() ; i++ )
    {
        vec3d offset = pnt + dir;
        double d = dist_pnt_2_line( pnt, offset, target_vec[i] );
        if ( d < close_dist )
        {
            close_ind  = i;
            close_dist = d;
        }
    }
    return close_ind;
}

int FeaSlice::FindClosestPnt( vec3d & pnt, vector< vec3d > & target_vec )
{
    int close_ind = -1;
    double close_dist = 1.0e12;
    for ( int i = 0 ; i < ( int )target_vec.size() ; i++ )
    {
        double d = dist_squared( pnt, target_vec[i] );
        if ( d < close_dist )
        {
            close_ind  = i;
            close_dist = d;
        }
    }
    return close_ind;
}

void FeaSlice::SnapUpperLowerToSkin(  vector < FeaNode* > & skinNodes )
{
    vector< vec3d > skin_pnts;
    for ( int i = 0 ; i < ( int )skinNodes.size() ; i++ )
    {
        skin_pnts.push_back( skinNodes[i]->m_Pnt );
    }

    for ( int i = 0 ; i < ( int )m_UpperPnts.size() ; i++ )
    {
        int id = FindClosestPnt( m_UpperPnts[i], skin_pnts );
        m_UpperPnts[i] = skin_pnts[id];
    }
    for ( int i = 0 ; i < ( int )m_LowerPnts.size() ; i++ )
    {
        int id = FindClosestPnt( m_LowerPnts[i], skin_pnts );
        m_LowerPnts[i] = skin_pnts[id];
    }
}



//void FeaSlice::DrawSlicePlane()
//{
//  vec3d p00 = m_Surf->CompPnt01( 0, 0 );
//  vec3d p10 = m_Surf->CompPnt01( 1, 0 );
//  vec3d p11 = m_Surf->CompPnt01( 1, 1 );
//  vec3d p01 = m_Surf->CompPnt01( 0, 1 );
//
//  glBegin( GL_LINES );
//  for ( int i = 0 ; i < 4 ; i ++ )
//  {
//      double fu = (double)i/3.0;
//      vec3d p0 = p00 + (p10 - p00)*fu;
//      vec3d p1 = p01 + (p11 - p01)*fu;
//      glVertex3dv( p0.data() );
//      glVertex3dv( p1.data() );
//  }
//  glEnd();
//  glBegin( GL_LINES );
//  for ( int i = 0 ; i < 4 ; i ++ )
//  {
//      double fw = (double)i/3.0;
//      vec3d p0 = p00 + (p01 - p00)*fw;
//      vec3d p1 = p10 + (p11 - p10)*fw;
//      glVertex3dv( p0.data() );
//      glVertex3dv( p1.data() );
//  }
//  glEnd();
//
//
//}
//
//
//void FeaSlice::Draw()
//{
//  glPointSize( 8.0 );
//
//  glBegin( GL_POINTS );
//      for ( int i = 0 ; i < (int)m_UpperPnts.size() ; i++ )
//          glVertex3dv( m_UpperPnts[i].data() );
//      for ( int i = 0 ; i < (int)m_LowerPnts.size() ; i++ )
//          glVertex3dv( m_LowerPnts[i].data() );
//  glEnd();
//
//  glBegin( GL_LINES );
//      for ( int i = 0 ; i < (int)m_UpperPnts.size() ; i++ )
//      {
//          glVertex3dv( m_UpperPnts[i].data() );
//          glVertex3dv( m_LowerPnts[i].data() );
//      }
//  glEnd();
//
//
//  //glBegin( GL_POINTS );
//  //
//  //list< ISegChain* >::iterator c;
//  //for ( c = m_LowerChains.begin() ; c != m_LowerChains.end(); c++ )
//  //{
//  //  for ( int i = 0 ; i < (int)(*c)->m_TessVec.size() ; i++ )
//  //      glVertex3dv(  (*c)->m_TessVec[i]->m_Pnt.data() );
//  //}
//  //glEnd();
//
//
//}

//============================================================================//
//============================================================================//
FeaSpar::FeaSpar()
{
    m_SectID = 0;
    m_PerChord = 0.5;
    m_Thick = 0.1;
    m_AbsSweepFlag = true;
    m_Sweep = 0.0;
    m_TrimFlag = true;
}

FeaSpar::~FeaSpar()
{
}

void FeaSpar::ComputeEndPoints()
{
    WingSection* sectPtr = FeaMeshMgr.GetWingSection( m_SectID );

    if ( !sectPtr )
    {
        return;
    }

    vec2d uwIn  = sectPtr->GetUW( WingSection::IN_CHORD, 1.0 - m_PerChord() );
    uwIn[0] = uwIn[0] + m_SectID;
    vec3d pntIn = FeaMeshMgr.ComputePoint( uwIn, true );

    vec3d axis  = sectPtr->m_Normal;
    vec3d norm  = cross( axis, sectPtr->m_ChordNormal );

    double sweep = m_Sweep();
    if ( !m_AbsSweepFlag )
    {
        vec2d uwOut  = sectPtr->GetUW( WingSection::OUT_CHORD, 1.0 - m_PerChord() );
        uwOut[0] = uwOut[0] + m_SectID;

        vec3d pntOut = FeaMeshMgr.ComputePoint( uwOut, true );

        vec3d vio = pntOut - pntIn;
        vec3d norm = FeaMeshMgr.GetNormal( m_SectID );

        sweep -= RAD_2_DEG * signed_angle( sectPtr->m_ChordNormal, vio, norm );
    }

    Matrix4d mat;
    mat.rotate( DEG_2_RAD * sweep, axis );
    vec3d rot_norm = mat.xform( norm );

    vec2d uw;
    vector< vec2d > uwVec;
    for ( int s = 0 ; s < FeaMeshMgr.GetNumSections() ; s++ )
    {
        if ( !m_TrimFlag || s == m_SectID )
        {
            sectPtr = FeaMeshMgr.GetWingSection( s );
            for ( int i = 0 ; i < WingSection::NUM_EDGES ; i++ )
            {
                if ( sectPtr->IntersectPlaneEdge( i, pntIn, rot_norm, uw ) )
                {
                    uwVec.push_back( vec2d( uw[0] + s, uw[1] ) );
                }
            }
        }
    }

    vec2d  max_uw;
    double max_dist = 0.0;
    for ( int i = 0 ; i < ( int )uwVec.size() ; i++ )
    {
        vec3d pnt = FeaMeshMgr.ComputePoint( uwVec[i], true );
        double d = dist( pntIn, pnt );
        if ( d > max_dist )
        {
            max_dist = d;
            max_uw = uwVec[i];
        }
    }

    if ( max_dist > 0.0 )
    {
        SetEndPoints( uwIn, max_uw );
    }
}

//void FeaSpar::Draw( bool highlight )
//{
//  if ( highlight )
//  {
//      glColor3ub( 255, 0, 0 );
//      glLineWidth(3.0);
//      DrawSlicePlane();
//  }
//  else
//  {
//      glColor3ub( 100, 150, 100 );
//      glLineWidth(2.0);
//      glBegin( GL_LINES );
//          vec3d p0 = (m_UpperStartChainPnt + m_LowerStartChainPnt)*0.5;
//          vec3d p1 = (m_UpperEndChainPnt   + m_LowerEndChainPnt)*0.5;
//          glVertex3dv( p0.data() );
//          glVertex3dv( p1.data() );
//      glEnd();
//  }
//}


//============================================================================//
//============================================================================//
FeaRib::FeaRib()
{
    m_SectID = 0;
    m_PerSpan = 0.5;
    m_Thick = 0.1;
    m_AbsSweepFlag = true;
    m_Sweep = 0.0;
    m_TrimFlag = true;
}

FeaRib::~FeaRib()
{
}

void FeaRib::ComputeEndPoints()
{
    WingSection* sectPtr = FeaMeshMgr.GetWingSection( m_SectID );

    if ( !sectPtr )
    {
        return;
    }

    vec2d uwLE  = sectPtr->GetUW( WingSection::LE, m_PerSpan() );
    uwLE[0] = uwLE[0] + m_SectID;
    vec3d pntLE = FeaMeshMgr.ComputePoint( uwLE, true );

    vec3d axis  = sectPtr->m_Normal;
    vec3d norm  = sectPtr->m_ChordNormal;

    double sweep = m_Sweep();
    if ( !m_AbsSweepFlag )
    {
        sweep += sectPtr->m_SweepLE;
    }

    Matrix4d mat;
    mat.rotate( DEG_2_RAD * sweep, axis );
    vec3d rot_norm = mat.xform( norm );

    vec2d uw;
    vector< vec2d > uwVec;
    for ( int s = 0 ; s < FeaMeshMgr.GetNumSections() ; s++ )
    {
        if ( !m_TrimFlag || s == m_SectID )
        {
            sectPtr = FeaMeshMgr.GetWingSection( s );
            for ( int i = 0 ; i < WingSection::NUM_EDGES ; i++ )
            {
                if ( sectPtr->IntersectPlaneEdge( i, pntLE, rot_norm, uw ) )
                {
                    uwVec.push_back( vec2d( uw[0] + s, uw[1] ) );
                }
            }
        }
    }

    vec2d  max_uw;
    double max_dist = 0.0;
    for ( int i = 0 ; i < ( int )uwVec.size() ; i++ )
    {
        vec3d pnt = FeaMeshMgr.ComputePoint( uwVec[i], true );
        double d = dist( pntLE, pnt );
        if ( d > max_dist )
        {
            max_dist = d;
            max_uw = uwVec[i];
        }
    }

    if ( max_dist > 0.0 )
    {
        SetEndPoints( max_uw, uwLE  );
    }

}

bool FeaRib::IsCap()
{
    if ( m_PerSpan() < 0.001 || m_PerSpan() > 0.999 )
    {
        if ( m_AbsSweepFlag && std::abs( m_Sweep() ) < 0.001 )
        {
            if ( m_PerSpan() < 0.001 )
            {
                m_CapInFlag = true;
            }
            else
            {
                m_CapInFlag = false;
            }
            return true;
        }
    }
    return false;
}



//void FeaRib::Draw( bool highlight )
//{
//  if ( highlight )
//  {
//      glColor3ub( 255, 0, 0 );
//      glLineWidth(3.0);
//      DrawSlicePlane();
//  }
//  else
//  {
//      glColor3ub( 150, 100, 150 );
//      glLineWidth(2.0);
//      glBegin( GL_LINES );
//          glVertex3dv( m_UpperEndPnts[0].data() );
//          glVertex3dv( m_UpperEndPnts[1].data() );
//      glEnd();
//  }
//}





//============================================================================//
//============================================================================//
FeaSkin::FeaSkin()
{
    m_Surf = NULL;
    m_WingSection = NULL;
    m_DefaultThick = 0.1;
    m_ExportFlag = true;
    m_CurrSpliceLineID = 0;
}

FeaSkin::~FeaSkin()
{
}

void FeaSkin::Clean()
{
    for ( int i = 0 ; i < ( int )m_Elements.size() ; i++ )
    {
        delete m_Elements[i];
    }
    m_Elements.clear();
}
void FeaSkin::WriteData( xmlNodePtr root )
{
    int i, j;

    XmlUtil::AddIntNode( root, "ExportFlag", m_ExportFlag );
    XmlUtil::AddDoubleNode( root, "DefaultThick", m_DefaultThick() );
    XmlUtil::AddDoubleNode( root, "Density", m_Density() );

    //===== Splice Lines =====//
    xmlNodePtr splice_line_list_node = xmlNewChild( root, NULL, ( const xmlChar * )"Splice_Line_List", NULL );
    for ( i = 0 ; i < ( int )m_SpliceLineVec.size() ; i++ )
    {
        xmlNodePtr sl_node = xmlNewChild( splice_line_list_node, NULL, ( const xmlChar * )"Splice_Line", NULL );
        XmlUtil::AddDoubleNode( sl_node, "PerSpan",     m_SpliceLineVec[i]->m_PerSpan() );
        //xmlAddDoubleNode( sl_node, "DefaultThick",    m_SpliceLineVec[i]->m_DefaultThick );

        //===== Splices =====//
        xmlNodePtr splice_list_node = xmlNewChild( sl_node, NULL, ( const xmlChar * )"Splice_List", NULL );
        for ( j = 0 ; j < ( int )m_SpliceLineVec[i]->m_SpliceVec.size() ; j++ )
        {
            FeaSplice* splice = m_SpliceLineVec[i]->m_SpliceVec[j];
            xmlNodePtr slice_node = xmlNewChild( splice_list_node, NULL, ( const xmlChar * )"Splice", NULL );

            XmlUtil::AddIntNode( slice_node, "FixedFlag", splice->m_FixedFlag );
            XmlUtil::AddDoubleNode( slice_node, "Thick",  splice->m_Thick() );
            XmlUtil::AddDoubleNode( slice_node, "Pos",  splice->m_Pos() );
        }
    }
}

void FeaSkin::ReadData(  xmlNodePtr root )
{
    int i, j;
    m_ExportFlag = ( XmlUtil::FindInt( root, "ExportFlag", 1 ) != 0 );  // Bool
    m_DefaultThick = XmlUtil::FindDouble( root, "DefaultThick", m_DefaultThick() );
    m_Density = XmlUtil::FindDouble( root, "Density", m_Density() );

    //===== Splice Lines =====//
    for ( i = 0 ; i < ( int )m_SpliceLineVec.size() ; i++ )
    {
        delete m_SpliceLineVec[i];
    }
    m_SpliceLineVec.clear();

    xmlNodePtr splice_line_list_node = XmlUtil::GetNode( root, "Splice_Line_List", 0 );
    if ( splice_line_list_node )
    {
        int num_splice_lines =  XmlUtil::GetNumNames( splice_line_list_node, "Splice_Line" );
        for ( i = 0 ; i < num_splice_lines ; i++ )
        {
            xmlNodePtr splice_line_node = XmlUtil::GetNode( splice_line_list_node, "Splice_Line", i );
            if ( splice_line_node )
            {
                FeaSpliceLine* sl = new FeaSpliceLine( m_DefaultThick() );
                sl->m_PerSpan = XmlUtil::FindDouble( splice_line_node, "PerSpan", sl->m_PerSpan() );
                m_SpliceLineVec.push_back( sl );

                xmlNodePtr splice_list_node = XmlUtil::GetNode( splice_line_node, "Splice_List", 0 );
                if ( splice_list_node )
                {
                    int num_splices = XmlUtil::GetNumNames( splice_list_node, "Splice" );
                    if ( num_splices )
                    {
                        sl->ClearSpliceVec();
                        for ( j = 0 ; j < num_splices ; j++ )
                        {
                            xmlNodePtr splice_node = XmlUtil::GetNode( splice_list_node, "Splice", j );
                            if ( splice_node )
                            {
                                FeaSplice* s = new FeaSplice();
                                s->m_FixedFlag = ( XmlUtil::FindInt( splice_node, "FixedFlag", 0 ) != 0 );  // Bool
                                s->m_Pos = XmlUtil::FindDouble( splice_node, "Pos", 0.0 );
                                s->m_Thick = XmlUtil::FindDouble( splice_node, "Thick", m_DefaultThick() );
                                sl->m_SpliceVec.push_back( s );
                            }
                        }
                    }
                }
                sort( sl->m_SpliceVec.begin(), sl->m_SpliceVec.end(), SpliceCompare );
            }
        }
    }
    ComputeSpliceLineEndPoints();
}

void FeaSkin::SetDefaultThick( double t )
{
    m_DefaultThick = t;
    for ( int i = 0 ; i < ( int )m_SpliceLineVec.size() ; i++ )
    {
        m_SpliceLineVec[i]->SetDefaultThick( m_DefaultThick() );
    }
}

void FeaSkin::AddSpliceLine()
{
    FeaSpliceLine* sl = new FeaSpliceLine( m_DefaultThick() );
    m_SpliceLineVec.push_back( sl );
    m_CurrSpliceLineID = ( int )m_SpliceLineVec.size() - 1;
    ComputeSpliceLineEndPoints();
}

void FeaSkin::DelCurrSpliceLine()
{
    vector< FeaSpliceLine* > tempVec;

    for ( int i = 0 ; i < ( int )m_SpliceLineVec.size() ; i++ )
    {
        if ( i !=  m_CurrSpliceLineID )
        {
            tempVec.push_back( m_SpliceLineVec[i] );
        }
        else
        {
            delete m_SpliceLineVec[i];
        }
    }
    m_SpliceLineVec = tempVec;

    m_CurrSpliceLineID = 0;


}

FeaSpliceLine* FeaSkin::GetCurrSpliceLine()
{
    if ( m_CurrSpliceLineID < ( int )m_SpliceLineVec.size() )
    {
        return m_SpliceLineVec[m_CurrSpliceLineID];
    }

    return NULL;
}

void FeaSkin::SetCurrSpliceLineID( int id )
{
    if ( id >= 0 && id < ( int )m_SpliceLineVec.size() )
    {
        m_CurrSpliceLineID = id;
    }
}


void FeaSkin::BuildMesh()
{
    if ( !m_Surf )
    {
        return;
    }

    //list< Tri* >::iterator t;
    //list <Tri*> tlist = m_Surf->GetMesh()->GetTriList();

    //for ( t = tlist.begin() ; t != tlist.end(); t++ )
    //{
    //  FeaTri* tri = new FeaTri;
    //  tri->Create( (*t)->n0->pnt, (*t)->n1->pnt, (*t)->n2->pnt );
    //  m_Elements.push_back( tri );
    //}
    //

    vector < vec3d >pvec = m_Surf->GetMesh()->GetSimpPntVec();
    vector < SimpTri > tvec = m_Surf->GetMesh()->GetSimpTriVec();
    for ( int i = 0 ; i < ( int )tvec.size() ; i++ )
    {
        FeaTri* tri = new FeaTri;
        tri->Create( pvec[tvec[i].ind0], pvec[tvec[i].ind1], pvec[tvec[i].ind2] );
        m_Elements.push_back( tri );
    }
}

bool SpliceLinePerSpanCompare( FeaSpliceLine* a, FeaSpliceLine* b )
{
    return ( a->m_PerSpan() < b->m_PerSpan() );
}

void FeaSkin::SetNodeThick()
{
    int i, j;

    //==== Load All Nodes Into Vector ====//
    vector< FeaNode* > nvec;
    for ( i = 0 ; i < ( int )m_Elements.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )m_Elements[i]->m_Corners.size() ; j++ )
        {
            nvec.push_back( m_Elements[i]->m_Corners[j] );
        }
        for (  j = 0 ; j < ( int )m_Elements[i]->m_Mids.size() ; j++ )
        {
            nvec.push_back( m_Elements[i]->m_Mids[j] );
        }

        nvec.back()->m_Thick = m_DefaultThick();
    }

    //==== Sort Splice Vec by Span Placement ====//
    sort( m_SpliceLineVec.begin(), m_SpliceLineVec.end(), SpliceLinePerSpanCompare );

    if ( !m_WingSection )
    {
        return;
    }
    //WingSection* sect = FeaMeshMgr.GetWingSection( m_SectID );
    //if ( !sect )
    //  return;

    double per_span, per_chord;
    for ( i = 0 ; i < ( int )nvec.size() ; i++ )
    {
        m_WingSection->ComputePerSpanChord( nvec[i]->m_Pnt, &per_span, &per_chord );

        //==== Find Bounding Splice Lines ====//
        double span_fract = 0.0;
        FeaSpliceLine* sp0 = NULL;
        FeaSpliceLine* sp1 = NULL;
        if ( m_SpliceLineVec.size() == 1 )
        {
            sp0 = m_SpliceLineVec[0];
            sp1 = m_SpliceLineVec[0];
        }
        else if ( m_SpliceLineVec.size() >= 2 )
        {
            if ( per_span <= m_SpliceLineVec[0]->m_PerSpan() )
            {
                sp0 = m_SpliceLineVec[0];
                sp1 = m_SpliceLineVec[0];
            }
            else if ( per_span >= m_SpliceLineVec.back()->m_PerSpan() )
            {
                sp0 = m_SpliceLineVec.back();
                sp1 = m_SpliceLineVec.back();
            }
            else
            {
                for ( j = 0 ; j < ( int )m_SpliceLineVec.size() - 1 ; j++ )
                {
                    if ( per_span >= m_SpliceLineVec[j]->m_PerSpan() &&
                            per_span <= m_SpliceLineVec[j + 1]->m_PerSpan() )
                    {
                        sp0 = m_SpliceLineVec[j];
                        sp1 = m_SpliceLineVec[j + 1];

                        double denom = sp1->m_PerSpan() - sp0->m_PerSpan();
                        if ( denom > DBL_EPSILON )
                        {
                            span_fract = ( per_span - sp0->m_PerSpan() ) / denom;
                        }
                        break;
                    }
                }
            }
        }

        //===== Interpolate Thickness From Each Splice Line =====//
        if ( sp0 && sp1 )
        {
            double t0 = sp0->ComputeThick( per_chord );
            double t1 = sp1->ComputeThick( per_chord );
            nvec[i]->m_Thick = t0 + span_fract * ( t1 - t0 );
        }
        else
        {
            nvec[i]->m_Thick = m_DefaultThick();
        }
    }
}

void FeaSkin::ComputeSpliceLineEndPoints()
{
    for ( int i = 0 ; i < ( int )m_SpliceLineVec.size() ; i++ )
    {
        double perSpan = m_SpliceLineVec[i]->m_PerSpan();
        vec3d lePnt = m_WingSection->CompPnt( WingSection::LE, perSpan );
        vec3d tePnt = m_WingSection->CompPnt( WingSection::TE, perSpan );
        m_SpliceLineVec[i]->SetEndPoints( lePnt, tePnt );
    }
}

//void FeaSkin::Draw( bool highlight )
//{
//  if ( !m_ExportFlag )
//      return;
//
//  for ( int i = 0 ; i < (int)m_SpliceLineVec.size() ; i++ )
//  {
//      if ( highlight && i == m_CurrSpliceLineID )
//      {
//          glColor3ub( 255, 0, 0 );
//          glLineWidth(4.0);
//      }
//      else
//      {
//          glColor3ub( 0, 0, 255 );
//          glLineWidth(2.0);
//      }
//      m_SpliceLineVec[i]->DrawMain();
//  }
//
//}

//============================================================================//
//============================================================================//
FeaPointMass::FeaPointMass()
{
}

FeaPointMass::~FeaPointMass()
{
}

//void FeaPointMass::Draw( bool highlight )
//{
//  if ( highlight )
//      glColor3ub( 255, 0, 0 );
//  else
//      glColor3ub( 100, 100, 100 );
//
//  glLineWidth(2.0);
//  glPointSize(6.0);
//  glBegin( GL_LINES );
//      glVertex3dv( m_Pos.data() );
//      glVertex3dv( m_AttachPos.data() );
//  glEnd();
//
//  glBegin( GL_POINTS );
//      glVertex3dv( m_AttachPos.data() );
//  glEnd();
//
//  glPointSize(12.0);
//  glColor3ub( 0, 0, 0 );
//  glBegin( GL_POINTS );
//      glVertex3dv( m_Pos.data() );
//  glEnd();
//
//
//}


