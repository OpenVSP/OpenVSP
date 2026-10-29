//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSecSurfGeom.h: A Geom whose surfaces are skinned through XSecSurfs placed in space.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPXSECSURFGEOM__INCLUDED_)
#define VSPXSECSURFGEOM__INCLUDED_

#include "Geom.h"
#include "ParmMgr.h"
#include "XSec.h"
#include "XSecSurf.h"

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//

//==== XSecSurf Geom ====//
/*
    A Geom of any number of surfaces, each skinned through an XSecSurf of BareXSecs that are placed in space with
    SetLoc.  A custom component's script builds them, and a built in component's UpdateSurf.
*/
class XSecSurfGeom : public Geom
{
public:
    XSecSurfGeom( Vehicle* vehicle_ptr );
    virtual ~XSecSurfGeom();

    virtual void ClearXSecSurfs();

    virtual int GetNumXSecSurfs() const                   { return ( int )m_XSecSurfVec.size(); }
    virtual XSecSurf* GetXSecSurf( int index );

    //==== Skin XSecs ====//
    virtual void SkinXSecSurf( bool closed_flag );
    virtual void CloneSurf( int index, Matrix4d & mat );
    virtual void TransformSurf( int index, Matrix4d & mat );

    //==== Set VSP Surf Type ====//
    // The type every surface is skinned and flagged as.
    virtual void SetVspSurfType( int type )               { m_VspSurfType = type; }
    virtual int GetVspSurfType( int surf_id ) const       { return m_VspSurfType; }
    virtual void SetVspSurfCfdType( int type )            { m_VspSurfCfdType = type; }

    //==== Offset XSecs For Conformal =====//
    virtual void OffsetXSecs( double off );

    // The ID of parms[i], or empty where there is none.
    template < class T >
    static string ParmIDAt( const vector< T* > & parms, int i )
    {
        if ( i < 0 || i >= ( int )parms.size() || !parms[i] )
        {
            return string();
        }
        return parms[i]->GetID();
    }

protected:

    // A built in component builds its surfaces between these, in the same order each update.  A surface or cross
    // section already in the place a new one is asked for, and of the same type, is used again, so it keeps its ID
    // from one update to the next.  Those not used again are deleted at the end.
    void BeginXSecSurfs();
    void EndXSecSurfs();

    // The next surface, its pointer rather than its ID, of cross sections of type xsec_type.
    XSecSurf* NewXSecSurf( int xsec_type = vsp::XSEC_BARE );

    // The next cross section of the surface, of type type, put at loc, with no continuity to its neighbors and its
    // skinning conditions unset.
    BareXSec* AppendXSec( XSecSurf* xsec_surf, int type, const vec3d & loc );

    // The multiple Tess_W less one must be of, for rows around to land on every corner of every section: the
    // least common multiple of the sections' numbers of corners, those of a linear section its points, a box's
    // eight counting the middles of its sides, and an ellipse's four, the ends of its axes.  A component sets
    // m_TessMult to 1 before it builds its sections, names each section's corners with NeedTessW, and then gives
    // it to Tess_W.
    void NeedTessW( int n );
    int m_TessMult;

    // A linear section at x through the points ( ys[i], zs[i] ), closed back to the first, its points spaced
    // evenly in its parameter.
    void AddPolySection( XSecSurf* surf, double x, const vector< double > & ys, const vector< double > & zs );

    // Sizes parms to n.  Those past n are deleted.  Each one added is a Parm of type parm_type named name_k, k its
    // place counted from first, in group, with value val between lower and upper.  Answers the size before, the
    // index the new Parms start at.
    template < class T >
    int ResizeParms( vector< T* > & parms, int n, int parm_type, const string & name, double val, double lower,
                     double upper, const string & descript, int first = 1, const string & group = "Design" )
    {
        n = std::max( n, 0 );
        int old = ( int )parms.size();
        for ( int i = n; i < old; i++ )
        {
            delete parms[i];
        }
        if ( n < old )
        {
            parms.resize( n );
        }
        for ( int i = old; i < n; i++ )
        {
            T* p = dynamic_cast< T* >( ParmMgr.CreateParm( parm_type ) );
            if ( p )
            {
                p->Init( name + "_" + to_string( i + first ), group, this, val, lower, upper );
                p->SetDescript( descript );
            }
            parms.push_back( p );
        }
        return old;
    }

    // Deletes the Parms of parms.
    template < class T >
    static void DeleteParms( vector< T* > & parms )
    {
        for ( int i = 0; i < ( int )parms.size(); i++ )
        {
            delete parms[i];
        }
        parms.clear();
    }

    // The value of the Parm name of group in a Geom's XML node, or def where it has none.  A Geom whose Parms are
    // sized by a count reads the count first, so the Parms are there for the rest of the node to set.
    static double DesignValue( xmlNodePtr & node, const char* name, double def, const char* group = "Design" );

    // A cross section at loc that is a point, an ellipse w across and h high, a circle d across, or a rectangle w
    // across and h high with square corners.
    BareXSec* AppendPointXSec( XSecSurf* xsec_surf, const vec3d & loc );
    BareXSec* AppendEllipseXSec( XSecSurf* xsec_surf, const vec3d & loc, double w, double h );
    BareXSec* AppendCircleXSec( XSecSurf* xsec_surf, const vec3d & loc, double d );
    BareXSec* AppendRectXSec( XSecSurf* xsec_surf, const vec3d & loc, double w, double h );

    vector< XSecSurf* > m_XSecSurfVec;

    // Where NewXSecSurf and each surface's AppendXSec are up to.
    int m_NextSurf;
    map< XSecSurf*, int > m_NextXSec;

    // Deletes the surfaces from index on.
    void DeleteXSecSurfsFrom( int index );

    int m_VspSurfType;
    int m_VspSurfCfdType;

    virtual void UpdateFlags();

    bool m_ConformalFlag;
    double m_ConformalOffset;
    virtual void ApplyConformalOffset( double off );
};


#endif // !defined(VSPXSECSURFGEOM__INCLUDED_)
