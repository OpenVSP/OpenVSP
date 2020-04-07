//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "XSecCurve.h"
#include "Geom.h"
//#include "SuperEllipse.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "FuselageGeom.h"
#include "BORGeom.h"
#include "Vehicle.h"
#include "Util.h"
#include "eli/geom/curve/length.hpp"
#include "Vec3d.h"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_point_creator<double, 3, curve_tolerance_type> piecewise_point_creator;
typedef eli::geom::curve::piecewise_circle_creator<double, 3, curve_tolerance_type> piecewise_circle_creator;
typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_ellipse_creator;
typedef eli::geom::curve::piecewise_superellipse_creator<double, 3, curve_tolerance_type> piecewise_superellipse_creator;
typedef eli::geom::curve::piecewise_linear_creator<double, 3, curve_tolerance_type> piecewise_linear_creator_type;

using std::string;
using namespace vsp;

//==== Default Constructor ====//
XSecCurve::XSecCurve()
{
    m_GroupName = "XSecCurve";
    m_GroupSuffix = -1;

    m_TECloseType.Init( "TE_Close_Type", "Close", this, CLOSE_NONE, CLOSE_NONE, CLOSE_NUM_TYPES - 1 );

    m_TECloseAbsRel.Init( "TE_Close_AbsRel", "Close", this, ABS, ABS, REL );

    m_TECloseThick.Init( "TE_Close_Thick", "Close", this, 0, 0, 1e12 );
    m_TECloseThick.SetDescript( "Thickness for trailing edge closure" );

    m_TECloseThickChord.Init( "TE_Close_Thick_Chord", "Close", this, 0, 0, 1e12 );
    m_TECloseThickChord.SetDescript( "T/C for trailing edge closure" );

    m_LECloseType.Init( "LE_Close_Type", "Close", this, CLOSE_NONE, CLOSE_NONE, CLOSE_NUM_TYPES - 1 );

    m_LECloseAbsRel.Init( "LE_Close_AbsRel", "Close", this, ABS, ABS, REL );

    m_LECloseThick.Init( "LE_Close_Thick", "Close", this, 0, 0, 1e12 );
    m_LECloseThick.SetDescript( "Thickness for leading edge closure" );

    m_LECloseThickChord.Init( "LE_Close_Thick_Chord", "Close", this, 0, 0, 1e12 );
    m_LECloseThickChord.SetDescript( "T/C for leading edge closure" );


    m_TETrimType.Init( "TE_Trim_Type", "Trim", this, TRIM_NONE, TRIM_NONE, TRIM_NUM_TYPES - 1 );

    m_TETrimAbsRel.Init( "TE_Trim_AbsRel", "Trim", this, ABS, ABS, REL );

    m_TETrimX.Init( "TE_Trim_X", "Trim", this, 0, 0, 1e12 );
    m_TETrimX.SetDescript( "X length to trim trailing edge" );

    m_TETrimXChord.Init( "TE_Trim_X_Chord", "Trim", this, 0, 0, 0.999 );
    m_TETrimXChord.SetDescript( "X/C length to trim trailing edge" );

    m_TETrimThick.Init( "TE_Trim_Thick", "Trim", this, 0, 0, 1e12 );
    m_TETrimThick.SetDescript( "Thickness to trim trailing edge" );

    m_TETrimThickChord.Init( "TE_Trim_Thick_Chord", "Trim", this, 0, 0, 1e12 );
    m_TETrimThickChord.SetDescript( "T/C to trim trailing edge" );

    m_LETrimType.Init( "LE_Trim_Type", "Trim", this, TRIM_NONE, TRIM_NONE, TRIM_NUM_TYPES - 1 );

    m_LETrimAbsRel.Init( "LE_Trim_AbsRel", "Trim", this, ABS, ABS, REL );

    m_LETrimX.Init( "LE_Trim_X", "Trim", this, 0, 0, 1e12 );
    m_LETrimX.SetDescript( "X length to trim leading edge" );

    m_LETrimXChord.Init( "LE_Trim_X_Chord", "Trim", this, 0, 0, 0.999 );
    m_LETrimXChord.SetDescript( "X/C length to trim leading edge" );

    m_LETrimThick.Init( "LE_Trim_Thick", "Trim", this, 0, 0, 1e12 );
    m_LETrimThick.SetDescript( "Thickness to trim leading edge" );

    m_LETrimThickChord.Init( "LE_Trim_Thick_Chord", "Trim", this, 0, 0, 1e12 );
    m_LETrimThickChord.SetDescript( "T/C to trim leading edge" );

    m_TECapType.Init( "TE_Cap_Type", "Cap", this, FLAT_END_CAP, FLAT_END_CAP, NUM_END_CAP_OPTIONS - 1 );
    m_TECapLength.Init( "TE_Cap_Length", "Cap", this, 1, 0, 20 );
    m_TECapOffset.Init( "TE_Cap_Offset", "Cap", this, 0, -20, 20 );
    m_TECapStrength.Init( "TE_Cap_Strength", "Cap", this, 0.5, 0, 1);

    m_LECapType.Init( "LE_Cap_Type", "Cap", this, FLAT_END_CAP, FLAT_END_CAP, NUM_END_CAP_OPTIONS - 1 );
    m_LECapLength.Init( "LE_Cap_Length", "Cap", this, 1, 0, 20 );
    m_LECapOffset.Init( "LE_Cap_Offset", "Cap", this, 0, -20, 20 );
    m_LECapStrength.Init( "LE_Cap_Strength", "Cap", this, 0.5, 0, 1);

    m_Theta.Init( "Theta", m_GroupName, this, 0, -180.0, 180.0 );
    m_Scale.Init( "Scale", m_GroupName, this, 1.0, 1e-12, 1e12 );
    m_DeltaX.Init( "DeltaX", m_GroupName, this, 0, -1e3, 1e3 );
    m_DeltaY.Init( "DeltaY", m_GroupName, this, 0, -1e3, 1e3 );
    m_ShiftLE.Init( "ShiftLE", m_GroupName, this, 0, -1.9, 1.9 );

    m_FakeWidth = 1.0;
    m_UseFakeWidth = false;
    m_ForceWingType = false;

    m_yscale = 1.0;
}

//==== Convert Any XSec to Cubic Bezier Edit Curve ====//
EditCurveXSec* XSecCurve::ConvertToEdit( XSecCurve* orig_curve )
{
    if ( orig_curve->GetType() == vsp::XS_EDIT_CURVE )
    {
        return dynamic_cast<EditCurveXSec*>( orig_curve );
    }

    VspCurve vsp_curve = orig_curve->GetBaseEditCurve();

    // Make the curve more coarse. Force wingtype to false to avoid TMAGIC curve spitting, since 
    // XSecCurve::Update() already does it.
    vsp_curve.ToBinaryCubic( false, 1e-3, 0.01, 0, 3 );

    vector < vec3d > point_vec;
    vector < double > param_vec;

    vsp_curve.GetCubicControlPoints( point_vec, param_vec );

    double offset = orig_curve->GetWidth() / 2;

    for ( size_t i = 0; i < param_vec.size(); i++ )
    {
        param_vec[i] = param_vec[i] / 4.0; // Store point parameter (0-1) internally
        // Shift by 1/2 width and nondimensionalize
        point_vec[i].set_x( ( point_vec[i].x() - offset ) / max( orig_curve->GetWidth(), 1E-9 ) );
        point_vec[i].set_y( point_vec[i].y() / max( orig_curve->GetHeight(), 1E-9 ) );
    }

    EditCurveXSec* xscrv_ptr = new EditCurveXSec();

    xscrv_ptr->CopyFrom( orig_curve );
    xscrv_ptr->m_SymType.Set( vsp::SYM_NONE );
    // TODO: Transfer symmetry and G1 continuity qualities

    // Transfer width and height parm values
    xscrv_ptr->SetWidthHeight( orig_curve->GetWidth(), orig_curve->GetHeight() );

    // Set Bezier control points
    xscrv_ptr->SetPntVecs( param_vec, point_vec );

    return xscrv_ptr;
}

void XSecCurve::SetGroupDisplaySuffix( int num )
{
    m_GroupSuffix = num;
    //==== Assign Group Suffix To All Parms ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

string XSecCurve::GetName()
{
    ParmContainer* pc = GetParentContainerPtr();

    if ( pc )
    {
        char str[256];
        sprintf( str, "_%d", m_GroupSuffix );

        ParmContainer* ppc = pc->GetParentContainerPtr();
        if ( ppc )
        {
            return ppc->GetName() + " " + m_GroupName + string(str);
        }
        return pc->GetName();
    }
    return ParmContainer::GetName();
}

//==== Set Scale ====//
void XSecCurve::SetScale( double scale )
{
    SetWidthHeight( GetWidth()*scale, GetHeight()*scale );

    if ( m_TECloseType() != XSEC_CLOSE_TYPE::CLOSE_NONE )
    {
        if ( m_TECloseAbsRel() == ABS )
        {
            m_TECloseThick.Set( m_TECloseThick() * scale );
        }
    }

    if ( m_LECloseType() != XSEC_CLOSE_TYPE::CLOSE_NONE )
    {
        if ( m_LECloseAbsRel() == ABS )
        {
            m_LECloseThick.Set( m_LECloseThick() * scale );
        }
    }

    if ( m_TETrimType() == XSEC_TRIM_TYPE::TRIM_THICK )
    {
        if ( m_TETrimAbsRel() == ABS )
        {
            m_TETrimThick.Set( m_TETrimThick() * scale );
        }
    }

    if ( m_TETrimType() == XSEC_TRIM_TYPE::TRIM_X )
    {
        if ( m_TETrimAbsRel() == ABS )
        {
            m_TETrimX.Set( m_TETrimX() * scale );
        }
    }

    if ( m_LETrimType() == XSEC_TRIM_TYPE::TRIM_THICK )
    {
        if ( m_LETrimAbsRel() == ABS )
        {
            m_LETrimThick.Set( m_LETrimThick() * scale );
        }
    }

    if ( m_LETrimType() == XSEC_TRIM_TYPE::TRIM_X )
    {
        if ( m_LETrimAbsRel() == ABS )
        {
            m_LETrimX.Set( m_LETrimX() * scale );
        }
    }
}

//==== Offset Curve ====//
void XSecCurve::OffsetCurve( double offset_val )
{
    double w = GetWidth();
    double h = GetHeight();

    SetWidthHeight( w - offset_val*2.0, h - offset_val*2.0 );

}

//==== Parm Changed ====//
void XSecCurve::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;

        //==== Notify Parent Container (XSec) ====//
        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            pc->ParmChanged( parm_ptr, type );
        }

        return;
    }

    Update();

    //==== Notify Parent Container (XSec) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

//==== Update ====//
void XSecCurve::Update()
{
    m_BaseEditCurve = m_Curve; // Baseline VspCurve to initialize an EditCurveXSec with 

    m_TETrimX.SetUpperLimit( 0.999 * GetWidth() );

    bool wingtype = false;

    ParmContainer* pc = GetParentContainerPtr();

    XSec* xs = dynamic_cast< XSec* > (pc);

    if ( xs )
    {
        if ( xs->GetType() == XSEC_WING || xs->GetType() == XSEC_PROP )
        {
            wingtype = true;
        }
    }
    else
    {
        BORGeom* bg = dynamic_cast< BORGeom* > (pc);

        if ( bg )
        {
            wingtype = true;
        }
    }

    if ( m_ForceWingType )
    {
        wingtype = true;
    }

    // Order of these curve modifiers matters.
    CloseTE( wingtype );
    TrimTE( wingtype );

    CloseLE( wingtype );
    TrimLE( wingtype );

    CapTE( wingtype );
    CapLE( wingtype );

    if ( m_Type != XS_POINT )
    {
        double w = GetWidth();
        double h = GetHeight();
        if ( w == 0 ) w = 1;
        if ( h == 0 ) h = 1;
        double lref = sqrt( w * w + h * h );
        m_Curve.ToBinaryCubic( wingtype, 1e-6 * lref );
    }

    RotTransScale();

    if ( m_yscale != 1.0 )
    {
        m_Curve.ScaleY( m_yscale );
    }

    m_LateUpdateFlag = false;
}

//==== Get Curve ====//
VspCurve& XSecCurve::GetCurve()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve;
}

//==== Encode XML ====//
xmlNodePtr XSecCurve::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSecCurve", NULL );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
    }
    return xsec_node;
}

//==== Copy From ====//
void XSecCurve::CopyFrom( XSecCurve* from_crv )
{
    string lastreset = ParmMgr.ResetRemapID();
    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );

    from_crv->EncodeXml( root );
    DecodeXml( root );

    xmlFreeNode( root );
    ParmMgr.ResetRemapID( lastreset );
}

//==== Compute Area ====//
double XSecCurve::ComputeArea()
{
    VspCurve curve = GetCurve();
    vector<vec3d> pnts;
    curve.TessAdapt( pnts, 1e-3, 10 );
    return poly_area( pnts );
}

void XSecCurve::CloseTE( bool wingtype )
{
    if ( m_Type ==  XS_POINT )
    {
        return;
    }

    piecewise_curve_type crv = m_Curve.GetCurve();

    double tmin, tmax;
    tmin = crv.get_parameter_min();
    tmax = crv.get_parameter_max();

    double thick;

    if ( m_TECloseAbsRel() == ABS )
    {
        if ( m_UseFakeWidth )
        {
            thick = m_TECloseThick() * GetWidth() / GetFakeWidth();
        }
        else
        {
            thick = m_TECloseThick();
        }
    }
    else
    {
        thick = m_TECloseThickChord() * GetWidth();
    }

    double tle = 2.0;

    threed_point_type plow = crv.f( tmin );
    threed_point_type pup = crv.f( tmax );
    threed_point_type ple = crv.f( tle );

    piecewise_curve_type c_up, c_low;
    crv.split( c_low, c_up, tle );

    double dx = plow[0] - ple[0];
    double dy = pup[1] - plow[1] - thick;

    if ( wingtype && m_TECloseType() != CLOSE_NONE && abs( dx ) > FLT_EPSILON )
    {
        if ( m_TECloseType() == CLOSE_SKEWLOW ||
             m_TECloseType() == CLOSE_SKEWUP  ||
             m_TECloseType() == CLOSE_SKEWBOTH )
        {

            piecewise_curve_type::rotation_matrix_type mat;
            mat.setIdentity();

            double slope = dy / dx;

            if ( m_TECloseType() == CLOSE_SKEWBOTH )
            {
                slope *= 0.5;
            }

            if ( m_TECloseType() == CLOSE_SKEWLOW ||
                 m_TECloseType() == CLOSE_SKEWBOTH )
            {
                mat(1,0) = slope;
                c_low.rotate( mat, ple );
            }

            slope = -slope;

            if ( m_TECloseType() == CLOSE_SKEWUP  ||
                 m_TECloseType() == CLOSE_SKEWBOTH )
            {
                mat(1,0) = slope;
                c_up.rotate( mat, ple );
            }

            crv = c_low;
            crv.push_back( c_up );
        }
        else if ( m_TECloseType() == CLOSE_EXTRAP )
        {
            threed_point_type fplow = crv.fp( tmin );
            threed_point_type fpup = crv.fp( tmax );

            threed_point_type dp = plow - pup;
            dp[1] += thick;

            double m = ( fplow[0] * fpup[1] - fpup[0] * fplow[1] );
            if ( m != 0.0 ) // Make sure lines aren't parallel.
            {
                // Solve for intersection point.
                double tlow = ( fpup[0] * dp[1] - dp[0] * fpup[1] ) / m;

                if ( tlow < 0.0 ) // Make sure intersection is aft of trailing edge.
                {
                    threed_point_type pnewlow, pnewup;

                    pnewlow = plow + tlow * fplow;
                    pnewup = pnewlow;
                    pnewup[1] += thick;

                    // Calculate arclen to scale parameters.
                    double lup = 0.0, llow = 0.0;

                    length( llow, c_low, 1.0e-4 );
                    length( lup, c_up, 1.0e-4 );

                    double dup, dlow;

                    dlow = dist( plow, pnewlow );
                    dup = dist( pup, pnewup );

                    double sup, slow;
                    sup = lup + dup;
                    slow = llow + dlow;

                    double dtup, dtlow;
                    dtup = ( tmax - tle ) * dup / sup;
                    dtlow = ( tle - tmin ) * dlow / slow;

                    c_low.scale_t( tmin + dtlow, tle );
                    c_up.scale_t( tle, tmax - dtup );

                    // Create curve extensions.
                    piecewise_linear_creator_type plc( 1 );

                    plc.set_t0( tmin );
                    plc.set_segment_dt( dtlow, 0 );
                    plc.set_corner( pnewlow, 0 );
                    plc.set_corner( plow, 1 );

                    plc.create( crv );

                    crv.push_back( c_low );
                    crv.push_back( c_up );

                    plc.set_t0( tmax - dtup );
                    plc.set_segment_dt( dtup, 0 );
                    plc.set_corner( pup, 0 );
                    plc.set_corner( pnewup, 1 );

                    piecewise_curve_type c_last;
                    plc.create( c_last );

                    crv.push_back( c_last );
                }
            }
        }
    }

    threed_point_type p1 = crv.f( tmin );
    threed_point_type p2 = crv.f( tmax );
    double d = dist( p1, p2 );

    double div = GetWidth();
    if ( div == 0.0 )
    {
        div = 1.0;
    }

    if ( wingtype || ( d / div ) > 1e-4 )
    {
        // Connect TE points with straight blunt face.  Place corners at appropriate parameter.
        piecewise_curve_type c_last, c_main;

        threed_point_type p3 = ( p1 + p2 ) / 2.0;

        c_main = crv;

        c_main.scale_t( TMAGIC, tmax - TMAGIC );

        piecewise_linear_creator_type plc( 1 );

        plc.set_t0( 0.0 );
        plc.set_segment_dt( TMAGIC, 0 );
        plc.set_corner( p3, 0 );
        plc.set_corner( p1, 1 );

        plc.create( crv );
        crv.push_back( c_main );

        plc.set_t0( tmax - TMAGIC );
        plc.set_segment_dt( TMAGIC, 0 );
        plc.set_corner( p2, 0 );
        plc.set_corner( p3, 1 );

        plc.create( c_last );

        crv.push_back( c_last );

        // Should be redundant, but just to be sure floating point error didn't accumulate.
        crv.set_tmax( tmax );
    }
    m_Curve.SetCurve( crv );

    if ( m_UseFakeWidth )
    {
        div = GetFakeWidth();
        if ( div == 0.0 )
        {
            div = 1.0;
        }
        d = d * div;
    }

    if ( m_TECloseType() != CLOSE_NONE && abs( dx ) > FLT_EPSILON )
    {

        if ( m_TECloseAbsRel() == ABS )
        {
            m_TECloseThickChord.Set( d / div );
        }
        else
        {
            m_TECloseThick.Set( d );
        }
    }
    else
    {
        m_TECloseThickChord.Set( d / div );
        m_TECloseThick.Set( d );
    }
}

void XSecCurve::CloseLE( bool wingtype )
{
    if ( m_Type ==  XS_POINT )
    {
        return;
    }

    if ( !wingtype )
    {
        return;
    }

    piecewise_curve_type crv = m_Curve.GetCurve();

    double tmin, tmax;
    tmin = crv.get_parameter_min();
    tmax = crv.get_parameter_max();

    double thick;

    if ( m_LECloseAbsRel() == ABS )
    {
        if ( m_UseFakeWidth )
        {
            thick = m_LECloseThick() * GetWidth() / GetFakeWidth();
        }
        else
        {
            thick = m_LECloseThick();
        }
    }
    else
    {
        thick = m_LECloseThickChord() * GetWidth();
    }

    double tle = 2.0;

    piecewise_curve_type c_up, c_low;
    crv.split( c_low, c_up, tle );

    threed_point_type plow = crv.f( tmin );
    threed_point_type pup = crv.f( tmax );
    threed_point_type pte = ( plow + pup ) * 0.5;
    threed_point_type ple = crv.f( tle );

    double dx = pte[0] - ple[0];
    double dy = thick;

    if ( m_LECloseType() != CLOSE_NONE && abs( dx ) > FLT_EPSILON )
    {
        piecewise_curve_type::rotation_matrix_type mat;
        mat.setIdentity();

        double slope = dy / dx;

        if ( m_LECloseType() == CLOSE_SKEWBOTH )
        {
            slope *= 0.5;
        }

        if ( m_LECloseType() == CLOSE_SKEWLOW ||
             m_LECloseType() == CLOSE_SKEWBOTH )
        {
            mat(1,0) = slope;
            c_low.rotate( mat, pte );
        }

        slope = -slope;

        if ( m_LECloseType() == CLOSE_SKEWUP  ||
             m_LECloseType() == CLOSE_SKEWBOTH )
        {
            mat(1,0) = slope;
            c_up.rotate( mat, pte );
        }
    }

    piecewise_curve_type c_up_te, c_up_main, c_low_te, c_low_main;

    c_low.split( c_low_te, c_low_main, TMAGIC );
    c_up.split( c_up_main, c_up_te, tmax - TMAGIC );

    c_low_main.scale_t( TMAGIC, tle - TMAGIC );
    c_up_main.scale_t( tle + TMAGIC, tmax - TMAGIC );

    threed_point_type p1 = c_low_main.f( tle - TMAGIC );
    threed_point_type p3 = c_up_main.f( tle + TMAGIC );
    threed_point_type p2 = ( p1 + p3 ) * 0.5;

    double d = dist( p1, p3 );

    // Connect LE points with straight blunt face.  Place corners at appropriate parameter.

    piecewise_linear_creator_type plc( 2 );

    plc.set_t0( tle - TMAGIC );
    plc.set_segment_dt( TMAGIC, 0 );
    plc.set_segment_dt( TMAGIC, 1 );
    plc.set_corner( p1, 0 );
    plc.set_corner( p2, 1 );
    plc.set_corner( p3, 2 );

    piecewise_curve_type cnew;

    plc.create( cnew );

    crv = c_low_te;
    crv.push_back( c_low_main );
    crv.push_back( cnew );
    crv.push_back( c_up_main );
    crv.push_back( c_up_te );

    // Should be redundant, but just to be sure floating point error didn't accumulate.
    crv.set_tmax( tmax );

    m_Curve.SetCurve( crv );

    double div = GetWidth();
    if ( div == 0.0 )
    {
        div = 1.0;
    }

    if ( m_UseFakeWidth )
    {
        div = GetFakeWidth();
        if ( div == 0.0 )
        {
            div = 1.0;
        }
        d = d * div;
    }

    if ( m_LECloseType() != CLOSE_NONE && abs( dx ) > FLT_EPSILON )
    {

        if ( m_LECloseAbsRel() == ABS )
        {
            m_LECloseThickChord.Set( d / div );
        }
        else
        {
            m_LECloseThick.Set( d );
        }
    }
    else
    {
        m_LECloseThickChord.Set( d / div );
        m_LECloseThick.Set( d );
    }
}

//==== Modify Curve ====//
void XSecCurve::TrimTE( bool wingtype )
{
    if ( m_Type == XS_POINT || !wingtype )
    {
        return;
    }

    double ttrim = 0.0;
    double xtrim = 0.0;

    piecewise_curve_type crv = m_Curve.GetCurve();

    double tmin, tmax;
    tmin = crv.get_parameter_min();
    tmax = crv.get_parameter_max();

    double ts1 = tmin;
    double ts2 = tmax;

    double umin, umax, umid, delta;
    umin = tmin + TMAGIC;
    umax = tmax - TMAGIC;
    umid = ( umin + umax ) * 0.5;

    vec3d telow = m_Curve.CompPnt( umin );
    vec3d teup = m_Curve.CompPnt( umax );
    vec3d te = ( telow + teup ) * 0.5;

    if ( m_TETrimType() != TRIM_NONE )
    {
        if ( m_TETrimType() == TRIM_X )
        {
            if ( m_UseFakeWidth )
            {
                xtrim = m_TETrimX() * GetWidth() / GetFakeWidth();
            }
            else
            {
                xtrim = m_TETrimX();
            }

            if ( m_TETrimAbsRel() == REL )
            {
                xtrim = m_TETrimXChord() * GetWidth();
            }

            // Use bisection solver with specified min/max parameters.
            m_Curve.FindDistant( ts1, telow, xtrim, umin, umid );
            m_Curve.FindDistant( ts2, teup, xtrim, umid, umax );
        }
        else if ( m_TETrimType() == TRIM_THICK )
        {
            if ( m_UseFakeWidth )
            {
                ttrim = m_TETrimThick() * GetWidth() / GetFakeWidth();
            }
            else
            {
                ttrim = m_TETrimThick();
            }

            if ( m_TETrimAbsRel() == REL )
            {
                ttrim = m_TETrimThickChord() * GetWidth();
            }

            const unsigned int ncache = 20;
            vector< vec3d > ptcache( ncache );
            vector< double > ucache( ncache );
            vector< double > dcache( ncache );
            vector< double > dupcache( ncache );
            vector< double > dlowcache( ncache );

            // Rough tessellation to find approximate trim locations.
            m_Curve.TesselateNoCorner( ncache, umin, umax, ptcache, ucache );

            // Find most distant point.
            unsigned int imax = 0;
            double mindu = 2.0 * umax;
            for ( int i = 0; i < ncache; i++ )
            {
                dcache[i] = dist( ptcache[i], te );
                dupcache[i] = dist( ptcache[i], teup );
                dlowcache[i] = dist( ptcache[i], telow );

                if ( std::abs( ucache[i] - umid ) < mindu )
                {
                    imax = i;
                    mindu = std::abs( ucache[i] - umid );
                }
            }

            delta = ( umax - umin ) / ( ncache - 1 );

            // Calculate approximate upper surface u parameter corresponding to lower points.
            vector< double > uuppermatch( imax );
            vector< double > iupper( imax );
            vector< vec3d > xupper( imax );
            vector< double > thick( imax );

            for ( int i = 0; i < imax; i++ )
            {
                double dlow = dcache[i];

                // Find bounding segment.  Nearest point last, traverse vector in reverse.
                int j;
                for ( j = ncache - 1; j > imax; j-- )
                {
                    if ( dcache[j] > dlow )
                    {
                        break;
                    }
                }

                // Linearly interpolate to find  matching point.
                double jmatch;
                vec3d x;
                if ( j == ncache - 1 )
                {
                    jmatch = 1.0 * j;
                    x = ptcache[j];
                }
                else
                {
                    double frac = ( dlow - dcache[j] ) / ( dcache[j+1] - dcache[j] );
                    jmatch = j + frac;
                    x = ptcache[j] + ( ptcache[j+1] - ptcache[j] ) * frac;
                }

                // Calculate properties at matching point.
                iupper[i] = jmatch;
                uuppermatch[i] = umin + delta * jmatch;
                xupper[i] = x;
                thick[i] = dist( xupper[i], ptcache[i] );
            }

            // Quick pass to find bounding thickness points.
            bool bounded = false;
            int i;
            for ( i = 0; i < imax - 1; i++ )
            {
                if ( ( thick[ i ] < ttrim && thick[ i + 1 ] > ttrim ) || ( thick[ i ] > ttrim && thick[ i + 1 ] < ttrim ) )
                {
                    bounded = true;
                    break;
                }
            }

            if ( bounded )
            {
                double frac = ( ttrim - thick[ i ] ) / ( thick[ i + 1 ] - thick[ i ] );

                double itarget = i + frac;

                double ulower = umin + delta * itarget;
                double uupper = uuppermatch[ i ] + frac * ( uuppermatch[ i + 1 ] - uuppermatch[ i ] );

                m_Curve.FindThickness( ts1, ts2, te, ttrim, ulower, uupper );
            }
            else
            {
                ts1 = umin;
                ts2 = umax;
            }
        }

        // Limit trimming to make sure something happens.
        ts1 = max( ts1, umin );
        ts2 = min( ts2, umax );

        // Limit parameters to upper/lower surfaces.
        ts1 = min( umid, ts1 );
        ts2 = max( umid, ts2 );

        // Actually trim the airfoil curve.
        piecewise_curve_type c1, c2, c3, c4;

        threed_point_type p1 = crv.f( ts1 );
        threed_point_type p2 = crv.f( ts2 );

        threed_point_type p3 = ( p1 + p2 ) / 2.0;

        crv.split( c1, c2, ts1 );

        c2.split(c3, c4, ts2 );

        c3.scale_t( umin, umax );

        piecewise_linear_creator_type plc( 1 );

        plc.set_t0( tmin );
        plc.set_segment_dt( TMAGIC, 0 );
        plc.set_corner( p3, 0 );
        plc.set_corner( p1, 1 );

        plc.create( crv );

        plc.set_t0( umax );
        plc.set_segment_dt( TMAGIC, 0 );
        plc.set_corner( p2, 0 );
        plc.set_corner( p3, 1 );

        plc.create( c2 );

        crv.push_back( c3 );
        crv.push_back( c2 );

        // Should be redundant, but just to be sure floating point error didn't accumulate.
        crv.set_tmax( tmax );

        m_Curve.SetCurve( crv );
    }

    vec3d cornerlow = m_Curve.CompPnt( umin );
    vec3d cornerup = m_Curve.CompPnt( umax );

    ttrim = dist( cornerlow, cornerup );
    xtrim = dist( cornerlow, telow );

    double div = GetWidth();
    if ( div == 0.0 )
    {
        div = 1.0;
    }

    if ( m_UseFakeWidth )
    {
        div = GetFakeWidth();
        if ( div == 0.0 )
        {
            div = 1.0;
        }
        xtrim = xtrim * div;
        ttrim = ttrim * div;
    }

    // Set non-driving parameters to match current state.
    if ( m_TETrimType() == TRIM_NONE )
    {
        m_TETrimX.Set( xtrim );
        m_TETrimXChord.Set( xtrim / div );
        m_TETrimThick.Set( ttrim );
        m_TETrimThickChord.Set( ttrim / div );
    }
    else if ( m_TETrimType() == TRIM_THICK )
    {
        m_TETrimX.Set( xtrim );
        m_TETrimXChord.Set( xtrim / div );

        if ( m_TETrimAbsRel() == ABS )
        {
            m_TETrimThickChord.Set( ttrim / div );
        }
        else
        {
            m_TETrimThick.Set( ttrim );
        }
    }
    else if ( m_TETrimType() == TRIM_X )
    {
        if ( m_TETrimAbsRel() == ABS )
        {
            m_TETrimXChord.Set( xtrim / div );
        }
        else
        {
            m_TETrimX.Set( xtrim );
        }
        m_TETrimThick.Set( ttrim );
        m_TETrimThickChord.Set( ttrim / div );
    }
}

//==== Modify Curve ====//
void XSecCurve::TrimLE( bool wingtype )
{
    if ( m_Type == XS_POINT || !wingtype )
    {
        return;
    }

    double ttrim = 0.0;
    double xtrim = 0.0;

    piecewise_curve_type crv = m_Curve.GetCurve();

    double tmin, tmax;
    tmin = crv.get_parameter_min();
    tmax = crv.get_parameter_max();


    double umin, umax, umid, umidlow, umidup, delta;
    umin = tmin + TMAGIC;
    umax = tmax - TMAGIC;
    umid = ( umin + umax ) * 0.5;

    umidlow = umid - TMAGIC;
    umidup = umid + TMAGIC;

    double ts1 = umidlow;
    double ts2 = umidup;

    vec3d lelow = m_Curve.CompPnt( umidlow );
    vec3d leup = m_Curve.CompPnt( umidup );
    vec3d le = m_Curve.CompPnt( umid );

    if ( m_LETrimType() != TRIM_NONE )
    {
        if ( m_LETrimType() == TRIM_X )
        {
            if ( m_UseFakeWidth )
            {
                xtrim = m_LETrimX() * GetWidth() / GetFakeWidth();
            }
            else
            {
                xtrim = m_LETrimX();
            }

            if ( m_LETrimAbsRel() == REL )
            {
                xtrim = m_LETrimXChord() * GetWidth();
            }

            // Use bisection solver with specified min/max parameters.
            m_Curve.FindDistant( ts1, lelow, xtrim, umin, umidlow );
            m_Curve.FindDistant( ts2, leup, xtrim, umidup, umax );
        }
        else if ( m_LETrimType() == TRIM_THICK )
        {
            if ( m_UseFakeWidth )
            {
                ttrim = m_LETrimThick() * GetWidth() / GetFakeWidth();
            }
            else
            {
                ttrim = m_LETrimThick();
            }

            if ( m_LETrimAbsRel() == REL )
            {
                ttrim = m_LETrimThickChord() * GetWidth();
            }

            const unsigned int ncache = 21;
            vector< vec3d > ptcache( ncache );
            vector< double > ucache( ncache );
            vector< double > dcache( ncache );
            vector< double > dupcache( ncache );
            vector< double > dlowcache( ncache );

            // Rough tessellation to find approximate trim locations.
            m_Curve.TesselateNoCorner( ncache, umin, umax, ptcache, ucache );

            // Find most distant point.
            unsigned int isplit = 0;
            double mindu = 2.0 * umax;
            for ( int i = 0; i < ncache; i++ )
            {
                dcache[i] = dist( ptcache[i], le );
                dupcache[i] = dist( ptcache[i], leup );
                dlowcache[i] = dist( ptcache[i], lelow );

                if ( std::abs( ucache[i] - umid ) < mindu )
                {
                    isplit = i;
                    mindu = std::abs( ucache[i] - umid );
                }
            }

            delta = ( umax - umin ) / ( ncache - 1 );

            // Calculate approximate upper surface u parameter corresponding to lower points.
            vector< double > uuppermatch( isplit + 1 );
            vector< double > iupper( isplit + 1 );
            vector< vec3d > xupper( isplit + 1 );
            vector< double > thick( isplit + 1 );

            for ( int i = 0; i <= isplit; i++ )
            {
                double dlow = dcache[i];

                int j;
                for ( j = isplit + 1; j < ncache; j++ )
                {
                    if ( dcache[j] >= dlow )
                    {
                        break;
                    }
                }

                // Linearly interpolate to find  matching point.
                double jmatch;
                vec3d x;

                double frac = ( dlow - dcache[j-1] ) / ( dcache[j] - dcache[j-1] );
                jmatch = j - 1 + frac;
                x = ptcache[j-1] + ( ptcache[j] - ptcache[j-1] ) * frac;

                // Calculate properties at matching point.
                iupper[i] = jmatch;
                uuppermatch[i] = umin + delta * jmatch;
                xupper[i] = x;
                thick[i] = dist( xupper[i], ptcache[i] );
            }

            // Quick pass to find bounding thickness points.
            bool bounded = false;
            int i;
            for ( i = isplit - 1; i >= 0; i-- )
            {
                if ( ( thick[ i ] < ttrim && thick[ i + 1 ] > ttrim ) || ( thick[ i ] > ttrim && thick[ i + 1 ] < ttrim ) )
                {
                    bounded = true;
                    break;
                }
            }

            if ( bounded )
            {
                double frac = ( ttrim - thick[ i ] ) / ( thick[ i + 1 ] - thick[ i ] );

                double itarget = i + frac;

                double ulower = umin + delta * itarget;
                double uupper = uuppermatch[ i ] + frac * ( uuppermatch[ i + 1 ] - uuppermatch[ i ] );

                m_Curve.FindThickness( ts1, ts2, le, ttrim, ulower, uupper );
            }
            else
            {
                ts1 = umidlow;
                ts2 = umidup;
            }
        }

        // Limit trimming to make sure something happens.
        ts1 = max( ts1, umin );
        ts2 = min( ts2, umax );

        // Limit parameters to upper/lower surfaces.
        ts1 = min( umidlow, ts1 );
        ts2 = max( umidup, ts2 );

        threed_point_type p1 = crv.f( ts1 );
        threed_point_type p2 = crv.f( ts2 );

        threed_point_type p3 = ( p1 + p2 ) / 2.0;

        // Actually trim the airfoil curve.
        piecewise_curve_type c1, c2, c3, c4;
        piecewise_curve_type c_up, c_low, c_up_te, c_up_main, c_low_te, c_low_main, c_lecap;

        crv.split( c_low, c_up, umid );

        c_low.split( c_low_te, c_low_main, umin );
        c_up.split( c_up_main, c_up_te, umax );

        c_low_main.split( c1, c2, ts1 );
        c_up_main.split( c3, c4, ts2 );

        c1.scale_t( umin, umidlow );
        c4.scale_t( umidup, umax );

        piecewise_linear_creator_type plc( 2 );

        plc.set_t0( umidlow );
        plc.set_segment_dt( TMAGIC, 0 );
        plc.set_segment_dt( TMAGIC, 1 );
        plc.set_corner( p1, 0 );
        plc.set_corner( p3, 1 );
        plc.set_corner( p2, 2 );


        plc.create( c_lecap );

        crv = c_low_te;
        crv.push_back( c1 );
        crv.push_back( c_lecap );
        crv.push_back( c4 );
        crv.push_back( c_up_te );

        // Should be redundant, but just to be sure floating point error didn't accumulate.
        crv.set_tmax( tmax );

        m_Curve.SetCurve( crv );
    }

    vec3d cornerlow = m_Curve.CompPnt( umidlow );
    vec3d cornerup = m_Curve.CompPnt( umidup );

    ttrim = dist( cornerlow, cornerup );
    xtrim = dist( cornerlow, lelow );

    double div = GetWidth();
    if ( div == 0.0 )
    {
        div = 1.0;
    }

    if ( m_UseFakeWidth )
    {
        div = GetFakeWidth();
        if ( div == 0.0 )
        {
            div = 1.0;
        }
        xtrim = xtrim * div;
        ttrim = ttrim * div;
    }

    // Set non-driving parameters to match current state.
    if ( m_LETrimType() == TRIM_NONE )
    {
        m_LETrimX.Set( xtrim );
        m_LETrimXChord.Set( xtrim / div );
        m_LETrimThick.Set( ttrim );
        m_LETrimThickChord.Set( ttrim / div );
    }
    else if ( m_LETrimType() == TRIM_THICK )
    {
        m_LETrimX.Set( xtrim );
        m_LETrimXChord.Set( xtrim / div );

        if ( m_LETrimAbsRel() == ABS )
        {
            m_LETrimThickChord.Set( ttrim / div );
        }
        else
        {
            m_LETrimThick.Set( ttrim );
        }
    }
    else if ( m_LETrimType() == TRIM_X )
    {
        if ( m_LETrimAbsRel() == ABS )
        {
            m_LETrimXChord.Set( xtrim / div );
        }
        else
        {
            m_LETrimX.Set( xtrim );
        }
        m_LETrimThick.Set( ttrim );
        m_LETrimThickChord.Set( ttrim / div );
    }
}

void XSecCurve::CapTE( bool wingtype )
{
    if ( m_Type ==  XS_POINT )
    {
        return;
    }

    if ( !wingtype )
    {
        return;
    }

    m_Curve.Modify( m_TECapType(), false, m_TECapLength(), m_TECapOffset(), m_TECapStrength() );

    switch( m_TECapType() ){
        case FLAT_END_CAP:
            m_TECapLength = 1.0;
            m_TECapOffset = 0.0;
            m_TECapStrength = 0.5;
            break;
        case ROUND_END_CAP:
            m_TECapStrength = 1.0;
            break;
        case EDGE_END_CAP:
            m_TECapStrength = 0.0;
            break;
        case SHARP_END_CAP:
            break;
    }
}

void XSecCurve::CapLE( bool wingtype )
{
    if ( m_Type ==  XS_POINT )
    {
        return;
    }

    if ( !wingtype )
    {
        return;
    }

    m_Curve.Modify( m_LECapType(), true, m_LECapLength(), m_LECapOffset(), m_LECapStrength() );

    switch( m_LECapType() ){
        case FLAT_END_CAP:
            m_LECapLength = 1.0;
            m_LECapOffset = 0.0;
            m_LECapStrength = 0.5;
            break;
        case ROUND_END_CAP:
            m_LECapStrength = 1.0;
            break;
        case EDGE_END_CAP:
            m_LECapStrength = 0.0;
            break;
        case SHARP_END_CAP:
            break;
    }
}

void XSecCurve::RotTransScale()
{

    double ttheta = m_Theta() * PI / 180.0;
    double ct(std::cos(ttheta)), st(std::sin(ttheta));
    double tscale = m_Scale();

    Eigen::Matrix<double, 3, 3> rot_mat;
    rot_mat << ct*tscale, -st*tscale, 0,
               st*tscale,  ct*tscale, 0,
               0,                0,   tscale;

    curve_point_type pttrans;
    pttrans << m_DeltaX() * GetWidth(), m_DeltaY() * GetWidth(), 0.0;


    piecewise_curve_type pwc;

    pwc = m_Curve.GetCurve();

    pwc.rotate( rot_mat );
    pwc.translate( pttrans );

    piecewise_curve_type before, after;

    pwc.split( before, after, 2.0 + m_ShiftLE() );

    before.scale_t( 0, 2 );
    after.scale_t( 2, 4 );

    before.push_back( after );

    m_Curve.SetCurve( before );
}

void XSecCurve::ReadV2FileFuse2( xmlNodePtr &root )
{
    double height = GetHeight();
    double width = GetWidth();

    height = XmlUtil::FindDouble( root, "Height", height );
    width = XmlUtil::FindDouble( root, "Width",  width );

    SetWidthHeight( width, height );
}

// *this was initialized as a copy of *start.
// All IntParms (Type and AbsRel) flags will be set from that copy.
// All Parms will be set based on linear interpolation of values.  Independent of flag
// settings and linearity or nonlinearity of chord changes between sections.  Reconcillation
// of the values (absolute vs. relative) will occur when the modifications are applied (i.e.
// in CloseXX(), TrimXX(), CapXX(), etc.).
void XSecCurve::Interp( XSecCurve *start, XSecCurve *end, double frac )
{
    INTERP_PARM( start, end, frac, m_TECloseThick );
    INTERP_PARM( start, end, frac, m_TECloseThickChord );

    INTERP_PARM( start, end, frac, m_LECloseThick );
    INTERP_PARM( start, end, frac, m_LECloseThickChord );

    INTERP_PARM( start, end, frac, m_TETrimX );
    INTERP_PARM( start, end, frac, m_TETrimXChord );
    INTERP_PARM( start, end, frac, m_TETrimThick );
    INTERP_PARM( start, end, frac, m_TETrimThickChord );

    INTERP_PARM( start, end, frac, m_LETrimX );
    INTERP_PARM( start, end, frac, m_LETrimXChord );
    INTERP_PARM( start, end, frac, m_LETrimThick );
    INTERP_PARM( start, end, frac, m_LETrimThickChord );

    INTERP_PARM( start, end, frac, m_TECapLength );
    INTERP_PARM( start, end, frac, m_TECapOffset );
    INTERP_PARM( start, end, frac, m_TECapStrength );

    INTERP_PARM( start, end, frac, m_LECapLength );
    INTERP_PARM( start, end, frac, m_LECapOffset );
    INTERP_PARM( start, end, frac, m_LECapStrength );

    INTERP_PARM( start, end, frac, m_Theta );
    INTERP_PARM( start, end, frac, m_Scale );
    INTERP_PARM( start, end, frac, m_DeltaX );
    INTERP_PARM( start, end, frac, m_DeltaY );
    INTERP_PARM( start, end, frac, m_ShiftLE );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
PointXSec::PointXSec( ) : XSecCurve( )
{
    m_Type = XS_POINT;
}

//==== Update Geometry ====//
void PointXSec::Update()
{
    piecewise_curve_type c;
    curve_point_type pt;
    pt << 0, 0, 0;

    // create point with 4 segments
    piecewise_point_creator ppc( 4 );

    // set point, make sure have 4 sections that go from 0 to 4
    ppc.set_point( pt );
    ppc.set_t0( 0 );
    ppc.set_segment_dt( 1, 0 );
    ppc.set_segment_dt( 1, 1 );
    ppc.set_segment_dt( 1, 2 );
    ppc.set_segment_dt( 1, 3 );
    if ( !ppc.create( c ) )
    {
        std::cerr << "Failed to create point XSec. " << __LINE__ << std::endl;
    }
    else
    {
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}
//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
CircleXSec::CircleXSec( ) : XSecCurve( )
{
    m_Type = XS_CIRCLE;
    m_Diameter.Init( "Circle_Diameter", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of Circle Cross-Section" );
}

//==== Set Width and Height ====//
void CircleXSec::SetWidthHeight( double w, double h )
{
    m_Diameter  = ( w + h ) / 2.0;
}

void CircleXSec::OffsetCurve( double off )
{
    m_Diameter = m_Diameter() - 2.0*off;
}

//==== Update Geometry ====//
void CircleXSec::Update()
{
    piecewise_curve_type c;
    piecewise_circle_creator pcc( 4 );
    curve_point_type origin, start, normal;

    origin << m_Diameter() / 2, 0, 0;
    normal << 0, 0, -1;

    // set circle params, make sure that entire curve goes from 0 to 4
    pcc.set_origin( origin );
    pcc.set_radius( m_Diameter() / 2 );
    pcc.set_t0( 0 );
    pcc.set_segment_dt( 1, 0 );
    pcc.set_segment_dt( 1, 1 );
    pcc.set_segment_dt( 1, 2 );
    pcc.set_segment_dt( 1, 3 );
    if ( !pcc.create( c ) )
    {
        std::cerr << "Failed to create circle XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
EllipseXSec::EllipseXSec( ) : XSecCurve( )
{
    m_Type = XS_ELLIPSE;

    m_Height.Init( "Ellipse_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Ellipse Cross-Section" );
    m_Width.Init( "Ellipse_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Ellipse Cross-Section" );
}

//==== Update Geometry ====//
void EllipseXSec::Update()
{
    piecewise_curve_type c;
    piecewise_ellipse_creator pec( 4 );
    curve_point_type origin, normal;

    origin << m_Width() / 2, 0, 0;
    normal << 0, 0, -1;

    // set circle params, make sure that entire curve goes from 0 to 4
    pec.set_origin( origin );
    pec.set_x_axis_radius( m_Width() / 2 );
    pec.set_y_axis_radius( m_Height() / 2 );
    pec.set_t0( 0 );
    pec.set_segment_dt( 1, 0 );
    pec.set_segment_dt( 1, 1 );
    pec.set_segment_dt( 1, 2 );
    pec.set_segment_dt( 1, 3 );
    if ( !pec.create( c ) )
    {
        std::cerr << "Failed to create ellipse XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}

//==== Set Width and Height ====//
void EllipseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
SuperXSec::SuperXSec( ) : XSecCurve( )
{
    m_Type = XS_SUPER_ELLIPSE;

    m_Height.Init( "Super_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Super Ellipse Cross-Section" );
    m_Width.Init( "Super_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Super Ellipse Cross-Section" );
    m_M.Init( "Super_M", m_GroupName, this, 2.0, 0.2, 5.0 );
    m_M.SetDescript( "Width of the Super Ellipse Cross-Section" );
    m_N.Init( "Super_N", m_GroupName, this, 2.0, 0.2, 5.0 );
    m_M_bot.Init( "Super_M_bot", m_GroupName, this, 2.0, 0.25, 5.0 );
    m_M_bot.SetDescript( "Generalized Super Ellipse M Exponent for Bottom Half" );
    m_N_bot.Init( "Super_N_bot", m_GroupName, this, 2.0, 0.25, 5.0 );
    m_N_bot.SetDescript( "Generalized Super Ellipse N Exponent for Bottom Half" );
    m_MaxWidthLoc.Init( "Super_MaxWidthLoc", m_GroupName, this, 0.0, -10, 10 );
    m_MaxWidthLoc.SetDescript( "Maximum Width Location for Super Ellipse" );
    m_TopBotSym.Init( "Super_TopBotSym", m_GroupName, this, true, 0, 1 );
    m_TopBotSym.SetDescript( "Toggle Symmetry for Top and Bottom Curve" );
}

//==== Update Geometry ====//
void SuperXSec::Update()
{
    piecewise_curve_type c;
    piecewise_superellipse_creator psc( 16 );
    curve_point_type origin, x, y;

    origin << m_Width() / 2, 0, 0;

    // check for top bottom symmetry toggle
    if ( m_TopBotSym() )
    {
        m_M_bot.Set( m_M() );
        m_N_bot.Set( m_N() );
    }

    // set hyperellipse params, make sure that entire curve goes from 0 to 4
    psc.set_axis( m_Width() / 2, m_Height() / 2 );
    psc.set_max_degree( 3 );
    psc.set_exponents( m_M(), m_N() );
    psc.set_exponents_bot( m_M_bot(), m_N_bot() );
    psc.set_origin( origin );
    psc.set_max_width_loc( m_MaxWidthLoc() * m_Height() * 0.5 );

    psc.set_t0( 0 );
    for ( int i = 0; i < psc.get_number_segments(); ++i )
    {
        psc.set_segment_dt( 4.0 / psc.get_number_segments(), i );
    }

    if ( !psc.create( c ) )
    {
        std::cerr << "Failed to create superellipse XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.InterpolateEqArcLenPCHIP( c );

        XSecCurve::Update();
    }
}

//==== Set Width and Height ====//
void SuperXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

// Interpolate all parameters of like-type XSecCurves -- except width, height, and cli.
void SuperXSec::Interp( XSecCurve *start, XSecCurve *end, double frac )
{
    SuperXSec *s = dynamic_cast< SuperXSec* > ( start );
    SuperXSec *e = dynamic_cast< SuperXSec* > ( end );

    if ( s && e )
    {
        INTERP_PARM( s, e, frac, m_M );
        INTERP_PARM( s, e, frac, m_N );
        INTERP_PARM( s, e, frac, m_M_bot );
        INTERP_PARM( s, e, frac, m_N_bot );
        INTERP_PARM( s, e, frac, m_MaxWidthLoc );
    }
    XSecCurve::Interp( start, end, frac );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
RoundedRectXSec::RoundedRectXSec( ) : XSecCurve( )
{
    m_Type = XS_ROUNDED_RECTANGLE;

    m_Height.Init( "RoundedRect_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "RoundedRect_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Radius.Init( "RoundRectXSec_Radius", m_GroupName,  this,  0.2, 0.0, 1.0e12 );
    m_Skew.Init("RoundRect_Skew", m_GroupName, this, 0.0, -10, 10);
    m_Keystone.Init("RoundRect_Keystone", m_GroupName, this, 0.5, 0.0, 1.0 );
    m_KeyCornerParm.Init( "RoundRectXSec_KeyCorner", m_GroupName, this, true, 0, 1 );
}

//==== Update Geometry ====//
void RoundedRectXSec::Update()
{
    double r = m_Curve.CreateRoundedRectangle( m_Width(), m_Height(), m_Keystone(), m_Skew(), m_Radius(), m_KeyCornerParm() );
    m_Radius.Set( r );

    XSecCurve::Update();
    return;
}

//==== Build Box of Even Spaced Points ====//
//==== Set Width and Height ====//
void RoundedRectXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

void RoundedRectXSec::SetScale( double scale )
{
    XSecCurve::SetScale( scale );

    m_Radius.Set( m_Radius() * scale );
}

void RoundedRectXSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    XSecCurve::ReadV2FileFuse2( root );

    int v2type = XmlUtil::FindInt( root, "Type", FuselageGeom::V2_FXS_RND_BOX );

    if ( v2type == FuselageGeom::V2_FXS_RND_BOX )
    {
        m_Radius = XmlUtil::FindDouble( root, "Corner_Radius", m_Radius() );
    }
    else
    {
        m_Radius = 0.0;
    }
}

// Interpolate all parameters of like-type XSecCurves -- except width, height, and cli.
void RoundedRectXSec::Interp( XSecCurve *start, XSecCurve *end, double frac )
{
    RoundedRectXSec *s = dynamic_cast< RoundedRectXSec* > ( start );
    RoundedRectXSec *e = dynamic_cast< RoundedRectXSec* > ( end );

    if ( s && e )
    {
        INTERP_PARM( s, e, frac, m_Radius );
        INTERP_PARM( s, e, frac, m_Skew );
        INTERP_PARM( s, e, frac, m_Keystone );
    }
    XSecCurve::Interp( start, end, frac );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
GeneralFuseXSec::GeneralFuseXSec( ) : XSecCurve( )
{
    m_Type = XS_GENERAL_FUSE;

    m_Height.Init( "Height",           m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "Width",             m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_MaxWidthLoc.Init( "MaxWidthLoc", m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
    m_CornerRad.Init( "CornerRad",     m_GroupName, this, 0.0, 0.0, 1.0e12 );
    m_TopTanAngle.Init( "TopTanAngle", m_GroupName, this, 90.0, 0.0, 90.0 );
    m_BotTanAngle.Init( "BotTanAngle", m_GroupName, this, 90.0, 0.0, 90.0 );
    m_TopStr.Init( "TopStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
    m_BotStr.Init( "BotStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
    m_UpStr.Init( "UpStr",             m_GroupName, this, 0.83, 0.0, 2.0 );
    m_LowStr.Init( "LowStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
}

//==== Set Width and Height ====//
void GeneralFuseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void GeneralFuseXSec::Update()
{
    double x, y;
    //==== Top Control Points ====//
    vec3d tp0( 0.0, m_Height() / 2.0, 0.0 );
    vec3d tp1 = tp0 + vec3d( m_TopStr() * m_Width() / 3.0, 0.0, 0.0 );

    x = m_Width() / 2.0;
    y = m_MaxWidthLoc() * m_Height() / 2.0;
    vec3d tp3( x, y, 0.0 );

    x = ( m_UpStr() * m_Height() * cos( DEG_2_RAD * m_TopTanAngle() ) ) / 3.0;
    y = ( -m_UpStr() * m_Height() * sin( DEG_2_RAD * m_TopTanAngle() ) ) / 3.0;
    vec3d tp2 = tp3 - vec3d( x, y, 0.0 );

    //==== Bot Control Points ====//
    x = m_Width() / 2.0;
    y = m_MaxWidthLoc() * m_Height() / 2.0;
    vec3d bp0( x, y, 0.0 );

    x = ( double )( -m_LowStr() * m_Height() * cos( DEG_2_RAD * m_BotTanAngle() ) ) / 3.0;
    y = ( double )( -m_LowStr() * m_Height() * sin( DEG_2_RAD * m_BotTanAngle() ) ) / 3.0;
    vec3d bp1 = bp0 + vec3d( x, y, 0.0 );

    vec3d bp3( 0.0, -m_Height() / 2.0, 0.0 );
    vec3d bp2 = bp3 - vec3d( -m_BotStr() * m_Width() / 3.0, 0.0, 0.0 );

    //==== Load Bezier Control Points ====//
    vector< vec3d > bez_pnts;
    bez_pnts.push_back( tp0 );
    bez_pnts.push_back( tp1 );
    bez_pnts.push_back( tp2 );
    bez_pnts.push_back( tp3 );

    int ite = bez_pnts.size() - 1;

    bez_pnts.push_back( bp1 );
    bez_pnts.push_back( bp2 );
    bez_pnts.push_back( bp3 );

    vec3d offset;
    offset.set_x( m_Width() / 2.0 );

    //==== Reflect ====//
    int nrp = bez_pnts.size();
    for ( int i = 2 ; i <= nrp ; i++ )
    {
        vec3d p = bez_pnts[nrp - i];
        p.set_x( -p.x() );
        bez_pnts.push_back( p );
    }

    vector< vec3d > roll_pnts;
    for( int i = ite; i < (int)bez_pnts.size(); i++ )
    {
        roll_pnts.push_back( bez_pnts[i] + offset );
    }
    for ( int i = 1; i <= ite; i++ )
    {
        roll_pnts.push_back( bez_pnts[i] + offset );
    }

    m_Curve.SetCubicControlPoints( roll_pnts );

    //==== Corner Points ====//
    VspCurve corner_curve;
    if ( m_CornerRad() > 0.001 )
    {
        m_Curve.RoundJoint( m_CornerRad() * m_Height(), 2 );
        m_Curve.RoundJoint( m_CornerRad() * m_Height(), 0 );
    }

    XSecCurve::Update();
}

void GeneralFuseXSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    XSecCurve::ReadV2FileFuse2( root );

    m_MaxWidthLoc = XmlUtil::FindDouble( root, "Max_Width_Location", m_MaxWidthLoc() );
    m_CornerRad = XmlUtil::FindDouble( root, "Corner_Radius", m_CornerRad() );

    m_TopTanAngle = XmlUtil::FindDouble( root, "Top_Tan_Angle", m_TopTanAngle());
    m_BotTanAngle = XmlUtil::FindDouble( root, "Bot_Tan_Angle", m_BotTanAngle());

    m_TopStr = XmlUtil::FindDouble( root, "Top_Tan_Strength", m_TopStr() );
    m_UpStr = XmlUtil::FindDouble( root, "Upper_Tan_Strength", m_UpStr() );
    m_LowStr = XmlUtil::FindDouble( root, "Lower_Tan_Strength", m_LowStr() );
    m_BotStr = XmlUtil::FindDouble( root, "Bottom_Tan_Strength", m_BotStr() );
}

// Interpolate all parameters of like-type XSecCurves -- except width, height, and cli.
void GeneralFuseXSec::Interp( XSecCurve *start, XSecCurve *end, double frac )
{
    GeneralFuseXSec *s = dynamic_cast< GeneralFuseXSec* > ( start );
    GeneralFuseXSec *e = dynamic_cast< GeneralFuseXSec* > ( end );

    if ( s && e )
    {
        INTERP_PARM( s, e, frac, m_MaxWidthLoc );
        INTERP_PARM( s, e, frac, m_CornerRad );
        INTERP_PARM( s, e, frac, m_TopTanAngle );
        INTERP_PARM( s, e, frac, m_BotTanAngle );
        INTERP_PARM( s, e, frac, m_TopStr );
        INTERP_PARM( s, e, frac, m_BotStr );
        INTERP_PARM( s, e, frac, m_UpStr );
        INTERP_PARM( s, e, frac, m_LowStr );
    }
    XSecCurve::Interp( start, end, frac );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FileXSec::FileXSec( ) : XSecCurve( )
{
    m_Type = XS_FILE_FUSE;

    // Initialize to closed circle.
    int n = 21;
    for ( int i = 0; i < n; i++ )
    {
        double theta = -2.0*PI*i/(n-1);
        m_UnityFilePnts.push_back( vec3d( 0.5*cos(theta), 0.5*sin(theta), 0.0 ) );
    }

    m_Height.Init( "Height",           m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "Width",             m_GroupName, this, 1.0, 0.0, 1.0e12 );

}

//==== Set Width and Height ====//
void FileXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void FileXSec::Update()
{
    //==== Scale File Points ====//
    vector< vec3d > scaled_file_pnts;
    vector< double > arclen;

    // Point set is closed with last point repeating first.
    int npts = m_UnityFilePnts.size() - 1;

    int itop = 0, ileft = 0, ibot = 0;

    for ( int i = 0 ; i < npts ; i++ )
    {
        double x = m_UnityFilePnts[i].x() * m_Width();
        double y = m_UnityFilePnts[i].y() * m_Height();
        scaled_file_pnts.push_back( vec3d( x + m_Width() / 2.0, y, 0.0 ) );

        if ( m_UnityFilePnts[i].y() < 0 && std::abs( m_UnityFilePnts[i].x() ) < std::abs(m_UnityFilePnts[ibot].x() ) )
        {
            ibot = i;
        }

        if ( m_UnityFilePnts[i].y() > 0 && std::abs( m_UnityFilePnts[i].x() ) < std::abs(m_UnityFilePnts[itop].x() ) )
        {
            itop = i;
        }

        if ( i > 0 )
        {
            double ds = dist( scaled_file_pnts[i], scaled_file_pnts[i-1] );
            if ( ds < 1e-8 )
            {
                ds = 1.0/npts;
            }
            arclen.push_back( arclen[i-1] + ds );
        }
        else
        {
            arclen.push_back( 0 );
        }

        // Calculate arclen to repeated final point.
        if ( i == npts - 1 )
        {
            x = m_UnityFilePnts[i+1].x() * m_Width();
            y = m_UnityFilePnts[i+1].y() * m_Height();
            double ds = dist( scaled_file_pnts[i], vec3d( x + m_Width() / 2.0, y, 0.0 )  );
            if ( ds < 1e-8 )
            {
                ds = 1.0/npts;
            }
            arclen.push_back( arclen[i] + ds );
        }
    }

    ileft = ibot * 2; // Make ileft symmetrical.

    double arcend1 = arclen[ibot];
    double arcend2 = arclen[ileft];
    double arcend3 = arclen[itop];

    double len1 = 1.0 / arcend1;
    double len2 = 1.0 / ( arcend2 - arcend1 );
    double len3 = 1.0 / ( arcend3 - arcend2 );
    double len4 = 1.0 / ( arclen.back() - arcend3 );

    int i = 0;
    for ( ; i <= ibot; i++ )
    {
        arclen[i] = arclen[i] * len1;
    }
    for ( ; i <= ileft; i++ )
    {
        arclen[i] = 1.0 + (arclen[i] - arcend1) * len2;
    }
    for ( ; i <= itop; i++ )
    {
        arclen[i] = 2.0 + (arclen[i] - arcend2) * len3;
    }
    for ( ; i < (int) arclen.size(); i++ )
    {
        arclen[i] = 3.0 + (arclen[i] - arcend3) * len4;
    }

    m_Curve.InterpolatePCHIP( scaled_file_pnts, arclen, true );

    XSecCurve::Update();
}

//==== Encode XML ====//
xmlNodePtr FileXSec::EncodeXml(  xmlNodePtr & node  )
{
    XSecCurve::EncodeXml( node );
    xmlNodePtr child_node  = xmlNewChild( node, NULL, BAD_CAST "FileXSec", NULL );
    if ( child_node )
    {
        XmlUtil::AddVectorVec3dNode( child_node, "UnityFilePnts", m_UnityFilePnts );
    }
    return child_node;
}

//==== Decode XML ====//
xmlNodePtr FileXSec::DecodeXml(  xmlNodePtr & node  )
{
    XSecCurve::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "FileXSec", 0 );
    if ( child_node )
    {
        m_UnityFilePnts = XmlUtil::ExtractVectorVec3dNode( child_node, "UnityFilePnts" );
    }
    return child_node;
}

//==== Read Fuse XSec File ====//
bool FileXSec::ReadXsecFile( string file_name )
{
    FILE* file_id =  fopen( file_name.c_str(), "r" );
    if ( file_id == ( FILE * )NULL )
    {
        return false;
    }

    m_FileName = file_name;

//    if ( ReadOldXSecFile( file_id ) == true )
//    {
//        fclose( file_id );
//        return true;
//    }
//    else if ( ReadXSecFile( file_id ) == true )
    if ( ReadXSecFile( file_id ) )
    {
        fclose( file_id );
        return true;
    }

    fclose( file_id );
    return false;
}

/*
//==== Read Old Fuse XSec File ====//
bool FileXSec::ReadOldXSecFile( FILE* file_id )
{
    char buff[256];
    char name[256];
    string line;
    int num_pnts;

    rewind( file_id );
    fgets( buff, 255, file_id );
    line.assign( buff );

    //==== Check For Valid File Type ====//
    size_t found = line.find( "FUSE XSEC FILE" );
    if ( found == string::npos )
    {
        return false;
    }

    //==== Find XSec Name ====//
    fgets( buff, 80, file_id );
    sscanf( buff, "%s", name );
    m_FileName = string( name );

    //==== Find Num Pnts ====//
    fgets( buff, 80, file_id );
    sscanf( buff, "%d", &num_pnts );
    if ( num_pnts < 3 )
    {
        return false;
    }

    //==== Find Points ====//
    float x, y;
    vector< vec3d > pnt_vec;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        if ( fgets( buff, 80, file_id ) )
        {
            sscanf( buff, "%f  %f", &x, &y );
            pnt_vec.push_back( vec3d( x, y, 0.0 ) );
        }
    }

    //==== Find Height & Width ====//
    vec3d p0 = pnt_vec[0];
    vec3d pn = pnt_vec[pnt_vec.size() - 1];
    m_Height.Set( max( std::abs( p0.y() - pn.y() ), 1.0e-12 ) );

    double max_x = 0;
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double x = pnt_vec[i].x();
        if  ( std::abs( y ) > max_x )
        {
            max_x = std::abs( x );
        }
    }
    m_Width.Set( max( 2.0 * max_x, 1.0e-12 ) );

    //==== Scale Point By Height & Width ====//
    m_UnityFilePnts.clear();
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double x = pnt_vec[i].x() / m_Width();
        double y = pnt_vec[i].y() / m_Height();
        m_UnityFilePnts.push_back( vec3d( x, y, 0.0 ) );
    }
    //==== Reflected Pnts ====//
    for ( int i = ( int )pnt_vec.size() - 2 ; i >= 0 ; i-- )
    {
        double x = pnt_vec[i].x() / m_Width();
        double y = pnt_vec[i].y() / m_Height();
        m_UnityFilePnts.push_back( vec3d( -x, y, 0.0 ) );
    }
    return true;
}
*/


//==== Read OpenVSP Fuse XSec File ====//
bool FileXSec::ReadXSecFile( FILE* file_id )
{
    char buff[256];
    string line;

    rewind( file_id );
    fgets( buff, 255, file_id );
    line.assign( buff );

    //==== Check For Valid File Type ====//
    size_t found = line.find( "OPENVSP_XSEC_FILE_V1" );
    if ( found == string::npos )
    {
        return false;
    }

    //==== Read Points ====//
    float x, y;
    vector< vec3d > pnt_vec;
    bool more_data = true;
    while( more_data )
    {
        more_data = false;
        if ( fgets( buff, 255, file_id ) )
        {
            sscanf( buff, "%f %f", &x, &y );
            if ( std::abs( x ) < 1.0e12 && std::abs( y ) <  1.0e12 )
            {
                pnt_vec.push_back( vec3d( x, y, 0.0 ) );
                more_data = true;
            }
        }
    }

    SetPnts( pnt_vec );

    //int num_pnts = (int)pnt_vec.size();

    ////==== Find Height & Width ====//
    //m_Width  = 1.0e-12;
    //m_Height = 1.0e-12;
    //for ( int i = 0 ; i < num_pnts ; i++ )
    //{
    //  for ( int j = 0 ; j < num_pnts ; j++ )
    //  {
    //      double w = std::abs( pnt_vec[i].y() - pnt_vec[j].y() );
    //      if ( w > m_Width() )    m_Width = w;
    //      double h = std::abs( pnt_vec[i].z() - pnt_vec[j].z() );
    //      if ( h > m_Height() )   m_Height = h;
    //  }
    //}

    ////==== Scale Point By Height & Width ====//
    //m_UnityFilePnts.clear();
    //for ( int i = 0 ; i < (int)pnt_vec.size() ; i++ )
    //{
    //  double y = pnt_vec[i].y()/m_Width();
    //  double z = pnt_vec[i].z()/m_Height();
    //  m_UnityFilePnts.push_back( vec3d( 0.0, y,  z ) );
    //}

    return true;
}

//==== Set Pnt Vec ====//
void FileXSec::SetPnts( vector< vec3d > & pnt_vec )
{
    // Check for repeated first/last point and close curve.
    double gap = dist( pnt_vec[0], pnt_vec.back() );
    if ( gap > 1e-8 )
    {
        pnt_vec.push_back( pnt_vec[0] );
    }

    int num_pnts = ( int )pnt_vec.size();

    //==== Find Height & Width ====//
    m_Width  = 1.0e-12;
    m_Height = 1.0e-12;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            double w = std::abs( pnt_vec[i].x() - pnt_vec[j].x() );
            if ( w > m_Width() )
            {
                m_Width = w;
            }
            double h = std::abs( pnt_vec[i].y() - pnt_vec[j].y() );
            if ( h > m_Height() )
            {
                m_Height = h;
            }
        }
    }

    //==== Scale Point By Height & Width ====//
    m_UnityFilePnts.clear();
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double x = pnt_vec[i].x() / m_Width();
        double y = pnt_vec[i].y() / m_Height();
        m_UnityFilePnts.push_back( vec3d( x, y, 0.0 ) );
    }
}

void FileXSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    XSecCurve::ReadV2FileFuse2( root );

    m_FileName = XmlUtil::FindString( root, "File_Name", m_FileName );

    xmlNodePtr yn = XmlUtil::GetNode( root, "File_Y_Pnts", 0 );
    xmlNodePtr zn = XmlUtil::GetNode( root, "File_Z_Pnts", 0 );

    if ( yn && zn )
    {
        int numy = XmlUtil::GetNumArray( yn, ',' );
        int numz = XmlUtil::GetNumArray( zn, ',' );
        if ( numy == numz )
        {
            int num_pnts = numy;
            double* arry = (double*)malloc( num_pnts*sizeof(double) );
            double* arrz = (double*)malloc( num_pnts*sizeof(double) );
            XmlUtil::ExtractDoubleArray( yn, ',', arry, num_pnts );
            XmlUtil::ExtractDoubleArray( zn, ',', arrz, num_pnts );

            int izzero = 0;

            vector < vec3d > pnt_vec;
            for ( int i = 0; i < num_pnts; i++ )
            {
                pnt_vec.push_back( vec3d( arry[i], arrz[i], 0.0 ) );
                if ( std::abs( arrz[i] ) < std::abs( arrz[izzero] ) )
                {
                    izzero = i;
                }
            }
            // Mirror points, do not repeat any points.
            for ( int i = num_pnts - 2; i > 0; i-- )
            {
                pnt_vec.push_back( vec3d( -arry[i], arrz[i], 0.0 ) );
            }

            free( arry );
            free( arrz );

            // Rotate to v3 start/end point.
            rotate( pnt_vec.begin(), pnt_vec.begin() + izzero, pnt_vec.end() );

            // Add duplicate start/end point.
            pnt_vec.push_back( pnt_vec[0] );

            SetPnts( pnt_vec );
        }
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

EditCurveXSec::EditCurveXSec() : XSecCurve()
{
    m_Type = XS_EDIT_CURVE;

    m_CloseFlag.Init( "CloseFlag", m_GroupName, this, true, false, true );
    m_CloseFlag.SetDescript( "Closed Curve Flag" );

    m_SymType.Init( "SymType", m_GroupName, this, SYM_RL, SYM_NONE, SYM_RL );
    m_SymType.SetDescript( "2D Symmetry Type" );

    m_ShapeType.Init( "ShapeType", m_GroupName, this, EDIT_XSEC_CIRCLE, EDIT_XSEC_CIRCLE, EDIT_XSEC_RECTANGLE );
    m_ShapeType.SetDescript( "Initial Shape Type" );

    m_Width.Init( "Width", m_GroupName, this, 1.0, 1.0e-12, 1.0e12 );
    m_Width.SetDescript( "Edit Curve XSec Width" );

    m_Height.Init( "Height", m_GroupName, this, 1.0, 1.0e-12, 1.0e12 );
    m_Height.SetDescript( "Edit Curve XSec Height" );

    m_CurveType.Init( "CurveType", m_GroupName, this, vsp::CEDIT, vsp::LINEAR, vsp::CEDIT );
    m_CurveType.SetDescript( "Curve Type" );

    m_ConvType.Init( "ConvType", m_GroupName, this, vsp::CEDIT, vsp::LINEAR, vsp::APPROX_CEDIT );
    m_ConvType.SetDescript( "Curve Conversion Type" );

    m_SplitU.Init( "SplitU", m_GroupName, this, 0.5, 0.0, 1.0 );
    m_SplitU.SetDescript( "Curve Split u Location (0.0 <= u <= 1.0)" );

    m_AbsoluteFlag.Init( "AbsoluteFlag", m_GroupName, this, false, false, true );
    m_AbsoluteFlag.SetDescript( "Flag indicating if control points are non-dimensional or absolute" );

    m_PreserveARFlag.Init( "PreserveARFlag", m_GroupName, this, false, false, true );
    m_PreserveARFlag.SetDescript( "Flag to preserve width to height aspect ratio" );

    m_SelectPntID = 0;
    m_EnforceG1Next = true;

    m_AspectRatio = 1.0;
}

void EditCurveXSec::ParmChanged( Parm* parm_ptr, int type )
{
    for ( size_t i = 0; i < m_EnforceG1Vec.size(); i++ )
    {
        Parm* g1_parm = dynamic_cast<Parm*> ( m_EnforceG1Vec[i] );

        if ( parm_ptr == g1_parm )
        {
            EnforceG1( (int)i );
            break;
        }
    }

    if ( parm_ptr == dynamic_cast<Parm*> ( &m_PreserveARFlag ) )
    {
        m_AspectRatio = GetWidth() / GetHeight();
    }

    if ( m_CurveType() == vsp::CEDIT )
    {
        for ( size_t i = 0; i < m_UParmVec.size(); i++ )
        {
            Parm* x_parm = dynamic_cast<Parm*> ( m_XParmVec[i] );
            Parm* y_parm = dynamic_cast<Parm*> ( m_YParmVec[i] );

            if ( parm_ptr == x_parm || parm_ptr == y_parm )
            {
                // Identify if next or previous control point is master G1 enforcer
                if ( i % 3 == 1 )
                {
                    m_EnforceG1Next = true;
                }
                else if ( i % 3 == 2 )
                {
                    m_EnforceG1Next = false;
                }
                else if ( i % 3 == 0 && m_EnforceG1Vec[i]->Get() && type == Parm::SET_FROM_DEVICE )
                {                    
                    // Move the neighboring control points if G1 is active and GUI slider is adjusted (not used for a click-and-drag event)
                    if ( parm_ptr == x_parm && !( m_SymType() == SYM_RL && ( m_UParmVec[i]->Get() == 0.25 || m_UParmVec[i]->Get() == 0.75 ) ) )
                    {
                        int prev_index = GetSelectedPntID();
                        SetSelectPntID( i );

                        // Adjust the X coordinates of the neighboring control points only. Parm for point on the curve already updated
                        double dx = x_parm->Get() - x_parm->GetLastVal();

                        MovePnt( x_parm->Get() + dx, y_parm->Get(), true );

                        SetSelectPntID( prev_index );
                    }
                    else if ( parm_ptr == y_parm )
                    {
                        int prev_index = GetSelectedPntID();
                        SetSelectPntID( i );

                        // Adjust the Y coordinates of the neighboring control points. Parm for point on the curve already updated
                        double dy = y_parm->Get() - y_parm->GetLastVal();

                        MovePnt( x_parm->Get(), y_parm->Get() + dy, true );

                        SetSelectPntID( prev_index );
                    }
                }

                break;
            }
        }
    }

    if ( type != Parm::SET )
    {
        if ( parm_ptr == &m_Height && m_PreserveARFlag() )
        {
            // Enforce AR preservation if height is set from API
            m_Width.Set( m_Height() * m_AspectRatio );
        }
    }
    else
    {
        m_LateUpdateFlag = true;

        //==== Notify Parent Container (XSecSurf) ====//
        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            pc->ParmChanged( parm_ptr, type );
        }

        return;
    }

    Update();

    //==== Notify Parent Container (XSecSurf) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr EditCurveXSec::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "EditCurveXSec", NULL );
    if ( child_node )
    {
        XmlUtil::AddIntNode( child_node, "NumPts", (int)m_XParmVec.size() );
    }

    XSecCurve::EncodeXml( node );

    return child_node;
}

xmlNodePtr EditCurveXSec::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = XmlUtil::GetNode( node, "EditCurveXSec", 0 );
    if ( child_node )
    {
        int npt = XmlUtil::FindInt( child_node, "NumPts", 0 );

        while ( m_XParmVec.size() < npt )
        {
            AddPt();
        }
    }

    XSecCurve::DecodeXml( node );

    // Must be done after the parms are decoded. Otherwise they are calculated when the 
    // m_PreserveARFlag is decoded, but width has not been (alphabetical decode order)
    m_AspectRatio = GetWidth() / GetHeight();

    return node;
}

void EditCurveXSec::InitShape()
{
    vector < vec3d > ctrl_pnts;
    vector < double > t_vec;

    // Initialize each shape to X:[-0.5, 0.5], Y:[-0.5, 0.5]

    if ( m_ShapeType() == EDIT_XSEC_CIRCLE )
    {
        m_CurveType.Set( vsp::CEDIT );

        vec3d pnt;

        ctrl_pnts.push_back( vec3d( 0.5, 0.0, 0.0 ) );

        pnt = vec3d( 0.5, 0.0, 0.0 ) + vec3d( 0.0, -0.5, 0.0 ) * ( ( 4 * sqrt( 2 ) - 4 ) / 3 );
        ctrl_pnts.push_back( pnt );

        pnt = vec3d( 0.0, -0.5, 0.0 ) + vec3d( 0.5, 0.0, 0.0 ) * ( ( 4 * sqrt( 2 ) - 4 ) / 3 );
        ctrl_pnts.push_back( pnt );

        ctrl_pnts.push_back( vec3d( 0.0, -0.5, 0.0 ) );

        pnt = vec3d( 0.0, -0.5, 0.0 ) + vec3d( -0.5, 0.0, 0.0 ) * ( ( 4 * sqrt( 2 ) - 4 ) / 3 );
        ctrl_pnts.push_back( pnt );

        pnt = vec3d( -0.5, 0.0, 0.0 ) + vec3d( 0.0, -0.5, 0.0 ) * ( ( 4 * sqrt( 2 ) - 4 ) / 3 );
        ctrl_pnts.push_back( pnt );

        ctrl_pnts.push_back( vec3d( -0.5, 0.0, 0.0 ) );

        // Apply X Symmetry
        size_t n_pts = ctrl_pnts.size();
        for ( size_t i = 2; i <= n_pts; i++ )
        {
            vec3d p = ctrl_pnts[n_pts - i];
            p.set_y( -p.y() );
            ctrl_pnts.push_back( p );
        }

        m_Curve.SetCubicControlPoints( ctrl_pnts ); // initialize curve as CEDIT
        m_Curve.GetCubicControlPoints( ctrl_pnts, t_vec );
    }
    else if ( m_ShapeType() == EDIT_XSEC_RECTANGLE ) 
    {
        m_CurveType.Set( vsp::LINEAR );

        ctrl_pnts.resize( 9 );
        ctrl_pnts[0].set_xyz( 0.5, 0.0, 0.0 );
        ctrl_pnts[1].set_xyz( 0.5, -0.5, 0.0 );
        ctrl_pnts[2].set_xyz( 0.0, -0.5, 0.0 );
        ctrl_pnts[3].set_xyz( -0.5, -0.5, 0.0 );
        ctrl_pnts[4].set_xyz( -0.5, 0.0, 0.0 );
        ctrl_pnts[5].set_xyz( -0.5, 0.5, 0.0 );
        ctrl_pnts[6].set_xyz( 0.0, 0.5, 0.0 );
        ctrl_pnts[7].set_xyz( 0.5, 0.5, 0.0 );
        ctrl_pnts[8].set_xyz( 0.5, 0.0, 0.0 );

        t_vec.resize( 9 );
        t_vec[0] = 0;
        t_vec[1] = 0.5;
        t_vec[2] = 1;
        t_vec[3] = 1.5;
        t_vec[4] = 2;
        t_vec[5] = 2.5;
        t_vec[6] = 3;
        t_vec[7] = 3.5;
        t_vec[8] = 4;

    }
    else if ( m_ShapeType() == EDIT_XSEC_ELLIPSE )
    {
        m_CurveType.Set( vsp::PCHIP );

        piecewise_curve_type c;
        piecewise_superellipse_creator psc( 16 );
        curve_point_type origin;

        origin << 0, 0, 0; // Will be shifted by 1/2 width during update

        // set hyperellipse params, make sure that entire curve goes from 0 to 4
        psc.set_axis( 0.5, 1.0 );
        psc.set_max_degree( 3 );
        psc.set_exponents( 2.0, 2.0 );
        psc.set_exponents_bot( 2.0, 2.0 );
        psc.set_origin( origin );
        psc.set_max_width_loc( 0.0 );

        psc.set_t0( 0 );
        for ( int i = 0; i < psc.get_number_segments(); ++i )
        {
            psc.set_segment_dt( 4.0 / psc.get_number_segments(), i );
        }

        if ( !psc.create( c ) )
        {
            std::cerr << "Failed to create superellipse XSec. " << __LINE__ << std::endl;
        }
        else
        {
            c.reverse();
            m_Curve.SetCurve( c );
        }

        vector < BezierSegment > segs = m_Curve.GetBezierSegments();
        double t_max = m_Curve.GetCurve().get_tmax();

        for ( size_t i = 0; i < segs.size(); i++ )
        {
            ctrl_pnts.push_back( segs[i].control_pnt_vec[0] );
            t_vec.push_back( t_max * segs[i].t0 );

            if ( i == segs.size() - 1 )
            {
                ctrl_pnts.push_back( segs[i].control_pnt_vec.back() );
            }
        }

        t_vec.push_back( t_max );
    }

    vector < double > ctrl_pnts_x( ctrl_pnts.size() );
    vector < double > ctrl_pnts_y( ctrl_pnts.size() );
    vector < double > u_vec( t_vec.size() ); // Set control pont parameter values (0-4), but show the user (0-1)

    for ( size_t i = 0; i < ctrl_pnts.size(); i++ )
    {
        ctrl_pnts_x[i] = ctrl_pnts[i].x();
        ctrl_pnts_y[i] = ctrl_pnts[i].y();
        u_vec[i] = t_vec[i] / 4.0;
    }

    m_SelectPntID = 0; // Ensure selected point is not greater than # of control points

    //==== Load Control Points ====//
    SetPntVecs( u_vec, ctrl_pnts_x, ctrl_pnts_y ); // initialize all 
}

void EditCurveXSec::AddPt( double default_u, double default_x, double default_y, bool default_g1 )
{
    Parm* p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = (int)m_UParmVec.size();
        char str[15];
        sprintf( str, "U_%d", i );
        p->Init( string( str ), m_GroupName, this, default_u, 0, 1.0 );
        p->SetDescript( "Curve point parameter (0-1)" );
        m_UParmVec.push_back( p );
    }

    p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = (int)m_XParmVec.size();
        char str[15];
        sprintf( str, "X_%d", i );
        p->Init( string( str ), m_GroupName, this, default_x, -1.0e12, 1.0e12 );
        p->SetDescript( "Control Point 2D X Location" );
        m_XParmVec.push_back( p );
    }

    p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = (int)m_YParmVec.size();
        char str[15];
        sprintf( str, "Y_%d", i );
        p->Init( string( str ), m_GroupName, this, default_y, -1.0e12, 1.0e12 );
        p->SetDescript( "Control Point 2D Y Location" );
        m_YParmVec.push_back( p );
    }

    BoolParm* bp = dynamic_cast< BoolParm* >( ParmMgr.CreateParm( vsp::PARM_BOOL_TYPE ) );
    if ( bp )
    {
        int i = (int)m_EnforceG1Vec.size();
        char str[15];
        sprintf( str, "G1_%d", i );
        bp->Init( string( str ), m_GroupName, this, default_g1, false, true );
        bp->SetDescript( "G1 Enforcement Flag" );
        m_EnforceG1Vec.push_back( bp );
    }
}

//==== Update Geometry ====//
void EditCurveXSec::Update()
{
    if ( m_UParmVec.empty() )
    {
        InitShape(); // Must always have a valid curve
    }

    if ( m_PreserveARFlag() )
    {
        m_Height.Set( m_Width() / m_AspectRatio );
    }
    
    ClearPtOrder();

    EnforcePtOrder();

    EnforceG1();

    // Symmetry enforcement must come after G1 enforcement
    EnforceSymmetry();

    // Enforce Closure
    EnforceClosure();

    vector< vec3d > ctrl_pnts = GetCtrlPntVec( false );
    vector < double > t_vec = GetTVec(); // Set control pont parameter values (0-4), but show the user (0-1)

    switch ( m_CurveType() )
    {
        case vsp::LINEAR:
            m_Curve.InterpolateLinear( ctrl_pnts, t_vec, false );
            break;
        case vsp::PCHIP:
            if ( ctrl_pnts.size() > 3 ) // Note, slighlty different than PCurve
            {
                m_Curve.InterpolatePCHIP( ctrl_pnts, t_vec, false );
            }
            else
            {
                m_Curve.InterpolateLinear( ctrl_pnts, t_vec, false );
            }
            break;
        case vsp::CEDIT:
            m_Curve.SetCubicControlPoints( ctrl_pnts, t_vec );
            break;
    }

    XSecCurve::Update(); // Note, this will add TE and LE Bezier Segments if Wing or BOR type

    m_Curve.OffsetX( 0.5 * m_Width() ); // Shift by 1/2 width (all XSec types are centered at (m_Width/2, 0, 0))

    UpdateG1Parms();

    EnforcePtOrder();

    return;
}

void EditCurveXSec::EnforceClosure()
{
    if ( m_CloseFlag() )
    {
        m_XParmVec[m_XParmVec.size() - 1]->Set( m_XParmVec[0]->Get() );
        m_YParmVec[m_YParmVec.size() - 1]->Set( m_YParmVec[0]->Get() );

        m_XParmVec[m_XParmVec.size() - 1]->Deactivate();
        m_YParmVec[m_YParmVec.size() - 1]->Deactivate();
        m_EnforceG1Vec[0]->Activate();
    }
    else
    {
        m_XParmVec[m_XParmVec.size() - 1]->Activate();
        m_YParmVec[m_YParmVec.size() - 1]->Activate();
        m_EnforceG1Vec[0]->Deactivate();
    }
}

void EditCurveXSec::UpdateG1Parms()
{
    if ( m_CurveType() == vsp::CEDIT )
    {
        for ( size_t i = 1; i < m_EnforceG1Vec.size() - 1; i++ )
        {
            BoolParm* p = m_EnforceG1Vec[i];
            
            if ( p )
            {
                if ( i % 3 != 0 ) // Deactivate G1 for interior control points
                {
                    p->Set( false );
                    p->Deactivate();
                }
                else if ( !( m_SymType() == SYM_RL && m_UParmVec[i]->Get() > 0.25 && m_UParmVec[i]->Get() < 0.75 ) )
                {
                    p->Activate();

                    if ( p->Get() )
                    {
                        if ( m_SymType() == SYM_RL && ( m_UParmVec[i]->Get() == 0.25 || m_UParmVec[i]->Get() == 0.75 ) )
                        {
                            // Force horizontal tanget at axis of symmetry
                            m_YParmVec[i + 1]->Deactivate();
                        }
                        else
                        {
                            // Set next control point as master over previous point
                            m_YParmVec[i + 1]->Activate();
                        }
                    }
                }
                else if ( m_SymType() == SYM_RL && m_UParmVec[i]->Get() > 0.25 && m_UParmVec[i]->Get() < 0.75 )
                {
                    p->Set( false );
                    p->Deactivate();
                }
            }
        }
    }
    else
    {
        for ( int j = 0; j < m_EnforceG1Vec.size() - 1; j++ )
        {
            m_EnforceG1Vec[j]->Set( false );
            m_EnforceG1Vec[j]->Deactivate(); // No G1 enforcement for linear or PCHIP
        }
    }

    if ( !m_CloseFlag() )
    {
        m_EnforceG1Vec[0]->Deactivate();
        m_EnforceG1Vec[0]->Set( false );
    }

    m_EnforceG1Vec[m_EnforceG1Vec.size() - 1]->Set( false );
    m_EnforceG1Vec[m_EnforceG1Vec.size() - 1]->Deactivate();
}

void EditCurveXSec::EnforcePtOrder( double rfirst, double rlast )
{
    double offset = 1e-4;

    if ( m_CurveType() == vsp::CEDIT )
    {
        int npt = (int)m_UParmVec.size();
        int nseg = ( npt - 1 ) / 3;

        Parm* pprev = NULL;
        for ( int i = 0; i < nseg; i++ )
        {
            int istart = i * 3;
            int iend = ( i + 1 ) * 3;

            Parm* pstart = m_UParmVec[istart];
            Parm* pend = m_UParmVec[iend];

            if ( i == 0 && pstart )
            {
                pstart->Set( rfirst );
                pstart->Deactivate();
                pstart->SetLowerLimit( rfirst );
                pstart->SetUpperLimit( rfirst );
            }

            if ( i == nseg - 1 && pend )
            {
                pend->Set( rlast );
                pend->Deactivate();
                pend->SetUpperLimit( rlast );
                pend->SetLowerLimit( rlast );
            }

            if ( pstart && pend )
            {
                pstart->SetUpperLimit( pend->Get() - offset );
                pend->SetLowerLimit( pstart->Get() + offset );

                // Keep intermediate points valid.
                double dt = pend->Get() - pstart->Get();
                m_UParmVec[iend - 1]->Set( pend->Get() - dt / 3.0 );
                m_UParmVec[istart + 1]->Set( pstart->Get() + dt / 3.0 );
            }
        }

        // Deactivate interior control points without setting limits.
        for ( int i = 0; i < npt; i++ )
        {
            int m = i % 3;
            if ( m == 1 || m == 2 )
            {
                Parm* p = m_UParmVec[i];
                if ( p )
                {
                    p->Deactivate();
                }
            }
        }
    }
    else  // LINEAR or PCHIP
    {
        Parm* pprev = NULL;
        for ( int j = 0; j < m_UParmVec.size(); j++ )
        {
            Parm* p = m_UParmVec[j];
            Parm* pnxt = NULL;
            if ( j < m_UParmVec.size() - 1 )
            {
                pnxt = m_UParmVec[j + 1];
            }
            if ( p )
            {
                if ( j == 0 )
                {
                    p->Set( rfirst );
                    p->Deactivate();
                    p->SetLowerLimit( rfirst );
                    p->SetUpperLimit( rfirst );
                }
                else if ( j == m_UParmVec.size() - 1 )
                {
                    p->Set( rlast );
                    p->Deactivate();
                    p->SetUpperLimit( rlast );
                    p->SetLowerLimit( rlast );
                }
                else
                {
                    if ( pprev )
                    {
                        p->SetLowerLimit( pprev->Get() + offset );
                    }
                    if ( pnxt )
                    {
                        p->SetUpperLimit( pnxt->Get() - offset );
                    }
                }
            }
            pprev = p;
        }
    }
}

void EditCurveXSec::ClearPtOrder()
{
    for ( int j = 0; j < m_UParmVec.size(); j++ )
    {
        Parm* p = m_UParmVec[j];

        if ( p )
        {
            p->SetLowerLimit( 0.0 );
            p->SetUpperLimit( 1.0 );
            p->Activate();
        }

        p = m_XParmVec[j];

        if ( p )
        {
            p->SetLowerLimit( -1.0e12 );
            p->SetUpperLimit( 1.0e12 );
            p->Activate();
        }

        p = m_YParmVec[j];

        if ( p )
        {
            p->SetLowerLimit( -1.0e12 );
            p->SetUpperLimit( 1.0e12 );
            p->Activate();
        }
    }
}

bool CompareUFunc( const std::pair<double, vec3d>& a, const std::pair<double, vec3d>& b )
{
    return a.first < b.first;
}

void EditCurveXSec::EnforceSymmetry()
{
    // Note: control points must lie on axis of symmetry or they will be forced there

    if ( m_SymType() == SYM_RL ) // Left/Right Symmetry
    {
        if ( !m_CloseFlag() )
        {
            EnforceClosure();
        }

        m_CloseFlag.Set( true ); // Must be closed to allow symmetry
        m_CloseFlag.Deactivate();

        bool found_top = false, found_bottom = false;
        double bottom_u = 0.25;
        double top_u = 0.75;

        for ( size_t i = 0; i < m_UParmVec.size(); i++ )
        {
            int prev_ind = (int)i - 1;
            int next_ind = (int)i + 1;

            if ( i == 0 )
            {
                prev_ind = (int)m_UParmVec.size() - 2; // Shouldn't need to define this
            }

            double current_u = m_UParmVec[i]->Get();
            // Force top and bottom point on Y-axis
            if ( current_u == bottom_u && ( m_CurveType() != CEDIT || i % 3 == 0 ) )
            {
                // Shift neighboring control points as well
                if ( m_CurveType() == CEDIT )
                {
                    double dx = -m_XParmVec[i]->Get();
                    m_XParmVec[prev_ind]->Set( m_XParmVec[prev_ind]->Get() + dx );
                    m_XParmVec[next_ind]->Set( m_XParmVec[next_ind]->Get() + dx );
                }

                found_bottom = true;
                m_XParmVec[i]->Set( 0.0 );
            }
            else if ( current_u == top_u && ( m_CurveType() != CEDIT || i % 3 == 0 ) )
            {
                // Shift neighboring control points as well
                if ( m_CurveType() == CEDIT )
                {
                    double dx = -m_XParmVec[i]->Get();
                    m_XParmVec[prev_ind]->Set( m_XParmVec[prev_ind]->Get() + dx );
                    m_XParmVec[next_ind]->Set( m_XParmVec[next_ind]->Get() + dx );
                }

                found_top = true;
                m_XParmVec[i]->Set( 0.0 );
            }
        }

        // Sort T vec and reorder X, Y, and G1 simultaneously
        // Weird, but place G1 in Z coordinate of vec3d to ensure everything is later sorted together
        vector < pair < double, vec3d > > sort_vec;

        // Force T of 1.0 and 3.0 on the Y axis
        if ( m_CurveType() != CEDIT )
        {
            if ( !found_top )
            {
                // Identify midpoint to use for Y
                for ( size_t i = 0; i < m_UParmVec.size() - 1; i++ )
                {
                    if ( m_UParmVec[i]->Get() < top_u && m_UParmVec[i + 1]->Get() > top_u )
                    {
                        sort_vec.push_back( make_pair( top_u, vec3d( 0.0, ( m_YParmVec[i]->Get() + m_YParmVec[i + 1]->Get() ) / 2.0, false ) ) );

                        break;
                    }
                }
            }
            if ( !found_bottom )
            {
                // Identify midpoint to use for Y
                for ( size_t i = 0; i < m_UParmVec.size() - 1; i++ )
                {
                    if ( m_UParmVec[i]->Get() < bottom_u && m_UParmVec[i + 1]->Get() > bottom_u )
                    {
                        sort_vec.push_back( make_pair( bottom_u, vec3d( 0.0, ( m_YParmVec[i]->Get() + m_YParmVec[i + 1]->Get() ) / 2.0, false ) ) );
                        
                        break;
                    }
                }
            }
        }
        else
        {
            int nseg = ( (int)m_UParmVec.size() - 1 ) / 3;

            if ( !found_top )
            {
                // Identify midpoint to use for Y
                for ( int i = 0; i < nseg; i++ )
                {
                    int istart = i * 3;
                    int iend = ( i + 1 ) * 3;

                    Parm* pend = m_UParmVec[iend];

                    if ( m_UParmVec[istart]->Get() < top_u && pend->Get() > top_u )
                    {
                        // Keep intermediate points valid.
                        double dt = pend->Get() - top_u;

                        // Shift U of next right side control point 
                        m_UParmVec[iend - 1]->Set( pend->Get() - dt / 3.0 );

                        // Add point on Y axis
                        sort_vec.push_back( make_pair( top_u, vec3d( 0.0, ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0, false ) ) );

                        // Add intermediate control point
                        sort_vec.push_back( make_pair( top_u + dt / 3.0, vec3d( 0.1 * m_Width(), ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0, false ) ) );
                        
                        break;
                    }
                }
            }
            if ( !found_bottom )
            {
                // Identify midpoint to use for Y
                for ( int i = 0; i < nseg; i++ )
                {
                    int istart = i * 3;
                    int iend = ( i + 1 ) * 3;

                    Parm* pstart = m_UParmVec[istart];

                    if ( pstart->Get() < bottom_u && m_UParmVec[iend]->Get() > bottom_u )
                    {
                        // Keep intermediate points valid.
                        double dt = bottom_u - pstart->Get();

                        // Shift U of previous right side control point 
                        m_UParmVec[istart + 1]->Set( pstart->Get() + dt / 3.0 );

                        // Add point on Y axis
                        sort_vec.push_back( make_pair( bottom_u, vec3d( 0.0, ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0, false ) ) );

                        // Add intermediate control point
                        sort_vec.push_back( make_pair( bottom_u - dt / 3.0, vec3d( 0.1 * m_Width(), ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0, false ) ) );

                        break;
                    }
                }
            }
        }

        if ( m_CurveType() == vsp::CEDIT )
        {
            // Include last two points
            sort_vec.push_back( make_pair( m_UParmVec[m_UParmVec.size() - 1]->Get(), vec3d( m_XParmVec[m_UParmVec.size() - 1]->Get(), m_YParmVec[m_UParmVec.size() - 1]->Get(), m_EnforceG1Vec[m_UParmVec.size() - 1]->Get() ) ) );
            sort_vec.push_back( make_pair( m_UParmVec[m_UParmVec.size() - 2]->Get(), vec3d( m_XParmVec[m_UParmVec.size() - 2]->Get(), m_YParmVec[m_UParmVec.size() - 2]->Get(), m_EnforceG1Vec[m_UParmVec.size() - 2]->Get() ) ) );

            // Include first two points
            sort_vec.push_back( make_pair( m_UParmVec[0]->Get(), vec3d( m_XParmVec[0]->Get(), m_YParmVec[0]->Get(), m_EnforceG1Vec[0]->Get() ) ) );
            sort_vec.push_back( make_pair( m_UParmVec[1]->Get(), vec3d( m_XParmVec[1]->Get(), m_YParmVec[1]->Get(), m_EnforceG1Vec[1]->Get() ) ) );

            int nseg = ( (int)m_UParmVec.size() - 1 ) / 3;

            for ( int i = 1; i < nseg; i++ )
            {
                int iseg = i * 3;
                int prev_ind = iseg - 1;
                int next_ind = iseg + 1;

                if ( m_UParmVec[iseg]->Get() == bottom_u )
                {
                    // Only keep right side (left will be reflected later)
                    sort_vec.push_back( make_pair( m_UParmVec[prev_ind]->Get(), vec3d( m_XParmVec[prev_ind]->Get(), m_YParmVec[prev_ind]->Get(), m_EnforceG1Vec[prev_ind]->Get() ) ) );
                    sort_vec.push_back( make_pair( m_UParmVec[iseg]->Get(), vec3d( m_XParmVec[iseg]->Get(), m_YParmVec[iseg]->Get(), m_EnforceG1Vec[iseg]->Get() ) ) );
                }
                else if ( m_UParmVec[iseg]->Get() == top_u )
                {
                    // Only keep right side (left will be reflected later)
                    sort_vec.push_back( make_pair( m_UParmVec[iseg]->Get(), vec3d( m_XParmVec[iseg]->Get(), m_YParmVec[iseg]->Get(), m_EnforceG1Vec[iseg]->Get() ) ) );
                    sort_vec.push_back( make_pair( m_UParmVec[next_ind]->Get(), vec3d( m_XParmVec[next_ind]->Get(), m_YParmVec[next_ind]->Get(), m_EnforceG1Vec[next_ind]->Get() ) ) );
                }
                else if ( ( m_UParmVec[iseg]->Get() <= bottom_u && m_UParmVec[prev_ind]->Get() <= bottom_u && m_UParmVec[next_ind]->Get() <= bottom_u ) ||
                    ( m_UParmVec[iseg]->Get() >= top_u && m_UParmVec[prev_ind]->Get() >= top_u && m_UParmVec[next_ind]->Get() >= top_u ) )
                {
                    // Only inlcude this control point if both neighbors are less than 0.25 or greater than 0.75
                    sort_vec.push_back( make_pair( m_UParmVec[prev_ind]->Get(), vec3d( m_XParmVec[prev_ind]->Get(), m_YParmVec[prev_ind]->Get(), m_EnforceG1Vec[prev_ind]->Get() ) ) );
                    sort_vec.push_back( make_pair( m_UParmVec[iseg]->Get(), vec3d( m_XParmVec[iseg]->Get(), m_YParmVec[iseg]->Get(), m_EnforceG1Vec[iseg]->Get() ) ) );
                    sort_vec.push_back( make_pair( m_UParmVec[next_ind]->Get(), vec3d( m_XParmVec[next_ind]->Get(), m_YParmVec[next_ind]->Get(), m_EnforceG1Vec[next_ind]->Get() ) ) );
                }
            }
        }
        else
        {
            for ( size_t i = 0; i < m_UParmVec.size(); i++ )
            {
                if ( m_UParmVec[i]->Get() <= bottom_u || m_UParmVec[i]->Get() >= top_u )
                {
                    sort_vec.push_back( make_pair( m_UParmVec[i]->Get(), vec3d( m_XParmVec[i]->Get(), m_YParmVec[i]->Get(), m_EnforceG1Vec[i]->Get() ) ) );
                }
            }
        }

        vector < pair < double, vec3d > > left_sort_vec;

        for ( size_t i = 0; i < sort_vec.size(); i++ )
        {
            double right_u = sort_vec[i].first;
            double right_x = sort_vec[i].second.x();
            double right_y = sort_vec[i].second.y();
            bool right_g1 = (bool)sort_vec[i].second.z();

            if ( right_u < bottom_u )
            {
                left_sort_vec.push_back( make_pair( 0.5 - right_u, vec3d( -right_x, right_y, right_g1 ) ) );
            }
            else if ( right_u > top_u && right_u != 1.0 )
            {
                left_sort_vec.push_back( make_pair( 1.5 - right_u, vec3d( -right_x, right_y, right_g1 ) ) );
            }
        }

        // Concatenate left and right
        sort_vec.insert( sort_vec.end(), left_sort_vec.begin(), left_sort_vec.end() );

        // Sort the vector by u value and reorder x, y, and g1 the same way
        sort( sort_vec.begin(), sort_vec.end(), CompareUFunc );

        vector < double > new_u_vec( sort_vec.size() );
        vector < vec3d > new_pnt_vec( sort_vec.size() );
        vector < bool > new_g1_vec( sort_vec.size() );

        for ( size_t i = 0; i < sort_vec.size(); i++ )
        {
            new_u_vec[i] = sort_vec[i].first;
            new_pnt_vec[i].set_x( sort_vec[i].second.x() );
            new_pnt_vec[i].set_y( sort_vec[i].second.y() );
            new_g1_vec[i] = (bool)sort_vec[i].second.z();
        }

        if ( new_u_vec.size() != m_UParmVec.size() )
        {
            SetPntVecs( new_u_vec, new_pnt_vec, new_g1_vec, false );
        }
        else
        {
            for ( size_t i = 0; i < new_u_vec.size(); i++ )
            {
                m_UParmVec[i]->Set( new_u_vec[i] );
                m_XParmVec[i]->Set( new_pnt_vec[i].x() );
                m_YParmVec[i]->Set( new_pnt_vec[i].y() );
                m_EnforceG1Vec[i]->Set( new_g1_vec[i] );
            }
        }

        // Deactivate left side parms and points on the axis of symmetry
        for ( size_t i = 0; i < m_UParmVec.size(); i++ )
        {
            if ( m_UParmVec[i]->Get() > bottom_u && m_UParmVec[i]->Get() < top_u )
            {
                m_UParmVec[i]->Deactivate();
                m_XParmVec[i]->Deactivate();
                m_YParmVec[i]->Deactivate();
                m_EnforceG1Vec[i]->Deactivate();
            }
            else if ( m_UParmVec[i]->Get() == bottom_u || m_UParmVec[i]->Get() == top_u )
            {
                m_UParmVec[i]->Deactivate();
                m_XParmVec[i]->Deactivate();
                // Allow translation along Y
            }
        }
    }
    else
    {
        m_CloseFlag.Activate();
    }
}

void EditCurveXSec::ConvertTo( int newtype )
{
    int prev_type = m_CurveType();
    m_CurveType = newtype;

    switch ( prev_type )
    {
        case vsp::LINEAR:
        {
            switch ( newtype )
            {
                case vsp::LINEAR:
                {
                    return;
                    break;
                }
                case vsp::PCHIP:
                {
                    break;
                }
                case vsp::CEDIT:
                {
                    vector < bool > prev_g1_vec = GetG1Vec();
                    m_BaseEditCurve.ToCubic(); // Promote the curve

                    vector < double > t_vec;
                    vector < vec3d> ctrl_pts;

                    m_BaseEditCurve.GetCubicControlPoints( ctrl_pts, t_vec );

                    vector < bool > new_g1_vec( ctrl_pts.size() );
                    vector < double > u_vec( t_vec.size() );

                    double offset = -m_Width() / 2.0;

                    for ( size_t i = 0; i < ctrl_pts.size(); i++ )
                    {
                        u_vec[i] = t_vec[i] / 4.0; // Store point parameter (0-1) internally

                        // Nondimensionalize by width and height
                        ctrl_pts[i].scale_x( 1.0 / m_Width() );
                        ctrl_pts[i].scale_y( 1.0 / m_Height() );

                        if ( i % 3 == 0 )
                        {
                            new_g1_vec[i] = prev_g1_vec[i / 3];
                        }
                        else
                        {
                            new_g1_vec[i] = false;
                        }
                    }

                    SetPntVecs( u_vec, ctrl_pts, new_g1_vec );
                    break;
                }
            }
        }
        break;
        case vsp::PCHIP:
        {
            switch ( newtype )
            {
                case vsp::LINEAR:
                {
                    break;
                }
                case vsp::PCHIP:
                {
                    return;
                    break;
                }
                case vsp::CEDIT:
                {
                    vector < bool > prev_g1_vec = GetG1Vec();

                    vector < double > t_vec;
                    vector < vec3d> ctrl_pts;

                    m_BaseEditCurve.GetCubicControlPoints( ctrl_pts, t_vec );

                    vector < double > u_vec( t_vec.size() );
                    vector < bool > new_g1_vec( ctrl_pts.size() );

                    double offset = -m_Width() / 2.0;

                    for ( size_t i = 0; i < ctrl_pts.size(); i++ )
                    {
                        u_vec[i] = t_vec[i] / 4.0; // Store point parameter (0-1) internally

                        // Nondimensionalize by width and height
                        ctrl_pts[i].scale_x( 1.0 / m_Width() );
                        ctrl_pts[i].scale_y( 1.0 / m_Height() );

                        if ( i % 3 == 0 )
                        {
                            new_g1_vec[i] = prev_g1_vec[i / 3];
                        }
                        else
                        {
                            new_g1_vec[i] = false;
                        }
                    }

                    SetPntVecs( u_vec, ctrl_pts, new_g1_vec );
                    break;
                }
            }
        }
        break;
        case vsp::CEDIT:
        {
            switch ( newtype )
            {
                case vsp::LINEAR:
                case vsp::PCHIP:
                {
                    vector < double > u_vec = GetUVec();
                    vector < double > x_vec = GetXVec();
                    vector < double > y_vec = GetYVec();
                    vector < bool > g1_vec = GetG1Vec();

                    vector < double > new_u_vec, new_x_vec, new_y_vec;
                    vector < bool > new_g1_vec;

                    int nseg = ( (int)x_vec.size() - 1 ) / 3;

                    for ( int i = 0; i < nseg + 1; i++ )
                    {
                        int ipt = 3 * i;
                        new_u_vec.push_back( u_vec[ipt] );
                        new_x_vec.push_back( x_vec[ipt] );
                        new_y_vec.push_back( y_vec[ipt] );
                        new_g1_vec.push_back( g1_vec[ipt] );
                    }

                    SetPntVecs( new_u_vec, new_x_vec, new_y_vec, new_g1_vec );
                }
                break;
                case vsp::CEDIT:
                {
                    return;
                }
            }
        }
        break;
    }

    RenameParms();

    // Reset selected control point (due to potentially added/removed points)
    m_SelectPntID = 0;

    m_LateUpdateFlag = true;
    ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
}

vector < double > EditCurveXSec::GetTVec()
{
    vector < double > retvec( m_UParmVec.size() );

    for ( size_t i = 0; i < m_UParmVec.size(); ++i )
    {
        Parm* p = m_UParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get() * 4.0; // Convert from U to T 
        }
    }

    return retvec;
}

vector < double > EditCurveXSec::GetUVec()
{
    vector < double > retvec( m_UParmVec.size() );

    for ( size_t i = 0; i < m_UParmVec.size(); ++i )
    {
        Parm* p = m_UParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < double > EditCurveXSec::GetXVec()
{
    vector < double > retvec( m_XParmVec.size() );

    for ( size_t i = 0; i < m_XParmVec.size(); ++i )
    {
        Parm* p = m_XParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < double > EditCurveXSec::GetYVec()
{
    vector < double > retvec( m_YParmVec.size() );

    for ( size_t i = 0; i < m_YParmVec.size(); ++i )
    {
        Parm* p = m_YParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < bool > EditCurveXSec::GetG1Vec()
{
    vector < bool > retvec( m_EnforceG1Vec.size() );

    for ( size_t i = 0; i < m_EnforceG1Vec.size(); ++i )
    {
        BoolParm* p = m_EnforceG1Vec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

void EditCurveXSec::SetWidthHeight( double w, double h )
{
    m_Width = w;
    m_Height = h;
}

void EditCurveXSec::SetSelectPntID( int id )
{
    if ( id < 0 || id > m_XParmVec.size() - 1 )
    {
        return;
    }

    if ( m_CurveType() == vsp::CEDIT )
    {
        if ( id % 3 == 1 )
        {
            m_EnforceG1Next = true;
        }
        else if ( id % 3 == 2 )
        {
            m_EnforceG1Next = false;
        }
    }

    m_SelectPntID = id;
}

void EditCurveXSec::MovePnt( int index, vec3d new_pnt, bool force_update )
{
    SetSelectPntID( index );
    MovePnt( new_pnt.x(), new_pnt.y() );

    if ( force_update )
    {
        ParmChanged( NULL, Parm::SET_FROM_DEVICE );
    }
}

void EditCurveXSec::MovePnt( double x, double y, bool neighbors_only )
{
    if ( m_SelectPntID < 0 || m_SelectPntID >= m_XParmVec.size() )
    {
        return;
    }

    Parm *xp = m_XParmVec[m_SelectPntID];
    Parm *yp = m_YParmVec[m_SelectPntID];

    if ( xp && yp )
    {

        switch ( this->m_CurveType() )
        {
            case vsp::LINEAR:
            case vsp::PCHIP:
                xp->Set( x );
                yp->Set( y );
                break;
            case vsp::CEDIT:
            {
                int m = m_SelectPntID % 3;
                if ( m == 0 )  // Changing an end point
                {
                    if ( !( m_SymType() == SYM_RL && ( m_UParmVec[m_SelectPntID]->Get() == 0.25 || m_UParmVec[m_SelectPntID]->Get() == 0.75 ) ) )
                    {
                        // Adjust the X coordinates of the neighboring control points
                        double dx = x - xp->Get();

                        Parm* xprev = NULL;
                        if ( m_SelectPntID > 0 )
                        {
                            xprev = m_XParmVec[m_SelectPntID - 1];
                            xprev->Set( xprev->Get() + dx );
                        }
                        else if ( m_SelectPntID == 0 && m_CloseFlag() )
                        {
                            xprev = m_XParmVec[m_XParmVec.size() - 2];
                            xprev->Set( xprev->Get() + dx );
                        }
                        Parm* xnext = NULL;
                        if ( m_SelectPntID < m_XParmVec.size() - 1 )
                        {
                            xnext = m_XParmVec[m_SelectPntID + 1];
                            xnext->Set( xnext->Get() + dx );
                        }
                        else if ( ( m_SelectPntID == m_XParmVec.size() - 1 ) && m_CloseFlag() )
                        {
                            xnext = m_XParmVec[1];
                            xnext->Set( xnext->Get() + dx );
                        }

                        if ( !neighbors_only )
                        {
                            xp->Set( x );
                        }
                    }

                    // Adjust the Y coordinates of the neighboring control points
                    double dy = y - yp->Get();

                    Parm *yprev = NULL;
                    if ( m_SelectPntID > 0 )
                    {
                        yprev = m_YParmVec[m_SelectPntID - 1];
                        yprev->Set( yprev->Get() + dy );
                    }
                    else if ( m_SelectPntID == 0 && m_CloseFlag() )
                    {
                        yprev = m_YParmVec[m_YParmVec.size() - 2];
                        yprev->Set( yprev->Get() + dy );
                    }
                    Parm *ynext = NULL;
                    if ( m_SelectPntID < m_YParmVec.size() - 1 )
                    {
                        ynext = m_YParmVec[m_SelectPntID + 1];
                        ynext->Set( ynext->Get() + dy );
                    }
                    else if ( ( m_SelectPntID == m_YParmVec.size() - 1 ) && m_CloseFlag() )
                    {
                        ynext = m_YParmVec[1];
                        ynext->Set( ynext->Get() + dy );
                    }
                }
                else if ( !neighbors_only )
                {
                    xp->Set( x );
                }

                if ( !neighbors_only )
                {
                    yp->Set( y );
                }

                break;
            }
        }
    }

    Update();
}

vector < vec3d > EditCurveXSec::GetCtrlPntVec( bool non_dimensional )
{
    vector < vec3d > return_vec( m_XParmVec.size() );
    for ( size_t i = 0; i < m_XParmVec.size(); i++ )
    {
        if ( non_dimensional )
        {
            return_vec[i] = vec3d( m_XParmVec[i]->Get(), m_YParmVec[i]->Get(), 0.0 );
        }
        else // Scale by width and height
        {
            return_vec[i] = vec3d( m_Width() * m_XParmVec[i]->Get(), m_Height() * m_YParmVec[i]->Get(), 0.0 );
        }
    }

    return return_vec;
}

void EditCurveXSec::SetPntVecs( vector < double > u_vec, vector < vec3d > pnt_vec, vector < bool > g1_vec, bool force_update )
{
    vector < double > x_vec( pnt_vec.size() );
    vector < double > y_vec( pnt_vec.size() );

    for ( size_t i = 0; i < pnt_vec.size(); i++ )
    {
        x_vec[i] = pnt_vec[i].x();
        y_vec[i] = pnt_vec[i].y();
    }

    SetPntVecs( u_vec, x_vec, y_vec, g1_vec, force_update );
}

void EditCurveXSec::SetPntVecs( vector < double > u_vec, vector < double > x_pnt_vec, vector < double > y_pnt_vec, vector < bool > g1_vec, bool force_update )
{
    // TODO: Limit the number of times this function is called, since it will cause parm IDs to be reset, 
    // causing links design variables, etc. to become broken.

    if ( u_vec.size() != x_pnt_vec.size() && u_vec.size() != y_pnt_vec.size() )
    {
        std::cerr << "Failed to Set Control Points: Number of U, X, and Y points must be equal " << __LINE__ << std::endl;
        assert( false );
        return;
    }

    if ( g1_vec.size() != u_vec.size() )
    {
        g1_vec = vector < bool > (u_vec.size(), false );
    }

    //==== Load Control Points ====//
    for ( size_t i = 0; i < m_UParmVec.size(); i++ ) // FIXME: Data lost 
    {
        delete m_UParmVec[i];
        delete m_XParmVec[i];
        delete m_YParmVec[i];
        delete m_EnforceG1Vec[i];
    }

    m_UParmVec.clear();
    m_XParmVec.clear();
    m_YParmVec.clear();
    m_EnforceG1Vec.clear();

    //==== Load Control Points ====//
    for ( size_t i = 0; i < u_vec.size(); i++ )
    {
        AddPt( u_vec[i], x_pnt_vec[i], y_pnt_vec[i], g1_vec[i] );
    }

    RenameParms();
    EnforcePtOrder();

    if ( force_update )
    {
        m_LateUpdateFlag = true;
        ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
    }
}

void EditCurveXSec::DeletePt()
{
    DeletePt( m_SelectPntID );
}

void EditCurveXSec::DeletePt( int indx )
{
    int nseg = ( (int)m_XParmVec.size() - 1 ) / 3;

    // TODO: Delete symmetric copy?

    if ( indx > 0 && indx < ( m_XParmVec.size() - 1 ) )
    {
        // Can only delete control points on the cubic Bezier curve. Must have at least 4 segments
        if ( ( m_CurveType() == vsp::CEDIT ) && ( ( indx % 3 ) == 0 ) && nseg >= 0 )
        {
            delete m_UParmVec[indx - 1];
            delete m_UParmVec[indx];
            delete m_UParmVec[indx + 1];

            m_UParmVec.erase( m_UParmVec.begin() + indx - 1, m_UParmVec.begin() + indx + 2 );

            delete m_XParmVec[indx - 1];
            delete m_XParmVec[indx];
            delete m_XParmVec[indx + 1];

            m_XParmVec.erase( m_XParmVec.begin() + indx - 1, m_XParmVec.begin() + indx + 2 );

            delete m_YParmVec[indx - 1];
            delete m_YParmVec[indx];
            delete m_YParmVec[indx + 1];
            m_YParmVec.erase( m_YParmVec.begin() + indx - 1, m_YParmVec.begin() + indx + 2 );

            delete m_EnforceG1Vec[indx - 1];
            delete m_EnforceG1Vec[indx];
            delete m_EnforceG1Vec[indx + 1];
            m_EnforceG1Vec.erase( m_EnforceG1Vec.begin() + indx - 1, m_EnforceG1Vec.begin() + indx + 2 );
        }
        else if ( m_CurveType() != vsp::CEDIT )
        {
            delete m_UParmVec[indx];
            m_UParmVec.erase( m_UParmVec.begin() + indx );

            delete m_XParmVec[indx];
            m_XParmVec.erase( m_XParmVec.begin() + indx );

            delete m_YParmVec[indx];
            m_YParmVec.erase( m_YParmVec.begin() + indx );

            delete m_EnforceG1Vec[indx];
            m_EnforceG1Vec.erase( m_EnforceG1Vec.begin() + indx );
        }

        RenameParms();

        // Reset selected control point (due to potentially added/removed points)
        m_SelectPntID = 0;

        m_LateUpdateFlag = true;
        ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
    }
}

void EditCurveXSec::RenameParms()
{
    for ( int i = 0; i < m_XParmVec.size(); i++ )
    {
        char str[15];
        sprintf( str, "X_%d", i );
        m_XParmVec[i]->SetName( string( str ) );

        sprintf( str, "Y_%d", i );
        m_YParmVec[i]->SetName( string( str ) );

        sprintf( str, "U_%d", i );
        m_UParmVec[i]->SetName( string( str ) );

        sprintf( str, "G1_%d", i );
        m_EnforceG1Vec[i]->SetName( string( str ) );
    }
}

int EditCurveXSec::Split01()
{
    double u_split = m_SplitU();
    return Split01( u_split );
}

int EditCurveXSec::Split01( double u_split )
{
    // Note, the curve will not be split if a control point already exists at the split location. 
    
    // Identify the index of the new point
    m_SelectPntID = 0;

    vector < double > u_vec = GetUVec(); 

    if ( m_CurveType() == CEDIT )
    {
        int nseg = ( (int)u_vec.size() - 1 ) / 3;

        for ( size_t i = 0; i < nseg; i++ )
        {
            if ( u_split > u_vec[i * 3] && u_split < u_vec[3 * (i + 1)] )
            {
                m_SelectPntID = 3 * ( (int)i + 1 );
                break;
            }
        }
    }
    else
    {
        for ( size_t i = 0; i < u_vec.size() - 1; i++ )
        {
            if ( u_split > u_vec[i] && u_split < u_vec[i + 1] )
            {
                m_SelectPntID = (int)i + 1;
                break;
            }
        }
    }

    if ( m_SymType() == SYM_RL && u_split > 0.75 )
    {
        // Correct for the additional reflected control point
        if ( m_CurveType() == vsp::CEDIT )
        {
            m_SelectPntID += 3;
        }
        else
        {
            m_SelectPntID += 1;
        }
    }

    switch ( m_CurveType() )
    {
        case vsp::LINEAR:
        case vsp::PCHIP:
        {
            vec3d split_pnt = m_BaseEditCurve.CompPnt( 4.0 * u_split );

            // Nondimensionalize by width and height
            split_pnt.scale_x( 1.0 / m_Width() );
            split_pnt.scale_y( 1.0 / m_Height() );

            u_vec = GetUVec();
            vector < double > x_vec = GetXVec();
            vector < double > y_vec = GetYVec();
            vector < bool > g1_vec = GetG1Vec();

            vector < double > new_u_vec, new_x_vec, new_y_vec;
            vector < bool > new_g1_vec;
            new_u_vec.reserve( u_vec.size() + 1 );
            new_x_vec.reserve( x_vec.size() + 1 );
            new_y_vec.reserve( y_vec.size() + 1 );
            new_g1_vec.reserve( g1_vec.size() + 1 );

            if ( u_split < u_vec[0] )
            {
                new_u_vec.push_back( u_split );
                new_x_vec.push_back( split_pnt.x() );
                new_y_vec.push_back( split_pnt.y() );
                new_g1_vec.push_back( false );
            }

            new_u_vec.push_back( u_vec[0] );
            new_x_vec.push_back( x_vec[0] );
            new_y_vec.push_back( y_vec[0] );
            new_g1_vec.push_back( g1_vec[0] );

            for ( size_t i = 1; i < u_vec.size(); i++ )
            {
                if ( u_split >= u_vec[i - 1] && u_split < u_vec[i] )
                {
                    new_u_vec.push_back( u_split );
                    new_x_vec.push_back( split_pnt.x() );
                    new_y_vec.push_back( split_pnt.y() );
                    new_g1_vec.push_back( false );
                }
                new_u_vec.push_back( u_vec[i] );
                new_x_vec.push_back( x_vec[i] );
                new_y_vec.push_back( y_vec[i] );
                new_g1_vec.push_back( g1_vec[i] );
            }

            if ( u_split >= u_vec.back() )
            {
                new_u_vec.push_back( u_split );
                new_x_vec.push_back( split_pnt.x() );
                new_x_vec.push_back( split_pnt.y() );
                new_g1_vec.push_back( false );
            }

            SetPntVecs( new_u_vec, new_x_vec, new_y_vec, new_g1_vec );
        }
        break;
        case vsp::CEDIT:
        {
            vector < bool > prev_g1_vec = GetG1Vec();
            m_BaseEditCurve.Split( 4.0 * u_split );

            u_vec.clear();
            vector < double > t_vec;
            vector < vec3d > ctrl_pnts;

            m_BaseEditCurve.GetCubicControlPoints( ctrl_pnts, t_vec );

            u_vec.resize( t_vec.size() );
            vector < bool > new_g1_vec( ctrl_pnts.size() );

            for ( size_t i = 0; i < ctrl_pnts.size(); i++ )
            {
                u_vec[i] = t_vec[i] / 4.0;

                // Nondimensionalize by width and height
                ctrl_pnts[i].scale_x( 1.0 / m_Width() );
                ctrl_pnts[i].scale_y( 1.0 / m_Height() );

                if ( ( i >= m_SelectPntID - 1 ) && ( i <= m_SelectPntID + 1 ) )
                {
                    new_g1_vec[i] = false;
                }
                else if ( i < m_SelectPntID - 1 )
                {
                    new_g1_vec[i] = prev_g1_vec[i];
                }
                else
                {
                    new_g1_vec[i] = prev_g1_vec[i - 3];
                }
            }

            SetPntVecs( u_vec, ctrl_pnts, new_g1_vec );
        }
        break;
    }

    RenameParms();

    return m_SelectPntID;
}

void EditCurveXSec::EnforceG1( int new_index )
{
    if ( m_CurveType() != vsp::CEDIT )
    {
        return;
    }

    for ( size_t i = 0; i < m_EnforceG1Vec.size() - 1; i++ )
    {
        if ( m_EnforceG1Vec[i]->Get() )
        {
            int prev_ind = (int)i - 1;
            int next_ind = (int)i + 1;

            if ( i == 0 )
            {
                prev_ind = (int)m_EnforceG1Vec.size() - 2;
            }

            vec3d prev_pnt = vec3d( m_XParmVec[prev_ind]->Get(), m_YParmVec[prev_ind]->Get(), 0.0 );
            vec3d curr_pnt = vec3d( m_XParmVec[i]->Get(), m_YParmVec[i]->Get(), 0.0 );
            vec3d next_pnt = vec3d( m_XParmVec[next_ind]->Get(), m_YParmVec[next_ind]->Get(), 0.0 );

            // Identify distances from point on the curve
            double prev_dist = dist( prev_pnt, curr_pnt );
            double next_dist = dist( next_pnt, curr_pnt );

            // Identify slopes
            double prev_m = ( curr_pnt.y() - prev_pnt.y() ) / ( curr_pnt.x() - prev_pnt.x() );
            double next_m = ( curr_pnt.y() - next_pnt.y() ) / ( curr_pnt.x() - next_pnt.x() );

            double prev_theta = atan( prev_m );
            double next_theta = atan( next_m );
            double avg_theta = ( prev_theta + next_theta ) / 2;

            vec3d next_to_center_vec = curr_pnt - next_pnt;
            vec3d prev_to_center_vec = curr_pnt - prev_pnt;
            next_to_center_vec.normalize();
            prev_to_center_vec.normalize();

            if ( abs( curr_pnt.x() - prev_pnt.x() ) < FLT_EPSILON && abs( curr_pnt.x() - next_pnt.x() ) < FLT_EPSILON )
            {
                continue; // Do nothing if both vertical
            }
            else if ( new_index == i )
            {
                // Rotate tangents to match average slope
                double next_rot, prev_rot;
                if ( ( next_theta < 0 && prev_theta > 0 ) || ( prev_theta < 0 && next_theta > 0 ) )
                {
                    next_rot = next_theta - avg_theta;
                    prev_rot = avg_theta - prev_theta;
                }
                else
                {
                    next_rot = avg_theta - next_theta;
                    prev_rot = prev_theta - avg_theta;
                }
                next_to_center_vec.rotate_z( cos( next_rot ), sin( next_rot ) );
                vec3d new_prev_pnt = curr_pnt + prev_dist * next_to_center_vec;

                m_XParmVec[prev_ind]->Set( new_prev_pnt.x() );
                m_YParmVec[prev_ind]->Set( new_prev_pnt.y() );

                prev_to_center_vec.rotate_z( cos( prev_rot ), sin( prev_rot ) );
                vec3d new_next_pnt = curr_pnt + next_dist * prev_to_center_vec;

                m_XParmVec[next_ind]->Set( new_next_pnt.x() );
                m_YParmVec[next_ind]->Set( new_next_pnt.y() );
            }
            else if ( m_SymType() == SYM_RL && ( m_UParmVec[i]->Get() == 0.25 || m_UParmVec[i]->Get() == 0.75 ) )
            {
                // Force horizontal tanget at axis of symmetry
                m_YParmVec[prev_ind]->Set( m_YParmVec[i]->Get() );
                m_YParmVec[next_ind]->Set( m_YParmVec[i]->Get() );
            }
            else if ( ( abs( prev_theta - next_theta ) > FLT_EPSILON ) ) 
            {
                // Identify the "master" vector and extend it to find the new point
                if ( m_EnforceG1Next )
                {
                    vec3d new_prev_pnt = curr_pnt + prev_dist * next_to_center_vec;

                    m_XParmVec[prev_ind]->Set( new_prev_pnt.x() );
                    m_YParmVec[prev_ind]->Set( new_prev_pnt.y() );
                }
                else
                {
                    vec3d new_next_pnt = curr_pnt + next_dist * prev_to_center_vec;

                    m_XParmVec[next_ind]->Set( new_next_pnt.x() );
                    m_YParmVec[next_ind]->Set( new_next_pnt.y() );
                }
            }
        }
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
InterpXSec::InterpXSec( ) : XSecCurve( )
{
    m_Type = XS_UNDEFINED; // Special XSecCurve that can not be constructed in conventional way.

    m_Height.Init( "Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Interp Cross-Section" );
    m_Width.Init( "Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Interp Cross-Section" );
}

//==== Update Geometry ====//
void InterpXSec::Update()
{
    m_Curve.MatchThick( m_Height() / m_Width() );
    m_Curve.Scale( m_Width() );
    XSecCurve::Update();
}

//==== Set Width and Height ====//
void InterpXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

// Interpolate all parameters of like-type XSecCurves -- except width, height, and cli.
void InterpXSec::Interp( XSecCurve *start, XSecCurve *end, double frac )
{
    XSecCurve::Interp( start, end, frac );

    vector< VspCurve > crv_vec;
    crv_vec.resize( 2 );

    start->SetLateUpdateFlag(true);
    crv_vec[0] = start->GetBaseEditCurve();

    double wc = start->GetWidth();
    if ( wc != 0 )
    {
        crv_vec[0].Scale( 1.0 / wc );
    }

    end->SetLateUpdateFlag(true);
    crv_vec[1] = end->GetBaseEditCurve();

    wc = end->GetWidth();
    if ( wc != 0 )
    {
        crv_vec[1].Scale( 1.0 / wc );
    }

    VspSurf srf;
    srf.SkinC0( crv_vec, false );

    srf.GetUConstCurve( m_Curve, frac );

}
