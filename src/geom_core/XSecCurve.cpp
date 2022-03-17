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
#include "FuselageGeom.h"
#include "BORGeom.h"
#include "VspUtil.h"
#include "StlHelper.h"
#include "eli/geom/curve/length.hpp"

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
    m_DriverGroup = new HWXSecCurveDriverGroup();

    XSecCurveDriverGroup *xscdg = dynamic_cast< XSecCurveDriverGroup* > ( m_DriverGroup );
    if ( xscdg )
    {
        xscdg->m_Parent = this;
    }

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
    m_TECapLength.Init( "TE_Cap_Length", "Cap", this, 1, -20, 20 );
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

    m_ChevronType.Init( "Chevron_Type", "Chevron", this, vsp::CHEVRON_NONE, vsp::CHEVRON_NONE, vsp::CHEVRON_NUM_TYPES - 1 );

    m_ChevNumber.Init( "Number", "Chevron", this, 8, 1, 1e3 );

    m_ChevOnDuty.Init( "On_Duty", "Chevron", this, 0, 0, 1 );
    m_ChevOffDuty.Init( "Off_Duty", "Chevron", this, 0, 0, 1 );

    m_ChevDirAngleAllSymFlag.Init( "AllSym", "Chevron", this, 1, 0, 1 );
    m_ChevDirAngleAllSymFlag.SetDescript( "Set all chevron angles equal." );
    m_ChevDirAngleTBSymFlag.Init( "TBSym", "Chevron", this, 1, 0, 1 );
    m_ChevDirAngleTBSymFlag.SetDescript( "Set top/bottom chevron angles equal." );
    m_ChevDirAngleRLSymFlag.Init( "RLSym", "Chevron", this, 1, 0, 1 );
    m_ChevDirAngleRLSymFlag.SetDescript( "Set left/right chevron angles equal." );

    m_ChevronExtentMode.Init( "W01_Mode", "Chevron", this, vsp::CHEVRON_W01_SE, vsp::CHEVRON_W01_SE, vsp::CHEVRON_W01_NUM_MODES - 1 );
    m_ChevW01StartGuide.Init( "W01_Start_Guide", "Chevron", this, vsp::W_FREE, vsp::W_RIGHT_0, vsp::W_FREE );
    m_ChevW01Start.Init( "W01_Start", "Chevron", this, 0, 0, 1 );
    m_ChevW01EndGuide.Init( "W01_End_Guide", "Chevron", this, vsp::W_FREE, vsp::W_RIGHT_0, vsp::W_FREE );
    m_ChevW01End.Init( "W01_End", "Chevron", this, 0.5, 0, 1 );
    m_ChevW01CenterGuide.Init( "W01_Center_Guide", "Chevron", this, vsp::W_FREE, vsp::W_RIGHT_0, vsp::W_FREE );
    m_ChevW01Center.Init( "W01_Center", "Chevron", this, 0.25, 0, 1 );
    m_ChevW01Width.Init( "W01_Width", "Chevron", this, 0.5, 0, 1 );

    m_ChevTopAmplitude.Init( "TopAmplitude", "Chevron", this, 1.0, -1e12, 1e12 );
    m_ChevBottomAmplitude.Init( "BottomAmplitude", "Chevron", this, 1.0, -1e12, 1e12 );
    m_ChevRightAmplitude.Init( "LeftAmplitude", "Chevron", this, 1.0, -1e12, 1e12 );
    m_ChevLeftAmplitude.Init( "RightAmplitude", "Chevron", this, 1.0, -1e12, 1e12 );

    m_ChevTopAngle.Init( "Top_Angle", "Chevron", this, 0.0, -360.0, 360.0 );
    m_ChevBottomAngle.Init( "Bottom_Angle", "Chevron", this, 0.0, -360.0, 360.0 );
    m_ChevRightAngle.Init( "Right_Angle", "Chevron", this, 0.0, -360.0, 360.0 );
    m_ChevLeftAngle.Init( "Left_Angle", "Chevron", this, 0.0, -360.0, 360.0 );

    m_ChevTopSlew.Init( "Top_Slew", "Chevron", this, 0.0, -360.0, 360.0 );
    m_ChevBottomSlew.Init( "Bottom_Slew", "Chevron", this, 0.0, -360.0, 360.0 );
    m_ChevRightSlew.Init( "Right_Slew", "Chevron", this, 0.0, -360.0, 360.0 );
    m_ChevLeftSlew.Init( "Left_Slew", "Chevron", this, 0.0, -360.0, 360.0 );

    m_ValleyRad.Init( "Valley_Radius", "Chevron", this, 0.0, 0.0, 1e12 );
    m_PeakRad.Init( "Peak_Radius", "Chevron", this, 0.0, 0.0, 1e12 );

    // Background Image Parms
    m_XSecImagePreserveAR.Init( "XSecImagePreserveAR", ( m_GroupName + "_Background" ), this, true, false, true );
    m_XSecImageFlag.Init( "XSecImageFlag", ( m_GroupName + "_Background" ), this, false, false, true );
    m_XSecLockImageFlag.Init( "XSecLockImageFlag", ( m_GroupName + "_Background" ), this, false, false, true );
    m_XSecImageW.Init( "XSecImageW", ( m_GroupName + "_Background" ), this, 1.0, -1.0e12, 1.0e12 );
    m_XSecImageH.Init( "XSecImageH", ( m_GroupName + "_Background" ), this, 1.0, -1.0e12, 1.0e12 );
    m_XSecImageXOffset.Init( "XSecImageXOffset", ( m_GroupName + "_Background" ), this, 0.0, -1.0e12, 1.0e12 );
    m_XSecImageYOffset.Init( "XSecImageYOffset", ( m_GroupName + "_Background" ), this, 0.0, -1.0e12, 1.0e12 );
    m_XSecFlipImageFlag.Init( "XSecFlipImageFlag", ( m_GroupName + "_Background" ), this, false, false, true );

    m_Area.Init( "Area", m_GroupName, this, 0, 0, 1e12 );
    m_Area.SetDescript( "XSec Area" );

    m_HWRatio.Init( "HWRatio", m_GroupName, this, 0, 0, 1e12 );
    m_HWRatio.SetDescript( "XSec H/W Ratio" );

    m_FakeWidth = 1.0;
    m_UseFakeWidth = false;
    m_ForceWingType = false;

    m_yscale = 1.0;
}

XSecCurve::~XSecCurve()
{
    delete m_DriverGroup;
}

//==== Convert Any XSec to Cubic Bezier Edit Curve ====//
EditCurveXSec* XSecCurve::ConvertToEdit()
{
    if ( GetType() == vsp::XS_EDIT_CURVE )
    {
        return dynamic_cast<EditCurveXSec*>( this );
    }

    VspCurve vsp_curve = GetBaseEditCurve();

    // Make the curve more coarse. Force wingtype to false to avoid TMAGIC curve spitting, since 
    // XSecCurve::Update() already does it.
    vsp_curve.ToBinaryCubic( false, 1e-3, 0.01, 0, 3 );

    vector < vec3d > point_vec;
    vector < double > param_vec;

    vsp_curve.GetCubicControlPoints( point_vec, param_vec );

    double offset = GetWidth() / 2;

    vector < double > r_vec( param_vec.size() );

    for ( size_t i = 0; i < param_vec.size(); i++ )
    {
        param_vec[i] = param_vec[i] / 4.0; // Store point parameter (0-1) internally
        // Shift by 1/2 width and nondimensionalize
        point_vec[i].set_x( ( point_vec[i].x() - offset ) / max( GetWidth(), 1E-9 ) );
        point_vec[i].set_y( point_vec[i].y() / max( GetHeight(), 1E-9 ) );
        r_vec[i] = 0.0;
    }

    EditCurveXSec* xscrv_ptr = new EditCurveXSec();

    xscrv_ptr->CopyFrom( this );
    xscrv_ptr->m_SymType.Set( vsp::SYM_NONE );
    // TODO: Transfer symmetry and G1 continuity qualities

    // Transfer width and height parm values
    xscrv_ptr->SetWidthHeight( GetWidth(), GetHeight() );

    // Set Bezier control points
    xscrv_ptr->SetPntVecs( param_vec, point_vec, r_vec );

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

void XSecCurve::CopyBackgroundSettings( XSecCurve* xsc )
{
    m_XSecImageW.Set( xsc->m_XSecImageW.Get() );
    m_XSecImageH.Set( xsc->m_XSecImageH.Get() );
    m_XSecImageXOffset.Set( xsc->m_XSecImageXOffset.Get() );
    m_XSecImageYOffset.Set( xsc->m_XSecImageYOffset.Get() );
    m_XSecImagePreserveAR.Set( xsc->m_XSecImagePreserveAR.Get() );
    m_XSecImageFlag.Set( xsc->m_XSecImageFlag.Get() );
    m_ImageFile = xsc->GetImageFile();
    m_XSecFlipImageFlag = xsc->m_XSecFlipImageFlag();
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
    m_TETrimX.SetUpperLimit( 0.999 * GetWidth() );

    // Reconcile height, width, area, and hwratio.
    // Potentially involves an iterative solution when area is a driver.
    // May include lofting the curve.
    m_DriverGroup->UpdateGroup( GetDriverParms() );

    UpdateCurve();

    if ( m_Type != XS_POINT )
    {
        if ( !m_DriverGroup->IsDriver( vsp::AREA_XSEC_DRIVER ) )
        {
            m_Area = m_Curve.CompArea( vsp::Y_DIR, vsp::X_DIR );
        }

        if ( !m_DriverGroup->IsDriver( vsp::HWRATIO_XSEC_DRIVER ) )
        {
            if ( GetWidth() > 1e-6 * m_Area() )
            {
                m_HWRatio = GetHeight() / GetWidth();
            }
            else
            {
                m_HWRatio = 1.0;
            }
        }

        XSecCurveDriverGroup *xscdg = dynamic_cast< XSecCurveDriverGroup* > ( m_DriverGroup );
        if ( xscdg )
        {
            xscdg->m_prevArea = m_Area();
        }
    }
    else
    {
        m_Area = 0.0;
        m_HWRatio = 1.0;
    }

    m_BaseEditCurve = m_Curve; // Baseline VspCurve to initialize an EditCurveXSec with

    bool wingtype = DetermineWingType();

    // Order of these curve modifiers matters.
    CloseTE( wingtype );
    TrimTE( wingtype );

    CloseLE( wingtype );
    TrimLE( wingtype );

    CapTE( wingtype );
    CapLE( wingtype );

    // Apply 3D Chevron modification.
    Chevron();

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

//==== Get Driver Parms ====//
vector< string > XSecCurve::GetDriverParms()
{
    vector< string > parm_ids;
    parm_ids.resize( vsp::NUM_XSEC_DRIVER );
    parm_ids[ vsp::WIDTH_XSEC_DRIVER ] = GetWidthParmID();
    parm_ids[ vsp::HEIGHT_XSEC_DRIVER ] = GetHeightParmID();
    parm_ids[ vsp::AREA_XSEC_DRIVER ] = m_Area.GetID();
    parm_ids[ vsp::HWRATIO_XSEC_DRIVER ] = m_HWRatio.GetID();

    return parm_ids;
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

        m_DriverGroup->EncodeXml( xsec_node );

        if( m_ImageFile.size() > 0 )
        {
            XmlUtil::AddStringNode( xsec_node, "ImageFile", m_ImageFile );
        }
    }
    return xsec_node;
}

xmlNodePtr XSecCurve::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    xmlNodePtr xscrv_node = XmlUtil::GetNode( node, "XSecCurve", 0 );
    if( xscrv_node )
    {
        m_DriverGroup->DecodeXml( xscrv_node );
        m_ImageFile = XmlUtil::FindString( xscrv_node, "ImageFile", m_ImageFile );
    }

    return xscrv_node;
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

bool XSecCurve::DetermineWingType()
{
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

    return wingtype;
}

//==== Compute Area ====//
double XSecCurve::ComputeArea()
{
    VspCurve curve = GetCurve();
    return std::abs( curve.CompArea( vsp::X_DIR, vsp::Y_DIR ) );
}

// This calculates the area of an XSecCurve based on the current value
// of m_Curve without calling Update() to ensure the curve is up-to-date.
//
// Most callers should call ComputeArea() which ensures a curve is up-to-date
// before calculating the area.  However, in the context of the nonlinear
// solver used when area is a driver, we want to calculate the area
// mid-update (and obviusly without triggering an update).
double XSecCurve::AreaNoUpdate()
{
    return std::abs( m_Curve.CompArea( vsp::X_DIR, vsp::Y_DIR ) );
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

    if ( wingtype && m_TECloseType() != CLOSE_NONE && std::abs( dx ) > FLT_EPSILON )
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

    if ( m_TECloseType() != CLOSE_NONE && std::abs( dx ) > FLT_EPSILON )
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

    if ( m_LECloseType() != CLOSE_NONE && std::abs( dx ) > FLT_EPSILON )
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

    if ( m_LECloseType() != CLOSE_NONE && std::abs( dx ) > FLT_EPSILON )
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

void XSecCurve::Chevron()
{
    if ( m_ChevronType() == vsp::CHEVRON_NONE ) // Fast path.
    {
        m_ChevTopAmplitude.Deactivate();
        m_ChevBottomAmplitude.Deactivate();
        m_ChevLeftAmplitude.Deactivate();
        m_ChevRightAmplitude.Deactivate();

        m_ChevNumber.Deactivate();

        m_ChevOnDuty.Deactivate();
        m_ChevOffDuty.Deactivate();

        m_PeakRad.Deactivate();
        m_ValleyRad.Deactivate();

        m_ChevronExtentMode.Deactivate();
        m_ChevW01StartGuide.Deactivate();
        m_ChevW01Start.Deactivate();
        m_ChevW01EndGuide.Deactivate();
        m_ChevW01End.Deactivate();
        m_ChevW01CenterGuide.Deactivate();
        m_ChevW01Center.Deactivate();
        m_ChevW01Width.Deactivate();

        m_ChevTopAngle.Deactivate();
        m_ChevBottomAngle.Deactivate();
        m_ChevRightAngle.Deactivate();
        m_ChevLeftAngle.Deactivate();

        m_ChevTopSlew.Deactivate();
        m_ChevBottomSlew.Deactivate();
        m_ChevRightSlew.Deactivate();
        m_ChevLeftSlew.Deactivate();

        m_ChevDirAngleAllSymFlag.Deactivate();
        m_ChevDirAngleTBSymFlag.Deactivate();
        m_ChevDirAngleRLSymFlag.Deactivate();
        return;
    }
    else
    {
        m_ChevTopAmplitude.Activate();
        m_ChevBottomAmplitude.Activate();
        m_ChevLeftAmplitude.Activate();
        m_ChevRightAmplitude.Activate();

        m_ChevNumber.Activate();

        m_ChevOnDuty.Activate();
        m_ChevOffDuty.Activate();

        m_PeakRad.Activate();
        m_ValleyRad.Activate();

        m_ChevronExtentMode.Activate();
        m_ChevW01StartGuide.Activate();
        m_ChevW01Start.Activate();
        m_ChevW01EndGuide.Activate();
        m_ChevW01End.Activate();
        m_ChevW01CenterGuide.Activate();
        m_ChevW01Center.Activate();
        m_ChevW01Width.Activate();

        m_ChevTopAngle.Activate();
        m_ChevBottomAngle.Activate();
        m_ChevRightAngle.Activate();
        m_ChevLeftAngle.Activate();

        m_ChevTopSlew.Activate();
        m_ChevBottomSlew.Activate();
        m_ChevRightSlew.Activate();
        m_ChevLeftSlew.Activate();

        m_ChevDirAngleAllSymFlag.Activate();
        m_ChevDirAngleTBSymFlag.Activate();
        m_ChevDirAngleRLSymFlag.Activate();
    }

    // Make sure duty cycles can't exceed 1.0.
    m_ChevOnDuty.SetUpperLimit( 0.999 - m_ChevOffDuty() );
    m_ChevOffDuty.SetUpperLimit( 0.999 - m_ChevOnDuty() );

    // Do calculations in local doubles because Parms are limited to [0, 1] and negative intermediates are needed.
    double startW = m_ChevW01Start();
    double endW = m_ChevW01End();
    double centerW = m_ChevW01Center();
    double widthW = m_ChevW01Width();

    if ( m_ChevronType() == vsp::CHEVRON_PARTIAL )
    {
        if ( m_ChevW01StartGuide() != vsp::W_FREE )
            startW = ( (double) m_ChevW01StartGuide() ) / 4.0;

        if ( m_ChevW01EndGuide() != vsp::W_FREE )
            endW = ( (double) m_ChevW01EndGuide() ) / 4.0;

        if ( m_ChevW01CenterGuide() != vsp::W_FREE )
            centerW = ( (double) m_ChevW01CenterGuide() ) / 4.0;

        if ( m_ChevronExtentMode() == CHEVRON_W01_SE )
        {
            widthW = endW - startW;
            if (widthW < 0.0 )
            {
                widthW = widthW + 1.0;
            }

            centerW = startW + widthW / 2.0;
            if (centerW < 0.0 )
            {
                centerW = centerW + 1.0;
            }
            else if (centerW > 1.0 )
            {
                centerW = centerW - 1.0;
            }

            m_ChevW01Center.Deactivate();
            m_ChevW01Width.Deactivate();

            if ( m_ChevW01StartGuide() != vsp::W_FREE )
                m_ChevW01Start.Deactivate();

            if ( m_ChevW01EndGuide() != vsp::W_FREE )
                m_ChevW01End.Deactivate();

            m_ChevW01CenterGuide.Deactivate();
        }
        else if ( m_ChevronExtentMode() == CHEVRON_W01_CW )
        {
            startW = centerW - widthW / 2.0;
            if (startW < 0.0 )
            {
                startW = startW + 1.0;
            }

            endW = centerW + widthW / 2.0;
            if (endW > 1.0 )
            {
                endW = endW - 1.0;
            }
            m_ChevW01Start.Deactivate();
            m_ChevW01End.Deactivate();

            if ( m_ChevW01CenterGuide() != vsp::W_FREE )
                m_ChevW01Center.Deactivate();

            m_ChevW01StartGuide.Deactivate();
            m_ChevW01EndGuide.Deactivate();
        }
    }
    else if ( m_ChevronType() == vsp::CHEVRON_FULL )
    {
        endW = startW + 1.0;
        centerW = 0.5;
        widthW = 1.0;

        m_ChevW01StartGuide = vsp::W_RIGHT_0;
        m_ChevW01EndGuide = vsp::W_RIGHT_1;
        m_ChevW01CenterGuide = vsp::W_LEFT;

        m_ChevW01StartGuide.Deactivate();
        m_ChevW01EndGuide.Deactivate();
        m_ChevW01CenterGuide.Deactivate();

        m_ChevronExtentMode.Deactivate();
        m_ChevW01End.Deactivate();
        m_ChevW01Center.Deactivate();
        m_ChevW01Width.Deactivate();
    }

    // Set parameters to consistent values from calculations.
    m_ChevW01Start = startW;
    m_ChevW01End = endW;
    m_ChevW01Center = centerW;
    m_ChevW01Width = widthW;

    if ( m_ChevDirAngleAllSymFlag() )
    {
        m_ChevBottomAmplitude = m_ChevTopAmplitude();
        m_ChevLeftAmplitude = m_ChevTopAmplitude();
        m_ChevRightAmplitude = m_ChevTopAmplitude();

        m_ChevBottomAngle = m_ChevTopAngle();
        m_ChevRightAngle = m_ChevTopAngle();
        m_ChevLeftAngle = m_ChevTopAngle();

        m_ChevBottomSlew = m_ChevTopSlew();
        m_ChevRightSlew = m_ChevTopSlew();
        m_ChevLeftSlew = m_ChevTopSlew();

        m_ChevBottomAmplitude.Deactivate();
        m_ChevLeftAmplitude.Deactivate();
        m_ChevRightAmplitude.Deactivate();

        m_ChevBottomAngle.Deactivate();
        m_ChevRightAngle.Deactivate();
        m_ChevLeftAngle.Deactivate();

        m_ChevBottomSlew.Deactivate();
        m_ChevRightSlew.Deactivate();
        m_ChevLeftSlew.Deactivate();

        m_ChevDirAngleTBSymFlag.Deactivate();
        m_ChevDirAngleRLSymFlag.Deactivate();
    }

    if ( m_ChevDirAngleTBSymFlag() )
    {
        m_ChevBottomAmplitude = m_ChevTopAmplitude();
        m_ChevBottomAngle = m_ChevTopAngle();
        m_ChevBottomSlew = m_ChevTopSlew();

        m_ChevBottomAmplitude.Deactivate();
        m_ChevBottomAngle.Deactivate();
        m_ChevBottomSlew.Deactivate();
    }

    if ( m_ChevDirAngleRLSymFlag() )
    {
        m_ChevLeftAmplitude = m_ChevRightAmplitude();
        m_ChevLeftAngle = m_ChevRightAngle();
        m_ChevLeftSlew = m_ChevRightSlew();

        m_ChevLeftAmplitude.Deactivate();
        m_ChevLeftAngle.Deactivate();
        m_ChevLeftSlew.Deactivate();
    }

    // Set up chevron curve as if start is always at 0.0.
    // This simplifies some of the logic including determing how many points are needed to define curve.
    int ptpercycle = 2;
    int peakpercycle = 1;
    int valleypercycle = 1;
    if ( m_ChevOnDuty() > 0.0 )
    {
        ptpercycle ++;
        peakpercycle ++;
    }
    if ( m_ChevOffDuty() > 0.0 )
    {
        ptpercycle += 2;
        valleypercycle ++;
    }
    int npt = m_ChevNumber() * ptpercycle + 1;

    int iextra = 0;
    if ( m_ChevW01Width() != 1.0 ) // Add one to finish off partial extents
    {
        npt ++;
        iextra ++;
    }

    vector < vec3d > pts( npt );
    vector < double > bump( npt, 0.0 );
    vector < double > ts( npt, 0.0 );

    vector < double > uvalley( valleypercycle * m_ChevNumber() + iextra, -1 );
    vector < double > upeak( peakpercycle * m_ChevNumber(), -1 );

    double dt = 4.0 * (widthW / m_ChevNumber() ); // Time of full period.
    double dtoff = dt * m_ChevOffDuty();          // Time off.
    double dtoff2 = dtoff * 0.5;                  // Half of time off.
    double dton = dt * m_ChevOnDuty();            // Time on.
    double dtrise = 0.5 * ( dt - dtoff - dton );  // Time for rise or fall.
    double t0 = 0.0;

    // First point is set to ts[0] = 0, bump[0] = 0.
    int ipt = 1;
    int ivalley = 0;
    int ipeak = 0;
    for ( int i = 0; i < m_ChevNumber(); i++ )
    {
        if ( m_ChevOffDuty() > 0.0 )
        {
            t0 += dtoff2;
            ts[ipt] = t0;
            bump[ipt] = 0.0; // Can skip as already off
            ipt ++;
        }

        uvalley[ivalley] = t0;
        ivalley ++;

        t0 += dtrise;
        ts[ipt] = t0;
        bump[ipt] = 1.0;
        ipt ++;

        upeak[ipeak] = t0;
        ipeak++;

        if ( m_ChevOnDuty() > 0.0 )
        {
            t0 += dton;
            ts[ipt] = t0;
            bump[ipt] = 1.0;
            ipt ++;
            upeak[ipeak] = t0;
            ipeak++;
        }

        t0 += dtrise;
        ts[ipt] = t0;
        bump[ipt] = 0.0;
        ipt ++;

        if ( m_ChevOffDuty() > 0.0 )
        {
            uvalley[ivalley] = t0;
            ivalley ++;
            t0 += dtoff2;
            ts[ipt] = t0;
            bump[ipt] = 0.0; // Can skip as already off
            ipt ++;
        }
    }

    if ( m_ChevW01Width() < 1.0 )
    {
        if ( m_ChevOffDuty() == 0.0 )
        {
            uvalley[ivalley] = t0;
            ivalley ++;
        }
    }
    ts[npt - 1] = 4.0; // Make sure last point is at 4.0.

    // Spin (shift in W) base curve to start of chevrons.
    if ( m_ChevW01Start() != 0.0 )
    {
        m_Curve.Spin01( m_ChevW01Start() );
    }

    // Build control curve.
    vector< curve_point_type > angles(5);
    angles[0] << m_ChevRightAngle() * PI / 180.0, m_ChevRightSlew() * PI / 180.0, 0.0;
    angles[1] << m_ChevBottomAngle() * PI / 180.0, -m_ChevBottomSlew() * PI / 180.0, 0.0;
    angles[2] << m_ChevLeftAngle() * PI / 180.0, -m_ChevLeftSlew() * PI / 180.0, 0.0;
    angles[3] << m_ChevTopAngle() * PI / 180.0, m_ChevTopSlew() * PI / 180.0, 0.0;
    angles[4] << angles[0];

    vector< curve_point_type > amplitudes(5);
    amplitudes[0] << m_ChevRightAmplitude(), 0.0, 0.0;
    amplitudes[1] << m_ChevBottomAmplitude(), 0.0, 0.0;
    amplitudes[2] << m_ChevLeftAmplitude(), 0.0, 0.0;
    amplitudes[3] << m_ChevTopAmplitude(), 0.0, 0.0;
    amplitudes[4] << amplitudes[0];

    // Set up cubic spline of desired controls
    piecewise_cubic_spline_creator_type pcc( 4 );
    piecewise_linear_creator_type plc( 4 );
    pcc.set_t0( 0 );
    plc.set_t0( 0 );
    for ( unsigned int i = 0; i < 4; ++i)
    {
        pcc.set_segment_dt( 1.0, i );
        plc.set_segment_dt( 1.0, i );
    }

    pcc.set_closed_cubic_spline( angles.begin() );
    for ( unsigned int i = 0; i < 5; ++i)
    {
        plc.set_corner( amplitudes[i], i);
    }

    // Build control curve.
    piecewise_curve_type angle_crv;
    pcc.create( angle_crv );
    piecewise_curve_type amplitude_crv;
    plc.create( amplitude_crv );

    for ( int i = 0; i < pts.size(); i++ )
    {
        // Parameter on un-shifted curve.
        double s = ts[i] + 4.0 * m_ChevW01Start();
        if ( s < 0.0 ) s += 4.0;
        if ( s > 4.0 ) s -= 4.0;

        curve_point_type angle = angle_crv.f( s );
        curve_point_type amplitude = amplitude_crv.f( s );

        Matrix4d basis;
        vec3d dum1, dum2, dir;
        // Mimic the construction of the skinning angle with a fake circle
        basis.rotate( 2.0 * PI * s / 4.0, vec3d(0.0, 0.0, 1.0 ) );
        basis.rotate( -angle.x(), vec3d( 0.0, 1.0, 0.0 ) );
        basis.rotate( -angle.y(), vec3d( 1.0, 0.0, 0.0 ) );

        // Pull out desired normal and tangent directions.
        basis.getBasis( dum1, dum2, dir );

        pts[i] = bump[i] * amplitude.x() * dir;
    }

    VspCurve chevron;
    chevron.InterpolateLinear( pts, ts, false );

    // Superimpose curves and set result.
    piecewise_curve_type crv, new_crv, chev_crv;
    crv = m_Curve.GetCurve();
    chev_crv = chevron.GetCurve();

    crv.match_pmap( chev_crv ); // Modifies both piecewise curves to have identical pmap.
    new_crv.sum( crv, chev_crv ); // Promotes segments as needed to required order.

    // Set resultant curve (still shifted to start of chevrons.)
    m_Curve.SetCurve( new_crv );

    if (m_ValleyRad() != 0 )
    {
        m_Curve.RoundJoints( m_ValleyRad(), uvalley );
    }

    if ( m_PeakRad() != 0 )
    {
        m_Curve.RoundJoints( m_PeakRad(), upeak );
    }

    // Un-do spin to restore curve starting point.
    if ( m_ChevW01Start() != 0.0 )
    {
        m_Curve.Spin01( 1.0 - m_ChevW01Start() );
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
void PointXSec::UpdateCurve( bool updateParms )
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
    }
}
//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
CircleXSec::CircleXSec( ) : XSecCurve( )
{
    if( m_DriverGroup )
    {
        // m_DriverGroup was initialized in the XSecCurve() constructor.  However, we want to use a different
        // DriverGroup type for Circle.
        delete m_DriverGroup;
    }

    m_DriverGroup = new DXSecCurveDriverGroup();

    XSecCurveDriverGroup *xscdg = dynamic_cast< XSecCurveDriverGroup* > ( m_DriverGroup );
    if ( xscdg )
    {
        xscdg->m_Parent = this;
    }

    m_Type = XS_CIRCLE;
    m_Diameter.Init( "Circle_Diameter", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of Circle Cross-Section" );
}

//==== Set Width and Height ====//
void CircleXSec::SetWidthHeight( double w, double h )
{
    m_Diameter  = ( w + h ) / 2.0;
}

//==== Get Driver Parms ====//
vector< string > CircleXSec::GetDriverParms()
{
    vector< string > parm_ids;
    parm_ids.resize( vsp::CIRCLE_NUM_XSEC_DRIVER );
    parm_ids[ vsp::WIDTH_XSEC_DRIVER ] = GetWidthParmID();
    parm_ids[ vsp::AREA_XSEC_DRIVER ] = m_Area.GetID();

    return parm_ids;
}

void CircleXSec::OffsetCurve( double off )
{
    m_Diameter = m_Diameter() - 2.0*off;
}

//==== Update Geometry ====//
void CircleXSec::Update()
{
    // Circle can have invalid drivers if it was recently converted from another XSecCurve type.  A curve that was
    // previously an ellipse (for example) may have had drivers set to Area and HWRatio.  The conversion calls
    // XSec::CopyFrom() to transfer parameters as best it can.  This includes a direct transfer of the drivers,
    // no matter whether they make sense or not.
    // The reverse conversion is OK because the non-circle driver groups are initialized with Width, Height.  When
    // converting from a circle, only the first of these is over-written -- either with Width or Area.
    // Consequently, any combination is valid.

    if ( !m_DriverGroup->ValidDrivers( m_DriverGroup->GetChoices() ) )
    {
        m_DriverGroup->SetChoice( 0, WIDTH_XSEC_DRIVER );
    }

    XSecCurve::Update();
}

void CircleXSec::UpdateCurve( bool updateParms )
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
void EllipseXSec::UpdateCurve( bool updateParms )
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
void SuperXSec::UpdateCurve( bool updateParms )
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
    m_RadiusSymmetryType.Init( "RoundedRect_RadiusSymmetryType", m_GroupName, this, vsp::SYM_ALL, vsp::SYM_NONE, vsp::SYM_ALL );
    m_RadiusBR.Init( "RoundRectXSec_RadiusBR", m_GroupName, this, 0.2, 0.0, 1.0e12 );
    m_RadiusBL.Init( "RoundRectXSec_RadiusBL", m_GroupName, this, 0.2, 0.0, 1.0e12 );
    m_RadiusTL.Init( "RoundRectXSec_RadiusTL", m_GroupName, this, 0.2, 0.0, 1.0e12 );
    m_RadiusTR.Init( "RoundRectXSec_Radius", m_GroupName, this, 0.2, 0.0, 1.0e12 ); // Primary radius if SYM_ALL
    m_Skew.Init("RoundRect_Skew", m_GroupName, this, 0.0, -1e6, 1e6 );
    m_Keystone.Init("RoundRect_Keystone", m_GroupName, this, 0.5, 0.0, 1.0 );
    m_KeyCornerParm.Init( "RoundRectXSec_KeyCorner", m_GroupName, this, true, 0, 1 );
    m_VSkew.Init( "RoundRect_VSkew", m_GroupName, this, 0.0, -1e6, 1e6 );
}

//==== Update Geometry ====//
void RoundedRectXSec::UpdateCurve( bool updateParms )
{
    double r1 = m_RadiusBR();
    double r2 = m_RadiusBL();
    double r3 = m_RadiusTL();
    double r4 = m_RadiusTR();

    if ( m_RadiusSymmetryType() == vsp::SYM_ALL )
    {
        r1 = r4;
        r2 = r4;
        r3 = r4;
    }
    else if ( m_RadiusSymmetryType() == vsp::SYM_TB )
    {
        r2 = r3;
        r1 = r4;
    }
    else if ( m_RadiusSymmetryType() == vsp::SYM_RL )
    {
        r2 = r1;
        r3 = r4;
    }

    m_Curve.CreateRoundedRectangle( m_Width(), m_Height(), m_Keystone(), m_Skew(), m_VSkew(), r1, r2, r3, r4, m_KeyCornerParm() );

    if ( updateParms )
    {
        m_RadiusBR.Set( r1 );
        m_RadiusBL.Set( r2 );
        m_RadiusTL.Set( r3 );
        m_RadiusTR.Set( r4 );
    }
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

    m_RadiusBR.Set( m_RadiusBR() * scale );
    m_RadiusBL.Set( m_RadiusBL() * scale );
    m_RadiusTL.Set( m_RadiusTL() * scale );
    m_RadiusTR.Set( m_RadiusTR() * scale );
}

void RoundedRectXSec::ReadV2FileFuse2( xmlNodePtr &root )
{
    XSecCurve::ReadV2FileFuse2( root );

    int v2type = XmlUtil::FindInt( root, "Type", FuselageGeom::V2_FXS_RND_BOX );

    if ( v2type == FuselageGeom::V2_FXS_RND_BOX )
    {
        m_RadiusTR = XmlUtil::FindDouble( root, "Corner_Radius", m_RadiusTR() );
    }
    else
    {
        m_RadiusTR = 0.0;
    }
}

// Interpolate all parameters of like-type XSecCurves -- except width, height, and cli.
void RoundedRectXSec::Interp( XSecCurve *start, XSecCurve *end, double frac )
{
    RoundedRectXSec *s = dynamic_cast< RoundedRectXSec* > ( start );
    RoundedRectXSec *e = dynamic_cast< RoundedRectXSec* > ( end );

    if ( s && e )
    {
        INTERP_PARM( s, e, frac, m_RadiusBR );
        INTERP_PARM( s, e, frac, m_RadiusBL );
        INTERP_PARM( s, e, frac, m_RadiusTL );
        INTERP_PARM( s, e, frac, m_RadiusTR );
        INTERP_PARM( s, e, frac, m_Skew );
        INTERP_PARM( s, e, frac, m_VSkew );
        INTERP_PARM( s, e, frac, m_Keystone );
    }
    XSecCurve::Interp( start, end, frac );
}

EditCurveXSec* RoundedRectXSec::ConvertToEdit()
{
    // Create curve without rounded corners.
    VspCurve vsp_curve;
    vsp_curve.CreateRoundedRectangle( m_Width(), m_Height(), m_Keystone(), m_Skew(), m_VSkew(), 0.0, 0.0, 0.0, 0.0, m_KeyCornerParm() );

    vector < vec3d > point_vec;
    vector < double > param_vec;
    vsp_curve.GetLinearControlPoints( point_vec, param_vec );

    double offset = GetWidth() / 2;

    vector < double > r_vec( param_vec.size(), 0.0 );
    for ( size_t i = 0; i < param_vec.size(); i++ )
    {
        param_vec[i] = param_vec[i] / 4.0; // Store point parameter (0-1) internally
        // Shift by 1/2 width and nondimensionalize
        point_vec[i].set_x( ( point_vec[i].x() - offset ) / max( GetWidth(), 1E-9 ) );
        point_vec[i].set_y( point_vec[i].y() / max( GetHeight(), 1E-9 ) );
    }

    r_vec[1] = m_RadiusBR();
    r_vec[3] = m_RadiusBL();
    r_vec[5] = m_RadiusTL();
    r_vec[7] = m_RadiusTR();

    EditCurveXSec* xscrv_ptr = new EditCurveXSec();

    xscrv_ptr->CopyFrom( this );
    xscrv_ptr->m_SymType.Set( vsp::SYM_NONE );
    xscrv_ptr->m_CurveType = vsp::LINEAR;

    // Transfer width and height parm values
    xscrv_ptr->SetWidthHeight( GetWidth(), GetHeight() );

    // Set Bezier control points
    xscrv_ptr->SetPntVecs( param_vec, point_vec, r_vec );

    return xscrv_ptr;
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
    m_TopTanAngle.Init( "TopTanAngle", m_GroupName, this, 90.0, -180.0, 180.0 );
    m_BotTanAngle.Init( "BotTanAngle", m_GroupName, this, 90.0, -180.0, 180.0 );
    m_TopStr.Init( "TopStr",           m_GroupName, this, 0.83, 0.0, 5.0 );
    m_BotStr.Init( "BotStr",           m_GroupName, this, 0.83, 0.0, 5.0 );
    m_UpStr.Init( "UpStr",             m_GroupName, this, 0.83, 0.0, 5.0 );
    m_LowStr.Init( "LowStr",           m_GroupName, this, 0.83, 0.0, 5.0 );
}

//==== Set Width and Height ====//
void GeneralFuseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void GeneralFuseXSec::UpdateCurve( bool updateParms )
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
void FileXSec::UpdateCurve( bool updateParms )
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
                pnt_vec.push_back( vec3d( arry[i] * m_Width() * 0.5, arrz[i] * m_Height() * 0.5, 0.0 ) );
                if ( std::abs( arrz[i] ) < std::abs( arrz[izzero] ) )
                {
                    izzero = i;
                }
            }
            // Mirror points, do not repeat any points.
            for ( int i = num_pnts - 2; i > 0; i-- )
            {
                pnt_vec.push_back( vec3d( -arry[i] * m_Width() * 0.5, arrz[i] * m_Height() * 0.5, 0.0 ) );
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

    m_View.Init( "View", m_GroupName, this, vsp::VIEW_FRONT, vsp::VIEW_LEFT, vsp::VIEW_NUM_TYPES - 1 );
    m_View.SetDescript( "View direction" );

    m_CloseFlag.Init( "CloseFlag", m_GroupName, this, true, false, true );
    m_CloseFlag.SetDescript( "Closed Curve Flag" );

    m_SymType.Init( "SymType", m_GroupName, this, SYM_RL, SYM_NONE, SYM_RL );
    m_SymType.SetDescript( "2D Symmetry Type" );

    m_ShapeType.Init( "ShapeType", m_GroupName, this, EDIT_XSEC_CIRCLE, EDIT_XSEC_CIRCLE, EDIT_XSEC_RECTANGLE );
    m_ShapeType.SetDescript( "Initial Shape Type" );

    m_Depth.Init( "Depth", m_GroupName, this, 1.0, 1.0e-12, 1.0e12 );
    m_Depth.SetDescript( "Edit Curve XSec Depth" );

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

    m_XSecPointSize.Init( "XSecPointSize", ( m_GroupName + "_Background" ), this, 8.0, 1e-4, 1e4 );
    m_XSecLineThickness.Init( "XSecLineThickness", ( m_GroupName + "_Background" ), this, 1.5, 1e-4, 1e4 );

    m_XSecPointColorFlag.Init( "XSecPointColorFlag", ( m_GroupName + "_Background" ), this, false, false, true );
    m_XSecPointColorFlag.SetDescript( "Flag to color XSec points" );

    m_XSecPointColorWheel.Init( "XSecPointColorWheel", ( m_GroupName + "_Background" ), this, -1, -1, 359 );
    m_XSecPointColorWheel.SetDescript( "Color wheel index for XS_EDIT_CURVE points" );

    m_SelectPntID = 0;
    m_EnforceG1Next = true;
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

    if ( m_CurveType() == vsp::CEDIT )
    {
        for ( size_t i = 0; i < m_UParmVec.size(); i++ )
        {
            Parm* x_parm = dynamic_cast<Parm*> ( m_XParmVec[i] );
            Parm* y_parm = dynamic_cast<Parm*> ( m_YParmVec[i] );
            Parm* z_parm = dynamic_cast<Parm*> ( m_ZParmVec[i] );

            if ( parm_ptr == x_parm || parm_ptr == y_parm || parm_ptr == z_parm )
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

                        MovePnt( x_parm->Get() + dx, y_parm->Get(), z_parm->Get(), true );

                        SetSelectPntID( prev_index );
                    }
                    else if ( parm_ptr == y_parm )
                    {
                        int prev_index = GetSelectedPntID();
                        SetSelectPntID( i );

                        // Adjust the Y coordinates of the neighboring control points. Parm for point on the curve already updated
                        double dy = y_parm->Get() - y_parm->GetLastVal();

                        MovePnt( x_parm->Get(), y_parm->Get() + dy, z_parm->Get(), true );

                        SetSelectPntID( prev_index );
                    }
                    else if ( parm_ptr == z_parm )
                    {
                        int prev_index = GetSelectedPntID();
                        SetSelectPntID( i );

                        // Adjust the Z coordinates of the neighboring control points. Parm for point on the curve already updated
                        double dz = z_parm->Get() - z_parm->GetLastVal();

                        MovePnt( x_parm->Get(), y_parm->Get(), z_parm->Get() + dz, true );

                        SetSelectPntID( prev_index );
                    }
                }

                break;
            }
        }
    }

    if ( type == Parm::SET )
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
        psc.set_axis( 0.5, 0.5 );
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
    vector < double > ctrl_pnts_z( ctrl_pnts.size() );
    vector < double > u_vec( t_vec.size() ); // Set control point parameter values (0-4), but show the user (0-1)
    vector < double > r_vec( t_vec.size() );

    for ( size_t i = 0; i < ctrl_pnts.size(); i++ )
    {
        ctrl_pnts_x[i] = ctrl_pnts[i].x();
        ctrl_pnts_y[i] = ctrl_pnts[i].y();
        ctrl_pnts_z[i] = ctrl_pnts[i].z();
        u_vec[i] = t_vec[i] / 4.0;
        r_vec[i] = 0.0;
    }

    m_SelectPntID = 0; // Ensure selected point is not greater than # of control points

    //==== Load Control Points ====//
    SetPntVecs( u_vec, ctrl_pnts_x, ctrl_pnts_y, ctrl_pnts_z, r_vec ); // initialize all
}

void EditCurveXSec::AddPt( double default_u, double default_x, double default_y, double default_z, double default_r, bool default_g1, bool default_fix_u )
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

    FractionParm* fp = (FractionParm* )ParmMgr.CreateParm( vsp::PARM_FRACTION_TYPE );
    if ( fp )
    {
        int i = (int)m_XParmVec.size();
        char str[15];
        sprintf( str, "X_%d", i );
        fp->Init( string( str ), m_GroupName, this, default_x, -1.0e12, 1.0e12 );
        fp->SetDescript( "Control Point 2D X Location" );
        m_XParmVec.push_back( fp );
    }

    fp = (FractionParm*)ParmMgr.CreateParm( vsp::PARM_FRACTION_TYPE );
    if ( fp )
    {
        int i = (int)m_YParmVec.size();
        char str[15];
        sprintf( str, "Y_%d", i );
        fp->Init( string( str ), m_GroupName, this, default_y, -1.0e12, 1.0e12 );
        fp->SetDescript( "Control Point 2D Y Location" );
        m_YParmVec.push_back( fp );
    }

    fp = (FractionParm*)ParmMgr.CreateParm( vsp::PARM_FRACTION_TYPE );
    if ( fp )
    {
        int i = (int)m_ZParmVec.size();
        char str[15];
        sprintf( str, "Z_%d", i );
        fp->Init( string( str ), m_GroupName, this, default_z, -1.0e12, 1.0e12 );
        fp->SetDescript( "Control Point 2D Z Location" );
        m_ZParmVec.push_back( fp );
    }

    p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = (int)m_RParmVec.size();
        char str[15];
        sprintf( str, "R_%d", i );
        p->Init( string( str ), m_GroupName, this, default_r, 0.0, 1e12 );
        p->SetDescript( "Corner radius parameter" );
        m_RParmVec.push_back( p );
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

    bp = dynamic_cast<BoolParm*>( ParmMgr.CreateParm( vsp::PARM_BOOL_TYPE ) );
    if ( bp )
    {
        int i = (int)m_FixedUVec.size();
        char str[20];
        sprintf( str, "FixedU_%d", i );
        bp->Init( string( str ), m_GroupName, this, default_fix_u, false, true );
        bp->SetDescript( "Fixed U Flag" );
        m_FixedUVec.push_back( bp );
    }
}

//==== Update Geometry ====//
void EditCurveXSec::UpdateCurve( bool updateParms )
{
    if ( m_UParmVec.empty() )
    {
        InitShape(); // Must always have a valid curve
    }

    ClearPtOrder();

    EnforcePtOrder();

    EnforceG1();

    // Symmetry enforcement must come after G1 enforcement
    EnforceSymmetry();

    // Enforce Closure
    EnforceClosure();

    UpdateG1Parms();

    vector < double > t_vec = GetTVec(); // Set control point parameter values (0-4), but show the user (0-1)

    if ( m_CurveType() == vsp::LINEAR )
    {
        vector< vec3d > ctrl_pnts = GetCtrlPntVec( false, m_CloseFlag() );
        m_Curve.InterpolateLinear( ctrl_pnts, t_vec, m_CloseFlag() );
    }
    else if ( m_CurveType() == vsp::PCHIP )
    {
        vector< vec3d > ctrl_pnts = GetCtrlPntVec( false, m_CloseFlag() );
        if ( ctrl_pnts.size() > 3 ) // Note, slightly different than PCurve
        {
            m_Curve.InterpolatePCHIP( ctrl_pnts, t_vec, m_CloseFlag() );
        }
        else
        {
            m_Curve.InterpolateLinear( ctrl_pnts, t_vec, m_CloseFlag() );
        }
    }
    else if ( m_CurveType() == vsp::CEDIT )
    {
        vector< vec3d > ctrl_pnts = GetCtrlPntVec( false );
        m_Curve.SetCubicControlPoints( ctrl_pnts, t_vec );
    }

    m_Curve.OffsetX( 0.5 * m_Width() ); // Shift by 1/2 width (all XSec types are centered at (m_Width/2, 0, 0))

    m_UnroundedCurve = m_Curve;

    RoundCorners();
}

void EditCurveXSec::RoundCorners()
{
    vector < double > r_vec = GetRVec();

    if( m_CurveType() == vsp::LINEAR )
    {
        for ( int i = r_vec.size() - 2; i >= 0; i-- ) // Loop backwards to preserve indexing.
        {
            m_Curve.RoundJoint( r_vec[i], i );
        }
    }
    else if( m_CurveType() == vsp::CEDIT )
    {
        int nseg = r_vec.size() / 3;
        for ( int i = nseg - 1; i >= 0; i-- ) // Loop backwards to preserve indexing.
        {
            int j = 3 * i;
            m_Curve.RoundJoint( r_vec[j], i );
        }
    }

}

void EditCurveXSec::SetScale( double scale )
{
    XSecCurve::SetScale( scale );

    m_Depth.Set( m_Depth.Get() * scale );

    for ( int i = 0; i < m_RParmVec.size(); i++ )
    {
        m_RParmVec[i]->Set( m_RParmVec[i]->Get() * scale );
    }
}

void EditCurveXSec::EnforceClosure()
{
    if ( m_CloseFlag() )
    {
        m_XParmVec[m_XParmVec.size() - 1]->Set( m_XParmVec[0]->Get() );
        m_YParmVec[m_YParmVec.size() - 1]->Set( m_YParmVec[0]->Get() );
        m_ZParmVec[m_ZParmVec.size() - 1]->Set( m_ZParmVec[0]->Get() );
        m_RParmVec[m_RParmVec.size() - 1]->Set( m_RParmVec[0]->Get() );

        m_XParmVec[m_XParmVec.size() - 1]->Deactivate();
        m_YParmVec[m_YParmVec.size() - 1]->Deactivate();
        m_ZParmVec[m_ZParmVec.size() - 1]->Deactivate();
        m_RParmVec[m_RParmVec.size() - 1]->Deactivate();
        m_EnforceG1Vec[0]->Activate();
    }
    else
    {
        m_XParmVec[m_XParmVec.size() - 1]->Activate();
        m_YParmVec[m_YParmVec.size() - 1]->Activate();
        m_ZParmVec[m_ZParmVec.size() - 1]->Activate();
        m_RParmVec[m_RParmVec.size() - 1]->Activate();
        m_EnforceG1Vec[0]->Deactivate();
    }
}

void EditCurveXSec::UpdateG1Parms()
{
    if ( m_CurveType() == vsp::CEDIT )
    {
        for ( size_t i = 0; i < m_EnforceG1Vec.size() - 1; i++ )
        {
            BoolParm* p = m_EnforceG1Vec[i];
            
            if ( p )
            {
                if ( i % 3 != 0 ) // Deactivate G1 for interior control points
                {
                    p->Set( false );
                    p->Deactivate();
                    m_RParmVec[i]->Set( 0.0 );
                    m_RParmVec[i]->Deactivate();
                }
                else if ( !( m_SymType() == SYM_RL && m_UParmVec[i]->Get() > 0.25 && m_UParmVec[i]->Get() < 0.75 ) )
                {
                    p->Activate();
                    m_RParmVec[i]->Activate();
                    if ( p->Get() )
                    {
                        if ( m_SymType() == SYM_RL && ( m_UParmVec[i]->Get() == 0.25 || m_UParmVec[i]->Get() == 0.75 ) )
                        {
                            // Force horizontal tangent at axis of symmetry
                            m_YParmVec[i + 1]->Deactivate();
                            m_ZParmVec[i + 1]->Deactivate();
                        }
                        else
                        {
                            // Set next control point as master over previous point
                            m_YParmVec[i + 1]->Activate();
                            m_ZParmVec[i + 1]->Activate();
                        }
                        m_RParmVec[i]->Set( 0.0 );
                        m_RParmVec[i]->Deactivate();
                    }
                }
                else if ( m_SymType() == SYM_RL && m_UParmVec[i]->Get() > 0.25 && m_UParmVec[i]->Get() < 0.75 )
                {
                    p->Deactivate();
                    m_RParmVec[i]->Deactivate();
                }
            }
        }
    }
    else if ( m_CurveType() == vsp::PCHIP )
    {
        for ( int j = 0; j < m_EnforceG1Vec.size() - 1; j++ )
        {
            m_EnforceG1Vec[j]->Set( false );
            m_EnforceG1Vec[j]->Deactivate(); // No G1 enforcement for PCHIP
            m_RParmVec[j]->Set( 0.0 );
            m_RParmVec[j]->Deactivate();
        }
    }
    else if ( m_CurveType() == vsp::LINEAR )
    {
        for ( int j = 0; j < m_EnforceG1Vec.size() - 1; j++ )
        {
            m_EnforceG1Vec[j]->Set( false );
            m_EnforceG1Vec[j]->Deactivate(); // No G1 enforcement for linear
            m_RParmVec[j]->Activate();
        }
    }

    // Disable first if not closed
    if ( !m_CloseFlag() )
    {
        m_EnforceG1Vec[0]->Deactivate();
        m_EnforceG1Vec[0]->Set( false );
        m_RParmVec[0]->Set( 0.0 );
        m_RParmVec[0]->Deactivate();
    }

    // Always disable last.
    m_EnforceG1Vec[m_EnforceG1Vec.size() - 1]->Set( false );
    m_EnforceG1Vec[m_EnforceG1Vec.size() - 1]->Deactivate();
    m_RParmVec[m_RParmVec.size() - 1]->Set( m_RParmVec[0]->Get() );
    m_RParmVec[m_RParmVec.size() - 1]->Deactivate();
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
                m_FixedUVec[istart]->Deactivate();
            }

            if ( i == nseg - 1 && pend )
            {
                pend->Set( rlast );
                pend->Deactivate();
                pend->SetUpperLimit( rlast );
                pend->SetLowerLimit( rlast );
                m_FixedUVec[iend]->Deactivate();
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
                    m_FixedUVec[i]->Deactivate();
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
                    m_FixedUVec[j]->Deactivate();
                }
                else if ( j == m_UParmVec.size() - 1 )
                {
                    p->Set( rlast );
                    p->Deactivate();
                    p->SetUpperLimit( rlast );
                    p->SetLowerLimit( rlast );
                    m_FixedUVec[j]->Deactivate();
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

        p = m_ZParmVec[j];
        if ( p )
        {
            p->SetLowerLimit( -1.0e12 );
            p->SetUpperLimit( 1.0e12 );
            p->Activate();
        }

        p = m_RParmVec[j];
        if ( p )
        {
            p->SetLowerLimit( 0.0 );
            p->SetUpperLimit( 1.0e12 );
            p->Activate();
        }

        BoolParm* bp = m_FixedUVec[j];
        if( bp )
        {
            bp->Activate();
        }
    }
}

bool CompareUFunc( const std::pair< double, vector < double > > &a, const std::pair< double, vector < double > > &b )
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

        // Sort T vec and reorder X, Y, Z, R, G1, and Fixed U simultaneously
        // double[6] is {x, y, z, r, g1, fixed_u} to keep everything together when sorting
        vector < pair < double, vector < double > > > sort_vec;

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
                        vector < double > pnt_vec = { 0.0, ( m_YParmVec[i]->Get() + m_YParmVec[i + 1]->Get() ) / 2.0,
                                                           ( m_ZParmVec[i]->Get() + m_ZParmVec[i + 1]->Get() ) / 2.0,
                                                           ( m_RParmVec[i]->Get() + m_RParmVec[i + 1]->Get() ) / 2.0, false, false };
                        sort_vec.push_back( make_pair( top_u, pnt_vec ) );
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
                        vector < double > pnt_vec = { 0.0, ( m_YParmVec[i]->Get() + m_YParmVec[i + 1]->Get() ) / 2.0,
                                                           ( m_ZParmVec[i]->Get() + m_ZParmVec[i + 1]->Get() ) / 2.0,
                                                           ( m_RParmVec[i]->Get() + m_RParmVec[i + 1]->Get() ) / 2.0, false, false };
                        sort_vec.push_back( make_pair( bottom_u, pnt_vec ) );
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
                        m_UParmVec[(int)iend - 1]->Set( pend->Get() - dt / 3.0 );

                        // Add point on Y axis
                        vector < double > pnt_vec = { 0.0, ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0,
                                                           ( m_ZParmVec[istart]->Get() + m_ZParmVec[iend]->Get() ) / 2.0,
                                                           ( m_RParmVec[istart]->Get() + m_RParmVec[iend]->Get() ) / 2.0, false, false };
                        sort_vec.push_back( make_pair( top_u, pnt_vec ) );

                        // Add intermediate control point
                        pnt_vec = { 0.1 * m_Width(), ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0,
                                                     ( m_ZParmVec[istart]->Get() + m_ZParmVec[iend]->Get() ) / 2.0,
                                                     ( m_RParmVec[istart]->Get() + m_RParmVec[iend]->Get() ) / 2.0, false, false };
                        sort_vec.push_back( make_pair( top_u + dt / 3.0, pnt_vec ) );
                        
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
                        m_UParmVec[(int)istart + 1]->Set( pstart->Get() + dt / 3.0 );

                        // Add point on Y axis
                        vector < double > pnt_vec = { 0.0, ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0,
                                                           ( m_ZParmVec[istart]->Get() + m_ZParmVec[iend]->Get() ) / 2.0,
                                                           ( m_RParmVec[istart]->Get() + m_RParmVec[iend]->Get() ) / 2.0, false, false };
                        sort_vec.push_back( make_pair( bottom_u, pnt_vec ) );

                        // Add intermediate control point
                        pnt_vec = { 0.1 * m_Width(), ( m_YParmVec[istart]->Get() + m_YParmVec[iend]->Get() ) / 2.0,
                                                     ( m_ZParmVec[istart]->Get() + m_ZParmVec[iend]->Get() ) / 2.0,
                                                     ( m_RParmVec[istart]->Get() + m_RParmVec[iend]->Get() ) / 2.0, false, false };
                        sort_vec.push_back( make_pair( bottom_u - dt / 3.0, pnt_vec ) );

                        break;
                    }
                }
            }
        }

        if ( m_CurveType() == vsp::CEDIT )
        {
            // Include last two points
            vector < double > pnt_vec = { m_XParmVec[m_UParmVec.size() - 1]->Get(), m_YParmVec[m_UParmVec.size() - 1]->Get(), m_ZParmVec[m_UParmVec.size() - 1]->Get(), m_RParmVec[m_UParmVec.size() - 1]->Get(), (double)m_EnforceG1Vec[m_UParmVec.size() - 1]->Get(), (double)m_FixedUVec[m_UParmVec.size() - 1]->Get() };
            sort_vec.push_back( make_pair( m_UParmVec[m_UParmVec.size() - 1]->Get(), pnt_vec ) );
            pnt_vec = { m_XParmVec[m_UParmVec.size() - 2]->Get(), m_YParmVec[m_UParmVec.size() - 2]->Get(), m_ZParmVec[m_UParmVec.size() - 2]->Get(), m_RParmVec[m_UParmVec.size() - 2]->Get(), (double)m_EnforceG1Vec[m_UParmVec.size() - 2]->Get(), (double)m_FixedUVec[m_UParmVec.size() - 2]->Get() };
            sort_vec.push_back( make_pair( m_UParmVec[m_UParmVec.size() - 2]->Get(), pnt_vec ) );

            // Include first two points
            pnt_vec = { m_XParmVec[0]->Get(), m_YParmVec[0]->Get(), m_ZParmVec[0]->Get(), m_RParmVec[0]->Get(), (double)m_EnforceG1Vec[0]->Get(), (double)m_FixedUVec[0]->Get() };
            sort_vec.push_back( make_pair( m_UParmVec[0]->Get(), pnt_vec ) );
            pnt_vec = { m_XParmVec[1]->Get(), m_YParmVec[1]->Get(), m_ZParmVec[1]->Get(), m_RParmVec[1]->Get(), (double)m_EnforceG1Vec[1]->Get(), (double)m_FixedUVec[1]->Get() };
            sort_vec.push_back( make_pair( m_UParmVec[1]->Get(), pnt_vec ) );

            int nseg = ( (int)m_UParmVec.size() - 1 ) / 3;

            for ( int i = 1; i < nseg; i++ )
            {
                int iseg = i * 3;
                int prev_ind = iseg - 1;
                int next_ind = iseg + 1;

                if ( m_UParmVec[iseg]->Get() == bottom_u )
                {
                    // Only keep right side (left will be reflected later)
                    pnt_vec = { m_XParmVec[prev_ind]->Get(), m_YParmVec[prev_ind]->Get(), m_ZParmVec[prev_ind]->Get(), m_RParmVec[prev_ind]->Get(), (double)m_EnforceG1Vec[prev_ind]->Get(), (double)m_FixedUVec[prev_ind]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[prev_ind]->Get(), pnt_vec ) );
                    pnt_vec = { m_XParmVec[iseg]->Get(), m_YParmVec[iseg]->Get(), m_ZParmVec[iseg]->Get(), m_RParmVec[iseg]->Get(), (double)m_EnforceG1Vec[iseg]->Get(), (double)m_FixedUVec[iseg]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[iseg]->Get(), pnt_vec ) );
                }
                else if ( m_UParmVec[iseg]->Get() == top_u )
                {
                    // Only keep right side (left will be reflected later)
                    pnt_vec = { m_XParmVec[iseg]->Get(), m_YParmVec[iseg]->Get(), m_ZParmVec[iseg]->Get(), m_RParmVec[iseg]->Get(), (double)m_EnforceG1Vec[iseg]->Get(), (double)m_FixedUVec[iseg]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[iseg]->Get(), pnt_vec ) );
                    pnt_vec = { m_XParmVec[next_ind]->Get(), m_YParmVec[next_ind]->Get(), m_ZParmVec[next_ind]->Get(), m_RParmVec[next_ind]->Get(), (double)m_EnforceG1Vec[next_ind]->Get(), (double)m_FixedUVec[next_ind]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[next_ind]->Get(), pnt_vec ) );
                }
                else if ( ( m_UParmVec[iseg]->Get() <= bottom_u && m_UParmVec[prev_ind]->Get() <= bottom_u && m_UParmVec[next_ind]->Get() <= bottom_u ) ||
                    ( m_UParmVec[iseg]->Get() >= top_u && m_UParmVec[prev_ind]->Get() >= top_u && m_UParmVec[next_ind]->Get() >= top_u ) )
                {
                    // Only include this control point if both neighbors are less than 0.25 or greater than 0.75
                    pnt_vec = { m_XParmVec[prev_ind]->Get(), m_YParmVec[prev_ind]->Get(), m_ZParmVec[prev_ind]->Get(), m_RParmVec[prev_ind]->Get(), (double)m_EnforceG1Vec[prev_ind]->Get(), (double)m_FixedUVec[prev_ind]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[prev_ind]->Get(), pnt_vec ) );
                    pnt_vec = { m_XParmVec[iseg]->Get(), m_YParmVec[iseg]->Get(), m_ZParmVec[iseg]->Get(), m_RParmVec[iseg]->Get(), (double)m_EnforceG1Vec[iseg]->Get(), (double)m_FixedUVec[iseg]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[iseg]->Get(), pnt_vec ) );
                    pnt_vec = { m_XParmVec[next_ind]->Get(), m_YParmVec[next_ind]->Get(), m_ZParmVec[next_ind]->Get(), m_RParmVec[next_ind]->Get(), (double)m_EnforceG1Vec[next_ind]->Get(), (double)m_FixedUVec[next_ind]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[next_ind]->Get(), pnt_vec ) );
                }
            }
        }
        else
        {
            for ( size_t i = 0; i < m_UParmVec.size(); i++ )
            {
                if ( m_UParmVec[i]->Get() <= bottom_u || m_UParmVec[i]->Get() >= top_u )
                {
                    vector < double > pnt_vec = { m_XParmVec[i]->Get(), m_YParmVec[i]->Get(), m_ZParmVec[i]->Get(), m_RParmVec[i]->Get(), (double)m_EnforceG1Vec[i]->Get(), (double)m_FixedUVec[i]->Get() };
                    sort_vec.push_back( make_pair( m_UParmVec[i]->Get(), pnt_vec ) );
                }
            }
        }

        vector < pair < double, vector < double > > > left_sort_vec;

        for ( size_t i = 0; i < sort_vec.size(); i++ )
        {
            double right_u = sort_vec[i].first;
            double right_x = sort_vec[i].second[0];
            double right_y = sort_vec[i].second[1];
            double right_z = sort_vec[i].second[2];
            double right_r = sort_vec[i].second[3];
            bool right_g1 = (bool)sort_vec[i].second[4];
            bool right_fix_u = (bool)sort_vec[i].second[5];

            vector < double > pnt_vec = { -right_x, right_y, right_z, right_r, (double)right_g1, (double)right_fix_u };

            if ( right_u < bottom_u )
            {
                left_sort_vec.push_back( make_pair( 0.5 - right_u, pnt_vec ) );
            }
            else if ( right_u > top_u && right_u != 1.0 )
            {
                left_sort_vec.push_back( make_pair( 1.5 - right_u, pnt_vec ) );
            }
        }

        // Concatenate left and right
        sort_vec.insert( sort_vec.end(), left_sort_vec.begin(), left_sort_vec.end() );

        // Sort the vector by u value and reorder x, y, z, and g1 the same way
        sort( sort_vec.begin(), sort_vec.end(), CompareUFunc );

        vector < double > new_u_vec( sort_vec.size() );
        vector < vec3d > new_pnt_vec( sort_vec.size() );
        vector < double > new_r_vec( sort_vec.size() );
        vector < bool > new_g1_vec( sort_vec.size() );
        vector < bool > new_fix_u_vec( sort_vec.size() ); // FIXME: copy fixed U flag

        for ( size_t i = 0; i < sort_vec.size(); i++ )
        {
            new_u_vec[i] = sort_vec[i].first;
            new_pnt_vec[i].set_x( sort_vec[i].second[0] );
            new_pnt_vec[i].set_y( sort_vec[i].second[1] );
            new_pnt_vec[i].set_z( sort_vec[i].second[2] );
            new_r_vec[i] = sort_vec[i].second[3];
            new_g1_vec[i] = (bool)sort_vec[i].second[4];
            new_fix_u_vec[i] = (bool)sort_vec[i].second[5];
        }

        if ( new_u_vec.size() != m_UParmVec.size() )
        {
            SetPntVecs( new_u_vec, new_pnt_vec, new_r_vec, new_g1_vec, new_fix_u_vec, false );
        }
        else
        {
            for ( size_t i = 0; i < new_u_vec.size(); i++ )
            {
                m_UParmVec[i]->Set( new_u_vec[i] );
                m_XParmVec[i]->Set( new_pnt_vec[i].x() );
                m_YParmVec[i]->Set( new_pnt_vec[i].y() );
                m_ZParmVec[i]->Set( new_pnt_vec[i].z() );
                m_RParmVec[i]->Set( new_r_vec[i] );
                m_EnforceG1Vec[i]->Set( new_g1_vec[i] );
                m_FixedUVec[i]->Set( new_fix_u_vec[i] );
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
                m_ZParmVec[i]->Deactivate();
                m_RParmVec[i]->Deactivate();
                m_EnforceG1Vec[i]->Deactivate();
                m_FixedUVec[i]->Deactivate();
            }
            else if ( m_UParmVec[i]->Get() == bottom_u || m_UParmVec[i]->Get() == top_u )
            {
                m_UParmVec[i]->Deactivate();
                m_XParmVec[i]->Deactivate();
                // Allow translation along Y
                m_FixedUVec[i]->Deactivate();
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
                    vector < double > prev_r_vec = GetRVec();
                    VspCurve crv = m_UnroundedCurve;
                    crv.ToCubic(); // Promote the curve

                    vector < double > t_vec;
                    vector < vec3d> ctrl_pts;

                    crv.GetCubicControlPoints( ctrl_pts, t_vec );

                    vector < bool > new_g1_vec( ctrl_pts.size() );
                    vector < double > u_vec( t_vec.size() );
                    vector < double > new_r_vec( t_vec.size() );

                    for ( size_t i = 0; i < ctrl_pts.size(); i++ )
                    {
                        u_vec[i] = t_vec[i] / 4.0; // Store point parameter (0-1) internally

                        // Shift back to be centered at origin
                        ctrl_pts[i].offset_x( -0.5 * m_Width() );

                        // Nondimensionalize by width and height
                        ctrl_pts[i].scale_x( 1.0 / m_Width() );
                        ctrl_pts[i].scale_y( 1.0 / m_Height() );
                        ctrl_pts[i].scale_z( 1.0 / m_Depth() );

                        if ( i % 3 == 0 )
                        {
                            new_g1_vec[i] = prev_g1_vec[i / 3];
                            new_r_vec[i] = prev_r_vec[i / 3];
                        }
                        else
                        {
                            new_g1_vec[i] = false;
                            new_r_vec[i] = 0.0;
                        }
                    }

                    SetPntVecs( u_vec, ctrl_pts, new_r_vec, new_g1_vec );
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

                    m_UnroundedCurve.GetCubicControlPoints( ctrl_pts, t_vec );

                    vector < double > u_vec( t_vec.size() );
                    vector < bool > new_g1_vec( ctrl_pts.size() );
                    vector < double > new_r_vec( t_vec.size() );

                    for ( size_t i = 0; i < ctrl_pts.size(); i++ )
                    {
                        u_vec[i] = t_vec[i] / 4.0; // Store point parameter (0-1) internally

                        // Shift back to be centered at origin
                        ctrl_pts[i].offset_x( -0.5 * m_Width() );

                        // Nondimensionalize by width and height
                        ctrl_pts[i].scale_x( 1.0 / m_Width() );
                        ctrl_pts[i].scale_y( 1.0 / m_Height() );
                        ctrl_pts[i].scale_z( 1.0 / m_Depth() );

                        new_r_vec[i] = 0.0;

                        if ( i % 3 == 0 )
                        {
                            new_g1_vec[i] = prev_g1_vec[i / 3];
                        }
                        else
                        {
                            new_g1_vec[i] = false;
                        }
                    }

                    SetPntVecs( u_vec, ctrl_pts, new_r_vec, new_g1_vec );
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
                {
                    vector < double > u_vec = GetUVec();
                    vector < double > x_vec = GetXVec();
                    vector < double > y_vec = GetYVec();
                    vector < double > z_vec = GetZVec();
                    vector < double > r_vec = GetRVec();
                    vector < bool > g1_vec = GetG1Vec();

                    vector < double > new_u_vec, new_x_vec, new_y_vec, new_z_vec, new_r_vec;
                    vector < bool > new_g1_vec;

                    int nseg = ( (int)x_vec.size() - 1 ) / 3;

                    for ( int i = 0; i < nseg + 1; i++ )
                    {
                        int ipt = 3 * i;
                        new_u_vec.push_back( u_vec[ipt] );
                        new_x_vec.push_back( x_vec[ipt] );
                        new_y_vec.push_back( y_vec[ipt] );
                        new_z_vec.push_back( z_vec[ipt] );
                        new_r_vec.push_back( r_vec[ipt] );
                        new_g1_vec.push_back( g1_vec[ipt] );
                    }

                    SetPntVecs( new_u_vec, new_x_vec, new_y_vec, new_z_vec, new_r_vec, new_g1_vec );
                }
                break;
                case vsp::PCHIP:
                {
                    vector < double > u_vec = GetUVec();
                    vector < double > x_vec = GetXVec();
                    vector < double > y_vec = GetYVec();
                    vector < double > z_vec = GetZVec();
                    vector < bool > g1_vec = GetG1Vec();

                    vector < double > new_u_vec, new_x_vec, new_y_vec, new_z_vec, new_r_vec;
                    vector < bool > new_g1_vec;

                    int nseg = ( (int)x_vec.size() - 1 ) / 3;

                    for ( int i = 0; i < nseg + 1; i++ )
                    {
                        int ipt = 3 * i;
                        new_u_vec.push_back( u_vec[ipt] );
                        new_x_vec.push_back( x_vec[ipt] );
                        new_y_vec.push_back( y_vec[ipt] );
                        new_z_vec.push_back( z_vec[ipt] );
                        new_r_vec.push_back( 0.0 );
                        new_g1_vec.push_back( g1_vec[ipt] );
                    }

                    SetPntVecs( new_u_vec, new_x_vec, new_y_vec, new_z_vec, new_r_vec, new_g1_vec );
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

void EditCurveXSec::ReparameterizeEqualArcLength()
{
    vector < double > u_vec = GetUVec();
    int npt = (int)u_vec.size();
    int nseg = npt - 1;

    if ( m_CurveType() == vsp::CEDIT )
    {
        nseg = ( npt - 1 ) / 3;
    }

    // Calculate total arc length
    double tol = 1e-6;
    double tot_len = m_BaseEditCurve.CompLength( tol );

    // Identify length of each section between fixed U values
    vector < double > seg_len_vec;
    double seg_len = 0;
    int i_seg_prev = 0;

    for ( size_t i = 0; i < nseg; i++ )
    {
        // Get length of curve segment
        curve_segment_type c;
        double len = 0;
        m_BaseEditCurve.GetCurve().get( c, i );
        eli::geom::curve::length( len, c, tol );

        seg_len_vec.push_back( len );
    }

    double u_prev = 0;

    for ( size_t i = 0; i < nseg; i++ )
    {
        int iend = i + 1;

        if( m_CurveType() == vsp::CEDIT )
        {
            iend = ( i + 1 ) * 3;
        }

        if ( m_SymType() == vsp::SYM_RL && ( m_UParmVec[iend]->Get() > 0.25 && m_UParmVec[iend]->Get() <= 0.75 ) )
        {
            if ( m_UParmVec[iend]->Get() == 0.75 )
            {
                i_seg_prev = iend; // Set previous index after skipping left half of XSec
            }
            continue; // Skip left half of XSec when symmetry is on - EnforceSymmetry will mirror the right side
        }
        else if ( m_FixedUVec[iend]->Get() || ( m_SymType() == vsp::SYM_RL && m_UParmVec[iend]->Get() == 0.25 ) || ( iend == npt - 1 ) )
        {
            double du_seg = u_vec[iend] - u_prev;

            double seg_len = 0;

            for ( size_t j = i_seg_prev; j <= i; j++ )
            {
                seg_len += seg_len_vec[j];
            }

            for ( size_t j = i_seg_prev; j <= i; j++ )
            {
                int jstart = j;
                int jend = j + 1;

                if ( m_CurveType() == vsp::CEDIT )
                {
                    jstart = j * 3;
                    jend = ( j + 1 ) * 3;
                }

                if ( jend == npt - 1 )
                {
                    u_vec[jend] = 1.0; // Avoid floating point precision errors from calculation
                }
                else
                {
                    u_vec[jend] = u_vec[jstart] + ( seg_len_vec[j] / seg_len ) * du_seg;
                }

                if ( m_CurveType() == vsp::CEDIT )
                {
                    // Keep intermediate points valid.
                    double dt = u_vec[jend] - u_vec[jstart];
                    u_vec[jend - 1] = u_vec[jend] - ( dt / 3.0 );
                    u_vec[jstart + 1] = u_vec[jstart] + ( dt / 3.0 );
                }
            }

            i_seg_prev = i + 1;
            u_prev = u_vec[iend];
        }
    }

    // Set new U values, leave X, Y, and G1 unchanged
    for( size_t i = 0; i < u_vec.size(); i++ )
    {
        m_UParmVec[i]->Set( u_vec[i] );
    }

    Update();
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

vector < double > EditCurveXSec::GetZVec()
{
    vector < double > retvec( m_ZParmVec.size() );

    for ( size_t i = 0; i < m_ZParmVec.size(); ++i )
    {
        Parm* p = m_ZParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < double > EditCurveXSec::GetRVec()
{
    vector < double > retvec( m_RParmVec.size() );

    for ( size_t i = 0; i < m_RParmVec.size(); ++i )
    {
        Parm* p = m_RParmVec[i];
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

vector < bool > EditCurveXSec::GetFixedUVec()
{
    vector < bool > retvec( m_FixedUVec.size() );

    for ( size_t i = 0; i < m_FixedUVec.size(); ++i )
    {
        BoolParm* p = m_FixedUVec[i];
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
    MovePnt( new_pnt.x(), new_pnt.y(), new_pnt.z() );

    if ( force_update )
    {
        ParmChanged( NULL, Parm::SET_FROM_DEVICE );
    }
}

void EditCurveXSec::MovePnt( double x, double y, double z, bool neighbors_only )
{
    if ( m_SelectPntID < 0 || m_SelectPntID >= m_XParmVec.size() )
    {
        return;
    }

    Parm *xp = m_XParmVec[m_SelectPntID];
    Parm *yp = m_YParmVec[m_SelectPntID];
    Parm *zp = m_ZParmVec[m_SelectPntID];

    if ( xp && yp && zp )
    {

        switch ( this->m_CurveType() )
        {
            case vsp::LINEAR:
            case vsp::PCHIP:
                xp->Set( x );
                yp->Set( y );
                zp->Set( z );
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

                    // Adjust the Z coordinates of the neighboring control points
                    double dz = z - zp->Get();

                    Parm *zprev = NULL;
                    if ( m_SelectPntID > 0 )
                    {
                        zprev = m_ZParmVec[m_SelectPntID - 1];
                        zprev->Set( zprev->Get() + dz );
                    }
                    else if ( m_SelectPntID == 0 && m_CloseFlag() )
                    {
                        zprev = m_ZParmVec[m_ZParmVec.size() - 2];
                        zprev->Set( zprev->Get() + dz );
                    }
                    Parm *znext = NULL;
                    if ( m_SelectPntID < m_ZParmVec.size() - 1 )
                    {
                        znext = m_ZParmVec[m_SelectPntID + 1];
                        znext->Set( znext->Get() + dz );
                    }
                    else if ( ( m_SelectPntID == m_ZParmVec.size() - 1 ) && m_CloseFlag() )
                    {
                        znext = m_ZParmVec[1];
                        znext->Set( znext->Get() + dz );
                    }


                }
                else if ( !neighbors_only )
                {
                    xp->Set( x );
                }

                if ( !neighbors_only )
                {
                    yp->Set( y );
                    zp->Set( z );
                }

                break;
            }
        }
    }

    Update();
}

void EditCurveXSec::MovePnt( vec3d mpt, int iignore, bool neighbors_only )
{
    double z = 0;
    if ( m_SelectPntID < 0 || m_SelectPntID >= m_XParmVec.size() )
    {
        return;
    }

    Parm *xp = m_XParmVec[m_SelectPntID];
    Parm *yp = m_YParmVec[m_SelectPntID];
    Parm *zp = m_ZParmVec[m_SelectPntID];

    if ( xp && yp && zp )
    {
        vec3d prev_pt( xp->Get(), yp->Get(), zp->Get() );
        mpt.v[iignore] = prev_pt.v[iignore];
    }

    MovePnt( mpt.x(), mpt.y(), mpt.z(), neighbors_only );
}

vector < vec3d > EditCurveXSec::GetCtrlPntVec( bool non_dimensional, bool skip_last )
{
    int n = m_XParmVec.size();
    if ( skip_last && n > 0 )
    {
        n = n - 1;
    }
    vector < vec3d > return_vec( n );

    for ( size_t i = 0; i < n; i++ )
    {
        if ( non_dimensional )
        {
            return_vec[i] = vec3d( m_XParmVec[i]->Get(), m_YParmVec[i]->Get(), m_ZParmVec[i]->Get() );
        }
        else // Scale by width and height
        {
            return_vec[i] = vec3d( m_Width() * m_XParmVec[i]->Get(), m_Height() * m_YParmVec[i]->Get(), m_Depth() * m_ZParmVec[i]->Get() );
        }
    }

    return return_vec;
}

void EditCurveXSec::SetPntVecs( vector < double > u_vec, vector < vec3d > pnt_vec, vector < double > r_vec, vector < bool > g1_vec, vector < bool > fix_u_vec, bool force_update )
{
    vector < double > x_vec( pnt_vec.size() );
    vector < double > y_vec( pnt_vec.size() );
    vector < double > z_vec( pnt_vec.size() );

    for ( size_t i = 0; i < pnt_vec.size(); i++ )
    {
        x_vec[i] = pnt_vec[i].x();
        y_vec[i] = pnt_vec[i].y();
        z_vec[i] = pnt_vec[i].z();
    }

    SetPntVecs( u_vec, x_vec, y_vec, z_vec, r_vec, g1_vec, fix_u_vec, force_update );
}

void EditCurveXSec::SetPntVecs( vector < double > u_vec, vector < double > x_pnt_vec, vector < double > y_pnt_vec, vector < double > z_pnt_vec, vector < double > r_vec, vector < bool > g1_vec, vector < bool > fix_u_vec, bool force_update )
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

    if( fix_u_vec.size() != u_vec.size() )
    {
        fix_u_vec = vector < bool >( u_vec.size(), false );
    }

    //==== Load Control Points ====//
    for ( size_t i = 0; i < m_UParmVec.size(); i++ ) // FIXME: Data lost 
    {
        delete m_UParmVec[i];
        delete m_XParmVec[i];
        delete m_YParmVec[i];
        delete m_ZParmVec[i];
        delete m_RParmVec[i];
        delete m_EnforceG1Vec[i];
        delete m_FixedUVec[i];
    }

    m_UParmVec.clear();
    m_XParmVec.clear();
    m_YParmVec.clear();
    m_ZParmVec.clear();
    m_RParmVec.clear();
    m_EnforceG1Vec.clear();
    m_FixedUVec.clear();

    //==== Load Control Points ====//
    for ( size_t i = 0; i < u_vec.size(); i++ )
    {
        AddPt( u_vec[i], x_pnt_vec[i], y_pnt_vec[i], z_pnt_vec[i], r_vec[i], g1_vec[i], fix_u_vec[i] );
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

            delete m_ZParmVec[indx - 1];
            delete m_ZParmVec[indx];
            delete m_ZParmVec[indx + 1];
            m_ZParmVec.erase( m_ZParmVec.begin() + indx - 1, m_ZParmVec.begin() + indx + 2 );

            delete m_RParmVec[indx - 1];
            delete m_RParmVec[indx];
            delete m_RParmVec[indx + 1];
            m_RParmVec.erase( m_RParmVec.begin() + indx - 1, m_RParmVec.begin() + indx + 2 );

            delete m_EnforceG1Vec[indx - 1];
            delete m_EnforceG1Vec[indx];
            delete m_EnforceG1Vec[indx + 1];
            m_EnforceG1Vec.erase( m_EnforceG1Vec.begin() + indx - 1, m_EnforceG1Vec.begin() + indx + 2 );

            delete m_FixedUVec[(int)indx - 1];
            delete m_FixedUVec[indx];
            delete m_FixedUVec[(int)indx + 1];
            m_FixedUVec.erase( m_FixedUVec.begin() + indx - 1, m_FixedUVec.begin() + indx + 2 );
        }
        else if ( m_CurveType() != vsp::CEDIT )
        {
            delete m_UParmVec[indx];
            m_UParmVec.erase( m_UParmVec.begin() + indx );

            delete m_XParmVec[indx];
            m_XParmVec.erase( m_XParmVec.begin() + indx );

            delete m_YParmVec[indx];
            m_YParmVec.erase( m_YParmVec.begin() + indx );

            delete m_ZParmVec[indx];
            m_ZParmVec.erase( m_ZParmVec.begin() + indx );

            delete m_RParmVec[indx];
            m_RParmVec.erase( m_RParmVec.begin() + indx );

            delete m_EnforceG1Vec[indx];
            m_EnforceG1Vec.erase( m_EnforceG1Vec.begin() + indx );

            delete m_FixedUVec[indx];
            m_FixedUVec.erase( m_FixedUVec.begin() + indx );
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

        sprintf( str, "Z_%d", i );
        m_ZParmVec[i]->SetName( string( str ) );

        sprintf( str, "R_%d", i );
        m_RParmVec[i]->SetName( string( str ) );

        sprintf( str, "U_%d", i );
        m_UParmVec[i]->SetName( string( str ) );

        sprintf( str, "G1_%d", i );
        m_EnforceG1Vec[i]->SetName( string( str ) );

        sprintf( str, "FixedU_%d", i );
        m_FixedUVec[i]->SetName( string( str ) );
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
    m_SelectPntID = 0;

    vector < double > u_vec = GetUVec(); 

    for( size_t i = 0; i < u_vec.size(); i++ )
    {
        if( std::abs( u_split - u_vec[i] ) < FLT_EPSILON )
        {
            return m_SelectPntID; // Do not split identical U values
        }
        else if( u_split < u_vec[i] )
        {
            break;
        }
    }
    
    // Identify the index of the new point
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
            vec3d split_pnt = m_UnroundedCurve.CompPnt( 4.0 * u_split );

            // Shift back to be centered at origin
            split_pnt.offset_x( -0.5 * m_Width() );

            // Nondimensionalize by width and height
            split_pnt.scale_x( 1.0 / m_Width() );
            split_pnt.scale_y( 1.0 / m_Height() );

            u_vec = GetUVec();
            vector < double > x_vec = GetXVec();
            vector < double > y_vec = GetYVec();
            vector < double > z_vec = GetZVec();
            vector < double > r_vec = GetRVec();
            vector < bool > g1_vec = GetG1Vec();
            vector < bool > fix_u_vec = GetFixedUVec();

            vector < double > new_u_vec, new_x_vec, new_y_vec, new_z_vec, new_r_vec;
            vector < bool > new_g1_vec, new_fix_u_vec;
            new_u_vec.reserve( u_vec.size() + 1 );
            new_x_vec.reserve( x_vec.size() + 1 );
            new_y_vec.reserve( y_vec.size() + 1 );
            new_z_vec.reserve( z_vec.size() + 1 );
            new_r_vec.reserve( r_vec.size() + 1 );
            new_g1_vec.reserve( g1_vec.size() + 1 );
            new_fix_u_vec.reserve( fix_u_vec.size() + 1 );

            if ( u_split < u_vec[0] )
            {
                new_u_vec.push_back( u_split );
                new_x_vec.push_back( split_pnt.x() );
                new_y_vec.push_back( split_pnt.y() );
                new_z_vec.push_back( split_pnt.z() );
                new_r_vec.push_back( 0.0 );
                new_g1_vec.push_back( false );
                new_fix_u_vec.push_back( false );
            }

            new_u_vec.push_back( u_vec[0] );
            new_x_vec.push_back( x_vec[0] );
            new_y_vec.push_back( y_vec[0] );
            new_z_vec.push_back( z_vec[0] );
            new_r_vec.push_back( r_vec[0] );
            new_g1_vec.push_back( g1_vec[0] );
            new_fix_u_vec.push_back( fix_u_vec[0] );

            for ( size_t i = 1; i < u_vec.size(); i++ )
            {
                if ( u_split >= u_vec[i - 1] && u_split < u_vec[i] )
                {
                    new_u_vec.push_back( u_split );
                    new_x_vec.push_back( split_pnt.x() );
                    new_y_vec.push_back( split_pnt.y() );
                    new_z_vec.push_back( split_pnt.z() );
                    new_r_vec.push_back( 0.0 );
                    new_g1_vec.push_back( false );
                    new_fix_u_vec.push_back( false );
                }
                new_u_vec.push_back( u_vec[i] );
                new_x_vec.push_back( x_vec[i] );
                new_y_vec.push_back( y_vec[i] );
                new_z_vec.push_back( z_vec[i] );
                new_r_vec.push_back( r_vec[i] );
                new_g1_vec.push_back( g1_vec[i] );
                new_fix_u_vec.push_back( fix_u_vec[i] );
            }

            if ( u_split >= u_vec.back() )
            {
                new_u_vec.push_back( u_split );
                new_x_vec.push_back( split_pnt.x() );
                new_y_vec.push_back( split_pnt.y() );
                new_z_vec.push_back( split_pnt.z() );
                new_r_vec.push_back( 0.0 );
                new_g1_vec.push_back( false );
                new_fix_u_vec.push_back( false );
            }

            SetPntVecs( new_u_vec, new_x_vec, new_y_vec, new_z_vec, new_r_vec, new_g1_vec, new_fix_u_vec ); // Need to pass new_r_vec
        }
        break;
        case vsp::CEDIT:
        {
            VspCurve crv = m_UnroundedCurve;
            vector < bool > prev_g1_vec = GetG1Vec();
            vector < double > prev_r_vec = GetRVec();
            vector < bool > prev_fix_u_vec = GetFixedUVec();
            crv.Split( 4.0 * u_split );

            u_vec.clear();
            vector < double > t_vec;
            vector < vec3d > ctrl_pnts;

            crv.GetCubicControlPoints( ctrl_pnts, t_vec );

            u_vec.resize( t_vec.size() );
            vector < bool > new_g1_vec( ctrl_pnts.size() );
            vector < double > new_r_vec( ctrl_pnts.size() );
            vector < bool > new_fix_u_vec( ctrl_pnts.size() );

            for ( size_t i = 0; i < ctrl_pnts.size(); i++ )
            {
                u_vec[i] = t_vec[i] / 4.0;

                // Shift back to be centered at origin
                ctrl_pnts[i].offset_x( -0.5 * m_Width() );

                // Nondimensionalize by width and height
                ctrl_pnts[i].scale_x( 1.0 / m_Width() );
                ctrl_pnts[i].scale_y( 1.0 / m_Height() );
                ctrl_pnts[i].scale_z( 1.0 / m_Depth() );

                if ( ( i >= m_SelectPntID - 1 ) && ( i <= m_SelectPntID + 1 ) )
                {
                    new_g1_vec[i] = false;
                    new_r_vec[i] = 0.0;
                    new_fix_u_vec[i] = false;
                }
                else if ( i < m_SelectPntID - 1 )
                {
                    new_g1_vec[i] = prev_g1_vec[i];
                    new_r_vec[i] = prev_r_vec[i];
                    new_fix_u_vec[i] = prev_fix_u_vec[i];
                }
                else
                {
                    new_g1_vec[i] = prev_g1_vec[i - 3];
                    new_r_vec[i] = prev_r_vec[i - 3];
                    new_fix_u_vec[i] = prev_fix_u_vec[i - 3];
                }
            }

            SetPntVecs( u_vec, ctrl_pnts, new_r_vec, new_g1_vec, new_fix_u_vec );
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

            vec3d prev_pnt = vec3d( m_XParmVec[prev_ind]->Get(), m_YParmVec[prev_ind]->Get(), m_ZParmVec[prev_ind]->Get() );
            vec3d curr_pnt = vec3d( m_XParmVec[i]->Get(), m_YParmVec[i]->Get(), m_ZParmVec[i]->Get() );
            vec3d next_pnt = vec3d( m_XParmVec[next_ind]->Get(), m_YParmVec[next_ind]->Get(), m_ZParmVec[next_ind]->Get() );

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

            if ( std::abs( curr_pnt.x() - prev_pnt.x() ) < FLT_EPSILON && std::abs( curr_pnt.x() - next_pnt.x() ) < FLT_EPSILON )
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
                m_ZParmVec[prev_ind]->Set( new_prev_pnt.z() );

                prev_to_center_vec.rotate_z( cos( prev_rot ), sin( prev_rot ) );
                vec3d new_next_pnt = curr_pnt + next_dist * prev_to_center_vec;

                m_XParmVec[next_ind]->Set( new_next_pnt.x() );
                m_YParmVec[next_ind]->Set( new_next_pnt.y() );
                m_ZParmVec[next_ind]->Set( new_next_pnt.z() );
            }
            else if ( m_SymType() == SYM_RL && ( m_UParmVec[i]->Get() == 0.25 || m_UParmVec[i]->Get() == 0.75 ) )
            {
                // Force horizontal tangent at axis of symmetry
                m_YParmVec[prev_ind]->Set( m_YParmVec[i]->Get() );
                m_YParmVec[next_ind]->Set( m_YParmVec[i]->Get() );
                m_ZParmVec[prev_ind]->Set( m_ZParmVec[i]->Get() );
                m_ZParmVec[next_ind]->Set( m_ZParmVec[i]->Get() );
            }
            else if ( ( std::abs( prev_theta - next_theta ) > FLT_EPSILON ) )
            {
                // Identify the "master" vector and extend it to find the new point
                if ( m_EnforceG1Next )
                {
                    vec3d new_prev_pnt = curr_pnt + prev_dist * next_to_center_vec;

                    m_XParmVec[prev_ind]->Set( new_prev_pnt.x() );
                    m_YParmVec[prev_ind]->Set( new_prev_pnt.y() );
                    m_ZParmVec[prev_ind]->Set( new_prev_pnt.z() );
                }
                else
                {
                    vec3d new_next_pnt = curr_pnt + next_dist * prev_to_center_vec;

                    m_XParmVec[next_ind]->Set( new_next_pnt.x() );
                    m_YParmVec[next_ind]->Set( new_next_pnt.y() );
                    m_ZParmVec[next_ind]->Set( new_next_pnt.z() );
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
void InterpXSec::UpdateCurve( bool updateParms )
{
    m_Curve.MatchThick( m_Height() / m_Width() );
    m_Curve.Scale( m_Width() );
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

    crv_vec[0].ToBinaryCubic( true, 1e-6 );
    crv_vec[1].ToBinaryCubic( true, 1e-6 );

    VspSurf srf;
    srf.SkinC0( crv_vec, false );

    srf.GetUConstCurve( m_Curve, frac );

}

//==========================================================================//
//==========================================================================//
//==========================================================================//

XSecCurveDriverGroup::XSecCurveDriverGroup( int Nvar, int Nchoice ) : DriverGroup( Nvar, Nchoice )
{
    m_Parent = NULL;

    m_prevArea = -1.0;
    m_Name = "XSecCurveDriverGroup";
}

HWXSecCurveDriverGroup::HWXSecCurveDriverGroup() : XSecCurveDriverGroup( NUM_XSEC_DRIVER, 2 )
{
    m_CurrChoices[0] = WIDTH_XSEC_DRIVER;
    m_CurrChoices[1] = HEIGHT_XSEC_DRIVER;
}

void HWXSecCurveDriverGroup::UpdateGroup( vector< string > parmIDs )
{
    vector< bool > uptodate( m_Nvar, false );

    for( int i = 0; i < m_Nchoice; i++ )
    {
        uptodate[m_CurrChoices[i]] = true;
    }

    if( uptodate[WIDTH_XSEC_DRIVER] && uptodate[HEIGHT_XSEC_DRIVER] )
    {
        // fast path
    }
    else
    {
        Parm* width = ParmMgr.FindParm( parmIDs[WIDTH_XSEC_DRIVER] );
        Parm* height = ParmMgr.FindParm( parmIDs[HEIGHT_XSEC_DRIVER] );
        Parm* area = ParmMgr.FindParm( parmIDs[AREA_XSEC_DRIVER] );
        Parm* hwratio = ParmMgr.FindParm( parmIDs[HWRATIO_XSEC_DRIVER] );

        // Area is a driver, first time through, calculate m_prevArea
        if ( uptodate[AREA_XSEC_DRIVER] && m_prevArea < 0 )
        {
            m_Parent->UpdateCurve( false );
            m_prevArea = m_Parent->AreaNoUpdate();
        }

        if ( uptodate[AREA_XSEC_DRIVER] ) // Area is a driver, may need iteration.
        {
            int iter = 0;
            double tol = 1e-6 * area->Get();
            if ( tol < 1e-12 ) tol = 1e-12;
            double err = 100 * tol;

            bool canconverge = true;

            while ( err > tol && iter < 10 && canconverge )
            {
                if( uptodate[HWRATIO_XSEC_DRIVER] )
                {
                    if ( m_prevArea == 0 )
                    {
                        width->Set( 1.0 );
                        height->Set( 1.0 );
                    }
                    else
                    {
                        double scale = sqrt( area->Get() / m_prevArea );
                        if ( hwratio->Get() == 0 )
                        {
                            scale = 1.0;
                            canconverge = false;
                        }

                        width->Set( scale * width->Get() );
                        height->Set( width->Get() * hwratio->Get() );
                    }
                }
                else if( uptodate[WIDTH_XSEC_DRIVER] )
                {
                    if ( m_prevArea == 0 )
                    {
                        height->Set( 1.0 );
                    }
                    else
                    {
                        double scale = area->Get() / m_prevArea;

                        if ( width->Get() == 0 )
                        {
                            scale = 1.0;
                            canconverge = false;
                        }
                        height->Set( scale * height->Get() );
                    }
                }
                else if( uptodate[HEIGHT_XSEC_DRIVER] )
                {
                    if ( m_prevArea == 0 )
                    {
                        width->Set( 1.0 );
                    }
                    else
                    {
                        double scale = area->Get() / m_prevArea;
                        if ( height->Get() == 0 )
                        {
                            scale = 1.0;
                            canconverge = false;
                        }
                        width->Set( scale * width->Get() );
                    }
                }

                // Minimal curve math update.
                m_Parent->UpdateCurve( false );
                double newarea = m_Parent->AreaNoUpdate();

                err = std::abs( newarea - area->Get() );

                m_prevArea = newarea;
                iter++;
            }

            uptodate[WIDTH_XSEC_DRIVER] = true;
            uptodate[HEIGHT_XSEC_DRIVER] = true;
        }
        else  // Area is not a driver, cases should be algebraic.
        {
            if( !uptodate[WIDTH_XSEC_DRIVER] )
            {
                if( uptodate[HEIGHT_XSEC_DRIVER] && uptodate[HWRATIO_XSEC_DRIVER] )
                {
                    width->Set( height->Get() / hwratio->Get() );
                    uptodate[WIDTH_XSEC_DRIVER] = true;
                }
            }

            if( !uptodate[HEIGHT_XSEC_DRIVER] )
            {
                if( uptodate[WIDTH_XSEC_DRIVER] && uptodate[HWRATIO_XSEC_DRIVER] )
                {
                    height->Set( width->Get() * hwratio->Get() );
                    uptodate[HEIGHT_XSEC_DRIVER] = true;
                }
            }
        }
    }
}

bool HWXSecCurveDriverGroup::ValidDrivers( vector< int > choices )
{
    // Check for duplicate selections.
    for( int i = 0; i < (int)choices.size() - 1; i++ )
    {
        for( int j = i + 1; j < (int)choices.size(); j++ )
        {
            if( choices[i] == choices[j] )
            {
                return false;
            }
        }
    }

    return true;
}

DXSecCurveDriverGroup::DXSecCurveDriverGroup() : XSecCurveDriverGroup( CIRCLE_NUM_XSEC_DRIVER, 1 )
{
    m_CurrChoices[0] = WIDTH_XSEC_DRIVER;
}

void DXSecCurveDriverGroup::UpdateGroup( vector< string > parmIDs )
{
    vector< bool > uptodate( m_Nvar, false );

    for( int i = 0; i < m_Nchoice; i++ )
    {
        uptodate[m_CurrChoices[i]] = true;
    }

    if( uptodate[WIDTH_XSEC_DRIVER] )
    {
        // fast path
    }
    else
    {
        Parm* width = ParmMgr.FindParm( parmIDs[WIDTH_XSEC_DRIVER] );
        Parm* area = ParmMgr.FindParm( parmIDs[AREA_XSEC_DRIVER] );

        // Area is a driver, first time through, calculate m_prevArea
        if ( uptodate[AREA_XSEC_DRIVER] && m_prevArea < 0 )
        {
            m_Parent->UpdateCurve( false );
            m_prevArea = m_Parent->AreaNoUpdate();
        }

        if ( uptodate[AREA_XSEC_DRIVER] ) // Area is a driver, may need iteration.
        {
            int iter = 0;
            double tol = 1e-6 * area->Get();
            if ( tol < 1e-12 ) tol = 1e-12;
            double err = 100 * tol;

            while ( err > tol && iter < 10 )
            {
                if ( m_prevArea == 0 )
                {
                    width->Set( 1.0 );
                }
                else
                {
                    double scale = sqrt( area->Get() / m_prevArea );
                    width->Set( scale * width->Get() );
                }

                // Minimal curve math update.
                m_Parent->UpdateCurve( false );
                double newarea = m_Parent->AreaNoUpdate();

                err = std::abs( newarea - area->Get() );

                m_prevArea = newarea;
                iter++;
            }

            uptodate[WIDTH_XSEC_DRIVER] = true;
        }
    }
}

bool DXSecCurveDriverGroup::ValidDrivers( vector< int > choices )
{
    return ( choices[0] == WIDTH_XSEC_DRIVER || choices[0] == AREA_XSEC_DRIVER );
}
