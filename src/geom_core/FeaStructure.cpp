//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaStructure.cpp
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#include "FeaStructure.h"

#include "Vehicle.h"
#include "ParmMgr.h"
#include "StructureMgr.h"
#include "LinkMgr.h"
#include "WingGeom.h"
#include "ConformalGeom.h"

#include <cfloat>

//////////////////////////////////////////////////////
//================== FeaStructure ==================//
//////////////////////////////////////////////////////

FeaStructure::FeaStructure( string geomID, int surf_index )
{
    m_ParentGeomID = geomID;
    m_MainSurfIndx = surf_index;

    m_FeaPartCount = 0;
    m_FeaSubSurfCount = 0;

    LinkMgr.RegisterContainer( m_StructSettings.GetID() );
    LinkMgr.RegisterContainer( m_FeaGridDensity.GetID() );
}

FeaStructure::~FeaStructure()
{
    // Delete FeaParts
    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        delete m_FeaPartVec[i];
    }
    m_FeaPartVec.clear();

    // Delete SubSurfaces
    for ( int i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
    {
        delete m_FeaSubSurfVec[i];
    }
    m_FeaSubSurfVec.clear();
}

void FeaStructure::Update()
{
    UpdateFeaParts();
    UpdateFeaSubSurfs();
}

xmlNodePtr FeaStructure::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_info = xmlNewChild( node, NULL, BAD_CAST "FeaStructureInfo", NULL );

    XmlUtil::AddStringNode( fea_info, "StructureName", m_FeaStructName );
    XmlUtil::AddStringNode( fea_info, "ParentGeomID", m_ParentGeomID );
    XmlUtil::AddIntNode( fea_info, "MainSurfIndx", m_MainSurfIndx );

    for ( unsigned int i = 0; i < m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->EncodeXml( fea_info );
    }

    for ( unsigned int i = 0; i < m_FeaSubSurfVec.size(); i++ )
    {
        xmlNodePtr sub_node = xmlNewChild( fea_info, NULL, BAD_CAST "FeaSubSurface", NULL );

        if ( sub_node )
        {
            m_FeaSubSurfVec[i]->EncodeXml( sub_node );
        }
    }

    m_StructSettings.EncodeXml( fea_info );
    m_FeaGridDensity.EncodeXml( fea_info );

    return fea_info;
}

xmlNodePtr FeaStructure::DecodeXml( xmlNodePtr & node )
{
    int numparts = XmlUtil::GetNumNames( node, "FeaPartInfo" );

    for ( unsigned int i = 0; i < numparts; i++ )
    {
        xmlNodePtr part_info = XmlUtil::GetNode( node, "FeaPartInfo", i );

        if ( part_info )
        {
            int type = XmlUtil::FindInt( part_info, "FeaPartType", 0 );

            if ( type != vsp::FEA_SKIN )
            {
                FeaPart* feapart = AddFeaPart( type );
                feapart->DecodeXml( part_info );
            }
            else
            {
                FeaPart* feaskin = new FeaSkin( m_ParentGeomID );
                feaskin->DecodeXml( part_info );
                m_FeaPartVec.push_back( feaskin );
            }
        }
    }

    int num_ss = XmlUtil::GetNumNames( node, "FeaSubSurface" );

    for ( int ss = 0; ss < num_ss; ss++ )
    {
        xmlNodePtr ss_node = XmlUtil::GetNode( node, "FeaSubSurface", ss );
        if ( ss_node )
        {
            xmlNodePtr ss_info_node = XmlUtil::GetNode( ss_node, "SubSurfaceInfo", 0 );
            if ( ss_info_node )
            {
                int type = XmlUtil::FindInt( ss_info_node, "Type", vsp::SS_LINE );

                SubSurface* ssurf = AddFeaSubSurf( type );
                if ( ssurf )
                {
                    ssurf->DecodeXml( ss_node );
                }
            }
        }
    }

    return node;
}

void FeaStructure::SetDrawFlag( bool flag )
{
    for ( size_t i = 0; i < m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->m_DrawFeaPartFlag.Set( flag );
    }

    for ( size_t i = 0; i < m_FeaSubSurfVec.size(); i++ )
    {
        m_FeaSubSurfVec[i]->m_DrawFeaPartFlag.Set( flag );
    }
}

FeaPart* FeaStructure::AddFeaPart( int type )
{
    FeaPart* feaprt = new FeaPart( m_ParentGeomID, type );

    if ( type == vsp::FEA_SLICE )
    {
        feaprt = new FeaSlice( m_ParentGeomID );
        feaprt->SetName( string( "Slice" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_RIB )
    {
        feaprt = new FeaRib( m_ParentGeomID );
        feaprt->SetName( string( "Rib" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_SPAR )
    {
        feaprt = new FeaSpar( m_ParentGeomID );
        feaprt->SetName( string( "Spar" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_FIX_POINT )
    {
        // Initially define the FeaFixPoint on the skin surface
        FeaPart* skin = GetFeaSkin();

        if ( skin )
        {
            feaprt = new FeaFixPoint( m_ParentGeomID, skin->GetID() );
            feaprt->SetName( string( "FixPoint" + std::to_string( m_FeaPartCount ) ) );
        }
    }
    else if ( type == vsp::FEA_DOME )
    {
        feaprt = new FeaDome( m_ParentGeomID );
        feaprt->SetName( string( "Dome" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_RIB_ARRAY )
    {
        feaprt = new FeaRibArray( m_ParentGeomID );
        feaprt->SetName( string( "RibArray" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_SLICE_ARRAY )
    {
        feaprt = new FeaSliceArray( m_ParentGeomID );
        feaprt->SetName( string( "SliceArray" + std::to_string( m_FeaPartCount ) ) );
    }

    if ( feaprt )
    {
        feaprt->m_MainSurfIndx.Set( m_MainSurfIndx );
        AddFeaPart( feaprt );
    }

    m_FeaPartCount++;

    return feaprt;
}

void FeaStructure::DelFeaPart( int ind )
{
    if ( ValidFeaPartInd( ind ) )
    {
        delete m_FeaPartVec[ind];
        m_FeaPartVec.erase( m_FeaPartVec.begin() + ind );
    }
}

void FeaStructure::ReorderFeaPart( int ind, int action )
{
    //==== Check SubSurface Index Validity ====//
    if ( !ValidFeaPartInd( ind ) )
    {
        return;
    }

    vector < FeaPart* > new_prt_vec;

    if ( action == Vehicle::REORDER_MOVE_TOP || action == Vehicle::REORDER_MOVE_BOTTOM )
    {
        if ( action == Vehicle::REORDER_MOVE_TOP )
        {
            new_prt_vec.push_back( GetFeaPart( ind ) );
        }

        for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
            if ( m_FeaPartVec[i] != GetFeaPart( ind ) )
            {
                new_prt_vec.push_back( m_FeaPartVec[i] );
            }

        if ( action == Vehicle::REORDER_MOVE_BOTTOM )
        {
            new_prt_vec.push_back( GetFeaPart( ind ) );
        }
    }
    else if ( action == Vehicle::REORDER_MOVE_UP || action == Vehicle::REORDER_MOVE_DOWN )
    {
        for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
        {
            if ( i < (int)( m_FeaPartVec.size() - 1 ) &&
                ( ( action == Vehicle::REORDER_MOVE_DOWN && m_FeaPartVec[i] == GetFeaPart( ind ) ) ||
                 ( action == Vehicle::REORDER_MOVE_UP   && m_FeaPartVec[i + 1] == GetFeaPart( ind ) ) ) )
            {
                new_prt_vec.push_back( m_FeaPartVec[i + 1] );
                new_prt_vec.push_back( m_FeaPartVec[i] );
                i++;
            }
            else
            {
                new_prt_vec.push_back( m_FeaPartVec[i] );
            }
        }
    }

    m_FeaPartVec = new_prt_vec;
}

//==== Highlight Active Subsurface ====//
void FeaStructure::RecolorFeaSubSurfs( vector < int > active_ind_vec )
{
    for ( int i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
    {
        m_FeaSubSurfVec[i]->SetLineColor( vec3d( 0, 0, 0 ) ); // Initially color all black
    }

    for ( size_t j = 0; j < active_ind_vec.size(); j++ )
    {
        for ( int i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
        {
            if ( i == active_ind_vec[j] )
            {
                m_FeaSubSurfVec[i]->SetLineColor( vec3d( 1, 0, 0 ) );
            }
        }
    }
}

SubSurface* FeaStructure::AddFeaSubSurf( int type )
{
    SubSurface* ssurf = NULL;

    if ( type == vsp::SS_LINE )
    {
        ssurf = new SSLine( m_ParentGeomID );
        ssurf->SetName( string( "SSLine" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_RECTANGLE )
    {
        ssurf = new SSRectangle( m_ParentGeomID );
        ssurf->SetName( string( "SSRect" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_ELLIPSE )
    {
        ssurf = new SSEllipse( m_ParentGeomID );
        ssurf->SetName( string( "SSEllipse" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_CONTROL )
    {
        ssurf = new SSControlSurf( m_ParentGeomID );
        ssurf->SetName( string( "SSConSurf" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_LINE_ARRAY )
    {
        ssurf = new SSLineArray( m_ParentGeomID );
        ssurf->SetName( string( "SSLineArray" + to_string( m_FeaSubSurfCount ) ) );
    }

    if ( ssurf )
    {
        ssurf->m_MainSurfIndx.Set( m_MainSurfIndx );
        m_FeaSubSurfVec.push_back( ssurf );
    }

    m_FeaSubSurfCount++;

    return ssurf;
}

bool FeaStructure::ValidFeaSubSurfInd( int ind )
{
    if ( (int)m_FeaSubSurfVec.size() > 0 && ind >= 0 && ind < (int)m_FeaSubSurfVec.size() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void FeaStructure::DelFeaSubSurf( int ind )
{
    if ( ValidFeaSubSurfInd( ind ) )
    {
        delete m_FeaSubSurfVec[ind];
        m_FeaSubSurfVec.erase( m_FeaSubSurfVec.begin() + ind );
    }
}

SubSurface* FeaStructure::GetFeaSubSurf( int ind )
{
    if ( ValidFeaSubSurfInd( ind ) )
    {
        return m_FeaSubSurfVec[ind];
    }
    return NULL;
}

void FeaStructure::ReorderFeaSubSurf( int ind, int action )
{
    //==== Check SubSurface Index Validity ====//
    if ( !ValidFeaSubSurfInd( ind ) )
    {
        return;
    }

    vector < SubSurface* > new_ss_vec;

    if ( action == Vehicle::REORDER_MOVE_TOP || action == Vehicle::REORDER_MOVE_BOTTOM )
    {
        if ( action == Vehicle::REORDER_MOVE_TOP )
        {
            new_ss_vec.push_back( GetFeaSubSurf( ind ) );
        }

        for ( int i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
            if ( m_FeaSubSurfVec[i] != GetFeaSubSurf( ind ) )
            {
                new_ss_vec.push_back( m_FeaSubSurfVec[i] );
            }

        if ( action == Vehicle::REORDER_MOVE_BOTTOM )
        {
            new_ss_vec.push_back( GetFeaSubSurf( ind ) );
        }
    }
    else if ( action == Vehicle::REORDER_MOVE_UP || action == Vehicle::REORDER_MOVE_DOWN )
    {
        for ( int i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
        {
            if ( i < (int)( m_FeaSubSurfVec.size() - 1 ) &&
                ( ( action == Vehicle::REORDER_MOVE_DOWN && m_FeaSubSurfVec[i] == GetFeaSubSurf( ind ) ) ||
                 ( action == Vehicle::REORDER_MOVE_UP   && m_FeaSubSurfVec[i + 1] == GetFeaSubSurf( ind ) ) ) )
            {
                new_ss_vec.push_back( m_FeaSubSurfVec[i + 1] );
                new_ss_vec.push_back( m_FeaSubSurfVec[i] );
                i++;
            }
            else
            {
                new_ss_vec.push_back( m_FeaSubSurfVec[i] );
            }
        }
    }

    m_FeaSubSurfVec = new_ss_vec;
}

bool FeaStructure::ValidFeaPartInd( int ind )
{
    if ( (int)m_FeaPartVec.size() > 0 && ind >= 0 && ind < (int)m_FeaPartVec.size() )
    {
        return true;
    }
    return false;
}

void FeaStructure::UpdateFeaParts()
{
    for ( unsigned int i = 0; i < m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->UpdateSymmIndex();
        m_FeaPartVec[i]->Update();

        if ( !FeaPartIsFixPoint( i ) && !FeaPartIsArray( i ) )
        {
            // Symmetric FixedPoints and Arrays are updated in their respective Update functions
            m_FeaPartVec[i]->UpdateSymmParts();
        }
    }
}

void FeaStructure::UpdateFeaSubSurfs()
{
    for ( unsigned int i = 0; i < m_FeaSubSurfVec.size(); i++ )
    {
        m_FeaSubSurfVec[i]->Update();
    }
}

vector < FeaPart* > FeaStructure::InitFeaSkin()
{
    m_FeaPartVec.clear();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* currgeom = veh->FindGeom( m_ParentGeomID );

        if ( currgeom )
        {
            FeaPart* feaskin;
            feaskin = new FeaSkin( m_ParentGeomID );

            if ( feaskin )
            {
                feaskin->SetName( string( "Skin" ) );
                feaskin->m_MainSurfIndx.Set( m_MainSurfIndx );
                
                feaskin->UpdateSymmIndex();
                feaskin->Update();
                feaskin->UpdateSymmParts();

                m_FeaPartVec.push_back( feaskin );
            }
        }
    }

    return m_FeaPartVec;
}

FeaPart* FeaStructure::GetFeaPart( int ind )
{
    if ( ValidFeaPartInd( ind ) )
    {
        return m_FeaPartVec[ind];
    }
    return NULL;
}

string FeaStructure::GetFeaPartName( int ind )
{
    string name;
    FeaPart* fea_part = GetFeaPart( ind );

    if ( fea_part )
    {
        name = fea_part->GetName();
    }
    return name;
}

bool FeaStructure::FeaPartIsFixPoint( int ind )
{
    FeaPart* fea_part = GetFeaPart( ind );

    if ( fea_part )
    {
        if ( fea_part->GetType() == vsp::FEA_FIX_POINT )
        {
            return true;
        }
    }
    return false;
}

int FeaStructure::GetNumFeaFixPoints()
{
    int fix_point_count = 0;

    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( FeaPartIsFixPoint( i ) )
        {
            fix_point_count++;
        }
    }
    return fix_point_count;
}

bool FeaStructure::FeaPartIsArray( int ind )
{
    FeaPart* fea_part = GetFeaPart( ind );

    if ( fea_part )
    {
        if ( fea_part->GetType() == vsp::FEA_RIB_ARRAY || fea_part->GetType() == vsp::FEA_SLICE_ARRAY )
        {
            return true;
        }
    }
    return false;
}

void FeaStructure::IndividualizeRibArray( int rib_array_ind )
{
    if ( !ValidFeaPartInd( rib_array_ind ) )
    {
        return;
    }

    FeaPart* prt = m_FeaPartVec[rib_array_ind];

    if ( !prt )
    {
        return;
    }

    if ( prt->GetType() == vsp::FEA_RIB_ARRAY )
    {
        FeaRibArray* rib_array = dynamic_cast<FeaRibArray*>( prt );
        assert( rib_array );

        double dir = 1;
        if ( !rib_array->m_PositiveDirectionFlag() )
        {
            dir = -1;
        }

        for ( size_t i = 0; i < rib_array->GetNumRibs(); i++ )
        {
            double center_location;

            if ( rib_array->m_AbsRelParmFlag() == vsp::REL )
            {
                center_location = rib_array->m_RelStartLocation() + dir * i * rib_array->m_RibRelSpacing();
            }
            else if ( rib_array->m_AbsRelParmFlag() == vsp::ABS )
            {
                center_location = rib_array->m_AbsStartLocation() + dir * i * rib_array->m_RibAbsSpacing();
            }

            FeaRib* rib = rib_array->AddFeaRib( center_location, i );
            AddFeaPart( rib );
        }

        DelFeaPart( rib_array_ind );
    }
}

void FeaStructure::IndividualizeSliceArray( int stiffener_array_ind )
{
    if ( !ValidFeaPartInd( stiffener_array_ind ) )
    {
        return;
    }

    FeaPart* prt = m_FeaPartVec[stiffener_array_ind];

    if ( !prt )
    {
        return;
    }

    if ( prt->GetType() == vsp::FEA_SLICE_ARRAY )
    {
        FeaSliceArray* slice_array = dynamic_cast<FeaSliceArray*>( prt );
        assert( slice_array );

        double dir = 1;
        if ( !slice_array->m_PositiveDirectionFlag() )
        {
            dir = -1;
        }

        for ( size_t i = 0; i < slice_array->GetNumSlices(); i++ )
        {
            double center_location;

            if ( slice_array->m_AbsRelParmFlag() == vsp::REL )
            {
                center_location = slice_array->m_RelStartLocation() + dir * i * slice_array->m_SliceRelSpacing();
            }
            else if ( slice_array->m_AbsRelParmFlag() == vsp::ABS )
            {
                center_location = slice_array->m_AbsStartLocation() + dir * i * slice_array->m_SliceAbsSpacing();
            }

            FeaSlice* slice = slice_array->AddFeaSlice( center_location, i );
            AddFeaPart( slice );
        }

        DelFeaPart( stiffener_array_ind );
    }
}

void FeaStructure::IndividualizeSSLineArray( int ssline_array_ind )
{
    if ( !ValidFeaSubSurfInd( ssline_array_ind ) )
    {
        return;
    }

    SubSurface* sub_surf = m_FeaSubSurfVec[ssline_array_ind];

    if ( !sub_surf )
    {
        return;
    }

    if ( sub_surf->GetType() == vsp::SS_LINE_ARRAY )
    {
        SSLineArray* ssline_array = dynamic_cast<SSLineArray*>( sub_surf );
        assert( ssline_array );

        double dir = 1;
        if ( !ssline_array->m_PositiveDirectionFlag() )
        {
            dir = -1;
        }

        for ( size_t i = 0; i < ssline_array->GetNumLines(); i++ )
        {
            double center_location = ssline_array->m_StartLocation() + dir * i * ssline_array->m_Spacing();

            SSLine* ssline = ssline_array->AddSSLine( center_location, i );
            AddFeaSubSurf( ssline );
        }

        DelFeaSubSurf( ssline_array_ind );
    }
}

FeaPart* FeaStructure::GetFeaSkin()
{
    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( m_FeaPartVec[i]->GetType() == vsp::FEA_SKIN )
        {
            return m_FeaPartVec[i];
        }
    }
    return NULL;
}

//==== Get FeaProperty Index from FeaPart Index =====//
int FeaStructure::GetFeaPropertyIndex( int fea_part_ind )
{
    if ( ValidFeaPartInd( fea_part_ind ) )
    {
        FeaPart* fea_part = GetFeaPart( fea_part_ind );
        if ( fea_part )
        {
            return fea_part->m_FeaPropertyIndex();
        }
    }
    return -1; // indicates an error
}

//==== Get Cap FeaProperty Index from FeaPart Index =====//
int FeaStructure::GetCapFeaPropertyIndex( int fea_part_ind )
{
    if ( ValidFeaPartInd( fea_part_ind ) )
    {
        FeaPart* fea_part = GetFeaPart( fea_part_ind );
        if ( fea_part )
        {
            return fea_part->m_CapFeaPropertyIndex();
        }
    }
    return -1; // indicates an error
}

int FeaStructure::GetFeaPartIndex( FeaPart* fea_prt )
{
    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( m_FeaPartVec[i] == fea_prt )
        {
            return i;
        }
    }
    return -1; // indicates an error
}

void FeaStructure::BuildSuppressList()
{
    m_Usuppress.clear();
    m_Wsuppress.clear();

    FeaSkin* skin = NULL;
    FeaPart* pskin = GetFeaSkin();
    if ( pskin )
    {
        skin = dynamic_cast< FeaSkin* > ( pskin );

        if ( !skin )
        {
            return;
        }
    }

    VspSurf* surf = skin->GetMainSurf();
    if ( surf )
    {
        vector < double > ufeature = surf->GetUFeature();
        vector < double > wfeature = surf->GetWFeature();

        double umax = surf->GetUMax();
        double wmax = surf->GetWMax();

        for ( int i = 0; i < ufeature.size(); i++ )
        {
            int npts = 5;
            vector < vec3d > pnts( npts );

            for ( int j = 0; j < npts; j++ )
            {
                double w = wmax * j * 1.0 / ( npts - 1 );
                pnts[j] = surf->CompPnt( ufeature[i], w );
            }

            if ( PtsOnAnyPlanarPart( pnts ) )
            {
                m_Usuppress.push_back( ufeature[i] );
            }
        }

        for ( int i = 0; i < wfeature.size(); i++ )
        {
            int npts = 5;
            vector < vec3d > pnts( npts );

            for ( int j = 0; j < npts; j++ )
            {
                double u = umax * j * 1.0 / ( npts - 1 );
                pnts[j] = surf->CompPnt( u, wfeature[i] );
            }

            if ( PtsOnAnyPlanarPart( pnts ) )
            {
                m_Wsuppress.push_back( wfeature[i] );
            }
        }
    }
}

bool FeaStructure::PtsOnAnyPlanarPart( const vector < vec3d > &pnts )
{
    // Loop over all parts.
    for ( int i  = 0; i < m_FeaPartVec.size(); i++ )
    {
        FeaPart* p = m_FeaPartVec[i];
        if ( p )
        {
            if ( p->PtsOnPlanarPart( pnts ) )
            {
                return true;
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////
//==================== FeaPart =====================//
//////////////////////////////////////////////////////

FeaPart::FeaPart( string geomID, int type )
{
    m_FeaPartType = type;
    m_ParentGeomID = geomID;

    m_MainSurfIndx.Init( "MainSurfIndx", "FeaPart", this, -1, -1, 1e12 );
    m_MainSurfIndx.SetDescript( "Surface Index for FeaPart" );

    m_IncludedElements.Init( "IncludedElements", "FeaPart", this, vsp::FEA_SHELL, vsp::FEA_SHELL, vsp::FEA_SHELL_AND_BEAM );
    m_IncludedElements.SetDescript( "Indicates the FeaElements to be Included for the FeaPart" );

    m_DrawFeaPartFlag.Init( "DrawFeaPartFlag", "FeaPart", this, true, false, true );
    m_DrawFeaPartFlag.SetDescript( "Flag to Draw FeaPart" );

    m_AbsRelParmFlag.Init( "AbsRelParmFlag", "FeaPart", this, vsp::REL, vsp::ABS, vsp::REL );
    m_AbsRelParmFlag.SetDescript( "Parameterization of Center Location as Absolute or Relative" );

    m_AbsCenterLocation.Init( "AbsCenterLocation", "FeaPart", this, 0.0, 0.0, 1e12 );
    m_AbsCenterLocation.SetDescript( "The Absolute Location of the Center of the FeaPart" );

    m_RelCenterLocation.Init( "RelCenterLocation", "FeaPart", this, 0.5, 0.0, 1.0 );
    m_RelCenterLocation.SetDescript( "The Relative Location of the Center of the FeaPart" );

    m_FeaPropertyIndex.Init( "FeaPropertyIndex", "FeaPart", this, 0, 0, 1e12 );; // Shell property default
    m_FeaPropertyIndex.SetDescript( "FeaPropertyIndex for Shell Elements" );

    m_CapFeaPropertyIndex.Init( "CapFeaPropertyIndex", "FeaPart", this, 1, 0, 1e12 );; // Beam property default
    m_CapFeaPropertyIndex.SetDescript( "FeaPropertyIndex for Beam (Cap) Elements" );
}

FeaPart::~FeaPart()
{

}

void FeaPart::Update()
{

}

void FeaPart::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr FeaPart::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr part_info = xmlNewChild( node, NULL, BAD_CAST "FeaPartInfo", NULL );

    XmlUtil::AddIntNode( part_info, "FeaPartType", m_FeaPartType );

    return ParmContainer::EncodeXml( part_info );
}

xmlNodePtr FeaPart::DecodeXml( xmlNodePtr & node )
{
    return ParmContainer::DecodeXml( node );
}

void FeaPart::UpdateSymmParts()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_geom = veh->FindGeom( m_ParentGeomID );
        if ( !current_geom || m_FeaPartSurfVec.size() == 0 )
        {
            return;
        }

        vector< VspSurf > surf_vec;
        current_geom->GetSurfVec( surf_vec );

        // Get Symmetric Translation Matrix
        vector<Matrix4d> transMats = current_geom->GetFeaTransMatVec();

        //==== Apply Transformations ====//
        for ( int i = 1; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i].Transform( transMats[i] ); // Apply total transformation to main FeaPart surface

            if ( surf_vec[i].GetFlipNormal() != m_FeaPartSurfVec[i].GetFlipNormal() )
            {
                m_FeaPartSurfVec[i].FlipNormal();
            }
        }
    }
}

void FeaPart::UpdateSymmIndex()
{
    m_SymmIndexVec.clear();
    m_FeaPartSurfVec.clear();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* currgeom = veh->FindGeom( m_ParentGeomID );

    if ( currgeom )
    {
        vector< VspSurf > surf_vec;
        currgeom->GetSurfVec( surf_vec );

        m_SymmIndexVec = currgeom->GetSymmIndexs( m_MainSurfIndx() );

        int ncopy = currgeom->GetNumSymmCopies();

        assert( ncopy == m_SymmIndexVec.size() );

        m_FeaPartSurfVec.resize( m_SymmIndexVec.size() );
    }
}

string FeaPart::GetTypeName( int type )
{
    if ( type == vsp::FEA_SLICE )
    {
        return string( "Slice" );
    }
    if ( type == vsp::FEA_RIB )
    {
        return string( "Rib" );
    }
    if ( type == vsp::FEA_SPAR )
    {
        return string( "Spar" );
    }
    if ( type == vsp::FEA_FIX_POINT )
    {
        return string( "Fixed_Point" );
    }
    if ( type == vsp::FEA_SKIN )
    {
        return string( "Skin" );
    }
    if ( type == vsp::FEA_RIB_ARRAY )
    {
        return string( "Rib_Array" );
    }
    if ( type == vsp::FEA_DOME )
    {
        return string( "Dome" );
    }
    if ( type == vsp::FEA_SLICE_ARRAY )
    {
        return string( "Slice_Array" );
    }

    return string( "NONE" );
}

bool FeaPart::RefFrameIsBody( int orientation_plane )
{
    if ( orientation_plane == vsp::XY_BODY || orientation_plane == vsp::YZ_BODY || orientation_plane == vsp::XZ_BODY )
    {
        return true;
    }
    return false;
}

void FeaPart::FetchFeaXFerSurf( vector< XferSurf > &xfersurfs, int compid, const vector < double > &usuppress, const vector < double > &wsuppress )
{
    for ( int p = 0; p < m_FeaPartSurfVec.size(); p++ )
    {
        // CFD_STRUCTURE and CFD_STIFFENER type surfaces have m_CompID starting at -9999
        m_FeaPartSurfVec[p].FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx(), compid, xfersurfs, usuppress, wsuppress );
    }
}

void FeaPart::DeleteFeaPartSurf( int ind )
{
    if ( m_FeaPartSurfVec.size() > 0 && ind < m_FeaPartSurfVec.size() && ind >= 0 )
    {
        m_FeaPartSurfVec.erase( m_FeaPartSurfVec.begin() + ind );
    }
}

void FeaPart::LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0; i < (int)m_FeaPartDO.size(); i++ )
    {
        draw_obj_vec.push_back( &m_FeaPartDO[i] );
    }
}

void FeaPart::UpdateDrawObjs( int id, bool highlight )
{
    m_FeaPartDO.clear();
    m_FeaPartDO.resize( m_FeaPartSurfVec.size() );

    for ( unsigned int j = 0; j < m_FeaPartSurfVec.size(); j++ )
    {
        m_FeaPartDO[j].m_PntVec.clear();

        m_FeaPartDO[j].m_GeomID = string( m_Name + "_" + std::to_string( id ) + "_" + std::to_string( j ) );
        m_FeaPartDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        if ( highlight )
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
            m_FeaPartDO[j].m_LineWidth = 3.0;
        }
        else
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
            m_FeaPartDO[j].m_LineWidth = 1.0;
        }

        m_FeaPartDO[j].m_Type = DrawObj::VSP_WIRE_SHADED_QUADS;

        vec3d p00 = m_FeaPartSurfVec[j].CompPnt01( 0, 0 );
        vec3d p10 = m_FeaPartSurfVec[j].CompPnt01( 1, 0 );
        vec3d p11 = m_FeaPartSurfVec[j].CompPnt01( 1, 1 );
        vec3d p01 = m_FeaPartSurfVec[j].CompPnt01( 0, 1 );

        m_FeaPartDO[j].m_PntVec.push_back( p00 );
        m_FeaPartDO[j].m_PntVec.push_back( p10 );
        m_FeaPartDO[j].m_PntVec.push_back( p11 );
        m_FeaPartDO[j].m_PntVec.push_back( p01 );

        // Get new normal
        vec3d quadnorm = cross( p10 - p00, p01 - p00 );
        quadnorm.normalize();

        for ( int i = 0; i < 4; i++ )
        {
            m_FeaPartDO[j].m_NormVec.push_back(quadnorm);
        }

        // Set plane color to medium glass
        for ( int i = 0; i < 4; i++ )
        {
            m_FeaPartDO[j].m_MaterialInfo.Ambient[i] = 0.2f;
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[i] = 0.1f;
            m_FeaPartDO[j].m_MaterialInfo.Specular[i] = 0.7f;
            m_FeaPartDO[j].m_MaterialInfo.Emission[i] = 0.0f;
        }

        if ( highlight )
        {
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.67f;
        }
        else
        {
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.33f;
        }

        m_FeaPartDO[j].m_MaterialInfo.Shininess = 5.0f;
        m_FeaPartDO[j].m_GeomChanged = true;
    }
}

int FeaPart::GetFeaMaterialIndex()
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex() );

    if ( fea_prop )
    {
        return fea_prop->m_FeaMaterialIndex();
    }
    return -1; // Indicates an error
}

void FeaPart::SetFeaMaterialIndex( int index )
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex() );

    if ( fea_prop )
    {
        fea_prop->m_FeaMaterialIndex.Set( index );
    }
}

VspSurf* FeaPart::GetMainSurf()
{
    VspSurf* retsurf = NULL;

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom *currgeom = veh->FindGeom( m_ParentGeomID );
        if ( currgeom )
        {
            retsurf = currgeom->GetSurfPtr( m_MainSurfIndx() );
        }
    }
    return retsurf;
}

bool FeaPart::PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind )
{
    double tol = 1.0e-6;

    VspSurf surf = m_FeaPartSurfVec[surf_ind];

    double umax = surf.GetUMax();
    double wmax = surf.GetWMax();

    vec3d o = surf.CompPnt( umax * 0.5, wmax * 0.5 );
    vec3d n = surf.CompNorm( umax * 0.5, wmax * 0.5 );

    // Find point furthest from surface.
    double dmax = 0.0;
    for ( int i = 0; i < pnts.size(); i++ )
    {
        double d;

        vec3d p = pnts[i];
        d = dist_pnt_2_plane( o, n, p );

        if ( d > dmax )
        {
            dmax = d;
        }
    }

    // If furthest point is within tolerance, all points are on surface.
    if ( dmax < tol )
    {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////
//==================== FeaSlice ====================//
//////////////////////////////////////////////////////

FeaSlice::FeaSlice( string geomID, int type ) : FeaPart( geomID, type )
{
    m_OrientationPlane.Init( "OrientationPlane", "FeaSlice", this, vsp::YZ_BODY, vsp::XY_BODY, vsp::SPINE_NORMAL );
    m_OrientationPlane.SetDescript( "Plane the FeaSlice Part will be Parallel to (Body or Absolute Reference Frame)" );

    m_RotationAxis.Init( "RotationAxis", "FeaSlice", this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_RotationAxis.SetDescript( "Slice Axis of Rotation" );

    m_XRot.Init( "XRot", "FeaSlice", this, 0.0, -90.0, 90.0 );
    m_XRot.SetDescript( "Rotation About Slice X Axis" );

    m_YRot.Init( "YRot", "FeaSlice", this, 0.0, -90.0, 90.0 );
    m_YRot.SetDescript( "Rotation About Slice Y Axis" );

    m_ZRot.Init( "ZRot", "FeaSlice", this, 0.0, -90.0, 90.0 );
    m_ZRot.SetDescript( "Rotation About Slice Z Axis" );
}

void FeaSlice::Update()
{
    UpdateParmLimits();

    // Must call UpdateSymmIndex before
    if ( m_FeaPartSurfVec.size() > 0 )
    {
        m_FeaPartSurfVec[0] = ComputeSliceSurf();

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, setup the symmetric copies to be definied in UpdateSymmParts 
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }
    }
    // Must call UpdateSymmParts next
}

void FeaSlice::UpdateParmLimits()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_geom = veh->FindGeom( m_ParentGeomID );
        if ( !current_geom )
        {
            return;
        }

        vector< VspSurf > surf_vec;
        current_geom->GetSurfVec( surf_vec );
        VspSurf current_surf = surf_vec[m_MainSurfIndx()];

        // Determine BndBox dimensions prior to rotating and translating
        Matrix4d model_matrix = current_geom->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = current_surf;
        orig_surf.Transform( model_matrix );

        if ( RefFrameIsBody( m_OrientationPlane() ) )
        {
            orig_surf.GetBoundingBox( m_SectBBox );
        }
        else
        {
            current_surf.GetBoundingBox( m_SectBBox );
        }

        double perp_dist; // Total distance perpendicular to the FeaSlice plane

        vec3d geom_center = m_SectBBox.GetCenter();
        double del_x = m_SectBBox.GetMax( 0 ) - m_SectBBox.GetMin( 0 );
        double del_y = m_SectBBox.GetMax( 1 ) - m_SectBBox.GetMin( 1 );
        double del_z = m_SectBBox.GetMax( 2 ) - m_SectBBox.GetMin( 2 );

        if ( m_RotationAxis() == vsp::X_DIR )
        {
            m_YRot.Set( 0.0 );
            m_ZRot.Set( 0.0 );
        }
        else if ( m_RotationAxis() == vsp::Y_DIR )
        {
            m_XRot.Set( 0.0 );
            m_ZRot.Set( 0.0 );
        }
        else if ( m_RotationAxis() == vsp::Z_DIR )
        {
            m_XRot.Set( 0.0 );
            m_YRot.Set( 0.0 );
        }

        if ( m_OrientationPlane() == vsp::XY_BODY || m_OrientationPlane() == vsp::XY_ABS )
        {
            perp_dist = m_SectBBox.GetMax( 2 ) - m_SectBBox.GetMin( 2 );

            m_Center = vec3d( geom_center.x(), geom_center.y(), m_SectBBox.GetMin( 2 ) + del_z * m_RelCenterLocation() );
        }
        else if ( m_OrientationPlane() == vsp::YZ_BODY || m_OrientationPlane() == vsp::YZ_ABS )
        {
            perp_dist = m_SectBBox.GetMax( 0 ) - m_SectBBox.GetMin( 0 );

            m_Center = vec3d( m_SectBBox.GetMin( 0 ) + del_x * m_RelCenterLocation(), geom_center.y(), geom_center.z() );
        }
        else if ( m_OrientationPlane() == vsp::XZ_BODY || m_OrientationPlane() == vsp::XZ_ABS )
        {
            perp_dist = m_SectBBox.GetMax( 1 ) - m_SectBBox.GetMin( 1 );

            m_Center = vec3d( geom_center.x(), m_SectBBox.GetMin( 1 ) + del_y * m_RelCenterLocation(), geom_center.z() );
        }
        else if ( m_OrientationPlane() == vsp::SPINE_NORMAL )
        {
            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( current_surf );

            perp_dist = cs.GetSpineLength();

            double u_max = current_surf.GetUMax();

            double spine_length = cs.GetSpineLength();
            double length_on_spine = m_RelCenterLocation() * spine_length;
            double per_u = cs.FindUGivenLengthAlongSpine( length_on_spine ) / u_max;

            m_Center = cs.FindCenterGivenU( per_u * u_max );
        }

        // Set Parm limits and values
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            m_AbsCenterLocation.Set( m_RelCenterLocation() * perp_dist );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            m_AbsCenterLocation.SetUpperLimit( perp_dist );
            m_RelCenterLocation.Set( m_AbsCenterLocation() / perp_dist );
        }
    }
}

VspSurf FeaSlice::ComputeSliceSurf()
{
    VspSurf slice_surf;
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_geom = veh->FindGeom( m_ParentGeomID );
        if ( !current_geom )
        {
            return slice_surf;
        }

        vector< VspSurf > surf_vec;
        current_geom->GetSurfVec( surf_vec );
        VspSurf current_surf = surf_vec[m_MainSurfIndx()];

        slice_surf = VspSurf(); // Create primary VspSurf

        if ( m_IncludedElements() == vsp::FEA_SHELL || m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
        {
            slice_surf.SetSurfCfdType( vsp::CFD_STRUCTURE );
        }
        else
        {
            slice_surf.SetSurfCfdType( vsp::CFD_STIFFENER );
        }

        // Determine BndBox dimensions prior to rotating and translating
        Matrix4d model_matrix = current_geom->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = current_surf;
        orig_surf.Transform( model_matrix );

        double u_max = current_surf.GetUMax();

        vec3d geom_center, cornerA, cornerB, cornerC, cornerD, x_axis, y_axis, z_axis,
            center_to_A, center_to_B, center_to_C, center_to_D;
        double del_x_plus, del_x_minus, del_y_plus, del_y_minus, del_z_plus, del_z_minus, max_length, del_x, del_y, del_z;

        x_axis.set_x( 1.0 );
        y_axis.set_y( 1.0 );
        z_axis.set_z( 1.0 );

        geom_center = m_SectBBox.GetCenter();
        del_x = m_SectBBox.GetMax( 0 ) - m_SectBBox.GetMin( 0 );
        del_y = m_SectBBox.GetMax( 1 ) - m_SectBBox.GetMin( 1 );
        del_z = m_SectBBox.GetMax( 2 ) - m_SectBBox.GetMin( 2 );

        // Identify expansion 
        double expan = m_SectBBox.GetLargestDist() * 1e-3;
        if ( expan < 1e-5 )
        {
            expan = 1e-5;
        }

        if ( m_OrientationPlane() == vsp::SPINE_NORMAL )
        {
            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( current_surf );

            double spine_length = cs.GetSpineLength();

            double length_on_spine = m_RelCenterLocation() * spine_length;
            double per_u = cs.FindUGivenLengthAlongSpine( length_on_spine ) / u_max;

            // Use small change in u along spline to get x axis of geom at center point
            double delta_u;

            if ( per_u < ( 1.0 - 2 * FLT_EPSILON ) )
            {
                delta_u = ( per_u * u_max ) + ( 2 * FLT_EPSILON );
            }
            else
            {
                delta_u = ( per_u * u_max ) - ( 2 * FLT_EPSILON );
            }

            vec3d delta_u_center = cs.FindCenterGivenU( delta_u );

            x_axis = delta_u_center - m_Center;
            x_axis.normalize();

            vec3d surf_pnt1 = current_surf.CompPnt01( per_u, 0.0 );

            z_axis = surf_pnt1 - m_Center;
            z_axis.normalize();

            y_axis = cross( x_axis, z_axis );
            y_axis.normalize();

            VspCurve u_curve;
            current_surf.GetU01ConstCurve( u_curve, per_u );

            BndBox xsec_box;
            u_curve.GetBoundingBox( xsec_box );
            max_length = xsec_box.GetLargestDist() + 1e-4;

            // TODO: Improve initial size and resize after rotations

            // TODO: Improve 45 deg assumption
            vec3d y_prime = max_length * y_axis * cos( PI / 4 ) + max_length * z_axis * sin( PI / 4 );
            vec3d z_prime = max_length * -1 * y_axis * sin( PI / 4 ) + max_length * z_axis * cos( PI / 4 );

            cornerA = m_Center + y_prime;
            cornerB = m_Center - z_prime;
            cornerC = m_Center + z_prime;
            cornerD = m_Center - y_prime;
        }
        else
        {
            // Increase size slighlty to avoid tangency errors in FeaMeshMgr
            del_x_minus = expan;
            del_x_plus = expan;
            del_y_minus = expan;
            del_y_plus = expan;
            del_z_minus = expan;
            del_z_plus = expan;

            double x_off = ( m_Center - geom_center ).x();
            double y_off = ( m_Center - geom_center ).y();
            double z_off = ( m_Center - geom_center ).z();

            if ( m_OrientationPlane() == vsp::YZ_BODY || m_OrientationPlane() == vsp::YZ_ABS )
            {
                // Resize for Y rotation
                if ( m_YRot() > 0 )
                {
                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x + 2 * x_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_plus += abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x - 2 * x_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_minus += abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }
                else
                {
                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x - 2 * x_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_plus += abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x + 2 * x_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_minus += abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }

                // Resize for Z rotation
                if ( m_ZRot() > 0 )
                {
                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x + 2 * x_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_minus += abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_minus += abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x - 2 * x_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_plus += abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_plus += abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }
                else
                {
                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x + 2 * x_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_plus += abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_plus += abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x - 2 * x_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_minus += abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_minus += abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }

                center_to_A.set_y( -0.5 * del_y_minus );
                center_to_A.set_z( -0.5 * del_z_minus );

                center_to_B.set_y( 0.5 * del_y_plus );
                center_to_B.set_z( -0.5 * del_z_minus );

                center_to_C.set_y( -0.5 * del_y_minus );
                center_to_C.set_z( 0.5 * del_z_plus );

                center_to_D.set_y( 0.5 * del_y_plus );
                center_to_D.set_z( 0.5 * del_z_plus );
            }
            else if ( m_OrientationPlane() == vsp::XY_BODY || m_OrientationPlane() == vsp::XY_ABS )
            {
                // Resize for Y rotation
                if ( m_YRot() > 0 )
                {
                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z + 2 * z_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_minus += abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_minus += abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z - 2 * z_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_plus += abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_plus += abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }
                else
                {
                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z + 2 * z_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_plus += abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_plus += abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z - 2 * z_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_minus += abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_minus += abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }

                // Resize for X rotation
                if ( m_XRot() > 0 )
                {
                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z + 2 * z_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_plus += abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_plus += abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z - 2 * z_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_minus += abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_minus += abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }
                }
                else
                {
                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z - 2 * z_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_plus += abs( ( del_z -2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_plus += abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z + 2 * z_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_minus += abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_minus += abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }
                }

                center_to_A.set_x( -0.5 * del_x_minus );
                center_to_A.set_y( -0.5 * del_y_minus );

                center_to_B.set_x( -0.5 * del_x_minus );
                center_to_B.set_y( 0.5 * del_y_plus );

                center_to_C.set_x( 0.5 * del_x_plus );
                center_to_C.set_y( -0.5 * del_y_minus );

                center_to_D.set_x( 0.5 * del_x_plus );
                center_to_D.set_y( 0.5 * del_y_plus );
            }
            else if ( m_OrientationPlane() == vsp::XZ_BODY || m_OrientationPlane() == vsp::XZ_ABS )
            {
                // Resize for Z rotation
                if ( m_ZRot() > 0 )
                {
                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y + 2 * y_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_plus += abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_plus += abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y - 2 * y_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_minus += abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_minus += abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }
                else 
                {
                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y - 2 * y_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_plus += abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_plus += abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y + 2 * y_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_minus += abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_minus += abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }

                // Resize for X rotation
                if ( m_XRot() > 0 )
                {
                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y + 2 * y_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_plus += abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y - 2 * y_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_minus += abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }
                }
                else
                {
                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y - 2 * y_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_plus += abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y + 2 * y_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_minus += abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }
                }

                center_to_A.set_x( -0.5 * del_x_minus );
                center_to_A.set_z( -0.5 * del_z_minus );

                center_to_B.set_x( 0.5 * del_x_plus );
                center_to_B.set_z( -0.5 * del_z_minus );

                center_to_C.set_x( -0.5 * del_x_minus );
                center_to_C.set_z( 0.5 * del_z_plus );

                center_to_D.set_x( 0.5 * del_x_plus );
                center_to_D.set_z( 0.5 * del_z_plus );
            }

            cornerA = m_Center + center_to_A;
            cornerB = m_Center + center_to_B;
            cornerC = m_Center + center_to_C;
            cornerD = m_Center + center_to_D;
        }

        // Make Planar Surface
        slice_surf.MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
        Matrix4d trans_mat_1, trans_mat_2, rot_mat_x, rot_mat_y, rot_mat_z;

        trans_mat_1.loadIdentity();
        trans_mat_1.translatef( m_Center.x() * -1, m_Center.y() * -1, m_Center.z() * -1 );
        slice_surf.Transform( trans_mat_1 );

        rot_mat_x.loadIdentity();
        rot_mat_x.rotate( DEG_2_RAD * m_XRot(), x_axis );
        slice_surf.Transform( rot_mat_x );

        rot_mat_y.loadIdentity();
        rot_mat_y.rotate( DEG_2_RAD * m_YRot(), y_axis );
        slice_surf.Transform( rot_mat_y );

        rot_mat_z.loadIdentity();
        rot_mat_z.rotate( DEG_2_RAD * m_ZRot(), z_axis );
        slice_surf.Transform( rot_mat_z );

        trans_mat_2.loadIdentity();
        trans_mat_2.translatef( m_Center.x(), m_Center.y(), m_Center.z() );
        slice_surf.Transform( trans_mat_2 );

        if ( RefFrameIsBody( m_OrientationPlane() ) )
        {
            // Transform to body coordinate frame
            model_matrix.affineInverse();
            slice_surf.Transform( model_matrix );
        }
    }

    return slice_surf;
}

void FeaSlice::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

//////////////////////////////////////////////////////
//===================== FeaSpar ====================//
//////////////////////////////////////////////////////

FeaSpar::FeaSpar( string geomID, int type ) : FeaSlice( geomID, type )
{
    m_Theta.Init( "Theta", "FeaSpar", this, 0.0, -90.0, 90.0 );
    m_Theta.SetDescript( "Rotation of Spar About Axis Normal to Wing Chord Line " );

    m_LimitSparToSectionFlag.Init( "LimitSparToSectionFlag", "FeaSpar", this, false, false, true );
    m_LimitSparToSectionFlag.SetDescript( "Flag to Limit Spar Length to Wing Section" );

    m_StartWingSection.Init( "StartWingSection", "FeaSpar", this, 1, 1, 1000 );
    m_StartWingSection.SetDescript( "Start Wing Section to Limit Spar Length to" );

    m_EndWingSection.Init( "EndWingSection", "FeaSpar", this, 1, 1, 1000 );
    m_EndWingSection.SetDescript( "End Wing Section to Limit Spar Length to" );

    m_BndBoxTrimFlag.Init( "BndBoxTrimFlag", "FeaSpar", this, false, false, true );
    m_BndBoxTrimFlag.SetDescript( "Flag to Trim Spar to Bounding Box Instead of Wing Surface" );
}

void FeaSpar::Update()
{
    UpdateParms();
    ComputePlanarSurf();
}

void FeaSpar::UpdateParms()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing || m_FeaPartSurfVec.size() == 0 )
        {
            return;
        }

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        vector< VspSurf > surf_vec;
        current_wing->GetSurfVec( surf_vec );
        VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

        int num_wing_sec = wing->NumXSec();
        double U_max = wing_surf.GetUMax();

        m_StartWingSection.SetLowerUpperLimits( 1, m_EndWingSection() );
        m_EndWingSection.SetLowerUpperLimits( m_StartWingSection(), num_wing_sec - 1 );

        // Determine U limits of spar
        if ( m_LimitSparToSectionFlag() )
        {
            if ( wing->m_CapUMinOption() == vsp::NO_END_CAP )
            {
                m_U_sec_min = ( m_StartWingSection() - 1 );
            }
            else
            {
                m_U_sec_min = m_StartWingSection();
            }

            m_U_sec_max = m_U_sec_min + 1 + ( m_EndWingSection() - m_StartWingSection() );
        }
        else
        {
            if ( wing->m_CapUMinOption() == vsp::NO_END_CAP )
            {
                m_U_sec_min = 0;
            }
            else
            {
                m_U_sec_min = 1;
            }
            if ( wing->m_CapUMaxOption() == vsp::NO_END_CAP )
            {
                m_U_sec_max = U_max;
            }
            else
            {
                m_U_sec_max = U_max - 1;
            }
        }

        double u_mid = ( ( m_U_sec_min + m_U_sec_max ) / 2 ) / U_max;

        double chord_length = dist( wing_surf.CompPnt01( u_mid, 0.5 ), wing_surf.CompPnt01( u_mid, 0.0 ) ); // average chord length

        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            m_AbsCenterLocation.Set( m_RelCenterLocation() * chord_length );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            m_AbsCenterLocation.SetUpperLimit( chord_length );
            m_RelCenterLocation.Set( m_AbsCenterLocation() / chord_length );
        }
    }
}

void FeaSpar::ComputePlanarSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing || m_FeaPartSurfVec.size() == 0 )
        {
            return;
        }

        m_FeaPartSurfVec[0] = VspSurf(); // Create primary VspSurf

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        vector< VspSurf > surf_vec;
        current_wing->GetSurfVec( surf_vec );
        VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

        // Get surface prior to rotating and translating
        Matrix4d model_matrix = current_wing->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = wing_surf;
        orig_surf.Transform( model_matrix );

        BndBox wing_bbox;
        orig_surf.GetBoundingBox( wing_bbox );

        double U_max = orig_surf.GetUMax();

        double u_mid = ( ( m_U_sec_min + m_U_sec_max ) / 2 ) / U_max;

        VspCurve constant_u_curve;
        orig_surf.GetU01ConstCurve( constant_u_curve, u_mid );

        piecewise_curve_type u_curve = constant_u_curve.GetCurve();

        double V_min = u_curve.get_parameter_min(); // Really must be 0.0
        double V_max = u_curve.get_parameter_max(); // Really should be 4.0
        double V_leading_edge = ( V_min + V_max ) * 0.5;

        // Wing corner points:
        vec3d min_trail_edge = orig_surf.CompPnt( m_U_sec_min, 0.0 );
        vec3d min_lead_edge = orig_surf.CompPnt( m_U_sec_min, V_leading_edge );
        vec3d max_trail_edge = orig_surf.CompPnt( m_U_sec_max, 0.0 );
        vec3d max_lead_edge = orig_surf.CompPnt( m_U_sec_max, V_leading_edge );

        // Determine inner edge and outer edge spar points before rotations
        vec3d inside_edge_vec = min_lead_edge - min_trail_edge;
        double inside_edge_length = inside_edge_vec.mag();
        inside_edge_vec.normalize();
        vec3d inside_edge_pnt = min_lead_edge - ( m_RelCenterLocation() * inside_edge_length ) * inside_edge_vec;

        vec3d outside_edge_vec = max_lead_edge - max_trail_edge;
        double outside_edge_length = outside_edge_vec.mag();
        outside_edge_vec.normalize();
        vec3d outside_edge_pnt = max_lead_edge - ( m_RelCenterLocation() * outside_edge_length ) * outside_edge_vec;

        double length_spar_0 = dist( inside_edge_pnt, outside_edge_pnt ) / 2; // Initial spar half length

        // Find two points slightly above and below the trailing edge
        double v_trail_edge_low = V_min + 2 * TMAGIC;
        double v_trail_edge_up = V_max - 2 * TMAGIC;

        vec3d trail_edge_up, trail_edge_low;
        trail_edge_up = u_curve.f( v_trail_edge_low );
        trail_edge_low = u_curve.f( v_trail_edge_up );

        vec3d wing_z_axis = trail_edge_up - trail_edge_low;
        wing_z_axis.normalize();

        // Identify expansion 
        double expan = wing_bbox.GetLargestDist() * 1e-3;
        if ( expan < 1e-5 )
        {
            expan = 1e-5;
        }

        double height = 0.5 * wing_bbox.GetSmallestDist() + expan; // Height of spar, slightly oversized

        vec3d center = ( inside_edge_pnt + outside_edge_pnt ) / 2; // center of spar

        vec3d center_to_inner_edge = inside_edge_pnt - center;
        vec3d center_to_outer_edge = outside_edge_pnt - center;

        center_to_inner_edge.normalize();
        center_to_outer_edge.normalize();

        // Wing edge vectors (assumes linearity)
        vec3d trail_edge_vec = max_trail_edge - min_trail_edge;
        vec3d lead_edge_vec = max_lead_edge - min_lead_edge;
        vec3d inner_edge_vec = min_trail_edge - min_lead_edge;
        vec3d outer_edge_vec = max_trail_edge - max_lead_edge;

        trail_edge_vec.normalize();
        lead_edge_vec.normalize();
        inner_edge_vec.normalize();
        outer_edge_vec.normalize();

        // Normal vector to wing chord line
        vec3d normal_vec;

        if ( abs( inner_edge_vec.mag() - 1.0 ) <= FLT_EPSILON )
        {
            normal_vec = cross( inner_edge_vec, lead_edge_vec );
        }
        else
        {
            normal_vec = cross( outer_edge_vec, lead_edge_vec );
        }

        normal_vec.normalize();

        double alpha_0 = ( PI / 2 ) - signed_angle( inner_edge_vec, center_to_outer_edge, normal_vec ); // Initial rotation
        double theta = DEG_2_RAD * m_Theta(); // User defined angle converted to Rad

        if ( m_BndBoxTrimFlag() )
        {
            // Get bounding box of wing sections
            BndBox sect_bbox;

            if ( m_LimitSparToSectionFlag() )
            {
                // Note: This method may not be valid for the bounding box of a wing surface that is blended
                sect_bbox.Reset();

                for ( size_t i = m_StartWingSection() - 1; i <= m_EndWingSection(); i++ )
                {
                    WingSect* wing_sec = wing->GetWingSect( i );

                    if ( wing_sec )
                    {
                        VspCurve xsec_curve = wing_sec->GetCurve();
                        BndBox bbox;
                        xsec_curve.GetBoundingBox( bbox );
                        sect_bbox.Update( bbox );
                    }
                }
            }
            else
            {
                sect_bbox = wing_bbox;
            }

            FeaSlice* temp_slice = NULL;
            temp_slice = new FeaSlice( m_ParentGeomID );

            if ( temp_slice )
            {
                temp_slice->SetCenter( center );
                temp_slice->SetSectionBBox( sect_bbox );
                temp_slice->m_OrientationPlane.Set( vsp::YZ_BODY );
                temp_slice->m_ZRot.Set( RAD_2_DEG * ( theta + alpha_0 ) );

                // Update Slice Relative Center Location
                double rel_center_location = ( center.x() - sect_bbox.GetMin( 0 ) ) / ( sect_bbox.GetMax( 0 ) - sect_bbox.GetMin( 0 ) );
                temp_slice->m_RelCenterLocation.Set( rel_center_location );

                m_FeaPartSurfVec[0] = temp_slice->ComputeSliceSurf();

                delete temp_slice;
            }
        }
        else
        {
            // Determine angle between center and corner points
            vec3d center_to_le_in_vec = min_lead_edge - center;
            vec3d center_to_te_in_vec = min_trail_edge - center;
            vec3d center_to_le_out_vec = max_lead_edge - center;
            vec3d center_to_te_out_vec = max_trail_edge - center;

            center_to_le_in_vec.normalize();
            center_to_te_in_vec.normalize();
            center_to_le_out_vec.normalize();
            center_to_te_out_vec.normalize();

            // Get maximum angles for spar to intersect wing edges
            double max_angle_inner_le = -1 * signed_angle( center_to_inner_edge, center_to_le_in_vec, normal_vec );
            double max_angle_inner_te = -1 * signed_angle( center_to_inner_edge, center_to_te_in_vec, normal_vec );
            double max_angle_outer_le = signed_angle( center_to_le_out_vec, center_to_outer_edge, normal_vec );
            double max_angle_outer_te = signed_angle( center_to_te_out_vec, center_to_outer_edge, normal_vec );

            double beta_te = -1 * signed_angle( center_to_outer_edge, trail_edge_vec, normal_vec ); // Angle between spar and trailing edge
            double beta_le = -1 * PI + signed_angle( center_to_inner_edge, lead_edge_vec, normal_vec ); // Angle between spar and leading edge

            // Slightly oversize spar length
            double length_spar_in = 1e-6;
            double length_spar_out = 1e-6;
            double perp_dist;

            // Determine if the rib intersects the leading/trailing edge or inner/outer edge
            if ( theta >= 0 )
            {
                if ( theta > max_angle_inner_le )
                {
                    if ( abs( sin( theta + beta_le ) ) <= FLT_EPSILON || ( min_lead_edge - max_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - max_lead_edge ).mag();
                        length_spar_in += abs( perp_dist / sin( theta + beta_le ) );
                    }
                }
                else
                {
                    if ( abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( min_trail_edge - min_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_lead_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - min_lead_edge ).mag();
                        length_spar_in += abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }

                if ( theta > max_angle_outer_te )
                {
                    if ( abs( sin( theta - beta_te ) ) <= FLT_EPSILON || ( min_trail_edge - max_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - max_trail_edge ).mag();
                        length_spar_out += abs( perp_dist / sin( theta - beta_te ) );
                    }
                }
                else
                {
                    if ( abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( max_trail_edge - max_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - max_trail_edge ) ).mag() / ( max_trail_edge - max_lead_edge ).mag();
                        length_spar_out += abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }
            }
            else
            {
                if ( theta < max_angle_inner_te )
                {
                    if ( abs( sin( theta - beta_te ) ) <= FLT_EPSILON || ( max_trail_edge - min_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - min_trail_edge ) ).mag() / ( max_trail_edge - min_trail_edge ).mag();
                        length_spar_in += abs( perp_dist / sin( theta - beta_te ) );
                    }
                }
                else
                {
                    if ( abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( min_trail_edge - min_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_lead_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - min_lead_edge ).mag();
                        length_spar_in += abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }

                if ( theta < max_angle_outer_le )
                {
                    if ( abs( sin( theta + beta_le ) ) <= FLT_EPSILON || ( max_lead_edge - min_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - min_lead_edge ) ).mag() / ( max_lead_edge - min_lead_edge ).mag();
                        length_spar_out += abs( perp_dist / sin( theta + beta_le ) );
                    }
                }
                else
                {
                    if ( abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( max_trail_edge - max_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - max_trail_edge ) ).mag() / ( max_trail_edge - max_lead_edge ).mag();
                        length_spar_out += abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }
            }

            // Apply Rodrigues' Rotation Formula
            vec3d spar_vec_in = center_to_inner_edge * cos( theta ) + cross( center_to_inner_edge, normal_vec ) * sin( theta ) + normal_vec * dot( center_to_inner_edge, normal_vec ) * ( 1 - cos( theta ) );
            vec3d spar_vec_out = center_to_outer_edge * cos( theta ) + cross( center_to_outer_edge, normal_vec ) * sin( theta ) + normal_vec * dot( center_to_outer_edge, normal_vec ) * ( 1 - cos( theta ) );

            spar_vec_in.normalize();
            spar_vec_out.normalize();

            // Calculate final end points
            vec3d inside_edge_f = center + length_spar_in * spar_vec_in;
            vec3d outside_edge_f = center + length_spar_out * spar_vec_out;

            // Identify corners of the plane
            vec3d cornerA, cornerB, cornerC, cornerD;

            cornerA = inside_edge_f + ( height * wing_z_axis );
            cornerB = inside_edge_f - ( height * wing_z_axis );
            cornerC = outside_edge_f + ( height * wing_z_axis );
            cornerD = outside_edge_f - ( height * wing_z_axis );

            // Make Planar Surface
            m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            // Transform to body coordinate frame
            model_matrix.affineInverse();
            m_FeaPartSurfVec[0].Transform( model_matrix );
        }

        // Set Surface CFD Type: 
        if ( m_IncludedElements() == vsp::FEA_SHELL || m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
        {
            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STRUCTURE );
        }
        else
        {
            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STIFFENER );
        }

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != wing_surf.GetFlipNormal() )
        {
            m_FeaPartSurfVec[0].FlipNormal();
        }

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, setup the symmetric copies to be definied in UpdateSymmParts 
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }
    }
}

void FeaSpar::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

//////////////////////////////////////////////////////
//===================== FeaRib =====================//
//////////////////////////////////////////////////////

FeaRib::FeaRib( string geomID, int type ) : FeaSlice( geomID, type )
{
    m_Theta.Init( "Theta", "FeaRib", this, 0.0, -90.0, 90.0 );
    m_Theta.SetDescript( "Rotation of FeaRib About Axis Normal to Wing Chord Line" );

    m_LimitRibToSectionFlag.Init( "LimitRibToSectionFlag", "FeaRib", this, false, false, true );
    m_LimitRibToSectionFlag.SetDescript( "Flag to Limit Rib Length to Wing Section" );

    m_StartWingSection.Init( "StartWingSection", "FeaRib", this, 1, 1, 1000 );
    m_StartWingSection.SetDescript( "Start Wing Section to Limit Rib to" );

    m_EndWingSection.Init( "EndWingSection", "FeaRib", this, 1, 1, 1000 );
    m_EndWingSection.SetDescript( "End Wing Section to Limit Rib to" );

    m_BndBoxTrimFlag.Init( "BndBoxTrimFlag", "FeaRib", this, false, false, true );
    m_BndBoxTrimFlag.SetDescript( "Flag to Trim Rib to Bounding Box Instead of Wing Surface" );
}

void FeaRib::Update()
{
    UpdateParmLimits();

    // Must call UpdateSymmIndex before
    if ( m_FeaPartSurfVec.size() > 0 )
    {
        GetRibPerU();
        m_FeaPartSurfVec[0] = ComputeRibSurf();

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, setup the symmetric copies to be definied in UpdateSymmParts 
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }
    }
    // Must call UpdateSymmParts next
}

void FeaRib::UpdateParmLimits()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing )
        {
            return;
        }

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        int num_wing_sec = wing->NumXSec();

        m_StartWingSection.SetLowerUpperLimits( 1, m_EndWingSection() );
        m_EndWingSection.SetLowerUpperLimits( m_StartWingSection(), num_wing_sec - 1 );

        // Init values:
        double span = 0.0;
        int start_sect, end_sect;
        int curr_sec_ind = -1;

        // Determine current wing section:
        if ( m_LimitRibToSectionFlag() )
        {
            start_sect = m_StartWingSection();
            end_sect = m_EndWingSection() + 1;
        }
        else
        {
            start_sect = 1;
            end_sect = num_wing_sec;
        }

        // Determine wing span:
        for ( size_t i = start_sect; i < end_sect; i++ )
        {
            WingSect* wing_sec = wing->GetWingSect( i );

            if ( wing_sec )
            {
                span += wing_sec->m_Span();
            }
        }

        m_RelCenterLocation.SetLowerUpperLimits( 0.0, 1.0 );
        m_AbsCenterLocation.SetLowerUpperLimits( 0.0, span );

        // Set parm limits and values
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            m_AbsCenterLocation.Set( span * m_RelCenterLocation() );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            m_RelCenterLocation.Set( m_AbsCenterLocation() / span );
        }
    }
}

xmlNodePtr FeaRib::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_prt_node = FeaPart::EncodeXml( node );

    if ( fea_prt_node )
    {
        XmlUtil::AddStringNode( fea_prt_node, "PerpendicularEdgeID", m_PerpendicularEdgeID );
    }

    return fea_prt_node;
}

xmlNodePtr FeaRib::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_prt_node = FeaPart::DecodeXml( node );

    if ( fea_prt_node )
    {
        m_PerpendicularEdgeID = XmlUtil::FindString( fea_prt_node, "PerpendicularEdgeID", m_PerpendicularEdgeID );
    }

    return fea_prt_node;
}

double FeaRib::GetRibPerU( )
{
    m_PerU = 0.0;
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( current_wing )
        {
            WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
            assert( wing );

            vector< VspSurf > surf_vec;
            current_wing->GetSurfVec( surf_vec );
            VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

            int num_wing_sec = wing->NumXSec();

            vector < double > wing_sec_span_vec; // Vector of wing span increasing by each wing section (first section has no length)
            wing_sec_span_vec.push_back( 0.0 );

            double U_max = wing_surf.GetUMax();

            // Init values:
            double span_0 = 0.0;
            double span_f = 0.0;
            double section_span = 0.0;
            int start_sect, end_sect;
            int curr_sec_ind = -1;

            // Determine current wing section:
            if ( m_LimitRibToSectionFlag() )
            {
                start_sect = m_StartWingSection();
                end_sect = m_EndWingSection() + 1;
            }
            else
            {
                start_sect = 1;
                end_sect = num_wing_sec;
            }

            for ( size_t i = start_sect; i < end_sect; i++ )
            {
                WingSect* wing_sec = wing->GetWingSect( i );

                if ( wing_sec )
                {
                    span_f += wing_sec->m_Span();
                    wing_sec_span_vec.push_back( span_f );

                    if ( m_AbsCenterLocation() >= span_0 && m_AbsCenterLocation() <= span_f )
                    {
                        curr_sec_ind = i - ( start_sect - 1 ); // must consider skipped sections
                        section_span = wing_sec->m_Span();
                    }

                    span_0 = span_f;
                }
            }

            if ( m_LimitRibToSectionFlag() )
            {
                if ( wing->m_CapUMinOption() == vsp::NO_END_CAP )
                {
                    m_U_sec_min = ( m_StartWingSection() - 1 );
                }
                else
                {
                    m_U_sec_min = m_StartWingSection();
                }

                m_U_sec_max = m_U_sec_min + 1 + ( m_EndWingSection() - m_StartWingSection() );
            }
            else
            {
                if ( wing->m_CapUMinOption() == vsp::NO_END_CAP )
                {
                    m_U_sec_min = 0;
                }
                else
                {
                    m_U_sec_min = 1;
                }
                if ( wing->m_CapUMaxOption() == vsp::NO_END_CAP )
                {
                    m_U_sec_max = U_max;
                }
                else
                {
                    m_U_sec_max = U_max - 1;
                }
            }

            double u_step = 1 / U_max;

            if ( curr_sec_ind >= 0 && curr_sec_ind < wing_sec_span_vec.size() )
            {
                m_PerU = m_U_sec_min / U_max + ( curr_sec_ind - 1 ) * u_step + ( ( m_AbsCenterLocation() - wing_sec_span_vec[curr_sec_ind - 1] ) / section_span ) * u_step;
            }
        }
    }
    
    return m_PerU;
}

double FeaRib::GetRibTotalRotation( )
{
    m_TotRot = 0.0;
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( current_wing )
        {
            vector< VspSurf > surf_vec;
            current_wing->GetSurfVec( surf_vec );
            VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

            // Get surface prior to rotating and translating
            Matrix4d model_matrix = current_wing->getModelMatrix();
            model_matrix.affineInverse();

            VspSurf orig_surf = wing_surf;
            orig_surf.Transform( model_matrix );

            // Find initial rotation (alpha) to perpendicular edge or spar
            double alpha = 0.0;
            double u_edge_out = m_PerU + 2 * FLT_EPSILON;
            double u_edge_in = m_PerU - 2 * FLT_EPSILON;

            VspCurve constant_u_curve;
            orig_surf.GetU01ConstCurve( constant_u_curve, m_PerU );

            piecewise_curve_type u_curve = constant_u_curve.GetCurve();

            double v_min = u_curve.get_parameter_min(); // Really must be 0.0
            double v_max = u_curve.get_parameter_max(); // Really should be 4.0
            double v_leading_edge = ( v_min + v_max ) * 0.5;

            vec3d trail_edge, lead_edge;
            trail_edge = u_curve.f( v_min );
            lead_edge = u_curve.f( v_leading_edge );

            vec3d chord_dir_vec = trail_edge - lead_edge;
            chord_dir_vec.normalize();

            if ( strcmp( m_PerpendicularEdgeID.c_str(), "Trailing Edge" ) == 0 )
            {
                vec3d trail_edge_out, trail_edge_in;
                trail_edge_out = orig_surf.CompPnt01( u_edge_out, v_min );
                trail_edge_in = orig_surf.CompPnt01( u_edge_in, v_min );

                vec3d trail_edge_dir_vec = trail_edge_out - trail_edge_in;
                trail_edge_dir_vec.normalize();

                alpha = ( PI / 2 ) - signed_angle( chord_dir_vec, trail_edge_dir_vec, m_WingNorm );
            }
            else if ( strcmp( m_PerpendicularEdgeID.c_str(), "Leading Edge" ) == 0 )
            {
                vec3d lead_edge_out, lead_edge_in;
                lead_edge_out = orig_surf.CompPnt01( u_edge_out, v_leading_edge / v_max );
                lead_edge_in = orig_surf.CompPnt01( u_edge_in, v_leading_edge / v_max );

                vec3d lead_edge_dir_vec = lead_edge_out - lead_edge_in;
                lead_edge_dir_vec.normalize();

                alpha = ( PI / 2 ) - signed_angle( chord_dir_vec, lead_edge_dir_vec, m_WingNorm );
            }
            else if ( strcmp( m_PerpendicularEdgeID.c_str(), "None" ) == 0 )
            {
                alpha = 0;
            }
            else
            {
                FeaPart* part = StructureMgr.GetFeaPart( m_PerpendicularEdgeID );

                if ( part )
                {
                    VspSurf surf = part->GetFeaPartSurfVec()[0];

                    vec3d edge1, edge2;
                    edge1 = surf.CompPnt01( 0.5, 0.0 );
                    edge2 = surf.CompPnt01( 0.5, 1.0 );

                    vec3d spar_dir_vec = edge2 - edge1;
                    spar_dir_vec.normalize();

                    alpha = ( PI / 2 ) - signed_angle( chord_dir_vec, spar_dir_vec, m_WingNorm );
                }
            }

            m_TotRot = alpha + m_Theta() * DEG_2_RAD;
        }
    }

    return m_TotRot;
}

VspSurf FeaRib::ComputeRibSurf()
{
    VspSurf rib_surf;
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing )
        {
            return rib_surf;
        }
        rib_surf = VspSurf(); // Create primary VspSurf

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        vector< VspSurf > surf_vec;
        current_wing->GetSurfVec( surf_vec );
        VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

        // Get surface prior to rotating and translating
        Matrix4d model_matrix = wing->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = wing_surf;
        orig_surf.Transform( model_matrix );

        BndBox wing_bbox;
        orig_surf.GetBoundingBox( wing_bbox );

        VspCurve constant_u_curve;
        orig_surf.GetU01ConstCurve( constant_u_curve, m_PerU );

        piecewise_curve_type u_curve = constant_u_curve.GetCurve();

        double v_min = u_curve.get_parameter_min(); // Really must be 0.0
        double v_max = u_curve.get_parameter_max(); // Really should be 4.0
        double v_leading_edge = ( v_min + v_max ) * 0.5;

        vec3d trail_edge, lead_edge;
        trail_edge = u_curve.f( v_min );
        lead_edge = u_curve.f( v_leading_edge );

        // Find two points slightly above and below the trailing edge
        double v_trail_edge_low = v_min + 2 * TMAGIC;
        double v_trail_edge_up = v_max - 2 * TMAGIC;

        vec3d trail_edge_up, trail_edge_low;
        trail_edge_up = u_curve.f( v_trail_edge_low );
        trail_edge_low = u_curve.f( v_trail_edge_up );

        vec3d wing_z_axis = trail_edge_up - trail_edge_low;
        wing_z_axis.normalize();

        vec3d center = ( trail_edge + lead_edge ) / 2; // Center of rib

        // Wing corner points:
        vec3d min_trail_edge = orig_surf.CompPnt( m_U_sec_min, 0.0 );
        vec3d min_lead_edge = orig_surf.CompPnt( m_U_sec_min, v_leading_edge );
        vec3d max_trail_edge = orig_surf.CompPnt( m_U_sec_max, 0.0 );
        vec3d max_lead_edge = orig_surf.CompPnt( m_U_sec_max, v_leading_edge );

        // Wing edge vectors (assumes linearity)
        vec3d trail_edge_vec = max_trail_edge - min_trail_edge;
        vec3d lead_edge_vec = max_lead_edge - min_lead_edge;
        vec3d inner_edge_vec = min_lead_edge - min_trail_edge;
        vec3d outer_edge_vec = max_lead_edge - min_trail_edge;

        trail_edge_vec.normalize();
        lead_edge_vec.normalize();
        inner_edge_vec.normalize();
        outer_edge_vec.normalize();

        // Normal vector to wing chord line
        if ( inner_edge_vec.mag() >= FLT_EPSILON )
        {
            m_WingNorm = cross( lead_edge_vec, inner_edge_vec );
        }
        else
        {
            m_WingNorm = cross( outer_edge_vec, lead_edge_vec );
        }

        m_WingNorm.normalize();

        // Get rotation
        GetRibTotalRotation();

        if ( m_BndBoxTrimFlag() )
        {
            // Get bounding box of wing sections
            BndBox sect_bbox;

            if ( m_LimitRibToSectionFlag() )
            {
                // Note: This method may not be valid for the bounding box of a wing surface that is blended
                sect_bbox.Reset();

                for ( size_t i = m_StartWingSection() - 1; i <= m_EndWingSection(); i++ )
                {
                    WingSect* wing_sec = wing->GetWingSect( i );

                    if ( wing_sec )
                    {
                        VspCurve xsec_curve = wing_sec->GetCurve();
                        BndBox bbox;
                        xsec_curve.GetBoundingBox( bbox );
                        sect_bbox.Update( bbox );
                    }
                }
            }
            else
            {
                sect_bbox = wing_bbox;
            }

            FeaSlice* temp_slice = NULL;
            temp_slice = new FeaSlice( m_ParentGeomID );

            if ( temp_slice )
            {
                temp_slice->SetCenter( center );
                temp_slice->SetSectionBBox( sect_bbox );
                temp_slice->m_OrientationPlane.Set( vsp::XZ_BODY );
                temp_slice->m_ZRot.Set( RAD_2_DEG * m_TotRot );

                // Update Slice Relative Center Location
                double rel_center_location =( center.y() - sect_bbox.GetMin( 1 ) ) / ( sect_bbox.GetMax( 1 ) - sect_bbox.GetMin( 1 ) );
                temp_slice->m_RelCenterLocation.Set( rel_center_location );

                rib_surf = temp_slice->ComputeSliceSurf();

                delete temp_slice;
            }
        }
        else
        {
            // Identify expansion 
            double expan = wing_bbox.GetLargestDist() * 1e-3;
            if ( expan < 1e-5 )
            {
                expan = 1e-5;
            }

            double length_rib_0 = ( dist( trail_edge, lead_edge ) / 2 ) + expan; // Rib half length before rotations, slightly oversized

            vec3d center_to_trail_edge = trail_edge - center;
            center_to_trail_edge.normalize();

            vec3d center_to_lead_edge = center - lead_edge;
            center_to_lead_edge.normalize();

            // Determine angle between center and corner points
            vec3d center_to_le_min_vec = min_lead_edge - center;
            vec3d center_to_te_min_vec = min_trail_edge - center;
            vec3d center_to_le_max_vec = max_lead_edge - center;
            vec3d center_to_te_max_vec = max_trail_edge - center;

            center_to_le_min_vec.normalize();
            center_to_te_min_vec.normalize();
            center_to_le_max_vec.normalize();
            center_to_te_max_vec.normalize();

            // Get maximum angles for rib to intersect wing edges
            double max_angle_inner_le = -PI + signed_angle( center_to_le_min_vec, center_to_lead_edge, m_WingNorm );
            double max_angle_inner_te = signed_angle( center_to_te_min_vec, center_to_trail_edge, m_WingNorm );
            double max_angle_outer_le = PI - signed_angle( center_to_lead_edge, center_to_le_max_vec, m_WingNorm );
            double max_angle_outer_te = signed_angle( center_to_te_max_vec, center_to_trail_edge, m_WingNorm );

            double sweep_te = -1 * signed_angle( trail_edge_vec, center_to_trail_edge, m_WingNorm ); // Trailing edge sweep
            double sweep_le = -1 * signed_angle( lead_edge_vec, center_to_lead_edge, m_WingNorm ); // Leading edge sweep

            double phi_te = PI - ( m_TotRot + sweep_te ); // Total angle for trailing edge side of rib
            double phi_le = PI - ( m_TotRot + sweep_le );// Total angle for leading edge side of rib

            double length_rib_te, length_rib_le, perp_dist;

            // Determine if the rib intersects the leading/trailing edge or inner/outer edge
            if ( m_TotRot <= 0 )
            {
                if ( m_TotRot <= max_angle_inner_le )
                {
                    if ( abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( min_lead_edge - min_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_le = length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_trail_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - min_trail_edge ).mag();
                        length_rib_le = abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( abs( sin( phi_le ) ) <= FLT_EPSILON )
                    {
                        length_rib_le = length_rib_0;
                    }
                    else
                    {
                        length_rib_le = abs( length_rib_0 * sin( sweep_le ) / sin( phi_le ) );
                    }
                }

                if ( m_TotRot <= max_angle_outer_te )
                {
                    if ( abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( max_lead_edge - max_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_te = length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - max_lead_edge ) ).mag() / ( max_lead_edge - max_trail_edge ).mag();
                        length_rib_te = abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( abs( sin( phi_te ) ) <= FLT_EPSILON )
                    {
                        length_rib_te = length_rib_0;
                    }
                    else
                    {
                        length_rib_te = abs( length_rib_0 * sin( sweep_te ) / sin( phi_te ) );
                    }
                }
            }
            else
            {
                if ( m_TotRot >= max_angle_inner_te )
                {
                    if ( abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( min_lead_edge - min_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_te = length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_trail_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - min_trail_edge ).mag();
                        length_rib_te = abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( abs( sin( phi_te ) ) <= FLT_EPSILON )
                    {
                        length_rib_te = length_rib_0;
                    }
                    else
                    {
                        length_rib_te = abs( length_rib_0 * sin( sweep_te ) / sin( phi_te ) );
                    }
                }

                if ( m_TotRot >= max_angle_outer_le )
                {
                    if ( abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( max_lead_edge - max_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_le = length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - max_lead_edge ) ).mag() / ( max_lead_edge - max_trail_edge ).mag();
                        length_rib_le = abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( abs( sin( phi_le ) ) <= FLT_EPSILON )
                    {
                        length_rib_le = length_rib_0;
                    }
                    else
                    {
                        length_rib_le = abs( length_rib_0 * sin( sweep_le ) / sin( phi_le ) );
                    }
                }
            }

            // Apply Rodrigues' Rotation Formula
            vec3d rib_vec_te = center_to_trail_edge * cos( m_TotRot ) + cross( center_to_trail_edge, m_WingNorm ) * sin( m_TotRot ) + m_WingNorm * dot( center_to_trail_edge, m_WingNorm ) * ( 1 - cos( m_TotRot ) );
            vec3d rib_vec_le = center_to_lead_edge * cos( m_TotRot ) + cross( center_to_lead_edge, m_WingNorm ) * sin( m_TotRot ) + m_WingNorm * dot( center_to_lead_edge, m_WingNorm ) * ( 1 - cos( m_TotRot ) );

            rib_vec_te.normalize();
            rib_vec_le.normalize();

            // Calculate final end points
            vec3d trail_edge_f = center + length_rib_te * rib_vec_te;
            vec3d lead_edge_f = center - length_rib_le * rib_vec_le;

            // Identify corners of the plane
            vec3d cornerA, cornerB, cornerC, cornerD;

            double height = 0.5 * wing_bbox.GetSmallestDist() + expan; // Height of Rib, slightly oversized

            cornerA = trail_edge_f + ( height * wing_z_axis );
            cornerB = trail_edge_f - ( height * wing_z_axis );
            cornerC = lead_edge_f + ( height * wing_z_axis );
            cornerD = lead_edge_f - ( height * wing_z_axis );

            // Make Planar Surface
            rib_surf.MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            // Transform to body coordinate frame
            model_matrix.affineInverse();
            rib_surf.Transform( model_matrix );
        }

        // Set Surface CFD Type: 
        if ( m_IncludedElements() == vsp::FEA_SHELL || m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
        {
            rib_surf.SetSurfCfdType( vsp::CFD_STRUCTURE );
        }
        else
        {
            rib_surf.SetSurfCfdType( vsp::CFD_STIFFENER );
        }

        if ( rib_surf.GetFlipNormal() != wing_surf.GetFlipNormal() )
        {
            rib_surf.FlipNormal();
        }
    }

    return rib_surf;
}

void FeaRib::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

////////////////////////////////////////////////////
//================= FeaFixPoint ==================//
////////////////////////////////////////////////////

FeaFixPoint::FeaFixPoint( string compID, string partID, int type ) : FeaPart( compID, type )
{
    m_ParentFeaPartID = partID;

    m_PosU.Init( "PosU", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosU.SetDescript( "Precent U Location" );

    m_PosW.Init( "PosW", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosW.SetDescript( "Precent W Location" );

    m_FixPointMassFlag.Init( "FixPointMassFlag", "FeaFixPoint", this, false, false, true );
    m_FixPointMassFlag.SetDescript( "Flag to Include Mass of FeaFixPoint" );

    m_FixPointMass.Init( "FixPointMass", "FeaFixPoint", this, 0.0, 0.0, 1e12 );
    m_FixPointMass.SetDescript( "FeaFixPoint Mass Value" );

    m_FeaPropertyIndex = -1; // No property
    m_CapFeaPropertyIndex = -1; // No property
    m_BorderFlag = false;
}

void FeaFixPoint::Update()
{
    m_FeaPartSurfVec.clear(); // FeaFixPoints are not a VspSurf
}

bool FeaFixPoint::PlaneAtYZero( piecewise_surface_type & surface ) const
{
    // PlaneAtZero is very similar to the function of the same name in SurfCore. It takes a piecewise surface
    //  as an input to determine if the surface contains points less than y = 0;
    double tol = 1.0e-6;

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    nupatch = surface.number_u_patches();
    nvpatch = surface.number_v_patches();

    for ( ip = 0; ip < nupatch; ++ip )
    {
        for ( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            const surface_patch_type *patch = surface.get_patch( ip, jp );

            for ( icp = 0; icp <= patch->degree_u(); ++icp )
            {
                for ( jcp = 0; jcp <= patch->degree_v(); ++jcp )
                {
                    piecewise_surface_type::point_type cp;
                    cp = patch->get_control_point( icp, jcp );
                    if ( std::abs( cp.y() ) > tol )
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool FeaFixPoint::LessThanY( piecewise_surface_type & surface, double val ) const
{
    // LessThanY is very similar to the function of the same name in SurfCore. It takes a piecewise surface
    //  as an input to determine if the surface contains points less than y = val;
    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;
    nupatch = surface.number_u_patches();
    nvpatch = surface.number_v_patches();

    for ( ip = 0; ip < nupatch; ++ip )
    {
        for ( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            const surface_patch_type *patch = surface.get_patch( ip, jp );

            for ( icp = 0; icp <= patch->degree_u(); ++icp )
            {
                for ( jcp = 0; jcp <= patch->degree_v(); ++jcp )
                {
                    piecewise_surface_type::point_type cp;
                    cp = patch->get_control_point( icp, jcp );
                    if ( cp.y() > val )
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void FeaFixPoint::IdentifySplitSurfIndex( bool half_mesh_flag, const vector < double > &usuppress, const vector < double > &wsuppress )
{
    // This function is called instead of FeaPart::FetchFeaXFerSurf when the FeaPart type is FEA_FIX_POINT, since 
    //  FeaFixPoints are not surfaces. This function determines the number of split surfaces for the FeaFixPoint 
    //  Parent Surface, and determines which split surface the FeaFixPoint lies on.

    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !parent_part || !veh )
    {
        return;
    }

    vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();

    m_SplitSurfIndex.clear();
    m_SplitSurfIndex.resize( parent_surf_vec.size() );

    for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
    {
        // Get FeaFixPoint U/W values
        vec2d uw = GetUW();

        double parent_Umax = parent_surf_vec[i].GetUMax();
        double parent_Wmax = parent_surf_vec[i].GetWMax();
        double parent_Wmin = 0.0;
        double parent_Umin = 0.0;

        // Check if U/W is closed, in which case the minimum and maximum U/W will be at the same point
        bool closedU = parent_surf_vec[i].IsClosedU();
        bool closedW = parent_surf_vec[i].IsClosedW();

        // Split the parent surface
        vector< XferSurf > tempxfersurfs;
        parent_surf_vec[i].FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx(), 0, tempxfersurfs, usuppress, wsuppress );

        // Check if the UW point is on a valid patch (invalid patches are discarded in FetchXFerSurf)
        bool on_valid_patch = false;

        int num_split_surfs = tempxfersurfs.size();

        // Check if the UW point is on a valid patch (invalid patches are discarded in FetchXFerSurf)
        for ( size_t j = 0; j < num_split_surfs; j++ )
        {
            double umin = tempxfersurfs[j].m_Surface.get_u0();
            double umax = tempxfersurfs[j].m_Surface.get_umax();
            double vmin = tempxfersurfs[j].m_Surface.get_v0();
            double vmax = tempxfersurfs[j].m_Surface.get_vmax();

            if ( uw[1] >= vmin && uw[1] <= vmax && uw[0] >= umin && uw[0] <= umax )
            {
                on_valid_patch = true; // The point is on the patch
            }
        }

        for ( size_t j = 0; j < num_split_surfs; j++ )
        {
            bool addSurfFlag = true;

            if ( half_mesh_flag && LessThanY( tempxfersurfs[j].m_Surface, 1e-6 ) )
            {
                addSurfFlag = false;
            }

            if ( half_mesh_flag && PlaneAtYZero( tempxfersurfs[j].m_Surface ) )
            {
                addSurfFlag = false;
            }

            if ( addSurfFlag )
            {
                double umax = tempxfersurfs[j].m_Surface.get_umax();
                double umin = tempxfersurfs[j].m_Surface.get_u0();
                double vmax = tempxfersurfs[j].m_Surface.get_vmax();
                double vmin = tempxfersurfs[j].m_Surface.get_v0();

                if ( parent_surf_vec[i].IsMagicVParm() && !on_valid_patch )
                {
                    vmin -= TMAGIC;
                    vmax += TMAGIC;
                }

                // Check if FeaFixPoint is on XferSurf or border curve. Note: Not all cases of FeaFixPoints on constant UW intersection curves 
                //  are checked, since a node will always be placed there automatically
                if ( uw.x() > umin && uw.x() < umax && uw.y() > vmin && uw.y() < vmax ) // FeaFixPoint on surface
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = false;
                }
                else if ( ( uw.x() > umin && uw.x() < umax ) && ( uw.y() == vmin || uw.y() == vmax ) ) // FeaFixPoint on constant W border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( uw.x() == umin || uw.x() == umax ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant U border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( uw.x() == umin || uw.x() == umax ) && ( uw.y() == vmin || uw.y() == vmax ) ) // FeaFixPoint on constant UW intersection (already a node)
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( closedU && umax == parent_Umax && uw.x() == parent_Umin ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant closedU border 
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( closedU && umin == parent_Umin && uw.y() == parent_Umax ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant closedU border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( uw.x() > umin && uw.x() < umax ) && ( closedW && vmax == parent_Wmax && uw.y() == parent_Wmin ) ) // FeaFixPoint on constant closedW border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( uw.x() > umin && uw.x() < umax ) && ( closedW && vmin == parent_Wmin && uw.y() == parent_Wmax ) ) // FeaFixPoint on constant closedW border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
            }
        }
    }
}

vector < vec3d > FeaFixPoint::GetPntVec()
{
    vector < vec3d > pnt_vec;

    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();
        pnt_vec.resize( parent_surf_vec.size() );

        for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
        {
            pnt_vec[i] = parent_surf_vec[i].CompPnt01( m_PosU(), m_PosW() );
        }
    }
    return pnt_vec;
}

vec2d FeaFixPoint::GetUW()
{
    vec2d uw;

    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();

        if ( parent_surf_vec.size() > 0 ) // Only consider main parent surface (same UW for symmetric copies)
        { 
            uw.set_x( parent_surf_vec[0].GetUMax() * m_PosU() );
            uw.set_y( parent_surf_vec[0].GetWMax() * m_PosW() );
        }
    }
    return uw;
}

xmlNodePtr FeaFixPoint::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_prt_node = FeaPart::EncodeXml( node );

    if ( fea_prt_node )
    {
        XmlUtil::AddStringNode( fea_prt_node, "ParentFeaPartID", m_ParentFeaPartID );
    }

    return fea_prt_node;
}

xmlNodePtr FeaFixPoint::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_prt_node = FeaPart::DecodeXml( node );

    if ( fea_prt_node )
    {
        m_ParentFeaPartID = XmlUtil::FindString( fea_prt_node, "ParentFeaPartID", m_ParentFeaPartID );
    }

    return fea_prt_node;
}

void FeaFixPoint::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();

        for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
        {
            m_FeaPartDO.resize( parent_surf_vec.size() );

            m_FeaPartDO[i].m_PntVec.clear();

            m_FeaPartDO[i].m_GeomID = string( "FeaFixPoint_" + std::to_string( id ) + "_" + std::to_string( i ) );
            m_FeaPartDO[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            m_FeaPartDO[i].m_Type = DrawObj::VSP_POINTS;
            m_FeaPartDO[i].m_PointSize = 8.0;

            if ( highlight )
            {
                m_FeaPartDO[i].m_PointColor = vec3d( 1.0, 0.0, 0.0 );
            }
            else
            {
                m_FeaPartDO[i].m_PointColor = vec3d( 0.0, 0.0, 0.0 );
            }

            vec3d fixpt = parent_surf_vec[i].CompPnt01( m_PosU(), m_PosW() );
            m_FeaPartDO[i].m_PntVec.push_back( fixpt );

            m_FeaPartDO[i].m_GeomChanged = true;
        }
    }
}

bool FeaFixPoint::PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind )
{
    return false;
}

////////////////////////////////////////////////////
//=================== FeaSkin ====================//
////////////////////////////////////////////////////

FeaSkin::FeaSkin( string geomID, int type ) : FeaPart( geomID, type )
{
    m_IncludedElements.Set( vsp::FEA_SHELL );
    m_DrawFeaPartFlag.Set( false );

    m_RemoveSkinTrisFlag.Init( "RemoveSkinTrisFlag", "FeaSkin", this, false, false, true );
    m_RemoveSkinTrisFlag.SetDescript( "Flag to Remove Skin Triangles" );
}

void FeaSkin::Update()
{
    BuildSkinSurf();
}

void FeaSkin::BuildSkinSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* currgeom = veh->FindGeom( m_ParentGeomID );

        if ( currgeom )
        {
            vector< VspSurf > surf_vec;
            currgeom->GetSurfVec( surf_vec );

            m_FeaPartSurfVec[0] = surf_vec[m_SymmIndexVec[0]];

            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_NORMAL );

            // Using the primary m_FeaPartSurfVec (index 0) as a reference, calculate and transform the symmetric copies
            for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
            {
                m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
            }
        }
    }
}

bool FeaSkin::PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind )
{
    return false;
}

////////////////////////////////////////////////////
//================= FeaDome ==================//
////////////////////////////////////////////////////

FeaDome::FeaDome( string geomID, int type ) : FeaPart( geomID, type )
{
    m_Aradius.Init( "A_Radius", "FeaDome", this, 1.0, 0.0, 1.0e12 );
    m_Aradius.SetDescript( "A (x) Radius of Dome" );

    m_Bradius.Init( "B_Radius", "FeaDome", this, 1.0, 0.0, 1.0e12 );
    m_Bradius.SetDescript( "B (y) Radius of Dome" );

    m_Cradius.Init( "C_Radius", "FeaDome", this, 1.0, 0.0, 1.0e12 );
    m_Cradius.SetDescript( "C (z) Radius of Dome" );

    m_XLoc.Init( "X_Location", "FeaDome", this, 0.0, -1.0e12, 1.0e12 );
    m_XLoc.SetDescript( "Location Along Body X Axis" );

    m_YLoc.Init( "Y_Location", "FeaDome", this, 0.0, -1.0e12, 1.0e12 );
    m_YLoc.SetDescript( "Location Along Body Y Axis" );

    m_ZLoc.Init( "Z_Location", "FeaDome", this, 0.0, -1.0e12, 1.0e12 );
    m_ZLoc.SetDescript( "Location Along Body Z Axis" );

    m_XRot.Init( "X_Rotation", "FeaDome", this, 0.0, -180, 180 );
    m_XRot.SetDescript( "Rotation About Body X Axis" );

    m_YRot.Init( "Y_Rotation", "FeaDome", this, 0.0, -180, 180 );
    m_YRot.SetDescript( "Rotation About Body Y Axis" );

    m_ZRot.Init( "Z_Rotation", "FeaDome", this, 0.0, -180, 180 );
    m_ZRot.SetDescript( "Rotation About Body Z Axis" );

    m_SpineAttachFlag.Init( "SpineAttachFlag", "FeaDome", this, false, false, true );
    m_SpineAttachFlag.SetDescript( "Flag to Attach to Spine of Parent Geom" );

    m_USpineLoc.Init( "USpineLoc", "FeaDome", this, 0.0, 0.0, 1.0 );
    m_USpineLoc.SetDescript( "Location of Dome Center Along Spine of Parent Geom" );

    m_FlipDirectionFlag.Init( "FlipDirectionFlag", "FeaDome", this, false, false, true );
    m_FlipDirectionFlag.SetDescript( "Flag to Flip the Direction of the FeaDome" );
}

void FeaDome::Update()
{
    BuildDomeSurf();
}

typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_dome_creator;

void FeaDome::BuildDomeSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* curr_geom = veh->FindGeom( m_ParentGeomID );

        if ( !curr_geom || m_FeaPartSurfVec.size() == 0 )
        {
            return;
        }

        m_FeaPartSurfVec[0] = VspSurf(); // Create primary VspSurf

        if ( m_IncludedElements() == vsp::FEA_SHELL || m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
        {
            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STRUCTURE );
        }
        else
        {
            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STIFFENER );
        }

        // Build unit circle
        piecewise_curve_type c, c1, c2;
        piecewise_dome_creator pbc( 4 );
        curve_point_type origin, normal;

        origin << 0, 0, 0;
        normal << 0, 1, 0;

        pbc.set_origin( origin );
        pbc.set_x_axis_radius( 1.0 );
        pbc.set_y_axis_radius( 1.0 );

        // set circle params, make sure that entire curve goes from 0 to 4
        pbc.set_t0( 0 );
        pbc.set_segment_dt( 1, 0 );
        pbc.set_segment_dt( 1, 1 );
        pbc.set_segment_dt( 1, 2 );
        pbc.set_segment_dt( 1, 3 );

        pbc.create( c );

        c.split( c1, c2, 1.0 ); // Create half sphere

        VspCurve stringer;
        stringer.SetCurve( c1 );

        if ( m_FlipDirectionFlag() )
        {
            stringer.ReflectYZ();
        }

        // Revolve to unit sphere
        m_FeaPartSurfVec[0].CreateBodyRevolution( stringer );

        // Scale to ellipsoid
        m_FeaPartSurfVec[0].ScaleX( m_Aradius() );
        m_FeaPartSurfVec[0].ScaleY( m_Bradius() );
        m_FeaPartSurfVec[0].ScaleZ( m_Cradius() );

        // Rotate at orgin and then translate to final location
        Matrix4d rot_mat_x, rot_mat_y, rot_mat_z;
        vec3d x_axis, y_axis, z_axis;

        x_axis.set_x( 1.0 );
        y_axis.set_y( 1.0 );
        z_axis.set_z( 1.0 );

        rot_mat_x.loadIdentity();
        rot_mat_x.rotate( DEG_2_RAD * m_XRot(), x_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat_x );

        rot_mat_y.loadIdentity();
        rot_mat_y.rotate( DEG_2_RAD * m_YRot(), y_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat_y );

        rot_mat_z.loadIdentity();
        rot_mat_z.rotate( DEG_2_RAD * m_ZRot(), z_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat_z );

        m_FeaPartSurfVec[0].OffsetX( m_XLoc() );
        m_FeaPartSurfVec[0].OffsetY( m_YLoc() );
        m_FeaPartSurfVec[0].OffsetZ( m_ZLoc() );

        // Transform to parent geom body coordinate frame
        Matrix4d model_matrix = curr_geom->getModelMatrix();
        m_FeaPartSurfVec[0].Transform( model_matrix );

        if ( m_SpineAttachFlag() )
        {
            vector< VspSurf > surf_vec;
            curr_geom->GetSurfVec( surf_vec );
            VspSurf current_surf = surf_vec[m_MainSurfIndx()];

            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( current_surf );

            vec3d spine_center = cs.FindCenterGivenU( m_USpineLoc() * current_surf.GetUMax() );

            m_FeaPartSurfVec[0].OffsetX( spine_center.x() - curr_geom->m_XLoc() );
            m_FeaPartSurfVec[0].OffsetY( spine_center.y() - curr_geom->m_YLoc() );
            m_FeaPartSurfVec[0].OffsetZ( spine_center.z() - curr_geom->m_ZLoc() );
        }

        m_FeaPartSurfVec[0].BuildFeatureLines();

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, setup the symmetric copies to be definied in UpdateSymmParts 
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }
    }
}

void FeaDome::UpdateDrawObjs( int id, bool highlight )
{
    // Two DrawObjs per Dome surface: index j correcponds to the surface (quads) and 
    //  j + 1 corresponds to the cross section feature line at u_max 

    m_FeaPartDO.clear();
    m_FeaPartDO.resize( 2 * m_FeaPartSurfVec.size() );

    for ( size_t j = 0; j < 2 * m_FeaPartSurfVec.size(); j += 2 )
    {
        m_FeaPartDO[j].m_GeomID = string( m_Name + "_" + std::to_string( id ) + "_" + std::to_string( j ) );
        m_FeaPartDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        m_FeaPartDO[j + 1].m_GeomID = string( m_Name + "_" + std::to_string( id ) + "_" + std::to_string( j + 1 ) );
        m_FeaPartDO[j + 1].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        if ( highlight )
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
            m_FeaPartDO[j].m_LineWidth = 3.0;
            m_FeaPartDO[j + 1].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
            m_FeaPartDO[j + 1].m_LineWidth = 3.0;
        }
        else
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
            m_FeaPartDO[j].m_LineWidth = 1.0;
            m_FeaPartDO[j + 1].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
            m_FeaPartDO[j + 1].m_LineWidth = 1.0;
        }

        // Tesselate the surface (can adjust num_u and num_v tesselation for smoothness) 
        vector < vector < vec3d > > pnts, norms, uw;
        m_FeaPartSurfVec[j / 2].Tesselate( 10, 18, pnts, norms, uw, 3, false );

        // Define quads for bulkhead surface
        m_FeaPartDO[j].m_Type = DrawObj::VSP_SHADED_QUADS;

        for ( size_t i = 0; i < pnts.size() - 1; i++ )
        {
            for ( size_t k = 0; k < pnts[i].size() - 1; k++ )
            {
                // Define quads
                vec3d corner1, corner2, corner3, corner4, norm;

                corner1 = pnts[i][k];
                corner2 = pnts[i + 1][k];
                corner3 = pnts[i + 1][k + 1];
                corner4 = pnts[i][k + 1];

                m_FeaPartDO[j].m_PntVec.push_back( corner1 );
                m_FeaPartDO[j].m_PntVec.push_back( corner2 );
                m_FeaPartDO[j].m_PntVec.push_back( corner3 );
                m_FeaPartDO[j].m_PntVec.push_back( corner4 );

                norm = norms[i][k];

                // Set normal vector
                for ( int m = 0; m < 4; m++ )
                {
                    m_FeaPartDO[j].m_NormVec.push_back( norm );
                }
            }
        }

        // Set plane color to medium glass
        for ( size_t i = 0; i < 4; i++ )
        {
            m_FeaPartDO[j].m_MaterialInfo.Ambient[i] = 0.2f;
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[i] = 0.1f;
            m_FeaPartDO[j].m_MaterialInfo.Specular[i] = 0.7f;
            m_FeaPartDO[j].m_MaterialInfo.Emission[i] = 0.0f;
        }

        if ( highlight )
        {
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.67f;
        }
        else
        {
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.33f;
        }

        m_FeaPartDO[j].m_MaterialInfo.Shininess = 5.0f;

        // Add points for bulkhead cross section at u_max
        m_FeaPartDO[j + 1].m_Type = DrawObj::VSP_LINE_LOOP;

        for ( size_t i = 0; i < pnts[pnts.size() - 1].size(); i++ )
        {
            m_FeaPartDO[j + 1].m_PntVec.push_back( pnts[pnts.size() - 1][i] );
        }

        m_FeaPartDO[j].m_GeomChanged = true;
        m_FeaPartDO[j + 1].m_GeomChanged = true;
    }
}

bool FeaDome::PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind )
{
    return false;
}

////////////////////////////////////////////////////
//================= FeaRibArray ==================//
////////////////////////////////////////////////////

FeaRibArray::FeaRibArray( string geomID, int type ) : FeaPart( geomID, type )
{
    m_RibAbsSpacing.Init( "RibAbsSpacing", "FeaRibArray", this, 0.1, 1e-6, 1e12 );
    m_RibAbsSpacing.SetDescript( "Absolute Spacing Between Ribs in Array" );

    m_RibRelSpacing.Init( "RibRelSpacing", "FeaRibArray", this, 0.2, 1e-6, 1e12 );
    m_RibRelSpacing.SetDescript( "Relative Spacing Between Ribs in Array" );

    m_PositiveDirectionFlag.Init( "PositiveDirectionFlag", "FeaRibArray", this, true, false, true );
    m_PositiveDirectionFlag.SetDescript( "Flag to Increment RibArray in Positive or Negative Direction" );

    m_AbsStartLocation.Init( "AbsStartLocation", "FeaRibArray", this, 0.0, 0.0, 1e12 );
    m_AbsStartLocation.SetDescript( "Absolute Starting Location for Primary Rib" );

    m_RelStartLocation.Init( "RelStartLocation", "FeaRibArray", this, 0.1, 0.0, 1.0 );
    m_RelStartLocation.SetDescript( "Relative Starting Location for Primary Rib" );

    m_AbsEndLocation.Init( "AbsEndLocation", "FeaRibArray", this, 0.0, 0.0, 1e12 );
    m_AbsEndLocation.SetDescript( "Absolute Location for Final Rib in Array" );

    m_RelEndLocation.Init( "RelEndLocation", "FeaRibArray", this, 0.9, 0.0, 1.0 );
    m_RelEndLocation.SetDescript( "Relative Location for Final Rib in Array" );

    m_Theta.Init( "Theta", "FeaRibArray", this, 0.0, -90.0, 90.0 );
    m_Theta.SetDescript( "Rotation of Each Rib in Array" );

    m_LimitArrayToSectionFlag.Init( "LimitRibToSectionFlag", "FeaRibArray", this, false, false, true );
    m_LimitArrayToSectionFlag.SetDescript( "Flag to Limit Rib Length to Wing Section" );

    m_StartWingSection.Init( "StartWingSection", "FeaRibArray", this, 1, 1, 1000 );
    m_StartWingSection.SetDescript( "Start Wing Section to Limit Array to" );

    m_EndWingSection.Init( "EndWingSection", "FeaRibArray", this, 1, 1, 1000 );
    m_EndWingSection.SetDescript( "End Wing Section to Limit Array to" );

    m_BndBoxTrimFlag.Init( "BndBoxTrimFlag", "FeaRibArray", this, false, false, true );
    m_BndBoxTrimFlag.SetDescript( "Flag to Trim Rib Array to Bounding Box Instead of Wing Surface" );

    m_NumRibs = 0;
}

FeaRibArray::~FeaRibArray()
{

}

void FeaRibArray::Update()
{
    CalcNumRibs();

    m_FeaPartSurfVec.clear();
    m_FeaPartSurfVec.resize( m_SymmIndexVec.size() * m_NumRibs );

    CreateFeaRibArray();
}


void FeaRibArray::CalcNumRibs()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing )
        {
            return;
        }

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        int num_wing_sec = wing->NumXSec();

        m_StartWingSection.SetLowerUpperLimits( 1, m_EndWingSection() );
        m_EndWingSection.SetLowerUpperLimits( m_StartWingSection(), num_wing_sec - 1 );

        // Init values and identify wing section:
        double span_f = 0.0;
        int start_sect, end_sect;

        if ( m_LimitArrayToSectionFlag() )
        {
            start_sect = m_StartWingSection();
            end_sect = m_EndWingSection() + 1;
        }
        else
        {
            start_sect = 1;
            end_sect = num_wing_sec;
        }

        // Determine wing span:
        for ( size_t i = start_sect; i < end_sect; i++ )
        {
            WingSect* wing_sec = wing->GetWingSect( i );

            if ( wing_sec )
            {
                span_f += wing_sec->m_Span();
            }
        }

        m_AbsStartLocation.SetLowerUpperLimits( 0.0, span_f );
        m_RelStartLocation.SetLowerUpperLimits( 0.0, 1.0 );

        // Calculate number of ribs and update Parm limits and values
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            m_AbsStartLocation.Set( m_RelStartLocation() * span_f );
            m_RibAbsSpacing.Set( m_RibRelSpacing() * span_f );
            m_AbsEndLocation.Set( m_RelEndLocation() * span_f );

            if ( m_PositiveDirectionFlag() )
            {
                // Flip start and end values if inconsistent with direction
                if ( m_RelEndLocation() < m_RelStartLocation() ) 
                {
                    double temp_end = m_RelEndLocation();
                    m_RelEndLocation.Set( m_RelStartLocation() );
                    m_RelStartLocation.Set( temp_end );
                }

                m_RelEndLocation.SetLowerUpperLimits( m_RelStartLocation(), 1.0 );
                m_RelStartLocation.SetLowerUpperLimits( 0.0, m_RelEndLocation() );

                m_RibRelSpacing.SetLowerUpperLimits( ( m_RelEndLocation() - m_RelStartLocation() ) / 100, ( m_RelEndLocation() - m_RelStartLocation() ) ); // Limit to 100 ribs
                m_NumRibs = 1 + (int)floor( ( m_RelEndLocation() - m_RelStartLocation() ) / m_RibRelSpacing() );
            }
            else
            {
                // Flip start and end values if inconsistent with direction
                if ( m_RelStartLocation() < m_RelEndLocation() )
                {
                    double temp_start = m_RelStartLocation();
                    m_RelStartLocation.Set( m_RelEndLocation() );
                    m_RelEndLocation.Set( temp_start );
                }

                m_RelStartLocation.SetLowerUpperLimits( m_RelEndLocation(), 1.0 );
                m_RelEndLocation.SetLowerUpperLimits( 0.0, m_RelStartLocation() );

                m_RibRelSpacing.SetLowerUpperLimits( ( m_RelStartLocation() - m_RelEndLocation() ) / 100, ( m_RelStartLocation() - m_RelEndLocation() ) ); // Limit to 100 ribs
                m_NumRibs = 1 + (int)floor( ( m_RelStartLocation() - m_RelEndLocation() ) / m_RibRelSpacing() );
            }
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            m_RelStartLocation.Set( m_AbsStartLocation() / span_f );
            m_RibRelSpacing.Set( m_RibAbsSpacing() / span_f );
            m_RelEndLocation.Set( m_AbsEndLocation() / span_f );

            if ( m_PositiveDirectionFlag() )
            {
                // Flip start and end values if inconsistent with direction
                if ( m_AbsEndLocation() < m_AbsStartLocation() )
                {
                    double temp_end = m_AbsEndLocation();
                    m_AbsEndLocation.Set( m_AbsStartLocation() );
                    m_AbsStartLocation.Set( temp_end );
                }

                m_AbsEndLocation.SetLowerUpperLimits( m_AbsStartLocation(), span_f );
                m_AbsStartLocation.SetLowerUpperLimits( 0.0, m_AbsEndLocation() );

                m_RibAbsSpacing.SetLowerUpperLimits( ( m_AbsEndLocation() - m_AbsStartLocation() ) / 100, ( m_AbsEndLocation() - m_AbsStartLocation() ) ); // Limit to 100 ribs
                m_NumRibs = 1 + (int)floor( ( m_AbsEndLocation() - m_AbsStartLocation() ) / m_RibAbsSpacing() );
            }
            else
            {
                // Flip start and end values if inconsistent with direction
                if ( m_AbsStartLocation() < m_AbsEndLocation() )
                {
                    double temp_start = m_AbsStartLocation();
                    m_AbsStartLocation.Set( m_AbsEndLocation() );
                    m_AbsEndLocation.Set( temp_start );
                }

                m_AbsStartLocation.SetLowerUpperLimits( m_AbsEndLocation(), span_f );
                m_AbsEndLocation.SetLowerUpperLimits( 0.0, m_AbsStartLocation() );

                m_RibAbsSpacing.SetLowerUpperLimits( ( m_AbsStartLocation() - m_AbsEndLocation() ) / 100, ( m_AbsStartLocation() - m_AbsEndLocation() ) ); // Limit to 100 ribs 
                m_NumRibs = 1 + (int)floor( ( m_AbsStartLocation() - m_AbsEndLocation() ) / m_RibAbsSpacing() );
            }
        }

        if ( m_NumRibs < 1 || m_NumRibs > 101 )
        {
            m_NumRibs = 1;
        }
    }
}

void FeaRibArray::CreateFeaRibArray()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing )
        {
            return;
        }

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        vector< VspSurf > surf_vec;
        current_wing->GetSurfVec( surf_vec );
        VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

        BndBox wing_bbox;
        wing_surf.GetBoundingBox( wing_bbox );

        for ( size_t i = 0; i < m_NumRibs; i++ )
        {
            double dir = 1;
            if ( !m_PositiveDirectionFlag() )
            {
                dir = -1;
            }

            // Create a rib to calculate surface from
            FeaRib* rib = NULL;
            rib = new FeaRib( m_ParentGeomID );
            if ( !rib )
            {
                return;
            }

            rib->m_Theta.Set( m_Theta() );
            rib->SetPerpendicularEdgeID( m_PerpendicularEdgeID );
            rib->m_LimitRibToSectionFlag.Set( m_LimitArrayToSectionFlag() );
            rib->m_StartWingSection.Set( m_StartWingSection() );
            rib->m_EndWingSection.Set( m_EndWingSection() );
            rib->m_BndBoxTrimFlag.Set( m_BndBoxTrimFlag() );

            // Update Rib Relative Center Location
            double rel_center_location =  m_RelStartLocation() + dir * i * m_RibRelSpacing();
            rib->m_RelCenterLocation.Set( rel_center_location );

            // Update
            rib->UpdateParmLimits();
            rib->GetRibPerU();

            // Get rib surface
            VspSurf main_rib_surf = rib->ComputeRibSurf();

            m_FeaPartSurfVec[i * m_SymmIndexVec.size()] = main_rib_surf;

            if ( m_FeaPartSurfVec[m_SymmIndexVec.size() * i].GetFlipNormal() != wing_surf.GetFlipNormal() )
            {
                m_FeaPartSurfVec[m_SymmIndexVec.size() * i].FlipNormal();
            }

            // Using the primary m_FeaPartSurfVec (index 0) as a reference, setup the symmetric copiesvto be transformed 
            for ( size_t j = 1; j < m_SymmIndexVec.size(); j++ )
            {
                m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j] = m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j - 1];
            }

            // Get Symmetric Translation Matrix
            vector<Matrix4d> transMats = current_wing->GetFeaTransMatVec();

            //==== Apply Transformations ====//
            for ( size_t j = 1; j < m_SymmIndexVec.size(); j++ )
            {
                m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j].Transform( transMats[j] ); // Apply total transformation to main FeaPart surface

                if ( surf_vec[j].GetFlipNormal() != m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j].GetFlipNormal() )
                {
                    m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j].FlipNormal();
                }
            }

            delete rib;
        }
    }
}

FeaRib* FeaRibArray::AddFeaRib( double center_location, int ind )
{
    FeaRib* fearib = new FeaRib( m_ParentGeomID );

    if ( fearib )
    {
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            fearib->m_RelCenterLocation.Set( center_location );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            fearib->m_AbsCenterLocation.Set( center_location );
        }

        fearib->m_AbsRelParmFlag.Set( m_AbsRelParmFlag() );
        fearib->m_IncludedElements.Set( m_IncludedElements() );
        fearib->m_FeaPropertyIndex.Set( m_FeaPropertyIndex() );
        fearib->m_CapFeaPropertyIndex.Set( m_CapFeaPropertyIndex() );
        fearib->m_Theta.Set( m_Theta() );
        fearib->SetPerpendicularEdgeID( m_PerpendicularEdgeID );
        fearib->m_BndBoxTrimFlag.Set( m_BndBoxTrimFlag() );
        fearib->m_LimitRibToSectionFlag.Set( m_LimitArrayToSectionFlag() );
        fearib->m_StartWingSection.Set( m_StartWingSection() );
        fearib->m_EndWingSection.Set( m_EndWingSection() );

        fearib->SetName( string( m_Name + "_Rib" + std::to_string( ind ) ) );

        fearib->UpdateSymmIndex();
        fearib->Update();
        fearib->UpdateSymmParts();
    }

    return fearib;
}

bool FeaRibArray::PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind )
{
    for ( size_t i = 0; i < m_NumRibs; i++ )
    {
        if ( FeaPart::PtsOnPlanarPart( pnts, i * m_SymmIndexVec.size() ) )
        {
            return true;
        }
    }
    return false;
}

xmlNodePtr FeaRibArray::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_prt_node = FeaPart::EncodeXml( node );

    if ( fea_prt_node )
    {
        XmlUtil::AddStringNode( fea_prt_node, "PerpendicularEdgeID", m_PerpendicularEdgeID );
    }

    return fea_prt_node;
}

xmlNodePtr FeaRibArray::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_prt_node = FeaPart::DecodeXml( node );

    if ( fea_prt_node )
    {
        m_PerpendicularEdgeID = XmlUtil::FindString( fea_prt_node, "PerpendicularEdgeID", m_PerpendicularEdgeID );
    }

    return fea_prt_node;
}

void FeaRibArray::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

////////////////////////////////////////////////////
//================= FeaSliceArray ==================//
////////////////////////////////////////////////////

FeaSliceArray::FeaSliceArray( string geomID, int type ) : FeaPart( geomID, type )
{
    m_SliceAbsSpacing.Init( "SliceAbsSpacing", "FeaSliceArray", this, 0.2, 1e-6, 1e12 );
    m_SliceAbsSpacing.SetDescript( "Absolute Spacing Between Slices in Array" );

    m_SliceRelSpacing.Init( "SliceRelSpacing", "FeaSliceArray", this, 0.2, 1e-6, 1.0 );
    m_SliceRelSpacing.SetDescript( "Relative Spacing Between Slices in Array" );

    m_PositiveDirectionFlag.Init( "PositiveDirectionFlag", "FeaSliceArray", this, true, false, true );
    m_PositiveDirectionFlag.SetDescript( "Flag to Increment SliceArray in Positive or Negative Direction" );

    m_AbsStartLocation.Init( "AbsStartLocation", "FeaSliceArray", this, 0.0, 0.0, 1e12 );
    m_AbsStartLocation.SetDescript( "Absolute Starting Location for First Slice in Array" );

    m_RelStartLocation.Init( "RelStartLocation", "FeaSliceArray", this, 0.0, 0.0, 1.0 );
    m_RelStartLocation.SetDescript( "Relative Starting Location for First Slice in Array" );

    m_AbsEndLocation.Init( "AbsEndLocation", "FeaSliceArray", this, 0.0, 0.0, 1e12 );
    m_AbsEndLocation.SetDescript( "Absolute Location for Final Slice in Array" );

    m_RelEndLocation.Init( "RelEndLocation", "FeaSliceArray", this, 1.0, 0.0, 1.0 );
    m_RelEndLocation.SetDescript( "Relative Location for Final Slice in Array" );

    m_OrientationPlane.Init( "OrientationPlane", "FeaSliceArray", this, vsp::YZ_BODY, vsp::XY_BODY, vsp::SPINE_NORMAL );
    m_OrientationPlane.SetDescript( "Plane the FeaSliceArray will be Parallel to (Body or Absolute Reference Frame)" );

    m_RotationAxis.Init( "RotationAxis", "FeaSliceArray", this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_RotationAxis.SetDescript( "Rotation Axis for Each Slice in Array" );

    m_XRot.Init( "XRot", "FeaSliceArray", this, 0.0, -90.0, 90.0 );
    m_XRot.SetDescript( "Rotation About Each Slice's X Axis" );

    m_YRot.Init( "YRot", "FeaSliceArray", this, 0.0, -90.0, 90.0 );
    m_YRot.SetDescript( "Rotation About Each Slice's Y Axis" );

    m_ZRot.Init( "ZRot", "FeaSliceArray", this, 0.0, -90.0, 90.0 );
    m_ZRot.SetDescript( "Rotation About Each Slice's Z Axis" );

    m_NumSlices = 0;
}

void FeaSliceArray::Update()
{
    CalcNumSlices();

    m_FeaPartSurfVec.clear(); 
    m_FeaPartSurfVec.resize( m_SymmIndexVec.size() * m_NumSlices );

    CreateFeaSliceArray();
}

void FeaSliceArray::CalcNumSlices()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_geom = veh->FindGeom( m_ParentGeomID );

        if ( !current_geom )
        {
            return;
        }

        vector< VspSurf > surf_vec;
        current_geom->GetSurfVec( surf_vec );
        VspSurf current_surf = surf_vec[m_MainSurfIndx()];

        // Determine BndBox dimensions prior to rotating and translating
        Matrix4d model_matrix = current_geom->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = current_surf;
        orig_surf.Transform( model_matrix );

        BndBox geom_bbox;

        if ( RefFrameIsBody( m_OrientationPlane() ) )
        {
            orig_surf.GetBoundingBox( geom_bbox );
        }
        else
        {
            current_surf.GetBoundingBox( geom_bbox );
        }

        double perp_dist; // Total distance perpendicular to the FeaSlice plane

        if ( m_OrientationPlane() == vsp::XY_BODY || m_OrientationPlane() == vsp::XY_ABS )
        {
            perp_dist = geom_bbox.GetMax( 2 ) - geom_bbox.GetMin( 2 );
        }
        else if ( m_OrientationPlane() == vsp::YZ_BODY || m_OrientationPlane() == vsp::YZ_ABS )
        {
            perp_dist = geom_bbox.GetMax( 0 ) - geom_bbox.GetMin( 0 );
        }
        else if ( m_OrientationPlane() == vsp::XZ_BODY || m_OrientationPlane() == vsp::XZ_ABS )
        {
            perp_dist = geom_bbox.GetMax( 1 ) - geom_bbox.GetMin( 1 );
        }
        else if ( m_OrientationPlane() == vsp::SPINE_NORMAL )
        {
            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( current_surf );

            perp_dist = cs.GetSpineLength();
        }

        //Calculate number of slices and update Parm limits and values
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            m_AbsStartLocation.Set( m_RelStartLocation() * perp_dist );
            m_SliceAbsSpacing.Set( m_SliceRelSpacing() * perp_dist );
            m_AbsEndLocation.Set( m_RelEndLocation() * perp_dist );

            if ( m_PositiveDirectionFlag() )
            {
                // Flip start and end values if inconsistent with direction
                if ( m_RelEndLocation() < m_RelStartLocation() )
                {
                    double temp_end = m_RelEndLocation();
                    m_RelEndLocation.Set( m_RelStartLocation() );
                    m_RelStartLocation.Set( temp_end );
                }

                m_RelEndLocation.SetLowerUpperLimits( m_RelStartLocation(), 1.0 );
                m_RelStartLocation.SetLowerUpperLimits( 0.0, m_RelEndLocation() );

                m_SliceRelSpacing.SetLowerUpperLimits( ( m_RelEndLocation() - m_RelStartLocation() ) / 100, ( m_RelEndLocation() - m_RelStartLocation() ) ); // Limit to 100 slices
                m_NumSlices = 1 + (int)floor( ( m_RelEndLocation() - m_RelStartLocation() ) / m_SliceRelSpacing() );
            }
            else
            {
                // Flip start and end values if inconsistent with direction
                if ( m_RelStartLocation() < m_RelEndLocation() )
                {
                    double temp_start = m_RelStartLocation();
                    m_RelStartLocation.Set( m_RelEndLocation() );
                    m_RelEndLocation.Set( temp_start );
                }

                m_RelStartLocation.SetLowerUpperLimits( m_RelEndLocation(), 1.0 );
                m_RelEndLocation.SetLowerUpperLimits( 0.0, m_RelStartLocation() );

                m_SliceRelSpacing.SetLowerUpperLimits( ( m_RelStartLocation() - m_RelEndLocation() ) / 100, ( m_RelStartLocation() - m_RelEndLocation() ) ); // Limit to 100 slices
                m_NumSlices = 1 + (int)floor( ( m_RelStartLocation() - m_RelEndLocation() ) / m_SliceRelSpacing() );
            }
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            m_RelStartLocation.Set( m_AbsStartLocation() / perp_dist );
            m_SliceRelSpacing.Set( m_SliceAbsSpacing() / perp_dist );
            m_RelEndLocation.Set( m_AbsEndLocation() / perp_dist );

            if ( m_PositiveDirectionFlag() )
            {
                // Flip start and end values if inconsistent with direction
                if ( m_AbsEndLocation() < m_AbsStartLocation() )
                {
                    double temp_end = m_AbsEndLocation();
                    m_AbsEndLocation.Set( m_AbsStartLocation() );
                    m_AbsStartLocation.Set( temp_end );
                }

                m_AbsEndLocation.SetLowerUpperLimits( m_AbsStartLocation(), perp_dist );
                m_AbsStartLocation.SetLowerUpperLimits( 0.0, m_AbsEndLocation() );

                m_SliceAbsSpacing.SetLowerUpperLimits( ( m_AbsEndLocation() - m_AbsStartLocation() ) / 100, ( m_AbsEndLocation() - m_AbsStartLocation() ) ); // Limit to 100 slices
                m_NumSlices = 1 + (int)floor( ( m_AbsEndLocation() - m_AbsStartLocation() ) / m_SliceAbsSpacing() );
            }
            else
            {
                // Flip start and end values if inconsistent with direction
                if ( m_AbsStartLocation() < m_AbsEndLocation() )
                {
                    double temp_start = m_AbsStartLocation();
                    m_AbsStartLocation.Set( m_AbsEndLocation() );
                    m_AbsEndLocation.Set( temp_start );
                }

                m_AbsStartLocation.SetLowerUpperLimits( m_AbsEndLocation(), perp_dist );
                m_AbsEndLocation.SetLowerUpperLimits( 0.0, m_AbsStartLocation() );

                m_SliceAbsSpacing.SetLowerUpperLimits( ( m_AbsStartLocation() - m_AbsEndLocation() ) / 100, ( m_AbsStartLocation() - m_AbsEndLocation() ) ); // Limit to 100 slices 
                m_NumSlices = 1 + (int)floor( ( m_AbsStartLocation() - m_AbsEndLocation() ) / m_SliceAbsSpacing() );
            }
        }

        if ( m_NumSlices < 1 || m_NumSlices > 101 )
        {
            m_NumSlices = 1;
        }
    }
}

void FeaSliceArray::CreateFeaSliceArray()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_geom = veh->FindGeom( m_ParentGeomID );

        if ( !current_geom )
        {
            return;
        }

        vector< VspSurf > surf_vec;
        current_geom->GetSurfVec( surf_vec );
        VspSurf current_surf = surf_vec[m_MainSurfIndx()];

        for ( size_t i = 0; i < m_NumSlices; i++ )
        {
            double dir = 1.0;
            if ( !m_PositiveDirectionFlag() )
            {
                dir = -1;
            }

            // Create a temporary slice to calculate surface from
            FeaSlice* slice = NULL;
            slice = new FeaSlice( m_ParentGeomID );
            if ( !slice )
            {
                return;
            }

            slice->m_OrientationPlane.Set( m_OrientationPlane() );
            slice->m_XRot.Set( m_XRot() );
            slice->m_YRot.Set( m_YRot() );
            slice->m_ZRot.Set( m_ZRot() );

            slice->UpdateParmLimits();

            // Update Slice Relative Center Location
            double rel_center_location = m_RelStartLocation() + dir * i * m_SliceRelSpacing();
            slice->m_RelCenterLocation.Set( rel_center_location );

            VspSurf main_slice_surf = slice->ComputeSliceSurf();

            m_FeaPartSurfVec[i * m_SymmIndexVec.size()] = main_slice_surf;

            if ( m_FeaPartSurfVec[m_SymmIndexVec.size() * i].GetFlipNormal() != current_surf.GetFlipNormal() )
            {
                m_FeaPartSurfVec[m_SymmIndexVec.size() * i].FlipNormal();
            }

            // Using the primary m_FeaPartSurfVec (index 0) as a reference, setup the symmetric copiesvto be transformed 
            for ( size_t j = 1; j < m_SymmIndexVec.size(); j++ )
            {
                m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j] = m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j - 1];
            }

            // Get Symmetric Translation Matrix
            vector<Matrix4d> transMats = current_geom->GetFeaTransMatVec();

            //==== Apply Transformations ====//
            for ( size_t j = 1; j < m_SymmIndexVec.size(); j++ )
            {
                m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j].Transform( transMats[j] ); // Apply total transformation to main FeaPart surface

                if ( surf_vec[j].GetFlipNormal() != m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j].GetFlipNormal() )
                {
                    m_FeaPartSurfVec[m_SymmIndexVec.size() * i + j].FlipNormal();
                }
            }

            delete slice;
        }
    }
}

FeaSlice* FeaSliceArray::AddFeaSlice( double center_location, int ind )
{
    FeaSlice* slice = new FeaSlice( m_ParentGeomID );

    if ( slice )
    {
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
            slice->m_RelCenterLocation.Set( center_location );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
            slice->m_AbsCenterLocation.Set( center_location );
        }

        slice->m_OrientationPlane.Set( m_OrientationPlane() );
        slice->m_AbsRelParmFlag.Set( m_AbsRelParmFlag() );
        slice->m_IncludedElements.Set( m_IncludedElements() );
        slice->m_FeaPropertyIndex.Set( m_FeaPropertyIndex() );
        slice->m_CapFeaPropertyIndex.Set( m_CapFeaPropertyIndex() );

        slice->SetName( string( m_Name + "_Slice" + std::to_string( ind ) ) );

        slice->UpdateSymmIndex();
        slice->Update();
        slice->UpdateSymmParts();
    }

    return slice;
}

bool FeaSliceArray::PtsOnPlanarPart( const vector < vec3d > & pnts, int surf_ind )
{
    for ( size_t i = 0; i < m_NumSlices; i++ )
    {
        if ( FeaPart::PtsOnPlanarPart( pnts, i * m_SymmIndexVec.size() ) )
        {
            return true;
        }
    }
    return false;
}

void FeaSliceArray::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

////////////////////////////////////////////////////
//================= FeaProperty ==================//
////////////////////////////////////////////////////

FeaProperty::FeaProperty() : ParmContainer()
{
    m_FeaPropertyType.Init( "FeaPropertyType", "FeaProperty", this, vsp::FEA_SHELL, vsp::FEA_SHELL, vsp::FEA_BEAM );
    m_FeaPropertyType.SetDescript( "FeaElement Property Type" );

    m_Thickness.Init( "Thickness", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Thickness.SetDescript( "Thickness of FeaElement" );

    m_CrossSecArea.Init( "CrossSecArea", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_CrossSecArea.SetDescript( "Cross-Sectional Area of FeaElement" );

    m_Izz.Init( "Izz", "FeaProperty", this, 0.1, -1.0e12, 1.0e12 );
    m_Izz.SetDescript( "Area Moment of Inertia for Bending in XY Plane of FeaElement Neutral Axis (I1)" );

    m_Iyy.Init( "Iyy", "FeaProperty", this, 0.1, -1.0e12, 1.0e12 );
    m_Iyy.SetDescript( "Area Moment of Inertia for Bending in XZ Plane of FeaElement Neutral Axis (I2)" );

    m_Izy.Init( "Izy", "FeaProperty", this, 0.0, -1.0e12, 1.0e12 );
    m_Izy.SetDescript( "Area Product of Inertia of FeaElement (I12)" );

    m_Ixx.Init( "Izz", "FeaProperty", this, 0.0, -1.0e12, 1.0e12 );
    m_Ixx.SetDescript( "Torsional Constant About FeaElement Neutral Axis (J)" );

    m_Dim1.Init( "Dim1", "FeaProperty", this, 0.0, 0.0, 1.0e12 );
    m_Dim1.SetDescript( "First Dimension of the Cross Section" );

    m_Dim2.Init( "Dim2", "FeaProperty", this, 0.0, 0.0, 1.0e12 );
    m_Dim2.SetDescript( "Second Dimension of the Cross Section" );

    m_Dim3.Init( "Dim3", "FeaProperty", this, 0.0, 0.0, 1.0e12 );
    m_Dim3.SetDescript( "Third Dimension of the Cross Section" );

    m_Dim4.Init( "Dim4", "FeaProperty", this, 0.0, 0.0, 1.0e12 );
    m_Dim4.SetDescript( "Fourth Dimension of the Cross Section" );

    m_Dim5.Init( "Dim5", "FeaProperty", this, 0.0, 0.0, 1.0e12 );
    m_Dim5.SetDescript( "Fifth Dimension of the Cross Section" );

    m_Dim6.Init( "Dim6", "FeaProperty", this, 0.0, 0.0, 1.0e12 );
    m_Dim6.SetDescript( "Sixth Dimension of the Cross Section" );

    m_CrossSectType.Init( "CrossSectType", "FeaProperty", this, vsp::FEA_XSEC_GENERAL, vsp::FEA_XSEC_GENERAL, vsp::FEA_XSEC_BOX );
    m_CrossSectType.SetDescript( "Cross Section Type" );

    m_FeaMaterialIndex.Init( "FeaMaterialIndex", "FeaProperty", this, 0, 0, 1e12 );
    m_FeaMaterialIndex.SetDescript( "FeaMaterial Index for FeaProperty" );
}

FeaProperty::~FeaProperty()
{

}

void FeaProperty::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr FeaProperty::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr prop_info = xmlNewChild( node, NULL, BAD_CAST "FeaPropertyInfo", NULL );

    ParmContainer::EncodeXml( prop_info );

    return prop_info;
}

xmlNodePtr FeaProperty::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    return node;
}

string FeaProperty::GetTypeName( )
{
    if ( m_FeaPropertyType() == vsp::FEA_SHELL )
    {
        return string( "Shell" );
    }
    if ( m_FeaPropertyType() == vsp::FEA_BEAM )
    {
        return string( "Beam" );
    }

    return string( "NONE" );
}

string FeaProperty::GetXSecName()
{
    if ( m_CrossSectType() == vsp::FEA_XSEC_GENERAL )
    {
        return string( "General" );
    }
    if ( m_CrossSectType() == vsp::FEA_XSEC_CIRC )
    {
        return string( "Circle" );
    }
    if ( m_CrossSectType() == vsp::FEA_XSEC_PIPE )
    {
        return string( "Pipe" );
    }
    if ( m_CrossSectType() == vsp::FEA_XSEC_I )
    {
        return string( "I" );
    }
    if ( m_CrossSectType() == vsp::FEA_XSEC_RECT )
    {
        return string( "Rectangle" );
    }
    if ( m_CrossSectType() == vsp::FEA_XSEC_BOX )
    {
        return string( "Box" );
    }

    return string( "NONE" );
}

////////////////////////////////////////////////////
//================= FeaMaterial ==================//
////////////////////////////////////////////////////

FeaMaterial::FeaMaterial() : ParmContainer()
{
    m_MassDensity.Init( "MassDensity", "FeaMaterial", this, 1.0, 0.0, 1.0e12 );
    m_MassDensity.SetDescript( "Mass Density of Material" );

    m_ElasticModulus.Init( "ElasticModulus", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_ElasticModulus.SetDescript( "Elastic (Young's) Modulus for Material" );

    m_PoissonRatio.Init( "PoissonRatio", "FeaMaterial", this, 0.0, 0.0, 1.0 );
    m_PoissonRatio.SetDescript( "Poisson's Ratio for Material" );

    m_ThermalExpanCoeff.Init( "ThermalExpanCoeff", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_ThermalExpanCoeff.SetDescript( "Thermal Expansion Coefficient for Material" );
}

FeaMaterial::~FeaMaterial()
{

}

void FeaMaterial::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr FeaMaterial::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr mat_info = xmlNewChild( node, NULL, BAD_CAST "FeaMaterialInfo", NULL );

    ParmContainer::EncodeXml( mat_info );

    return mat_info;
}

xmlNodePtr FeaMaterial::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    return node;
}

double FeaMaterial::GetShearModulus()
{
    return ( m_ElasticModulus() / ( 2 * ( m_PoissonRatio() + 1 ) ) );
}
