//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcFinGeom.h"
#include "MdcBodGeom.h"
#include "APIDefines.h"
#include "SubSurface.h"
#include "IDMgr.h"

#include <algorithm>

using namespace vsp;


//==== Constructor ====//
MdcFinGeom::MdcFinGeom( Vehicle* vehicle_ptr ) : XSecSurfGeom( vehicle_ptr )
{
    m_Name = "MDC_FIN";
    m_Type.m_Name = "MDC_FIN";
    m_Type.m_Type = MDC_FIN_GEOM_TYPE;

    m_RootDepth = 0.0;
    m_BuriedChord = 0.0;

    // The end caps are set from the shape, so they are not offered.
    HideEndCaps();

    // Its members are placed about the axis, not about a point along it, so there is nothing to rotate about.
    m_Origin.Deactivate();

    // Along a new MDC_BOD, clear of its nose.
    m_XRelLoc.Set( 8.0 );

    m_SecTyp.Init( "SECTYP", "Design", this, HEX, HEX, USER );
    m_SecTyp.SetDescript( "Airfoil section type: HEX, NACA, ARC or USER" );
    m_LERType.Init( "LERType", "Design", this, LER_SHARP, LER_SHARP, LER_KEEP_ANGLE );
    m_LERType.SetDescript( "What LER does to a HEX or ARC section: nothing, the leading edge sharp as DATCOM draws it, "
                           "or rounds it, keeping the crest or the angle (not a FINSET input)" );
    m_NPanel.Init( "NPANEL", "Design", this, 4, 1, MAXPANEL );
    m_NPanel.SetDescript( "Number of panels in the fin set" );
    m_NSpan.Init( "NSPAN", "Design", this, 2, 2, MAXSTN );
    m_NSpan.SetDescript( "Number of span stations" );
    m_RBody.Init( "RBODY", "Design", this, 0.5, 0.0, 1.0e12 );
    m_RBody.SetDescript( "Distance of the root chord's middle from the axis, the body's radius there, where SSPAN_1 "
                         "is zero" );
    m_BuryRoot.Init( "BuryRoot", "Design", this, false, false, true );
    m_BuryRoot.SetDescript( "Flag to carry a root on the body into it, else the root is flat on the body as DATCOM "
                            "draws it (not a FINSET input)" );
    m_BuryRootType.Init( "BuryRootType", "Design", this, BURY_STRAIGHT, BURY_STRAIGHT, BURY_EXTRAPOLATE );
    m_BuryRootType.SetDescript( "How a buried root goes into the body: straight, or extrapolating the first "
                                "segment's taper and sweep (not a FINSET input)" );

    m_Station.Init( "STATION", "Design", this, 1, 1, MAXSTN );
    m_Station.SetDescript( "Span station the planform and section inputs show" );
    m_Panel.Init( "PANEL", "Design", this, 1, 1, MAXPANEL );
    m_Panel.SetDescript( "Panel the roll and dihedral inputs show" );

    m_ZLowerRelFlag.Init( "ZLOWERRelFlag", "Design", this, true, false, true );
    m_ZLowerRelFlag.SetDescript( "Flag to set ZLOWER as a ratio to ZUPPER" );
    m_LMaxLRelFlag.Init( "LMAXLRelFlag", "Design", this, true, false, true );
    m_LMaxLRelFlag.SetDescript( "Flag to set LMAXL as a ratio to LMAXU" );
    m_LFlatLRelFlag.Init( "LFLATLRelFlag", "Design", this, true, false, true );
    m_LFlatLRelFlag.SetDescript( "Flag to set LFLATL as a ratio to LFLATU" );

    m_PhiFFlag.Init( "PHIFFlag", "Design", this, false, false, true );
    m_PhiFFlag.SetDescript( "Flag to give PHIF, else the panels are evenly spaced from PHIF = 0" );

    m_NacaSer.Init( "NACASER", "Design", this, NACA4, NACA4, NACAS3 );
    m_NacaSer.SetDescript( "NACA series: 4 digit, modified 4 digit, 5 digit, modified 5 digit, 1, 6, or supersonic "
                           "diamond, circular arc or hexagonal" );
    m_NacaDes.Init( "NACADES", "Design", this, 12.0, 0.0, 99999.99 );
    m_NacaDes.SetDescript( "NACA designation as a number, as 2412 or 23012, with the thickness in % chord as its "
                           "last two digits and decimals, or the supersonic series' thickness in % chord" );
    m_NacaMod.Init( "NACAMOD", "Design", this, 63, 0, 99 );
    m_NacaMod.SetDescript( "Modified series' leading edge radius digit and position of maximum thickness digit" );
    m_NacaSea.Init( "NACASEA", "Design", this, false, false, true );
    m_NacaSea.SetDescript( "Flag for a 6 series 'A' section, straight from 80% chord to the trailing edge" );
    m_NacaMean.Init( "NACAMEAN", "Design", this, 1.0, 0.1, 1.0 );
    m_NacaMean.SetDescript( "Mean line parameter of the 6 series" );
    m_NacaXMax.Init( "NACAXMAX", "Design", this, 50.0, 1.0, 99.0 );
    m_NacaXMax.SetDescript( "Distance to maximum thickness of the supersonic series in % chord" );
    m_NacaFlat.Init( "NACAFLAT", "Design", this, 0.0, 0.0, 99.0 );
    m_NacaFlat.SetDescript( "Length of constant thickness section of the supersonic hexagonal series in % chord" );

    SizeParms();

    // The USER section is a 5% biconvex airfoil until a file is read.
    m_UserXSecSurf = new XSecSurf();
    m_UserXSecSurf->SetXSecType( XSEC_BARE );
    m_UserXSecSurf->SetParentContainer( GetID() );
    m_UserXSecSurf->AddXSec( XS_FILE_AIRFOIL );

    vector< vec3d > up;
    vector< vec3d > lo;
    for ( int i = 0; i <= 20; i++ )
    {
        double x = i / 20.0;
        up.push_back( vec3d( x, 0.1 * x * ( 1.0 - x ), 0.0 ) );
        lo.push_back( vec3d( x, -0.1 * x * ( 1.0 - x ), 0.0 ) );
    }
    GetUserAirfoil()->SetAirfoilPnts( up, lo );

    m_Preset.Init( "Preset", "Design", this, vsp::MDC_FIN_PRESET_DEFAULT, vsp::MDC_FIN_PRESET_DEFAULT, vsp::NUM_MDC_FIN_PRESET - 1 );
    m_Preset.SetDescript( "Preset shape Set gives the fin set: as new, or a generic rocket's fins or strakes" );
    GetPresetValues( { "Design" }, { "Preset", "STATION", "PANEL" }, m_DefaultValues );

    m_TessU = 5;
    m_TessW = 41;
}

//==== Destructor ====//
MdcFinGeom::~MdcFinGeom()
{
    delete m_UserXSecSurf;

    DeleteParms( m_SSpan );
    DeleteParms( m_Chord );
    DeleteParms( m_XLE );
    DeleteParms( m_LER );
    DeleteParms( m_CFOC );
    DeleteParms( m_Sweep );
    DeleteParms( m_Sta );
    DeleteParms( m_XLEFlag );
    DeleteParms( m_SectTessU );
    DeleteParms( m_ZUpper );
    DeleteParms( m_ZLower );
    DeleteParms( m_ZLowerRatio );
    DeleteParms( m_LMaxU );
    DeleteParms( m_LMaxL );
    DeleteParms( m_LMaxLRatio );
    DeleteParms( m_LFlatU );
    DeleteParms( m_LFlatL );
    DeleteParms( m_LFlatLRatio );
    DeleteParms( m_PhiF );
    DeleteParms( m_Gam );
}

void MdcFinGeom::SizeParms()
{
    int nspan = m_NSpan();
    int d = vsp::PARM_DOUBLE_TYPE;

    int old = ResizeParms( m_SSpan, nspan, d, "SSPAN", 0.0, 0.0, 1.0e12, "Semi-span location of the station" );
    for ( int i = old; i < nspan; i++ )
    {
        m_SSpan[i]->Set( i );
    }
    old = ResizeParms( m_Chord, nspan, d, "CHORD", 0.5, 0.0, 1.0e12, "Panel chord at the station" );
    for ( int i = old; i < nspan && i < 2; i++ )
    {
        m_Chord[i]->Set( 1.5 / ( i + 1.0 ) );
    }
    old = ResizeParms( m_XLE, nspan - 1, d, "XLE", 0.0, -1.0e12, 1.0e12,
                       "Distance from the root's leading edge to the chord's leading edge at the station", 2 );
    for ( int i = old; i < nspan - 1; i++ )
    {
        m_XLE[i]->Set( 0.75 * ( i + 1.0 ) );
    }
    ResizeParms( m_LER, nspan, d, "LER", 0.0, 0.0, 1.0e12, "Leading edge radius at the station, for HEX and ARC "
                 "sections" );
    ResizeParms( m_CFOC, nspan, d, "CFOC", 1.0, 0.0, 1.0, "Flap chord to fin chord ratio at the station" );

    ResizeParms( m_Sweep, nspan - 1, d, "SWEEP", 0.0, -89.9, 89.9, "Sweepback angle of the segment from the "
                 "station to the next" );
    ResizeParms( m_Sta, nspan - 1, d, "STA", 1.0, 0.0, 1.0, "Chord station sweep is measured at in the segment, 0 "
                 "leading edge and 1 trailing" );
    ResizeParms( m_XLEFlag, nspan - 1, vsp::PARM_BOOL_TYPE, "XLEFlag", false, false, true, "Flag to give XLE of the "
                 "station after the segment and have SWEEP follow it" );
    ResizeParms( m_SectTessU, nspan - 1, vsp::PARM_INT_TYPE, "SectTess_U", 5, 2, 1000, "Number of rows along the "
                 "span of the segment from the station to the next", 1, "Shape" );

    ResizeParms( m_ZUpper, nspan, d, "ZUPPER", 0.025, 0.0, 1.0, "Height of the upper surface above the chord line "
                 "to chord ratio at the station" );
    ResizeParms( m_ZLower, nspan, d, "ZLOWER", 0.025, 0.0, 1.0, "Height of the lower surface below the chord line "
                 "to chord ratio at the station" );
    ResizeParms( m_ZLowerRatio, nspan, d, "ZLOWERRatio", 1.0, 0.0, 1.0e6, "Ratio of ZLOWER to ZUPPER at the "
                 "station" );
    ResizeParms( m_LMaxU, nspan, d, "LMAXU", 0.5, 0.0, 1.0, "Fraction of chord from leading edge to maximum "
                 "thickness of upper surface at the station" );
    ResizeParms( m_LMaxL, nspan, d, "LMAXL", 0.5, 0.0, 1.0, "Fraction of chord from leading edge to maximum "
                 "thickness of lower surface at the station" );
    ResizeParms( m_LMaxLRatio, nspan, d, "LMAXLRatio", 1.0, 0.0, 1.0e6, "Ratio of LMAXL to LMAXU at the station" );
    ResizeParms( m_LFlatU, nspan, d, "LFLATU", 0.0, 0.0, 1.0, "Fraction of chord of constant thickness section of "
                 "upper surface at the station" );
    ResizeParms( m_LFlatL, nspan, d, "LFLATL", 0.0, 0.0, 1.0, "Fraction of chord of constant thickness section of "
                 "lower surface at the station" );
    ResizeParms( m_LFlatLRatio, nspan, d, "LFLATLRatio", 1.0, 0.0, 1.0e6, "Ratio of LFLATL to LFLATU at the "
                 "station" );

    int npanel = m_NPanel();
    old = ResizeParms( m_PhiF, npanel, d, "PHIF", 0.0, -360.0, 720.0, "Roll angle of the panel, clockwise from top "
                       "vertical looking forward" );
    for ( int i = old; i < npanel; i++ )
    {
        m_PhiF[i]->Set( 90.0 * i );
    }
    ResizeParms( m_Gam, npanel, d, "GAM", 0.0, -180.0, 180.0, "Dihedral of the panel, positive when PHIF is "
                 "increased" );
}

// The generic rocket's four tail fins, and its pair of strakes, on its body of radius 5.
static const XSecSurfGeom::PresetValue rocket_fin[] = {
    { "Design", "CFOC_1", 1.0 },
    { "Design", "CFOC_2", 1.0 },
    { "Design", "CFOC_3", 1.0 },
    { "Design", "CHORD_1", 12.0 },
    { "Design", "CHORD_2", 12.0 },
    { "Design", "CHORD_3", 2.0 },
    { "Design", "GAM_1", 0.0 },
    { "Design", "GAM_2", 0.0 },
    { "Design", "GAM_3", 0.0 },
    { "Design", "GAM_4", 0.0 },
    { "Design", "LER_1", 0.0 },
    { "Design", "LER_2", 0.0 },
    { "Design", "LER_3", 0.0 },
    { "Design", "LFLATLRatio_1", 1.0 },
    { "Design", "LFLATLRatio_2", 1.0 },
    { "Design", "LFLATLRatio_3", 1.0 },
    { "Design", "LFLATLRelFlag", 1.0 },
    { "Design", "LFLATL_1", 0.0 },
    { "Design", "LFLATL_2", 0.0 },
    { "Design", "LFLATL_3", 0.0 },
    { "Design", "LFLATU_1", 0.0 },
    { "Design", "LFLATU_2", 0.0 },
    { "Design", "LFLATU_3", 0.0 },
    { "Design", "LMAXLRatio_1", 1.0 },
    { "Design", "LMAXLRatio_2", 1.0 },
    { "Design", "LMAXLRatio_3", 1.0 },
    { "Design", "LMAXLRelFlag", 1.0 },
    { "Design", "LMAXL_1", 0.5 },
    { "Design", "LMAXL_2", 0.5 },
    { "Design", "LMAXL_3", 0.5 },
    { "Design", "LMAXU_1", 0.5 },
    { "Design", "LMAXU_2", 0.5 },
    { "Design", "LMAXU_3", 0.5 },
    { "Design", "NACADES", 12.0 },
    { "Design", "NACAFLAT", 0.0 },
    { "Design", "NACAMEAN", 1.0 },
    { "Design", "NACAMOD", 63.0 },
    { "Design", "NACASEA", 0.0 },
    { "Design", "NACASER", 0.0 },
    { "Design", "NACAXMAX", 50.0 },
    { "Design", "NPANEL", 4.0 },
    { "Design", "NSPAN", 3.0 },
    { "Design", "PHIFFlag", 0.0 },
    { "Design", "PHIF_1", 0.0 },
    { "Design", "PHIF_2", 90.0 },
    { "Design", "PHIF_3", 180.0 },
    { "Design", "PHIF_4", 270.0 },
    { "Design", "RBODY", 5.0 },
    { "Design", "SECTYP", 0.0 },
    { "Design", "SSPAN_1", 0.0 },
    { "Design", "SSPAN_2", 1.0 },
    { "Design", "SSPAN_3", 8.0 },
    { "Design", "STA_1", 1.0 },
    { "Design", "STA_2", 1.0 },
    { "Design", "SWEEP_1", 0.0 },
    { "Design", "SWEEP_2", 0.0 },
    { "Design", "XLEFlag_1", 0.0 },
    { "Design", "XLEFlag_2", 0.0 },
    { "Design", "XLE_2", 0.0 },
    { "Design", "XLE_3", 10.0 },
    { "Design", "ZLOWERRatio_1", 1.0 },
    { "Design", "ZLOWERRatio_2", 1.0 },
    { "Design", "ZLOWERRatio_3", 1.0 },
    { "Design", "ZLOWERRelFlag", 1.0 },
    { "Design", "ZLOWER_1", 0.025 },
    { "Design", "ZLOWER_2", 0.025 },
    { "Design", "ZLOWER_3", 0.025 },
    { "Design", "ZUPPER_1", 0.025 },
    { "Design", "ZUPPER_2", 0.025 },
    { "Design", "ZUPPER_3", 0.025 }
};

static const XSecSurfGeom::PresetValue rocket_strake[] = {
    { "Design", "CFOC_1", 1.0 },
    { "Design", "CFOC_2", 1.0 },
    { "Design", "CFOC_3", 1.0 },
    { "Design", "CHORD_1", 90.0 },
    { "Design", "CHORD_2", 80.0 },
    { "Design", "CHORD_3", 75.0 },
    { "Design", "GAM_1", 0.0 },
    { "Design", "GAM_2", 0.0 },
    { "Design", "LER_1", 0.0 },
    { "Design", "LER_2", 0.0 },
    { "Design", "LER_3", 0.0 },
    { "Design", "LFLATLRatio_1", 1.0 },
    { "Design", "LFLATLRatio_2", 1.0 },
    { "Design", "LFLATLRatio_3", 1.0 },
    { "Design", "LFLATLRelFlag", 1.0 },
    { "Design", "LFLATL_1", 0.25 },
    { "Design", "LFLATL_2", 0.25 },
    { "Design", "LFLATL_3", 0.0 },
    { "Design", "LFLATU_1", 0.25 },
    { "Design", "LFLATU_2", 0.25 },
    { "Design", "LFLATU_3", 0.0 },
    { "Design", "LMAXLRatio_1", 1.0 },
    { "Design", "LMAXLRatio_2", 1.0 },
    { "Design", "LMAXLRatio_3", 1.0 },
    { "Design", "LMAXLRelFlag", 1.0 },
    { "Design", "LMAXL_1", 0.5 },
    { "Design", "LMAXL_2", 0.5 },
    { "Design", "LMAXL_3", 0.5 },
    { "Design", "LMAXU_1", 0.5 },
    { "Design", "LMAXU_2", 0.5 },
    { "Design", "LMAXU_3", 0.5 },
    { "Design", "NACADES", 12.0 },
    { "Design", "NACAFLAT", 0.0 },
    { "Design", "NACAMEAN", 1.0 },
    { "Design", "NACAMOD", 63.0 },
    { "Design", "NACASEA", 0.0 },
    { "Design", "NACASER", 0.0 },
    { "Design", "NACAXMAX", 50.0 },
    { "Design", "NPANEL", 2.0 },
    { "Design", "NSPAN", 3.0 },
    { "Design", "PHIFFlag", 1.0 },
    { "Design", "PHIF_1", 90.0 },
    { "Design", "PHIF_2", 270.0 },
    { "Design", "RBODY", 5.0 },
    { "Design", "SECTYP", 0.0 },
    { "Design", "SSPAN_1", 0.0 },
    { "Design", "SSPAN_2", 1.0 },
    { "Design", "SSPAN_3", 2.0 },
    { "Design", "STA_1", 0.5 },
    { "Design", "STA_2", 0.5 },
    { "Design", "SWEEP_1", 0.0 },
    { "Design", "SWEEP_2", 0.0 },
    { "Design", "XLEFlag_1", 0.0 },
    { "Design", "XLEFlag_2", 0.0 },
    { "Design", "XLE_2", 5.0 },
    { "Design", "XLE_3", 7.5 },
    { "Design", "ZLOWERRatio_1", 1.0 },
    { "Design", "ZLOWERRatio_2", 1.0 },
    { "Design", "ZLOWERRatio_3", 1.0 },
    { "Design", "ZLOWERRelFlag", 1.0 },
    { "Design", "ZLOWER_1", 0.005 },
    { "Design", "ZLOWER_2", 0.003 },
    { "Design", "ZLOWER_3", 0.001 },
    { "Design", "ZUPPER_1", 0.005 },
    { "Design", "ZUPPER_2", 0.003 },
    { "Design", "ZUPPER_3", 0.001 }
};

void MdcFinGeom::SetPreset()
{
    vector< PresetValue > vals = m_DefaultValues;
    double scale = 1.0;
    if ( m_Preset() == vsp::MDC_FIN_PRESET_GENERIC_ROCKET_FINS || m_Preset() == vsp::MDC_FIN_PRESET_GENERIC_ROCKET_STRAKES )
    {
        if ( m_Preset() == vsp::MDC_FIN_PRESET_GENERIC_ROCKET_FINS )
        {
            vals = vector< PresetValue >( rocket_fin, rocket_fin + sizeof( rocket_fin ) / sizeof( rocket_fin[0] ) );
        }
        else
        {
            vals = vector< PresetValue >( rocket_strake, rocket_strake + sizeof( rocket_strake ) / sizeof( rocket_strake[0] ) );
        }
        double rmax = 0.0;
        if ( MdcBodGeom::MaxRadiusOfParent( this, rmax ) )
        {
            scale = rmax / 5.0;
        }
    }

    // The counts first, then the stations and panels they size.
    SetPresetValues( vals );
    SizeParms();
    SetPresetValues( vals );
    if ( scale != 1.0 )
    {
        ApplyScale( scale );
    }
}

//==== Scale ====//
void MdcFinGeom::ApplyScale( double currentScale )
{
    m_RBody.Set( currentScale * m_RBody() );
    for ( int k = 0; k < ( int )m_SSpan.size(); k++ )
    {
        m_SSpan[k]->Set( currentScale * m_SSpan[k]->Get() );
        m_Chord[k]->Set( currentScale * m_Chord[k]->Get() );
        m_LER[k]->Set( currentScale * m_LER[k]->Get() );
    }
    for ( int k = 0; k < ( int )m_XLE.size(); k++ )
    {
        m_XLE[k]->Set( currentScale * m_XLE[k]->Get() );
    }
}

XSec* MdcFinGeom::GetUserXSec()
{
    return m_UserXSecSurf->FindXSec( 0 );
}

FileAirfoil* MdcFinGeom::GetUserAirfoil()
{
    return dynamic_cast< FileAirfoil* >( GetUserXSec()->GetXSecCurve() );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr MdcFinGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr fin_node = xmlNewChild( node, nullptr, BAD_CAST "MdcFinGeom", nullptr );
    if ( fin_node )
    {
        m_UserXSecSurf->EncodeXml( fin_node );

        for ( int i = 0; i < ( int )m_FlapIDVec.size(); i++ )
        {
            XmlUtil::AddStringNode( fin_node, "FlapID", m_FlapIDVec[i] );
        }
    }
    Geom::EncodeXml( node );

    return fin_node;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr MdcFinGeom::DecodeXml( xmlNodePtr & node )
{
    m_NSpan.Set( DesignValue( node, "NSPAN", m_NSpan() ) );
    m_NPanel.Set( DesignValue( node, "NPANEL", m_NPanel() ) );
    SizeParms();

    xmlNodePtr fin_node = XmlUtil::GetNode( node, "MdcFinGeom", 0 );
    if ( fin_node )
    {
        if ( XmlUtil::GetNode( fin_node, "XSecSurf", 0 ) )
        {
            m_UserXSecSurf->DecodeXml( fin_node );
        }

        m_FlapIDVec.clear();
        int nflap = XmlUtil::GetNumNames( fin_node, "FlapID" );
        for ( int i = 0; i < nflap; i++ )
        {
            xmlNodePtr flap_node = XmlUtil::GetNode( fin_node, "FlapID", i );
            m_FlapIDVec.push_back( IDMgr.RemapRefID( XmlUtil::ExtractString( flap_node ) ) );
        }
    }
    Geom::DecodeXml( node );

    // A fin set saved before it had rows by segment takes Tess_U for each.
    if ( DesignValue( node, "SectTess_U_1", -1.0, "Shape" ) < 0.0 )
    {
        for ( int k = 0; k < ( int )m_SectTessU.size(); k++ )
        {
            m_SectTessU[k]->Set( m_TessU() );
        }
    }

    return fin_node;
}

// A cap, a buried root's segment, each segment by its own, and a cap, as a Wing tessellates them.
bool MdcFinGeom::GetTessUVec( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess,
                              vector< int > &tessvec ) const
{
    tessvec.clear();
    if ( m_CapUMinOption() != NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
    }
    if ( m_RootDepth > 0.0 )
    {
        tessvec.push_back( BURIED_TESS_U );
    }
    for ( int k = 0; k < ( int )m_SectTessU.size(); k++ )
    {
        tessvec.push_back( m_SectTessU[k]->Get() );
    }
    if ( m_CapUMaxOption() != NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_CapUMinTess() );
    }
    return true;
}

void MdcFinGeom::SetWedge( Wedge* wedge, double c, double zu, double zl, double mu, double ml, double fu, double fl )
{
    wedge->m_Chord.Set( c );
    wedge->m_SymmThick.Set( false );
    wedge->m_ThickChord.Set( zu + zl );
    wedge->m_ZCamber.SetLowerLimit( -1.0 );
    wedge->m_ZCamber.Set( 0.5 * ( zu - zl ) );
    wedge->m_ThickLoc.SetLowerLimit( 1.0e-3 );
    wedge->m_ThickLocLow.SetLowerLimit( 1.0e-3 );
    wedge->m_FlatUp.SetUpperLimit( 1.0 );
    wedge->m_FlatLow.SetUpperLimit( 1.0 );
    wedge->m_ThickLoc.Set( mu );
    wedge->m_ThickLocLow.Set( ml );
    wedge->m_FlatUp.Set( std::min( fu, 0.999 - mu ) );
    wedge->m_FlatLow.Set( std::min( fl, 0.999 - ml ) );
}

BareXSec* MdcFinGeom::WedgeSection( XSecSurf* surf, int k, double c )
{
    int i = k - 1;
    double zu = m_ZUpper[i]->Get();
    double mu = m_LMaxU[i]->Get();
    double fu = m_LFlatU[i]->Get();
    double zl = RelAbs( *m_ZLower[i], *m_ZLowerRatio[i], m_ZLowerRelFlag, zu );
    double ml = RelAbs( *m_LMaxL[i], *m_LMaxLRatio[i], m_LMaxLRelFlag, mu );
    double fl = RelAbs( *m_LFlatL[i], *m_LFlatLRatio[i], m_LFlatLRelFlag, fu );

    BareXSec* xs = AppendXSec( surf, XS_WEDGE, vec3d() );
    SetWedge( dynamic_cast< Wedge* >( xs->GetXSecCurve() ), c, zu, zl, mu, ml, fu, fl );
    return xs;
}

BareXSec* MdcFinGeom::NacaSection( XSecSurf* surf, double c )
{
    int ser = m_NacaSer();
    double des = m_NacaDes();
    int lead = int( floor( des / 100.0 ) );
    double thick = ( des - 100.0 * lead ) / 100.0;
    int mod = m_NacaMod();

    BareXSec* xs = nullptr;
    Airfoil* af = nullptr;
    if ( ser == NACA4 )
    {
        xs = AppendXSec( surf, XS_FOUR_SERIES, vec3d() );
        FourSeries* four = dynamic_cast< FourSeries* >( xs->GetXSecCurve() );
        four->m_Camber.Set( ( lead / 10 ) / 100.0 );
        four->m_CamberLoc.Set( std::max( ( lead % 10 ) / 10.0, 0.1 ) );
        af = four;
    }
    else if ( ser == NACA4M )
    {
        xs = AppendXSec( surf, XS_FOUR_DIGIT_MOD, vec3d() );
        FourDigMod* four = dynamic_cast< FourDigMod* >( xs->GetXSecCurve() );
        four->m_LERadIndx.Set( mod / 10 );
        four->m_ThickLoc.Set( ( mod % 10 ) / 10.0 );
        four->m_Camber.Set( ( lead / 10 ) / 100.0 );
        four->m_CamberLoc.Set( std::max( ( lead % 10 ) / 10.0, 0.1 ) );
        af = four;
    }
    else if ( ser == NACA5 )
    {
        xs = AppendXSec( surf, XS_FIVE_DIGIT, vec3d() );
        FiveDig* five = dynamic_cast< FiveDig* >( xs->GetXSecCurve() );
        five->m_IdealCl.Set( 0.15 * ( lead / 100 ) );
        five->m_CamberLoc.Set( ( lead % 100 ) / 200.0 );
        af = five;
    }
    else if ( ser == NACA5M )
    {
        xs = AppendXSec( surf, XS_FIVE_DIGIT_MOD, vec3d() );
        FiveDigMod* five = dynamic_cast< FiveDigMod* >( xs->GetXSecCurve() );
        five->m_LERadIndx.Set( mod / 10 );
        five->m_ThickLoc.Set( ( mod % 10 ) / 10.0 );
        five->m_IdealCl.Set( 0.15 * ( lead / 100 ) );
        five->m_CamberLoc.Set( ( lead % 100 ) / 200.0 );
        af = five;
    }
    else if ( ser == NACA1 )
    {
        xs = AppendXSec( surf, XS_ONE_SIX_SERIES, vec3d() );
        OneSixSeries* one_six = dynamic_cast< OneSixSeries* >( xs->GetXSecCurve() );
        one_six->m_IdealCl.Set( ( lead % 10 ) / 10.0 );
        af = one_six;
    }
    else if ( ser == NACA6 )
    {
        xs = AppendXSec( surf, XS_SIX_SERIES, vec3d() );
        SixSeries* six = dynamic_cast< SixSeries* >( xs->GetXSecCurve() );
        int pos = std::min( std::max( ( lead / 10 ) % 10, 3 ), 7 );
        int series = pos - 3;
        if ( m_NacaSea() )
        {
            series = 5 + std::min( std::max( pos - 3, 0 ), 2 );
        }
        six->m_Series.Set( series );
        six->m_IdealCl.Set( ( lead % 10 ) / 10.0 );
        six->m_A.Set( m_NacaMean() );
        af = six;
    }
    else if ( ser == NACAS2 )
    {
        // A supersonic section: thickness in %, its maximum at NACAXMAX % chord.
        xs = AppendXSec( surf, XS_BICONVEX, vec3d() );
        Biconvex* arc = dynamic_cast< Biconvex* >( xs->GetXSecCurve() );
        arc->m_ThickLoc.Set( 0.01 * m_NacaXMax() );
        arc->m_ThickLocLow.Set( 0.01 * m_NacaXMax() );
        arc->m_UseCircleArc.Set( true );
        arc->m_ThickChord.Set( 0.01 * des );
        arc->m_Chord.Set( c );
        return xs;
    }
    else
    {
        // A supersonic diamond or hexagon: thickness in %, its maximum at NACAXMAX % chord, and the hexagon's flat.
        double flat = 0.0;
        if ( ser == NACAS3 )
        {
            flat = 0.01 * m_NacaFlat();
        }
        double z = 0.005 * des;
        double xm = 0.01 * m_NacaXMax();
        xs = AppendXSec( surf, XS_WEDGE, vec3d() );
        SetWedge( dynamic_cast< Wedge* >( xs->GetXSecCurve() ), c, z, z, xm, xm, flat, flat );
        return xs;
    }

    af->m_ThickChord.Set( thick );
    af->m_Chord.Set( c );
    return xs;
}

BareXSec* MdcFinGeom::BiconvexSection( XSecSurf* surf, int k, double c )
{
    int i = k - 1;
    double zu = m_ZUpper[i]->Get();
    double zl = RelAbs( *m_ZLower[i], *m_ZLowerRatio[i], m_ZLowerRelFlag, zu );

    BareXSec* xs = AppendXSec( surf, XS_BICONVEX, vec3d() );
    Biconvex* arc = dynamic_cast< Biconvex* >( xs->GetXSecCurve() );
    arc->m_UseCircleArc.Set( true );
    arc->m_Chord.Set( c );
    arc->m_ThickChord.Set( zu + zl );
    if ( zu + zl > 0.0 )
    {
        arc->m_ThickFracUp.Set( zu / ( zu + zl ) );
    }
    else
    {
        arc->m_ThickFracUp.Set( 0.5 );
    }
    return xs;
}

// LER rounds the leading edge of the airfoil, as LERType says.
void MdcFinGeom::RoundLeadingEdge( BareXSec* xs, int k, double c )
{
    int i = k - 1;
    double r = 0.0;
    if ( m_LERType() != LER_SHARP && c > 0.0 )
    {
        r = m_LER[i]->Get() / c;
    }
    int keep = vsp::LE_RADIUS_KEEP_CREST;
    if ( m_LERType() == LER_KEEP_ANGLE )
    {
        keep = vsp::LE_RADIUS_KEEP_ANGLE;
    }

    // The leading edge is the airfoil's own, with no trim or cap.
    XSecCurve* crv = xs->GetXSecCurve();
    crv->m_LETrimType.Set( TRIM_NONE );
    crv->m_LECapType.Set( FLAT_END_CAP );

    Wedge* wedge = dynamic_cast< Wedge* >( crv );
    if ( wedge )
    {
        wedge->m_LERadiusType.Set( keep );
        wedge->m_LERadius.Set( r );
    }
    Biconvex* arc = dynamic_cast< Biconvex* >( crv );
    if ( arc )
    {
        arc->m_LERadiusType.Set( keep );
        arc->m_LERadius.Set( r );
    }
}

BareXSec* MdcFinGeom::StationSection( XSecSurf* surf, int k, double c, int sectyp )
{
    if ( sectyp == HEX )
    {
        BareXSec* xs = WedgeSection( surf, k, c );
        RoundLeadingEdge( xs, k, c );
        return xs;
    }
    if ( sectyp == ARC )
    {
        BareXSec* xs = BiconvexSection( surf, k, c );
        RoundLeadingEdge( xs, k, c );
        return xs;
    }
    if ( sectyp == NACA )
    {
        return NacaSection( surf, c );
    }
    BareXSec* xs = AppendXSec( surf, XS_FILE_AIRFOIL, vec3d() );
    FileAirfoil* to = dynamic_cast< FileAirfoil* >( xs->GetXSecCurve() );
    FileAirfoil* from = GetUserAirfoil();
    if ( to && from )
    {
        to->SetAirfoilName( from->GetAirfoilName() );
        to->SetAirfoilPnts( from->GetUpperPnts(), from->GetLowerPnts() );
    }
    to->m_Chord.Set( c );
    return xs;
}

void MdcFinGeom::BuryShape( BareXSec* root, const vector< double > & span, const vector< double > & chord,
                            const vector< double > & xle, const vector< double > & phif, double & depth, double & cb,
                            double & xb )
{
    // The root section's thickness is across its smaller extent, from its chord line at zero.
    BndBox bb;
    root->GetXSecCurve()->GetCurve().GetBoundingBox( bb );
    int across = 1;
    if ( bb.GetMax( 0 ) - bb.GetMin( 0 ) < bb.GetMax( 1 ) - bb.GetMin( 1 ) )
    {
        across = 0;
    }
    double c0 = chord[0];
    double ratio = std::max( fabs( bb.GetMin( across ) ), fabs( bb.GetMax( across ) ) ) / c0;

    // The chord's and leading edge's change per unit of span into the body.
    double dc = 0.0;
    double dx = 0.0;
    if ( m_BuryRootType() == BURY_EXTRAPOLATE && span.size() > 1 && span[1] - span[0] > 1.0e-12 )
    {
        dc = ( chord[0] - chord[1] ) / ( span[1] - span[0] );
        dx = -( xle[1] - xle[0] ) / ( span[1] - span[0] );
    }

    double rmax = 0.0;
    for ( int p = 0; p < ( int )m_RootRadiusVec.size(); p++ )
    {
        rmax = std::max( rmax, m_RootRadiusVec[p] );
    }

    // The section's place moves with its depth where it is extrapolated, so the depth is found again from it until
    // it no longer grows.
    depth = 0.0;
    for ( int it = 0; it < 8; it++ )
    {
        cb = std::max( c0 + dc * depth, 0.01 * c0 );
        xb = xle[0] + dx * depth;
        double half = ratio * cb;
        double fall = 0.0;
        for ( int p = 0; p < ( int )m_RootRadiusVec.size(); p++ )
        {
            double rr = m_RootRadiusVec[p];
            double gam = m_Gam[p]->Get();
            fall = std::max( fall, RootFall( xb, phif[p], gam, rr, half ) );
            fall = std::max( fall, RootFall( xb + cb, phif[p], gam, rr, half ) );
        }
        double next = fall + 0.01 * rmax;
        if ( next <= depth * ( 1.0 + 1.0e-9 ) )
        {
            break;
        }
        depth = next;
    }
    cb = std::max( c0 + dc * depth, 0.01 * c0 );
    xb = xle[0] + dx * depth;
}

double MdcFinGeom::RootFall( double x, double phi, double gam, double rr, double half )
{
    // The root at rr along phi, across the body; the panel out along psi, and its thickness square to that.
    double p = phi * M_PI / 180.0;
    double psi = ( phi + gam ) * M_PI / 180.0;
    double ry = rr * sin( p );
    double rz = rr * cos( p );
    double sy = sin( psi );
    double sz = cos( psi );

    // Along the panel the side's line crosses the body as a chord, so how far outside it its point is falls to a least
    // value and rises again.  The fall is the first t inside, before that least; where the line misses the body, the
    // root cannot be buried and the fall is the t of that least, which joins the other where the line just touches.
    const double golden = 0.5 * ( sqrt( 5.0 ) - 1.0 );
    double fall = 0.0;
    for ( int side = -1; side <= 1; side += 2 )
    {
        double off = side * half;
        if ( Outside( x, ry, rz, sy, sz, off, 0.0 ) <= 0.0 )
        {
            continue;
        }

        double a = 0.0;
        double b = 2.0 * rr;
        double c = b - golden * ( b - a );
        double d = a + golden * ( b - a );
        double fc = Outside( x, ry, rz, sy, sz, off, c );
        double fd = Outside( x, ry, rz, sy, sz, off, d );
        for ( int it = 0; it < 80; it++ )
        {
            if ( fc < fd )
            {
                b = d;
                d = c;
                fd = fc;
                c = b - golden * ( b - a );
                fc = Outside( x, ry, rz, sy, sz, off, c );
            }
            else
            {
                a = c;
                c = d;
                fc = fd;
                d = a + golden * ( b - a );
                fd = Outside( x, ry, rz, sy, sz, off, d );
            }
        }
        double tleast = 0.5 * ( a + b );
        if ( Outside( x, ry, rz, sy, sz, off, tleast ) > 0.0 )
        {
            fall = std::max( fall, tleast );
            continue;
        }

        double lo = 0.0;
        double hi = tleast;
        for ( int it = 0; it < 60; it++ )
        {
            double mid = 0.5 * ( lo + hi );
            if ( Outside( x, ry, rz, sy, sz, off, mid ) > 0.0 )
            {
                lo = mid;
            }
            else
            {
                hi = mid;
            }
        }
        fall = std::max( fall, hi );
    }
    return fall;
}

double MdcFinGeom::Outside( double x, double ry, double rz, double sy, double sz, double off, double t )
{
    double y = ry - t * sy + off * sz;
    double z = rz - t * sz - off * sy;
    double r = hypot( y, z );
    double rb = hypot( ry, rz );
    MdcBodGeom::RadiusUnder( this, x, atan2( y, z ) * 180.0 / M_PI, rb );
    return r - rb;
}

void MdcFinGeom::PanelAngles( int npanel, vector< double > & phif )
{
    SpaceRolls( m_PhiFFlag(), npanel, m_PhiF );
    for ( int k = 1; k <= npanel; k++ )
    {
        phif.push_back( m_PhiF[k - 1]->Get() );
    }
}

double MdcFinGeom::RootRadius( int panel ) const
{
    if ( panel >= 0 && panel < ( int )m_RootRadiusVec.size() )
    {
        return m_RootRadiusVec[panel];
    }
    if ( m_SSpan[0]->Get() > 1.0e-12 )
    {
        return m_SSpan[0]->Get();
    }
    return m_RBody();
}

bool MdcFinGeom::Reflected( double phi )
{
    double p = phi - 360.0 * floor( phi / 360.0 );
    return p > 180.0 + 1.0e-9 || p < 1.0e-9 || p > 360.0 - 1.0e-9;
}

void MdcFinGeom::UpdateSurf()
{
    SizeParms();

    int sectyp = m_SecTyp();
    int npanel = m_NPanel();
    int nspan = m_NSpan();

    // The selectors stop at the last station and panel.
    m_Station.SetUpperLimit( nspan );
    m_Station.Set( std::min( m_Station(), nspan ) );
    m_Panel.SetUpperLimit( npanel );
    m_Panel.Set( std::min( m_Panel(), npanel ) );

    // The stations' span from the root, the chords and the leading edges.
    vector< double > span;
    vector< double > chord;
    vector< double > xle;
    double offset = 0.0;
    if ( m_SSpan[0]->Get() > 1.0e-12 )
    {
        offset = m_SSpan[0]->Get();
    }
    for ( int i = 0; i < nspan; i++ )
    {
        double s = m_SSpan[i]->Get() - offset;
        if ( i > 0 )
        {
            s = std::max( s, span[i - 1] + 1.0e-6 );
        }
        span.push_back( s );
        chord.push_back( m_Chord[i]->Get() );
    }
    xle.push_back( 0.0 );
    for ( int i = 0; i + 1 < nspan; i++ )
    {
        double f = m_Sta[i]->Get();
        double dy = span[i + 1] - span[i];
        double xf = xle[i] + f * chord[i];
        if ( m_XLEFlag[i]->Get() )
        {
            xle.push_back( m_XLE[i]->Get() );
            double dx = xle[i + 1] + f * chord[i + 1] - xf;
            m_Sweep[i]->Set( atan2( dx, dy ) * 180.0 / M_PI );
        }
        else
        {
            xle.push_back( xf + dy * tan( m_Sweep[i]->Get() * M_PI / 180.0 ) - f * chord[i + 1] );
            m_XLE[i]->Set( xle[i + 1] );
        }
    }

    vector< double > phif;
    PanelAngles( npanel, phif );

    // Each panel's root's distance from the axis: SSPAN_1 where it is not zero.  Else a parent MDC_BOD's radius
    // under the root chord's middle at the panel's roll, as DATCOM takes it from the body, which RBODY shows; else
    // RBODY.
    m_RootRadiusVec.assign( npanel, m_RBody() );
    bool attached = true;
    for ( int p = 0; p < npanel && attached; p++ )
    {
        attached = MdcBodGeom::RadiusUnder( this, 0.5 * chord[0], phif[p], m_RootRadiusVec[p] );
    }
    if ( attached )
    {
        m_RBody.Set( m_RootRadiusVec[0] );
        m_RBody.Deactivate();
    }
    else
    {
        m_RootRadiusVec.assign( npanel, m_RBody() );
        m_RBody.Activate();
    }
    if ( offset > 0.0 )
    {
        m_RootRadiusVec.assign( npanel, offset );
    }

    // A root on the body carried into it is a section like the root's before it, placed once its depth is known.
    bool bury = m_BuryRoot() && offset == 0.0 && chord[0] > 1.0e-9;
    m_RootDepth = 0.0;
    m_BuriedChord = 0.0;
    double xb = 0.0;

    BeginXSecSurfs();
    for ( int p = 0; p < npanel; p++ )
    {
        XSecSurf* surf = NewXSecSurf();
        bool reflect = Reflected( phif[p] );

        BareXSec* buried = nullptr;
        if ( bury )
        {
            buried = StationSection( surf, 1, chord[0], sectyp );
            dynamic_cast< Airfoil* >( buried->GetXSecCurve() )->m_Invert.Set( !reflect );
            buried->SetContinuity( 0 );
        }

        BareXSec* rootxs = nullptr;
        for ( int i = 0; i < nspan; i++ )
        {
            BareXSec* xs = nullptr;
            if ( chord[i] > 1.0e-9 )
            {
                xs = StationSection( surf, i + 1, chord[i], sectyp );
                dynamic_cast< Airfoil* >( xs->GetXSecCurve() )->m_Invert.Set( !reflect );
            }
            else
            {
                xs = AppendPointXSec( surf, vec3d() );
            }
            xs->SetLoc( vec3d( span[i], xle[i] + 0.5 * chord[i], 0.0 ) );
            xs->SetContinuity( 0 );
            if ( i == 0 )
            {
                rootxs = xs;
            }
        }

        // The buried section was made with the root's chord; where it has another it is made again with that.
        if ( buried )
        {
            if ( p == 0 )
            {
                BuryShape( rootxs, span, chord, xle, phif, m_RootDepth, m_BuriedChord, xb );
            }
            if ( m_BuriedChord != chord[0] )
            {
                int next = SetNextXSec( surf, 0 );
                buried = StationSection( surf, 1, m_BuriedChord, sectyp );
                dynamic_cast< Airfoil* >( buried->GetXSecCurve() )->m_Invert.Set( !reflect );
                buried->SetContinuity( 0 );
                SetNextXSec( surf, next );
            }
            buried->SetLoc( vec3d( span[0] - m_RootDepth, xb + 0.5 * m_BuriedChord, 0.0 ) );
        }

        // The skin is along x with its sections' chords along y and thickness along z.  It is turned to the
        // fin's frame, span along y and thickness along z, rolled to the panel's direction, PHIF + GAM clockwise
        // from the top, and moved to its root on the body.
        double psi = phif[p] + m_Gam[p]->Get();
        Matrix4d mat;
        mat.loadIdentity();
        double root = m_RootRadiusVec[p];
        mat.translatef( 0.0, root * sin( phif[p] * M_PI / 180.0 ), root * cos( phif[p] * M_PI / 180.0 ) );
        mat.rotateX( 90.0 - psi );
        mat.rotateX( 180.0 );
        mat.rotateZ( -90.0 );
        surf->SetGlobalXForm( mat );
    }

    // Panels are closed by flat caps, or by none where they come to a point.
    if ( chord[nspan - 1] > 1.0e-9 )
    {
        m_CapUMaxOption.Set( FLAT_END_CAP );
    }
    else
    {
        m_CapUMaxOption.Set( NO_END_CAP );
    }
    if ( chord[0] > 1.0e-9 )
    {
        m_CapUMinOption.Set( FLAT_END_CAP );
    }
    else
    {
        m_CapUMinOption.Set( NO_END_CAP );
    }

    SetVspSurfType( WING_SURF );
    EndXSecSurfs();
    SkinXSecSurf( false );

    UpdateFlaps( nspan, chord );
}

void MdcFinGeom::UpdateFlaps( int nspan, const vector< double > & chord )
{
    int caps = 0;
    if ( chord[0] > 1.0e-9 )
    {
        caps++;
    }
    if ( chord[nspan - 1] > 1.0e-9 )
    {
        caps++;
    }
    int buried = 0;
    if ( m_RootDepth > 0.0 )
    {
        buried = 1;
    }
    double patches = nspan - 1 + caps + buried;
    double first = buried;
    if ( chord[0] > 1.0e-9 )
    {
        first += 1.0;
    }

    vector< bool > wanted;
    for ( int k = 1; k < nspan; k++ )
    {
        wanted.push_back( m_CFOC[0]->Get() < 1.0 && m_CFOC[k - 1]->Get() > 0.0 && m_CFOC[k]->Get() > 0.0 );
    }

    // The flaps no longer wanted, deleted, and a place for each segment's.
    for ( int k = 1; k <= ( int )m_FlapIDVec.size(); k++ )
    {
        bool keep = k < nspan && wanted[k - 1];
        if ( !keep && GetSubSurf( m_FlapIDVec[k - 1] ) )
        {
            DelSubSurf( GetSubSurfIndex( m_FlapIDVec[k - 1] ) );
        }
        if ( !keep )
        {
            m_FlapIDVec[k - 1] = string();
        }
    }
    m_FlapIDVec.resize( nspan - 1 );

    for ( int k = 1; k < nspan; k++ )
    {
        if ( !wanted[k - 1] )
        {
            continue;
        }

        SubSurface* ss = GetSubSurf( m_FlapIDVec[k - 1] );
        if ( !ss )
        {
            ss = AddSubSurf( SS_CONTROL, 0 );
            if ( ss )
            {
                ss->SetName( "FLAP_" + to_string( k ) );
                m_FlapIDVec[k - 1] = ss->GetID();
            }
        }

        SSControlSurf* cs = dynamic_cast< SSControlSurf* >( ss );
        if ( !cs )
        {
            continue;
        }

        cs->m_MainSurfIndx.Set( -1 );
        cs->m_ConstFlag.Set( false );
        cs->m_LEFlag.Set( false );
        cs->m_SurfType.Set( vsp::SS_CONTROL_BOTH_SURF );
        cs->m_UStart.Set( ( first + k - 1 ) / patches );
        cs->m_UEnd.Set( ( first + k ) / patches );
        cs->m_StartLenFrac.Set( m_CFOC[k - 1]->Get() );
        cs->m_EndLenFrac.Set( m_CFOC[k]->Get() );
    }
}
