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
#include "WingGeom.h"
#include "ConformalGeom.h"
#include "UnitConversion.h"

#include <cfloat>

//////////////////////////////////////////////////////
//================== FeaStructure ==================//
//////////////////////////////////////////////////////

FeaStructure::FeaStructure( const string& geomID, int surf_index )
{
    m_ParentGeomID = geomID;
    m_MainSurfIndx = surf_index;

    m_FeaPartCount = 0;
    m_FeaSubSurfCount = 0;
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

void FeaStructure::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* geom = veh->FindGeom( m_ParentGeomID );
        if ( geom  )
        {
            geom->ParmChanged( parm_ptr, type );
        }
    }
}

xmlNodePtr FeaStructure::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr fea_info = xmlNewChild( node, NULL, BAD_CAST "FeaStructureInfo", NULL );

    ParmContainer::EncodeXml( fea_info );

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
    ParmContainer::DecodeXml( node );

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

void FeaStructure::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    ReSuffixGroupNames();

    for ( size_t i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }

    for ( size_t i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
    {
        m_FeaSubSurfVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }

    m_StructSettings.AddLinkableParms( linkable_parm_vec, m_ID );
    m_FeaGridDensity.AddLinkableParms( linkable_parm_vec, m_ID );
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

void FeaStructure::ReSuffixGroupNames()
{
    // Note: This function differs from SubSurfaceMgr.ReSuffixGroupNames in the use of the FeaPart/SubSurface index
    //  as the suffix. This avoids different instances of the base FeaPart or SubSurface class having the same suffix. 

    for ( size_t i = 0; i < m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->SetDisplaySuffix( i );
    }

    for ( size_t i = 0; i < m_FeaSubSurfVec.size(); i++ )
    {
        m_FeaSubSurfVec[i]->SetDisplaySuffix( i );
    }
}

FeaPart* FeaStructure::AddFeaPart( int type )
{
    FeaPart* feaprt = NULL;

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
    else if ( type == vsp::FEA_TRIM )
    {
        feaprt = new FeaPartTrim( m_ParentGeomID );
        feaprt->SetName( string( "Trim" + std::to_string( m_FeaPartCount ) ) );
    }

    if ( feaprt )
    {
        feaprt->m_MainSurfIndx = m_MainSurfIndx;
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

//==== Highlight Active FeaParts ====//
void FeaStructure::HighlightFeaParts( vector < int > active_ind_vec )
{
    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->SetDrawObjHighlight( false ); // Initially no highlight
    }

    for ( size_t j = 0; j < active_ind_vec.size(); j++ )
    {
        m_FeaPartVec[active_ind_vec[j]]->SetDrawObjHighlight( true );
    }
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
        if ( active_ind_vec[j] < m_FeaSubSurfVec.size() )
        {
            m_FeaSubSurfVec[active_ind_vec[j]]->SetLineColor( vec3d( 1, 0, 0 ) );
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
    else if ( type == vsp::SS_FINITE_LINE )
    {
        ssurf = new SSFiniteLine( m_ParentGeomID );
        ssurf->SetName( string( "SSFiniteLine" + to_string( m_FeaSubSurfCount ) ) );
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
        if ( !FeaPartIsTrim( i ) && !FeaPartIsFixPoint( i ) ) // Update all normal parts first.
        {
            m_FeaPartVec[ i ]->Update();
        }
    }

    for ( unsigned int i = 0; i < m_FeaPartVec.size(); i++ )
    {
        if ( FeaPartIsTrim( i ) || FeaPartIsFixPoint( i ) ) // Update all trim and fix point parts on second pass.
        {
            m_FeaPartVec[i]->Update();
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
                feaskin->m_MainSurfIndx = m_MainSurfIndx;
                
                feaskin->Update();

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

void FeaStructure::FetchAllTrimPlanes( vector < vector < vec3d > > &pt, vector < vector < vec3d > > &norm )
{
    pt.clear();
    norm.clear();

    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( FeaPartIsTrim( i ) )
        {
            FeaPartTrim * trim = dynamic_cast< FeaPartTrim * >( m_FeaPartVec[i] );

            if ( trim )
            {
                vector < vector < vec3d > > pti;
                vector < vector < vec3d > > normi;

                trim->FetchTrimPlanes( pti, normi );

                int nadd = pti.size();

                pt.reserve( pt.size() + nadd );
                norm.reserve( norm.size() + nadd );

                for ( int j = 0; j < nadd; j++ )
                {
                    pt.push_back( pti[j] );
                    norm.push_back( normi[j] );
                }
            }
        }
    }
}

bool FeaStructure::FeaPartIsFixPoint( int ind )
{
    return FeaPartIsType( ind, vsp::FEA_FIX_POINT );
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
    return FeaPartIsType( ind, vsp::FEA_RIB_ARRAY ) || FeaPartIsType( ind, vsp::FEA_SLICE_ARRAY );
}

bool FeaStructure::FeaPartIsSkin( int ind )
{
    return FeaPartIsType( ind, vsp::FEA_SKIN );
}

bool FeaStructure::FeaPartIsTrim( int ind )
{
    return FeaPartIsType( ind, vsp::FEA_TRIM );
}

bool FeaStructure::FeaPartIsType( int ind, int type )
{
    FeaPart* fea_part = GetFeaPart( ind );

    if ( fea_part )
    {
        if ( fea_part->GetType() == type )
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
            double center_location = 0.0;

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
            double center_location = 0.0;

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
            const unsigned int npts = 5;
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
            const unsigned int npts = 5;
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
    double minlen = m_FeaGridDensity.m_MinLen();
    // Loop over all parts.
    for ( int i  = 0; i < m_FeaPartVec.size(); i++ )
    {
        FeaPart* p = m_FeaPartVec[i];
        if ( p )
        {
            if ( p->PtsOnPlanarPart( pnts, minlen ) )
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

FeaPart::FeaPart( const string& geomID, int type )
{
    m_FeaPartType = type;
    m_ParentGeomID = geomID;

    m_MainSurfIndx = 0;

    m_IncludedElements.Init( "IncludedElements", "FeaPart", this, vsp::FEA_SHELL, vsp::FEA_SHELL, vsp::FEA_NUM_ELEMENT_TYPES - 1 );
    m_IncludedElements.SetDescript( "Indicates the FeaElements to be Included for the FeaPart" );

    m_OrientationType.Init( "Orientation", "FeaPart", this, vsp::FEA_ORIENT_PART_U, vsp::FEA_ORIENT_GLOBAL_X, vsp::FEA_NUM_ORIENT_TYPES - 1 );
    m_OrientationType.SetDescript( "Part material orientation type" );

    m_DrawFeaPartFlag.Init( "DrawFeaPartFlag", "FeaPart", this, true, false, true );
    m_DrawFeaPartFlag.SetDescript( "Flag to Draw FeaPart" );

    m_AbsRelParmFlag.Init( "AbsRelParmFlag", "FeaPart", this, vsp::REL, vsp::ABS, vsp::REL );
    m_AbsRelParmFlag.SetDescript( "Parameterization of Center Location as Absolute or Relative" );

    m_AbsCenterLocation.Init( "AbsCenterLocation", "FeaPart", this, 0.0, 0.0, 1e12 );
    m_AbsCenterLocation.SetDescript( "The Absolute Location of the Center of the FeaPart" );

    m_RelCenterLocation.Init( "RelCenterLocation", "FeaPart", this, 0.5, 0.0, 1.0 );
    m_RelCenterLocation.SetDescript( "The Relative Location of the Center of the FeaPart" );

    m_FeaPropertyIndex.Init( "FeaPropertyIndex", "FeaPart", this, 0, 0, 1e12 ); // Shell property default
    m_FeaPropertyIndex.SetDescript( "FeaPropertyIndex for Shell Elements" );

    m_CapFeaPropertyIndex.Init( "CapFeaPropertyIndex", "FeaPart", this, 1, 0, 1e12 ); // Beam property default
    m_CapFeaPropertyIndex.SetDescript( "FeaPropertyIndex for Beam (Cap) Elements" );
}

FeaPart::~FeaPart()
{

}

void FeaPart::Update()
{
    m_LateUpdateFlag = false;

    UpdateSurface();

    UpdateFlags();

    UpdateOrientation();

    UpdateSymmParts();

    UpdateDrawObjs();
}

void FeaPart::UpdateFlags()
{
    for ( int i = 0; i < m_MainFeaPartSurfVec.size(); i++ )
    {
        if ( GetType() == vsp::FEA_SKIN )
        {
            m_MainFeaPartSurfVec[ i ].SetSurfCfdType( vsp::CFD_NORMAL );
        }
        else
        {
            if ( m_IncludedElements() == vsp::FEA_SHELL || m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
            {
                m_MainFeaPartSurfVec[ i ].SetSurfCfdType( vsp::CFD_STRUCTURE );
            }
            else
            {
                m_MainFeaPartSurfVec[ i ].SetSurfCfdType( vsp::CFD_STIFFENER );
            }
        }
    }
}

// Compute the part material orientation after main surface creation, before symmetric surfacecopy and transformations
// and certainly before a mesh has been created.  Consequently, we do not know the U, V coordinates of element centers
// required to find the local directions used by NASTRAN in some cases.  Instead, in all cases, we will
// calculate the part-constant direction used by CalculiX.  This will get transformed as symmetric copies are made.
void FeaPart::UpdateOrientation()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    Geom *parent_geom = veh->FindGeom( m_ParentGeomID );

    if ( !parent_geom )
    {
        return;
    }

    for ( int i = 0; i < m_MainFeaPartSurfVec.size(); i++ )
    {
        vec3d orient;
        if ( m_OrientationType() == vsp::FEA_ORIENT_GLOBAL_X ||
             m_OrientationType() == vsp::FEA_ORIENT_GLOBAL_Y ||
             m_OrientationType() == vsp::FEA_ORIENT_GLOBAL_Z )
        {
            orient = vec3d( 0, 0, 0 );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_COMP_X )
        {
            orient = vec3d( 1.0, 0, 0 );
            Matrix4d model_matrix = parent_geom->getModelMatrix();
            orient = model_matrix.xformnorm( orient );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_COMP_Y )
        {
            orient = vec3d( 0, 1.0, 0 );
            Matrix4d model_matrix = parent_geom->getModelMatrix();
            orient = model_matrix.xformnorm( orient );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_COMP_Z )
        {
            orient = vec3d( 0, 0, 1.0 );
            Matrix4d model_matrix = parent_geom->getModelMatrix();
            orient = model_matrix.xformnorm( orient );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_OML_R ||
                  m_OrientationType() == vsp::FEA_ORIENT_OML_U ||
                ( m_OrientationType() == vsp::FEA_ORIENT_PART_U && GetType() == vsp::FEA_SKIN ) )
        {
            orient = parent_geom->CompTanR( m_MainSurfIndx, 0.5, 0.25, 0.5 );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_OML_S ||
                  m_OrientationType() == vsp::FEA_ORIENT_OML_V ||
                ( m_OrientationType() == vsp::FEA_ORIENT_PART_V && GetType() == vsp::FEA_SKIN ) )
        {
            orient = parent_geom->CompTanS( m_MainSurfIndx, 0.5, 0.25, 0.5 );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_OML_T )
        {
            orient = parent_geom->CompTanT( m_MainSurfIndx, 0.5, 0.25, 0.5 );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_PART_U )
        {
            orient = m_MainFeaPartSurfVec[ i ].CompTanU01( 0.5, 0.5 );
        }
        else if ( m_OrientationType() == vsp::FEA_ORIENT_PART_V )
        {
            orient = m_MainFeaPartSurfVec[ i ].CompTanW01( 0.5, 0.5 );
        }

        orient.normalize();
        m_MainFeaPartSurfVec[ i ].SetFeaOrientation( m_OrientationType(), orient );
    }
}

// This should really call FeaStructure::ParmChanged, but there is no clear way to get a pointer to the
// FeaStructure that contains this FeaPart
void FeaPart::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* geom = veh->FindGeom( m_ParentGeomID );
        if ( geom  )
        {
            geom->ParmChanged( parm_ptr, type );
        }
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

void FeaPart::SetDisplaySuffix( int num )
{
    for ( int i = 0; i < (int)m_ParmVec.size(); i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );

        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

void FeaPart::UpdateSymmParts()
{
    m_SymmIndexVec.clear();
    m_FeaPartSurfVec.clear();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    Geom* currgeom = veh->FindGeom( m_ParentGeomID );

    if ( !currgeom )
    {
        return;
    }

    currgeom->GetSymmIndexs( m_MainSurfIndx, m_SymmIndexVec );

    int nsurf = m_MainFeaPartSurfVec.size();
    int nsymm = m_SymmIndexVec.size();

    m_FeaPartSurfVec.resize( nsurf * nsymm );

    // Surface vector -- used for checking flipnormal.  Seems wasteful, but it works.
    vector< VspSurf > surf_vec;
    surf_vec = currgeom->GetSurfVecConstRef();

    // Translation matrices for all Geom's surfaces.
    vector<Matrix4d> transMats = currgeom->GetFeaTransMatVec();

    bool flipnormal = surf_vec[ m_MainSurfIndx ].GetFlipNormal();

    for ( size_t i = 0; i < nsurf; i++ )
    {
        if ( m_MainFeaPartSurfVec[i].GetFlipNormal() != flipnormal ) // Make sure base copy oriented right.
        {
            m_MainFeaPartSurfVec[i].FlipNormal();
        }
        m_FeaPartSurfVec[i] = m_MainFeaPartSurfVec[i]; // Initialize first set.
    }

    for ( size_t i = 0; i < nsurf; i++ )
    {
        for ( size_t j = 1; j < nsymm; j++ )
        {
            m_FeaPartSurfVec[ nsurf * j + i ] = m_FeaPartSurfVec[ i ]; // Copy from base set.

            flipnormal = surf_vec[ m_SymmIndexVec[j] ].GetFlipNormal();
            if ( m_FeaPartSurfVec[ nsurf * j + i ].GetFlipNormal() != flipnormal ) // Make sure symmetric copies oriented right.
            {
                m_FeaPartSurfVec[ nsurf * j + i ].FlipNormal();
            }

            m_FeaPartSurfVec[ nsurf * j + i ].Transform( transMats[ m_SymmIndexVec[ j ] ] );
        }
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
    if ( type == vsp::FEA_TRIM )
    {
        return string( "Trim" );
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
        m_FeaPartSurfVec[p].FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx, compid, p, xfersurfs, usuppress, wsuppress );
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
        draw_obj_vec.push_back( &m_FeaPartDO[ i ] );
    }

    for ( int i = 0; i < (int)m_FeaHighlightDO.size(); i++ )
    {
        draw_obj_vec.push_back( &m_FeaHighlightDO[i] );
    }
}

void FeaPart::UpdateDrawObjs()
{
    // 2 DrawObj per FeaPart: One for the planar surface and the other for the outline. This is done to avoid
    //  OpenGL transparency ordering issues. 
    m_FeaPartDO.clear();
    m_FeaPartDO.resize( m_FeaPartSurfVec.size() );
    m_FeaHighlightDO.clear();
    m_FeaHighlightDO.resize( m_FeaPartSurfVec.size() );

    for ( unsigned int j = 0; j < m_FeaPartSurfVec.size(); j++ )
    {
        m_FeaPartDO[j].m_GeomID = string( GetID() + "_" + std::to_string( j ) + "_" + m_Name );
        m_FeaPartDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        m_FeaHighlightDO[j].m_GeomID = string( GetID() + "_hl_" + std::to_string( j ) + "_" + m_Name );
        m_FeaHighlightDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        m_FeaHighlightDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
        m_FeaHighlightDO[j].m_LineWidth = 1.0;

        m_FeaPartDO[j].m_Type = DrawObj::VSP_SHADED_QUADS;
        m_FeaHighlightDO[j].m_Type = DrawObj::VSP_LINE_LOOP;

        vec3d p00 = m_FeaPartSurfVec[j].CompPnt01( 0, 0 );
        vec3d p10 = m_FeaPartSurfVec[j].CompPnt01( 1, 0 );
        vec3d p11 = m_FeaPartSurfVec[j].CompPnt01( 1, 1 );
        vec3d p01 = m_FeaPartSurfVec[j].CompPnt01( 0, 1 );

        m_FeaPartDO[j].m_PntVec.push_back( p00 );
        m_FeaPartDO[j].m_PntVec.push_back( p10 );
        m_FeaPartDO[j].m_PntVec.push_back( p11 );
        m_FeaPartDO[j].m_PntVec.push_back( p01 );

        m_FeaHighlightDO[j].m_PntVec.push_back( p00 );
        m_FeaHighlightDO[j].m_PntVec.push_back( p10 );
        m_FeaHighlightDO[j].m_PntVec.push_back( p11 );
        m_FeaHighlightDO[j].m_PntVec.push_back( p01 );

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

        m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.33f;

        m_FeaPartDO[j].m_MaterialInfo.Shininess = 5.0f;

        m_FeaPartDO[j].m_GeomChanged = true;
        m_FeaHighlightDO[j].m_GeomChanged = true;
    }
}

void FeaPart::SetDrawObjHighlight( bool highlight )
{
    if ( highlight )
    {
        for ( unsigned int j = 0; j < m_FeaHighlightDO.size(); j++ )
        {
            m_FeaHighlightDO[j].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
            m_FeaHighlightDO[j].m_LineWidth = 3.0;
        }

        for ( unsigned int j = 0; j < m_FeaPartDO.size(); j++ )
        {
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.67f;
        }
    }
    else
    {
        for ( unsigned int j = 0; j < m_FeaHighlightDO.size(); j++ )
        {
            m_FeaHighlightDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
            m_FeaHighlightDO[j].m_LineWidth = 1.0;
        }

        for ( unsigned int j = 0; j < m_FeaPartDO.size(); j++ )
        {
            m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.33f;
        }
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
            retsurf = currgeom->GetSurfPtr( m_MainSurfIndx );
        }
    }
    return retsurf;
}

bool FeaPart::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    double tol = minlen / 10.0;

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

FeaSlice::FeaSlice( const string& geomID, int type ) : FeaPart( geomID, type )
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

void FeaSlice::UpdateSurface()
{
    UpdateParmLimits();

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( 1 );

    m_MainFeaPartSurfVec[0] = ComputeSliceSurf();
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
        surf_vec = current_geom->GetSurfVecConstRef();

        // Determine BndBox dimensions prior to rotating and translating
        Matrix4d model_matrix = current_geom->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = surf_vec[m_MainSurfIndx];
        orig_surf.Transform( model_matrix );

        if ( RefFrameIsBody( m_OrientationPlane() ) )
        {
            orig_surf.GetBoundingBox( m_SectBBox );
        }
        else
        {
            surf_vec[m_MainSurfIndx].GetBoundingBox( m_SectBBox );
        }

        double perp_dist = 0.0; // Total distance perpendicular to the FeaSlice plane

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
            perp_dist = del_z;
        }
        else if ( m_OrientationPlane() == vsp::YZ_BODY || m_OrientationPlane() == vsp::YZ_ABS )
        {
            perp_dist = del_x;
        }
        else if ( m_OrientationPlane() == vsp::XZ_BODY || m_OrientationPlane() == vsp::XZ_ABS )
        {
            perp_dist = del_y;
        }
        else if ( m_OrientationPlane() == vsp::SPINE_NORMAL )
        {
            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( surf_vec[m_MainSurfIndx] );

            perp_dist = cs.GetSpineLength();
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

        // Determine Slice Center in 3D
        if ( m_OrientationPlane() == vsp::XY_BODY || m_OrientationPlane() == vsp::XY_ABS )
        {
            m_Center = vec3d( geom_center.x(), geom_center.y(), m_SectBBox.GetMin( 2 ) + del_z * m_RelCenterLocation() );
        }
        else if ( m_OrientationPlane() == vsp::YZ_BODY || m_OrientationPlane() == vsp::YZ_ABS )
        {
            m_Center = vec3d( m_SectBBox.GetMin( 0 ) + del_x * m_RelCenterLocation(), geom_center.y(), geom_center.z() );
        }
        else if ( m_OrientationPlane() == vsp::XZ_BODY || m_OrientationPlane() == vsp::XZ_ABS )
        {
            m_Center = vec3d( geom_center.x(), m_SectBBox.GetMin( 1 ) + del_y * m_RelCenterLocation(), geom_center.z() );
        }
        else if ( m_OrientationPlane() == vsp::SPINE_NORMAL )
        {
            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( surf_vec[m_MainSurfIndx] );

            double u_max = surf_vec[m_MainSurfIndx].GetUMax();
            double spine_length = cs.GetSpineLength();
            double length_on_spine = m_RelCenterLocation() * spine_length;
            double per_u = cs.FindUGivenLengthAlongSpine( length_on_spine ) / u_max;

            m_Center = cs.FindCenterGivenU( per_u * u_max );
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
        surf_vec = current_geom->GetSurfVecConstRef();

        slice_surf = VspSurf(); // Create primary VspSurf

        // Determine BndBox dimensions prior to rotating and translating
        Matrix4d model_matrix = current_geom->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = surf_vec[m_MainSurfIndx];
        orig_surf.Transform( model_matrix );

        double u_max = surf_vec[m_MainSurfIndx].GetUMax();

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
        double expan = m_SectBBox.GetLargestDist() * FEA_PART_EXPANSION_FACTOR;
        if ( expan < FEA_PART_EXPANSION_FACTOR )
        {
            expan = FEA_PART_EXPANSION_FACTOR;
        }

        if ( m_OrientationPlane() == vsp::SPINE_NORMAL )
        {
            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( surf_vec[m_MainSurfIndx] );

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

            vec3d surf_pnt1 = surf_vec[m_MainSurfIndx].CompPnt01( per_u, 0.0 );

            z_axis = surf_pnt1 - m_Center;
            z_axis.normalize();

            y_axis = cross( x_axis, z_axis );
            y_axis.normalize();

            VspCurve u_curve;
            surf_vec[m_MainSurfIndx].GetU01ConstCurve( u_curve, per_u );

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
            // Increase size slightly to avoid tangency errors in FeaMeshMgr
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
                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x + 2 * x_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += std::abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_plus += std::abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x - 2 * x_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += std::abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_minus += std::abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }
                else
                {
                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x - 2 * x_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += std::abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_plus += std::abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_x + 2 * x_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += std::abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_z_minus += std::abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }

                // Resize for Z rotation
                if ( m_ZRot() > 0 )
                {
                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x + 2 * x_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_minus += std::abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_minus += std::abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x - 2 * x_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_plus += std::abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_plus += std::abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }
                else
                {
                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x + 2 * x_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_plus += std::abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_plus += std::abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_x - 2 * x_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_minus += std::abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_y_minus += std::abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_ZRot() ) );
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
                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z + 2 * z_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_minus += std::abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_minus += std::abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z - 2 * z_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_plus += std::abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_plus += std::abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }
                else
                {
                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z + 2 * z_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_plus += std::abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_plus += std::abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_YRot() ) > atan( ( del_z - 2 * z_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_minus += std::abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_YRot() ) );
                    }
                    else
                    {
                        del_x_minus += std::abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_YRot() ) );
                    }
                }

                // Resize for X rotation
                if ( m_XRot() > 0 )
                {
                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z + 2 * z_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_plus += std::abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_plus += std::abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z - 2 * z_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_minus += std::abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_minus += std::abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }
                }
                else
                {
                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z - 2 * z_off ) / ( del_y - 2 * y_off ) ) )
                    {
                        del_y_plus += std::abs( ( del_z -2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_plus += std::abs( ( del_y - 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_z + 2 * z_off ) / ( del_y + 2 * y_off ) ) )
                    {
                        del_y_minus += std::abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_y_minus += std::abs( ( del_y + 2 * y_off ) / cos( DEG_2_RAD * m_XRot() ) );
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
                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y + 2 * y_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_plus += std::abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_plus += std::abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y - 2 * y_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_minus += std::abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_minus += std::abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }
                else 
                {
                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y - 2 * y_off ) / ( del_x - 2 * x_off ) ) )
                    {
                        del_x_plus += std::abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_plus += std::abs( ( del_x - 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_ZRot() ) > atan( ( del_y + 2 * y_off ) / ( del_x + 2 * x_off ) ) )
                    {
                        del_x_minus += std::abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_ZRot() ) );
                    }
                    else
                    {
                        del_x_minus += std::abs( ( del_x + 2 * x_off ) / cos( DEG_2_RAD * m_ZRot() ) );
                    }
                }

                // Resize for X rotation
                if ( m_XRot() > 0 )
                {
                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y + 2 * y_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += std::abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_plus += std::abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y - 2 * y_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += std::abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_minus += std::abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }
                }
                else
                {
                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y - 2 * y_off ) / ( del_z - 2 * z_off ) ) )
                    {
                        del_z_plus += std::abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_plus += std::abs( ( del_z - 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
                    }

                    if ( std::abs( DEG_2_RAD * m_XRot() ) > atan( ( del_y + 2 * y_off ) / ( del_z + 2 * z_off ) ) )
                    {
                        del_z_minus += std::abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_XRot() ) );
                    }
                    else
                    {
                        del_z_minus += std::abs( ( del_z + 2 * z_off ) / cos( DEG_2_RAD * m_XRot() ) );
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
        slice_surf.MakePlaneSurf( cornerA, cornerB, cornerC, cornerD, 1.1 );

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

//////////////////////////////////////////////////////
//===================== FeaSpar ====================//
//////////////////////////////////////////////////////

FeaSpar::FeaSpar( const string& geomID, int type ) : FeaSlice( geomID, type )
{
    m_Theta.Init( "Theta", "FeaSpar", this, 0.0, -90.0, 90.0 );
    m_Theta.SetDescript( "Rotation of Spar About Axis Normal to Wing Chord Line " );

    m_LimitSparToSectionFlag.Init( "LimitSparToSectionFlag", "FeaSpar", this, false, false, true );
    m_LimitSparToSectionFlag.SetDescript( "Flag to Limit Spar Length to Wing Section" );

    m_StartWingSection.Init( "StartWingSection", "FeaSpar", this, 1, 1, 1000 );
    m_StartWingSection.SetDescript( "Start Wing Section to Limit Spar Length to" );

    m_EndWingSection.Init( "EndWingSection", "FeaSpar", this, 1, 1, 1000 );
    m_EndWingSection.SetDescript( "End Wing Section to Limit Spar Length to" );

    m_BndBoxTrimFlag.Init( "BndBoxTrimFlag", "FeaSpar", this, true, false, true );
    m_BndBoxTrimFlag.SetDescript( "Flag to Trim Spar to Bounding Box Instead of Wing Surface" );

    m_UsePercentChord.Init( "UsePercentChord", "FeaSpar", this, false, false, true );
    m_UsePercentChord.SetDescript( "Flag to Set Spar Rotation by Percent Chord" );

    m_PercentRootChord.Init( "PercentRootChord", "FeaSpar", this, 0.5, 0.0, 1.0 );
    m_PercentRootChord.SetDescript( "Starting Location of the Spar as Percentage of Root Chord" );

    m_PercentTipChord.Init( "PercentTipChord", "FeaSpar", this, 0.5, 0.0, 1.0 );
    m_PercentTipChord.SetDescript( "Starting Location of the Spar as Percentage of Tip Chord" );
}

void FeaSpar::UpdateSurface()
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

        if ( !current_wing )
        {
            return;
        }

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        vector< VspSurf > surf_vec;
        surf_vec = current_wing->GetSurfVecConstRef();

        int num_wing_sec = wing->NumXSec();
        double U_max = surf_vec[m_MainSurfIndx].GetUMax();

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

        double chord_length = dist( surf_vec[m_MainSurfIndx].CompPnt01( u_mid, 0.5 ), surf_vec[m_MainSurfIndx].CompPnt01( u_mid, 0.0 ) ); // average chord length

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

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( 1 );

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing )
        {
            return;
        }

        m_MainFeaPartSurfVec[0] = VspSurf(); // Create primary VspSurf

        WingGeom* wing = dynamic_cast<WingGeom*>( current_wing );
        assert( wing );

        vector< VspSurf > surf_vec;
        surf_vec = current_wing->GetSurfVecConstRef();

        // Get surface prior to rotating and translating
        Matrix4d model_matrix = current_wing->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = surf_vec[m_MainSurfIndx];
        orig_surf.Transform( model_matrix );

        BndBox wing_bbox;
        orig_surf.GetBoundingBox( wing_bbox );

        double U_max = orig_surf.GetUMax();

        double u_mid = ( ( m_U_sec_min + m_U_sec_max ) / 2 ) / U_max;

        double V_min = 0.0;
        double V_max = orig_surf.GetWMax(); // Really should be 4.0
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
        vec3d inside_edge_pnt;

        vec3d outside_edge_vec = max_lead_edge - max_trail_edge;
        double outside_edge_length = outside_edge_vec.mag();
        outside_edge_vec.normalize();
        vec3d outside_edge_pnt;

        if( m_UsePercentChord() )
        {
            inside_edge_pnt = min_lead_edge - ( m_PercentRootChord() * inside_edge_length ) * inside_edge_vec;
            outside_edge_pnt = max_lead_edge - ( m_PercentTipChord() * outside_edge_length ) * outside_edge_vec;
        }
        else
        {
            inside_edge_pnt = min_lead_edge - ( m_RelCenterLocation() * inside_edge_length ) * inside_edge_vec;
            outside_edge_pnt = max_lead_edge - ( m_RelCenterLocation() * outside_edge_length ) * outside_edge_vec;
        }

        double length_spar_0 = dist( inside_edge_pnt, outside_edge_pnt ) / 2; // Initial spar half length

        vec3d foil_mid_up, foil_mid_low;
        foil_mid_up = orig_surf.CompPnt01( u_mid, 0.75 );
        foil_mid_low = orig_surf.CompPnt01( u_mid, 0.25 );

        vec3d wing_z_axis = foil_mid_up - foil_mid_low;
        wing_z_axis.normalize();

        // Identify expansion 
        double expan = wing_bbox.GetLargestDist() * FEA_PART_EXPANSION_FACTOR;
        if ( expan < FEA_PART_EXPANSION_FACTOR )
        {
            expan = FEA_PART_EXPANSION_FACTOR;
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

        if ( std::abs( inner_edge_vec.mag() - 1.0 ) <= FLT_EPSILON )
        {
            normal_vec = cross( inner_edge_vec, lead_edge_vec );
        }
        else
        {
            normal_vec = cross( outer_edge_vec, lead_edge_vec );
        }

        normal_vec.normalize();

        double alpha_0 = ( PI / 2 ) - signed_angle( inner_edge_vec, center_to_outer_edge, normal_vec ); // Initial rotation
        double theta;

        if( m_UsePercentChord() )
        {
            theta = 0;

            // Set center location and rotation from root/tip chord parameters
            vec3d mid_lead_edge = orig_surf.CompPnt01( u_mid, V_leading_edge / V_max );
            vec3d mid_trail_edge = orig_surf.CompPnt01( u_mid, 0.0 );

            double rel_center = dist( center, mid_lead_edge ) / dist( mid_trail_edge, mid_lead_edge );

            vec3d outside_edge_pnt_0 = max_lead_edge - ( rel_center  * outside_edge_length ) * outside_edge_vec;

            vec3d center_to_outer_pnt = outside_edge_pnt_0 - center;
            center_to_outer_pnt.normalize();

            double alpha_f = signed_angle( inner_edge_vec, center_to_outer_pnt, normal_vec );

            m_Theta.Set( -1 * ( RAD_2_DEG * ( ( PI / 2 ) - alpha_f - alpha_0 ) ) );

            if( m_AbsRelParmFlag() == vsp::REL )
            {
                m_RelCenterLocation.Set( rel_center );
            }
            else
            {
                m_AbsCenterLocation.Set( dist( center, mid_lead_edge ) );
            }
        }
        else
        {
            theta = DEG_2_RAD * m_Theta(); // User defined angle converted to Rad

            vec3d mid_lead_edge = orig_surf.CompPnt01( u_mid, V_leading_edge / V_max );
            vec3d mid_trail_edge = orig_surf.CompPnt01( u_mid, 0.0 );
            double rel_center = dist( center, mid_lead_edge ) / dist( mid_trail_edge, mid_lead_edge );

            vec3d center_to_out_edge_vec = ( outside_edge_pnt - center );
            center_to_out_edge_vec.normalize();

            center_to_out_edge_vec = RotateArbAxis( center_to_out_edge_vec, -1 * theta, normal_vec );

            // Perform line line intersection. The lines do not need to intersect but must be coplanar. 
            // 0 <= t <= 1 means the spar intersects between the input edge endpoints
            vec3d spar_end_max = center + center_to_out_edge_vec;
            vec3d spar_end_min = center - center_to_out_edge_vec;

            double t_tip, t_root, s; // ignore s (percent spar intersection point), only need percent edge intersection
            bool tip_coplanar = line_line_intersect( center, spar_end_max, max_lead_edge, max_trail_edge, &s, &t_tip );
            bool root_coplanar = line_line_intersect( center, spar_end_min, min_lead_edge, min_trail_edge, &s, &t_root );

            if ( tip_coplanar && root_coplanar )
            {
                // No need to clamp between 0 and 1, when SetValCheckLimits is called
                m_PercentTipChord.Set( t_tip );
                m_PercentRootChord.Set( t_root );
            }
            else
            {
                printf( "ERROR: Non-coplanar FEA Spar Intersection \n" );
                m_PercentTipChord.Set( 0 );
                m_PercentRootChord.Set( 0 );
            }
        }

        if ( m_BndBoxTrimFlag() )
        {
            // Get bounding box of wing sections
            BndBox sect_bbox;

            if ( m_LimitSparToSectionFlag() )
            {
                //Determine wing section bounding box
                sect_bbox.Reset();

                orig_surf.GetLimitedBoundingBox( sect_bbox, m_U_sec_min, m_U_sec_max, 0.0, orig_surf.GetWMax() );
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

                m_MainFeaPartSurfVec[0] = temp_slice->ComputeSliceSurf();

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
                    if ( std::abs( sin( theta + beta_le ) ) <= FLT_EPSILON || ( min_lead_edge - max_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - max_lead_edge ).mag();
                        length_spar_in += std::abs( perp_dist / sin( theta + beta_le ) );
                    }
                }
                else
                {
                    if ( std::abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( min_trail_edge - min_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_lead_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - min_lead_edge ).mag();
                        length_spar_in += std::abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }

                if ( theta > max_angle_outer_te )
                {
                    if ( std::abs( sin( theta - beta_te ) ) <= FLT_EPSILON || ( min_trail_edge - max_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - max_trail_edge ).mag();
                        length_spar_out += std::abs( perp_dist / sin( theta - beta_te ) );
                    }
                }
                else
                {
                    if ( std::abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( max_trail_edge - max_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - max_trail_edge ) ).mag() / ( max_trail_edge - max_lead_edge ).mag();
                        length_spar_out += std::abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }
            }
            else
            {
                if ( theta < max_angle_inner_te )
                {
                    if ( std::abs( sin( theta - beta_te ) ) <= FLT_EPSILON || ( max_trail_edge - min_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - min_trail_edge ) ).mag() / ( max_trail_edge - min_trail_edge ).mag();
                        length_spar_in += std::abs( perp_dist / sin( theta - beta_te ) );
                    }
                }
                else
                {
                    if ( std::abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( min_trail_edge - min_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_in += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_lead_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - min_lead_edge ).mag();
                        length_spar_in += std::abs( perp_dist / cos( theta + alpha_0 ) );
                    }
                }

                if ( theta < max_angle_outer_le )
                {
                    if ( std::abs( sin( theta + beta_le ) ) <= FLT_EPSILON || ( max_lead_edge - min_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - min_lead_edge ) ).mag() / ( max_lead_edge - min_lead_edge ).mag();
                        length_spar_out += std::abs( perp_dist / sin( theta + beta_le ) );
                    }
                }
                else
                {
                    if ( std::abs( cos( theta + alpha_0 ) ) <= FLT_EPSILON || ( max_trail_edge - max_lead_edge ).mag() <= FLT_EPSILON )
                    {
                        length_spar_out += length_spar_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_lead_edge ), ( center - max_trail_edge ) ).mag() / ( max_trail_edge - max_lead_edge ).mag();
                        length_spar_out += std::abs( perp_dist / cos( theta + alpha_0 ) );
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
            m_MainFeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            // Transform to body coordinate frame
            model_matrix.affineInverse();
            m_MainFeaPartSurfVec[0].Transform( model_matrix );
        }
    }
}

//////////////////////////////////////////////////////
//===================== FeaRib =====================//
//////////////////////////////////////////////////////

FeaRib::FeaRib( const string& geomID, int type ) : FeaSlice( geomID, type )
{
    m_Theta.Init( "Theta", "FeaRib", this, 0.0, -90.0, 90.0 );
    m_Theta.SetDescript( "Rotation of FeaRib About Axis Normal to Wing Chord Line" );

    m_LimitRibToSectionFlag.Init( "LimitRibToSectionFlag", "FeaRib", this, false, false, true );
    m_LimitRibToSectionFlag.SetDescript( "Flag to Limit Rib Length to Wing Section" );

    m_StartWingSection.Init( "StartWingSection", "FeaRib", this, 1, 1, 1000 );
    m_StartWingSection.SetDescript( "Start Wing Section to Limit Rib to" );

    m_EndWingSection.Init( "EndWingSection", "FeaRib", this, 1, 1, 1000 );
    m_EndWingSection.SetDescript( "End Wing Section to Limit Rib to" );

    m_BndBoxTrimFlag.Init( "BndBoxTrimFlag", "FeaRib", this, true, false, true );
    m_BndBoxTrimFlag.SetDescript( "Flag to Trim Rib to Bounding Box Instead of Wing Surface" );

    m_PerpendicularEdgeType.Init( "PerpendicularEdgeType", "FeaRib", this, vsp::NO_NORMAL, vsp::NO_NORMAL, vsp::SPAR_NORMAL );
    m_PerpendicularEdgeType.SetDescript( "Identifies the Perpendicular Edge Type for the Rib" );

    m_MatchDihedralFlag.Init( "MatchDihedralFlag", "FeaRib", this, true, false, true );
    m_MatchDihedralFlag.SetDescript( "Flag to Rotate the Rib with the Dihedral Angle of the Wing" );
}

void FeaRib::UpdateSurface()
{
    UpdateParmLimits();

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( 1 );

    GetRibPerU();
    m_MainFeaPartSurfVec[0] = ComputeRibSurf();
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

        // Check for previous implementation of perpendicular edge for ribs to enable compatibility in new VSP versions
        xmlNodePtr child_node = XmlUtil::GetNode( fea_prt_node, "FeaRib", 0 );
        if ( XmlUtil::FindInt( child_node, "PerpendicularEdgeType", -1 ) == -1 )
        {
            if ( strcmp( m_PerpendicularEdgeID.c_str(), "Trailing Edge" ) == 0 )
            {
                m_PerpendicularEdgeType.Set( vsp::TE_NORMAL );
            }
            else if ( strcmp( m_PerpendicularEdgeID.c_str(), "Leading Edge" ) == 0 )
            {
                m_PerpendicularEdgeType.Set( vsp::LE_NORMAL );
            }
            else if ( strcmp( m_PerpendicularEdgeID.c_str(), "None" ) == 0 )
            {
                m_PerpendicularEdgeType.Set( vsp::NO_NORMAL );
            }
            else
            {
                m_PerpendicularEdgeType.Set( vsp::SPAR_NORMAL );
            }
        }
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
            surf_vec = current_wing->GetSurfVecConstRef();

            int num_wing_sec = wing->NumXSec();

            vector < double > wing_sec_span_vec; // Vector of wing span increasing by each wing section (first section has no length)
            wing_sec_span_vec.push_back( 0.0 );

            double U_max = surf_vec[m_MainSurfIndx].GetUMax();

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

            if ( curr_sec_ind > 0 && curr_sec_ind < wing_sec_span_vec.size() )
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
            surf_vec = current_wing->GetSurfVecConstRef();

            // Get surface prior to rotating and translating
            Matrix4d model_matrix = current_wing->getModelMatrix();
            model_matrix.affineInverse();

            VspSurf orig_surf = surf_vec[m_MainSurfIndx];
            orig_surf.Transform( model_matrix );

            // Find initial rotation (alpha) to perpendicular edge or spar
            double alpha = 0.0;
            double u_edge_out = m_PerU + 2 * FLT_EPSILON;
            double u_edge_in = m_PerU - 2 * FLT_EPSILON;

            double V_min = 0.0;
            double V_max = orig_surf.GetWMax(); // Really should be 4.0
            double V_leading_edge = ( V_min + V_max ) * 0.5;

            vec3d trail_edge, lead_edge;
            trail_edge = orig_surf.CompPnt01( m_PerU, 0.0 );
            lead_edge = orig_surf.CompPnt01( m_PerU, V_leading_edge / V_max );

            vec3d chord_dir_vec = trail_edge - lead_edge;
            chord_dir_vec.normalize();

            if ( m_PerpendicularEdgeType() == vsp::TE_NORMAL )
            {
                vec3d trail_edge_out, trail_edge_in;
                trail_edge_out = orig_surf.CompPnt01( u_edge_out, 0.0 );
                trail_edge_in = orig_surf.CompPnt01( u_edge_in, 0.0 );

                vec3d trail_edge_dir_vec = trail_edge_out - trail_edge_in;
                trail_edge_dir_vec.normalize();

                alpha = ( PI / 2 ) - signed_angle( chord_dir_vec, trail_edge_dir_vec, m_WingNorm );
            }
            else if ( m_PerpendicularEdgeType() == vsp::LE_NORMAL )
            {
                vec3d lead_edge_out, lead_edge_in;
                lead_edge_out = orig_surf.CompPnt01( u_edge_out, V_leading_edge / V_max );
                lead_edge_in = orig_surf.CompPnt01( u_edge_in, V_leading_edge / V_max );

                vec3d lead_edge_dir_vec = lead_edge_out - lead_edge_in;
                lead_edge_dir_vec.normalize();

                alpha = ( PI / 2 ) - signed_angle( chord_dir_vec, lead_edge_dir_vec, m_WingNorm );
            }
            else if ( m_PerpendicularEdgeType() == vsp::SPAR_NORMAL )
            {
                FeaPart* part = StructureMgr.GetFeaPart( m_PerpendicularEdgeID );

                if ( part && part->GetFeaPartSurfVec().size() > 0 )
                {
                    VspSurf surf = part->GetFeaPartSurfVec()[0];

                    vec3d edge1, edge2;
                    edge1 = surf.CompPnt01( 0.0, 0.5 );
                    edge2 = surf.CompPnt01( 1.0, 0.5 );

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
        surf_vec = current_wing->GetSurfVecConstRef();

        // Get surface prior to rotating and translating
        Matrix4d model_matrix = wing->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = surf_vec[m_MainSurfIndx];
        orig_surf.Transform( model_matrix );

        BndBox wing_bbox;
        orig_surf.GetBoundingBox( wing_bbox );

        double V_min = 0.0;
        double V_max = orig_surf.GetWMax(); // Really should be 4.0
        double V_leading_edge = ( V_min + V_max ) * 0.5;

        vec3d trail_edge, lead_edge;
        trail_edge = orig_surf.CompPnt01( m_PerU, 0.0 );
        lead_edge = orig_surf.CompPnt01( m_PerU, V_leading_edge / V_max );

        // Find two points slightly above and below the trailing edge
        double V_trail_edge_low = V_min + 2 * TMAGIC;
        double V_trail_edge_up = V_max - 2 * TMAGIC;

        vec3d trail_edge_up, trail_edge_low;

        trail_edge_up = orig_surf.CompPnt01( m_PerU, V_trail_edge_low / V_max );
        trail_edge_low = orig_surf.CompPnt01( m_PerU, V_trail_edge_up / V_max );

        vec3d wing_z_axis = trail_edge_up - trail_edge_low;
        wing_z_axis.normalize();

        vec3d center = ( trail_edge + lead_edge ) / 2; // Center of rib

        // Wing corner points:
        vec3d min_trail_edge = orig_surf.CompPnt( m_U_sec_min, 0.0 );
        vec3d min_lead_edge = orig_surf.CompPnt( m_U_sec_min, V_leading_edge );
        vec3d max_trail_edge = orig_surf.CompPnt( m_U_sec_max, 0.0 );
        vec3d max_lead_edge = orig_surf.CompPnt( m_U_sec_max, V_leading_edge );

        // Wing edge vectors (assumes linearity)
        vec3d trail_edge_vec = max_trail_edge - min_trail_edge;
        vec3d lead_edge_vec = max_lead_edge - min_lead_edge;
        vec3d inner_edge_vec = min_lead_edge - min_trail_edge;
        vec3d outer_edge_vec = max_lead_edge - min_trail_edge;

        trail_edge_vec.normalize();
        lead_edge_vec.normalize();
        inner_edge_vec.normalize();
        outer_edge_vec.normalize();

        double x_rot = 0;

        if ( m_MatchDihedralFlag() )
        {
            int min_sec, max_sec;

            if ( m_LimitRibToSectionFlag() )
            {
                min_sec = m_StartWingSection();
                max_sec = m_EndWingSection();
            }
            else
            {
                min_sec = 1;
                max_sec = wing->NumXSec() - 1;
            }

            for ( size_t i_sec = min_sec; i_sec <= max_sec; i_sec++ )
            {
                WingSect* ws = (WingSect*)wing->GetXSecSurf( 0 )->FindXSec( i_sec );
                x_rot += ws->m_Dihedral();
            }

            x_rot /= ( 1 + max_sec - min_sec );
        }

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
                //Determine wing section bounding box
                sect_bbox.Reset();

                orig_surf.GetLimitedBoundingBox( sect_bbox, m_U_sec_min, m_U_sec_max, 0.0, orig_surf.GetWMax() );
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
                temp_slice->m_XRot.Set( -1 * x_rot );

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
            double expan = wing_bbox.GetLargestDist() * FEA_PART_EXPANSION_FACTOR;
            if ( expan < FEA_PART_EXPANSION_FACTOR )
            {
                expan = FEA_PART_EXPANSION_FACTOR;
            }

            double length_rib_0 = ( dist( trail_edge, lead_edge ) / 2 ); // Rib half length before rotations

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

            double perp_dist;
            double length_rib_te = 1e-6;
            double length_rib_le = 1e-6;

            // Determine if the rib intersects the leading/trailing edge or inner/outer edge
            if ( m_TotRot <= 0 )
            {
                if ( m_TotRot <= max_angle_inner_le )
                {
                    if ( std::abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( min_lead_edge - min_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_le += length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_trail_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - min_trail_edge ).mag();
                        length_rib_le += std::abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( std::abs( sin( phi_le ) ) <= FLT_EPSILON )
                    {
                        length_rib_le += length_rib_0;
                    }
                    else
                    {
                        length_rib_le += std::abs( length_rib_0 * sin( sweep_le ) / sin( phi_le ) );
                    }
                }

                if ( m_TotRot <= max_angle_outer_te )
                {
                    if ( std::abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( max_lead_edge - max_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_te += length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - max_lead_edge ) ).mag() / ( max_lead_edge - max_trail_edge ).mag();
                        length_rib_te += std::abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( std::abs( sin( phi_te ) ) <= FLT_EPSILON )
                    {
                        length_rib_te += length_rib_0;
                    }
                    else
                    {
                        length_rib_te += std::abs( length_rib_0 * sin( sweep_te ) / sin( phi_te ) );
                    }
                }
            }
            else
            {
                if ( m_TotRot >= max_angle_inner_te )
                {
                    if ( std::abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( min_lead_edge - min_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_te += length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - min_trail_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - min_trail_edge ).mag();
                        length_rib_te += std::abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( std::abs( sin( phi_te ) ) <= FLT_EPSILON )
                    {
                        length_rib_te += length_rib_0;
                    }
                    else
                    {
                        length_rib_te += std::abs( length_rib_0 * sin( sweep_te ) / sin( phi_te ) );
                    }
                }

                if ( m_TotRot >= max_angle_outer_le )
                {
                    if ( std::abs( sin( m_TotRot ) ) <= FLT_EPSILON || ( max_lead_edge - max_trail_edge ).mag() <= FLT_EPSILON )
                    {
                        length_rib_le += length_rib_0;
                    }
                    else
                    {
                        perp_dist = cross( ( center - max_trail_edge ), ( center - max_lead_edge ) ).mag() / ( max_lead_edge - max_trail_edge ).mag();
                        length_rib_le += std::abs( perp_dist / sin( m_TotRot ) );
                    }
                }
                else
                {
                    if ( std::abs( sin( phi_le ) ) <= FLT_EPSILON )
                    {
                        length_rib_le += length_rib_0;
                    }
                    else
                    {
                        length_rib_le += std::abs( length_rib_0 * sin( sweep_le ) / sin( phi_le ) );
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
            rib_surf.MakePlaneSurf( cornerA, cornerB, cornerC, cornerD, 1.1 );

            // Translate to the origin, rotate, and translate back to center
            Matrix4d trans_rot_mat;

            trans_rot_mat.loadIdentity();
            trans_rot_mat.translatef( center.x() * -1, center.y() * -1, center.z() * -1 );
            rib_surf.Transform( trans_rot_mat );

            trans_rot_mat.loadIdentity();
            trans_rot_mat.rotateX( x_rot );
            rib_surf.Transform( trans_rot_mat );

            trans_rot_mat.loadIdentity();
            trans_rot_mat.translatef( center.x(), center.y(), center.z() );
            rib_surf.Transform( trans_rot_mat );

            // Transform to body coordinate frame
            model_matrix.affineInverse();
            rib_surf.Transform( model_matrix );
        }
    }

    return rib_surf;
}

////////////////////////////////////////////////////
//================= FeaFixPoint ==================//
////////////////////////////////////////////////////

FeaFixPoint::FeaFixPoint( const string& compID, const string& partID, int type ) : FeaPart( compID, type )
{
    m_ParentFeaPartID = partID;

    m_PosU.Init( "PosU", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosU.SetDescript( "Percent U Location" );

    m_PosW.Init( "PosW", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosW.SetDescript( "Percent W Location" );

    m_FixPointMassFlag.Init( "FixPointMassFlag", "FeaFixPoint", this, false, false, true );
    m_FixPointMassFlag.SetDescript( "Flag to Include Mass of FeaFixPoint" );

    m_FixPointMass.Init( "FixPointMass", "FeaFixPoint", this, 0.0, 0.0, 1e12 );
    m_FixPointMass.SetDescript( "FeaFixPoint Mass Value" );

    m_FeaPropertyIndex = -1; // No property
    m_CapFeaPropertyIndex = -1; // No property
}

void FeaFixPoint::UpdateSurface()
{
    m_MainFeaPartSurfVec.clear(); // FeaFixPoints are not a VspSurf
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

void FeaFixPoint::UpdateDrawObjs()
{
    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();
        m_FeaPartDO.resize( parent_surf_vec.size() );

        for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
        {
            m_FeaPartDO[i].m_PntVec.clear();

            m_FeaPartDO[i].m_GeomID = string( GetID() + "_" + std::to_string( i ) + "_FeaFixPoint" );
            m_FeaPartDO[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            m_FeaPartDO[i].m_Type = DrawObj::VSP_POINTS;
            m_FeaPartDO[i].m_PointSize = 8.0;

            m_FeaPartDO[i].m_PointColor = vec3d( 0.0, 0.0, 0.0 );

            vec3d fixpt = parent_surf_vec[i].CompPnt01( m_PosU(), m_PosW() );
            m_FeaPartDO[i].m_PntVec.push_back( fixpt );

            m_FeaPartDO[i].m_GeomChanged = true;
        }
    }
}

void FeaFixPoint::SetDrawObjHighlight( bool highlight )
{
    if ( highlight )
    {
        for ( unsigned int j = 0; j < m_FeaPartDO.size(); j++ )
        {
            m_FeaPartDO[j].m_PointColor = vec3d( 1.0, 0.0, 0.0 );
        }
    }
    else
    {
        for ( unsigned int j = 0; j < m_FeaPartDO.size(); j++ )
        {
            m_FeaPartDO[j].m_PointColor = vec3d( 0.0, 0.0, 0.0 );
        }
    }
}

bool FeaFixPoint::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    return false;
}

////////////////////////////////////////////////////
//================= FeaPartTrim ==================//
////////////////////////////////////////////////////

FeaPartTrim::FeaPartTrim( const string& geomID, int type ) : FeaPart( geomID, type )
{
    m_IncludedElements.Set( vsp::FEA_NO_ELEMENTS );

    m_FeaPropertyIndex = -1; // No property
    m_CapFeaPropertyIndex = -1; // No property
}

FeaPartTrim::~FeaPartTrim()
{
    Clear();
}

void FeaPartTrim::Clear()
{
    for ( int i = 0; i < m_FlipFlagVec.size(); i++ )
    {
        delete m_FlipFlagVec[i];
    }
    m_FlipFlagVec.clear();

    m_TrimFeaPartIDVec.clear();
}

void FeaPartTrim::UpdateSurface()
{
    m_MainFeaPartSurfVec.clear(); // FeaFixPoints are not a VspSurf
}

xmlNodePtr FeaPartTrim::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr part_info = xmlNewChild( node, NULL, BAD_CAST "FeaPartInfo", NULL );

    if ( part_info )
    {
        XmlUtil::AddIntNode( part_info, "FeaPartType", m_FeaPartType );

        xmlNodePtr tlist_node = xmlNewChild( part_info, NULL, BAD_CAST "TrimList", NULL );
        for ( int i = 0 ; i < ( int )m_TrimFeaPartIDVec.size() ; i++ )
        {
            xmlNodePtr trim_node = xmlNewChild( tlist_node, NULL, BAD_CAST "TrimPart", NULL );
            XmlUtil::AddStringNode( trim_node, "ID", m_TrimFeaPartIDVec[i] );
        }

        ParmContainer::EncodeXml( part_info );
    }

    return part_info;
}

xmlNodePtr FeaPartTrim::DecodeXml( xmlNodePtr & node )
{
    Clear();

    xmlNodePtr tl_node = XmlUtil::GetNode( node, "TrimList", 0 );
    int num_trim = XmlUtil::GetNumNames( tl_node, "TrimPart" );

    for ( int i = 0 ; i < num_trim ; i++ )
    {
        xmlNodePtr n = XmlUtil::GetNode( tl_node, "TrimPart", i );
        AddTrimPart( ParmMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
    }

    ParmContainer::DecodeXml( node );

    return node;
}

void FeaPartTrim::UpdateDrawObjs()
{
    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    DrawObj arrowHeadDO;
    DrawObj arrowLineDO;

    for ( unsigned int ipart = 0; ipart < m_TrimFeaPartIDVec.size(); ipart++ )
    {
        FeaPart* parent_part = StructureMgr.GetFeaPart( m_TrimFeaPartIDVec[ipart] );

        if ( parent_part )
        {
            vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();

            for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
            {
                VspSurf s = parent_surf_vec[i];

                vec3d cen = s.CompPnt01( 0.5, 0.5 );
                vec3d dir = s.CompNorm01( 0.5, 0.5 );

                if ( ipart < m_FlipFlagVec.size() )
                {
                    if ( m_FlipFlagVec[ ipart ] )
                    {
                        if ( m_FlipFlagVec[ ipart ]->Get() )
                        {
                            dir = -1.0 * dir;
                        }
                    }
                }

                arrowLineDO.m_PntVec.push_back( cen );
                arrowLineDO.m_PntVec.push_back( cen + dir * axlen );

                MakeArrowhead( cen + dir * axlen, dir, 0.25 * axlen, arrowHeadDO.m_PntVec );

            }
        }
    }

    arrowHeadDO.m_GeomID = m_ID + "Arrows";
    arrowHeadDO.m_LineWidth = 1.0;
    arrowHeadDO.m_Type = DrawObj::VSP_SHADED_TRIS;
    arrowHeadDO.m_NormVec = vector <vec3d> ( arrowHeadDO.m_PntVec.size() );

    for ( int i = 0; i < 3; i++ )
    {
        arrowHeadDO.m_MaterialInfo.Ambient[i] = 0.2f;
        arrowHeadDO.m_MaterialInfo.Diffuse[i] = 0.1f;
        arrowHeadDO.m_MaterialInfo.Specular[i] = 0.7f;
        arrowHeadDO.m_MaterialInfo.Emission[i] = 0.0f;
    }
    arrowHeadDO.m_MaterialInfo.Diffuse[3] = 0.5;
    arrowHeadDO.m_MaterialInfo.Shininess = 5.0;


    arrowLineDO.m_GeomID = m_ID + "ALines";
    arrowLineDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    arrowLineDO.m_LineWidth = 2.0;
    arrowLineDO.m_LineColor = vec3d( 0, 0, 0 );
    arrowLineDO.m_Type = DrawObj::VSP_LINES;

    arrowLineDO.m_GeomChanged = true;
    arrowHeadDO.m_GeomChanged = true;

    m_FeaPartDO.clear();
    m_FeaPartDO.push_back( arrowHeadDO );
    m_FeaPartDO.push_back( arrowLineDO );
}

void FeaPartTrim::SetDrawObjHighlight( bool highlight )
{
    if ( highlight )
    {
        for ( unsigned int j = 0; j < m_FeaPartDO.size(); j++ )
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
            m_FeaPartDO[j].m_MaterialInfo.Ambient[0] = 1.0;
        }
    }
    else
    {
        for ( unsigned int j = 0; j < m_FeaPartDO.size(); j++ )
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            m_FeaPartDO[j].m_MaterialInfo.Ambient[0] = 0.2;
        }
    }
}

void FeaPartTrim::FetchTrimPlanes( vector < vector < vec3d > > &pt, vector < vector < vec3d > > &norm )
{
    // This nested loop is most naturally accessed with ipart as the outer loop.  However, going forward we will
    // need pt and norm set up with ipart as the inner loop.  First we loop through to check that the referenced
    // parts all have the same symmetry (which determines the outer loop size).

    // Determine number of symmetrical copies.
    int nsymm = -1;
    bool samesize = true;
    for ( unsigned int ipart = 0; ipart < m_TrimFeaPartIDVec.size(); ipart++ )
    {
        FeaPart *parent_part = StructureMgr.GetFeaPart( m_TrimFeaPartIDVec[ ipart ] );

        if ( parent_part )
        {
            vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();

            if ( nsymm < 0 )
            {
                nsymm = parent_surf_vec.size();
            }
            else
            {
                if ( nsymm != parent_surf_vec.size() )
                {
                    printf( "Error, parts used by FEA Trim do not have the same number of symmetrical copies\n" );
                    samesize = false;
                }
            }
        }
    }

    if ( samesize )
    {
        pt.resize( nsymm );
        norm.resize( nsymm );

        int npart = m_TrimFeaPartIDVec.size();

        for ( size_t isymm = 0; isymm < nsymm; isymm++ )
        {
            pt[isymm].resize( npart );
            norm[isymm].resize( npart );

            for ( unsigned int ipart = 0; ipart < npart; ipart++ )
            {
                FeaPart *parent_part = StructureMgr.GetFeaPart( m_TrimFeaPartIDVec[ ipart ] );

                if ( parent_part )
                {
                    vector < VspSurf > parent_surf_vec = parent_part->GetFeaPartSurfVec();

                    VspSurf s = parent_surf_vec[isymm];

                    vec3d cen = s.CompPnt01( 0.5, 0.5 );
                    vec3d dir = s.CompNorm01( 0.5, 0.5 );

                    if ( m_FlipFlagVec[ipart]->Get() )
                    {
                        dir = -1.0 * dir;
                    }

                    pt[isymm][ipart] = cen;
                    norm[isymm][ipart] = dir;
                }
            }
        }
    }
}

bool FeaPartTrim::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    return false;
}

void FeaPartTrim::AddTrimPart( string partID )
{
    BoolParm* bp = dynamic_cast<BoolParm*>( ParmMgr.CreateParm( vsp::PARM_BOOL_TYPE ) );
    if ( bp )
    {
        int i = (int)m_FlipFlagVec.size();
        char str[15];
        sprintf( str, "FlipFlag_%d", i );
        bp->Init( string( str ), "FeaPartTrim", this, false, false, true );
        bp->SetDescript( "Trim direction flip flag" );
        m_FlipFlagVec.push_back( bp );

    }

    m_TrimFeaPartIDVec.push_back( partID );

    m_LateUpdateFlag = true;
    ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
}

void FeaPartTrim::DeleteTrimPart( int indx )
{
    if ( indx >= 0 && indx < m_FlipFlagVec.size() )
    {
        delete m_FlipFlagVec[ indx ];
        m_FlipFlagVec.erase( m_FlipFlagVec.begin() + indx );

        m_TrimFeaPartIDVec.erase( m_TrimFeaPartIDVec.begin() + indx );

        RenameParms();

        m_LateUpdateFlag = true;
        ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
    }
}

void FeaPartTrim::RenameParms()
{
    for ( int i = 0; i < m_FlipFlagVec.size(); i++ )
    {
        char str[255];
        sprintf( str, "FlipFlag_%d", i );
        m_FlipFlagVec[i]->SetName( string( str ) );
    }
}

////////////////////////////////////////////////////
//=================== FeaSkin ====================//
////////////////////////////////////////////////////

FeaSkin::FeaSkin( const string& geomID, int type ) : FeaPart( geomID, type )
{
    m_IncludedElements.Set( vsp::FEA_SHELL );
    m_DrawFeaPartFlag.Set( false );

    m_RemoveSkinFlag.Init( "RemoveSkinTrisFlag", "FeaSkin", this, false, false, true );
    m_RemoveSkinFlag.SetDescript( "Flag to Remove Skin Surface and Triangles after Intersections" );
}

void FeaSkin::UpdateSurface()
{
    BuildSkinSurf();
}

void FeaSkin::BuildSkinSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( 1 );

    if ( veh )
    {
        Geom* currgeom = veh->FindGeom( m_ParentGeomID );

        if ( currgeom )
        {
            vector< VspSurf > surf_vec;
            surf_vec = currgeom->GetSurfVecConstRef(  );

            m_MainFeaPartSurfVec[0] = surf_vec[m_MainSurfIndx];
        }
    }
}

bool FeaSkin::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    return false;
}

////////////////////////////////////////////////////
//================= FeaDome ==================//
////////////////////////////////////////////////////

FeaDome::FeaDome( const string& geomID, int type ) : FeaPart( geomID, type )
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

void FeaDome::UpdateSurface()
{
    BuildDomeSurf();
}

typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_dome_creator;

void FeaDome::BuildDomeSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( 1 );

    if ( veh )
    {
        Geom* curr_geom = veh->FindGeom( m_ParentGeomID );

        if ( !curr_geom )
        {
            return;
        }

        m_MainFeaPartSurfVec[0] = VspSurf(); // Create primary VspSurf

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
        m_MainFeaPartSurfVec[0].CreateBodyRevolution( stringer );

        // Scale to ellipsoid
        m_MainFeaPartSurfVec[0].ScaleX( m_Aradius() );
        m_MainFeaPartSurfVec[0].ScaleY( m_Bradius() );
        m_MainFeaPartSurfVec[0].ScaleZ( m_Cradius() );

        // Rotate at orgin and then translate to final location
        Matrix4d rot_mat_x, rot_mat_y, rot_mat_z;
        vec3d x_axis, y_axis, z_axis;

        x_axis.set_x( 1.0 );
        y_axis.set_y( 1.0 );
        z_axis.set_z( 1.0 );

        rot_mat_x.loadIdentity();
        rot_mat_x.rotate( DEG_2_RAD * m_XRot(), x_axis );
        m_MainFeaPartSurfVec[0].Transform( rot_mat_x );

        rot_mat_y.loadIdentity();
        rot_mat_y.rotate( DEG_2_RAD * m_YRot(), y_axis );
        m_MainFeaPartSurfVec[0].Transform( rot_mat_y );

        rot_mat_z.loadIdentity();
        rot_mat_z.rotate( DEG_2_RAD * m_ZRot(), z_axis );
        m_MainFeaPartSurfVec[0].Transform( rot_mat_z );

        m_MainFeaPartSurfVec[0].OffsetX( m_XLoc() );
        m_MainFeaPartSurfVec[0].OffsetY( m_YLoc() );
        m_MainFeaPartSurfVec[0].OffsetZ( m_ZLoc() );

        // Transform to parent geom body coordinate frame
        Matrix4d model_matrix = curr_geom->getModelMatrix();
        m_MainFeaPartSurfVec[0].Transform( model_matrix );

        if ( m_SpineAttachFlag() )
        {
            vector< VspSurf > surf_vec;
            surf_vec = curr_geom->GetSurfVecConstRef();

            // Build conformal spine from parent geom
            ConformalSpine cs;
            cs.Build( surf_vec[m_MainSurfIndx] );

            vec3d spine_center = cs.FindCenterGivenU( m_USpineLoc() * surf_vec[m_MainSurfIndx].GetUMax() );

            m_MainFeaPartSurfVec[0].OffsetX( spine_center.x() - curr_geom->m_XLoc() );
            m_MainFeaPartSurfVec[0].OffsetY( spine_center.y() - curr_geom->m_YLoc() );
            m_MainFeaPartSurfVec[0].OffsetZ( spine_center.z() - curr_geom->m_ZLoc() );
        }

        m_MainFeaPartSurfVec[0].BuildFeatureLines();
    }
}

void FeaDome::UpdateDrawObjs()
{
    // Two DrawObjs per Dome surface: index j correcponds to the surface (quads) and 
    //  j + 1 corresponds to the cross section feature line at u_max 

    m_FeaPartDO.clear();
    m_FeaPartDO.resize( m_FeaPartSurfVec.size() );
    m_FeaHighlightDO.clear();
    m_FeaHighlightDO.resize( m_FeaPartSurfVec.size() );

    for ( size_t j = 0; j < m_FeaPartSurfVec.size(); j++ )
    {
        m_FeaPartDO[j].m_GeomID = string( GetID() + "_" + std::to_string( j ) + "_" + m_Name );
        m_FeaPartDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        m_FeaHighlightDO[j].m_GeomID = string( GetID() + "_hl_" + std::to_string( j ) + "_" + m_Name );
        m_FeaHighlightDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        m_FeaPartDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
        m_FeaPartDO[j].m_LineWidth = 1.0;
        m_FeaHighlightDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
        m_FeaHighlightDO[j].m_LineWidth = 1.0;

        // Tesselate the surface (can adjust num_u and num_v Tessellation for smoothness) 
        vector < vector < vec3d > > pnts, norms, uw;
        m_FeaPartSurfVec[j].Tesselate( 10, 18, pnts, norms, uw, 3, false );

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

        m_FeaPartDO[j].m_MaterialInfo.Diffuse[3] = 0.33f;

        m_FeaPartDO[j].m_MaterialInfo.Shininess = 5.0f;

        // Add points for bulkhead cross section at u_max
        m_FeaHighlightDO[j].m_Type = DrawObj::VSP_LINE_LOOP;

        for ( size_t i = 0; i < pnts[pnts.size() - 1].size(); i++ )
        {
            m_FeaHighlightDO[j].m_PntVec.push_back( pnts[pnts.size() - 1][i] );
        }

        m_FeaPartDO[j].m_GeomChanged = true;
        m_FeaHighlightDO[j].m_GeomChanged = true;
    }
}

bool FeaDome::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    return false;
}

////////////////////////////////////////////////////
//================= FeaRibArray ==================//
////////////////////////////////////////////////////

FeaRibArray::FeaRibArray( const string& geomID, int type ) : FeaPart( geomID, type )
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

    m_BndBoxTrimFlag.Init( "BndBoxTrimFlag", "FeaRibArray", this, true, false, true );
    m_BndBoxTrimFlag.SetDescript( "Flag to Trim Rib Array to Bounding Box Instead of Wing Surface" );

    m_PerpendicularEdgeType.Init( "PerpendicularEdgeType", "FeaRibArray", this, vsp::NO_NORMAL, vsp::NO_NORMAL, vsp::SPAR_NORMAL );
    m_PerpendicularEdgeType.SetDescript( "Identifies the Perpendicular Edge Type for the Rib Array" );

    m_MatchDihedralFlag.Init( "MatchDihedralFlag", "FeaRibArray", this, true, false, true );
    m_MatchDihedralFlag.SetDescript( "Flag to Rotate the Rib Array with the Dihedral Angle of the Wing" );

    m_NumRibs = 0;
}

FeaRibArray::~FeaRibArray()
{

}

void FeaRibArray::UpdateSurface()
{
    CalcNumRibs();

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( m_NumRibs );

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

            m_AbsStartLocation.Set( m_RelStartLocation() * span_f );
            m_RibAbsSpacing.Set( m_RibRelSpacing() * span_f );
            m_AbsEndLocation.Set( m_RelEndLocation() * span_f );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
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

            m_RelStartLocation.Set( m_AbsStartLocation() / span_f );
            m_RibRelSpacing.Set( m_RibAbsSpacing() / span_f );
            m_RelEndLocation.Set( m_AbsEndLocation() / span_f );
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
        surf_vec = current_wing->GetSurfVecConstRef();

        BndBox wing_bbox;
        surf_vec[m_MainSurfIndx].GetBoundingBox( wing_bbox );

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
            rib->m_PerpendicularEdgeType.Set( m_PerpendicularEdgeType() );
            rib->SetPerpendicularEdgeID( m_PerpendicularEdgeID );
            rib->m_LimitRibToSectionFlag.Set( m_LimitArrayToSectionFlag() );
            rib->m_StartWingSection.Set( m_StartWingSection() );
            rib->m_EndWingSection.Set( m_EndWingSection() );
            rib->m_BndBoxTrimFlag.Set( m_BndBoxTrimFlag() );
            rib->m_MatchDihedralFlag.Set( m_MatchDihedralFlag() );

            // Update Rib Relative Center Location
            double rel_center_location =  m_RelStartLocation() + dir * i * m_RibRelSpacing();
            rib->m_RelCenterLocation.Set( rel_center_location );

            // Update
            rib->UpdateParmLimits();
            rib->GetRibPerU();

            // Get rib surface
            m_MainFeaPartSurfVec[i] = rib->ComputeRibSurf();

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
        fearib->m_PerpendicularEdgeType.Set( m_PerpendicularEdgeType() );
        fearib->SetPerpendicularEdgeID( m_PerpendicularEdgeID );
        fearib->m_BndBoxTrimFlag.Set( m_BndBoxTrimFlag() );
        fearib->m_LimitRibToSectionFlag.Set( m_LimitArrayToSectionFlag() );
        fearib->m_StartWingSection.Set( m_StartWingSection() );
        fearib->m_EndWingSection.Set( m_EndWingSection() );
        fearib->m_MatchDihedralFlag.Set( m_MatchDihedralFlag() );

        fearib->SetName( string( m_Name + "_Rib" + std::to_string( ind ) ) );

        fearib->Update();
    }

    return fearib;
}

bool FeaRibArray::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    for ( size_t i = 0; i < m_NumRibs; i++ )
    {
        if ( FeaPart::PtsOnPlanarPart( pnts, minlen, i * m_SymmIndexVec.size() ) )
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

        // Check for previous implementation of perpendicular edge for ribs to enable compatibility in new VSP versions
        xmlNodePtr child_node = XmlUtil::GetNode( fea_prt_node, "FeaRib", 0 );
        if ( XmlUtil::FindInt( child_node, "PerpendicularEdgeType", -1 ) == -1 )
        {
            if ( strcmp( m_PerpendicularEdgeID.c_str(), "Trailing Edge" ) == 0 )
            {
                m_PerpendicularEdgeType.Set( vsp::TE_NORMAL );
            }
            else if ( strcmp( m_PerpendicularEdgeID.c_str(), "Leading Edge" ) == 0 )
            {
                m_PerpendicularEdgeType.Set( vsp::LE_NORMAL );
            }
            else if ( strcmp( m_PerpendicularEdgeID.c_str(), "None" ) == 0 )
            {
                m_PerpendicularEdgeType.Set( vsp::NO_NORMAL );
            }
            else
            {
                m_PerpendicularEdgeType.Set( vsp::SPAR_NORMAL );
            }
        }
    }

    return fea_prt_node;
}

////////////////////////////////////////////////////
//================= FeaSliceArray ==================//
////////////////////////////////////////////////////

FeaSliceArray::FeaSliceArray( const string& geomID, int type ) : FeaPart( geomID, type )
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

void FeaSliceArray::UpdateSurface()
{
    CalcNumSlices();

    m_MainFeaPartSurfVec.clear();
    m_MainFeaPartSurfVec.resize( m_NumSlices );

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
        surf_vec = current_geom->GetSurfVecConstRef();

        // Determine BndBox dimensions prior to rotating and translating
        Matrix4d model_matrix = current_geom->getModelMatrix();
        model_matrix.affineInverse();

        VspSurf orig_surf = surf_vec[m_MainSurfIndx];
        orig_surf.Transform( model_matrix );

        BndBox geom_bbox;

        if ( RefFrameIsBody( m_OrientationPlane() ) )
        {
            orig_surf.GetBoundingBox( geom_bbox );
        }
        else
        {
            surf_vec[m_MainSurfIndx].GetBoundingBox( geom_bbox );
        }

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

        double perp_dist = 0.0; // Total distance perpendicular to the FeaSlice plane

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
            cs.Build( surf_vec[m_MainSurfIndx] );

            perp_dist = cs.GetSpineLength();
        }

        //Calculate number of slices and update Parm limits and values
        if ( m_AbsRelParmFlag() == vsp::REL )
        {
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

            m_AbsStartLocation.Set( m_RelStartLocation() * perp_dist );
            m_SliceAbsSpacing.Set( m_SliceRelSpacing() * perp_dist );
            m_AbsEndLocation.Set( m_RelEndLocation() * perp_dist );
        }
        else if ( m_AbsRelParmFlag() == vsp::ABS )
        {
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

            m_RelStartLocation.Set( m_AbsStartLocation() / perp_dist );
            m_SliceRelSpacing.Set( m_SliceAbsSpacing() / perp_dist );
            m_RelEndLocation.Set( m_AbsEndLocation() / perp_dist );
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
        surf_vec = current_geom->GetSurfVecConstRef();

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
            slice->m_RotationAxis.Set( m_RotationAxis() );
            slice->m_XRot.Set( m_XRot() );
            slice->m_YRot.Set( m_YRot() );
            slice->m_ZRot.Set( m_ZRot() );

            // Update Slice Relative Center Location
            double rel_center_location = m_RelStartLocation() + dir * i * m_SliceRelSpacing();
            slice->m_RelCenterLocation.Set( rel_center_location );

            slice->UpdateParmLimits();

            m_MainFeaPartSurfVec[i] = slice->ComputeSliceSurf();

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

        slice->Update();
    }

    return slice;
}

bool FeaSliceArray::PtsOnPlanarPart( const vector < vec3d > & pnts, double minlen, int surf_ind )
{
    for ( size_t i = 0; i < m_NumSlices; i++ )
    {
        if ( FeaPart::PtsOnPlanarPart( pnts, minlen, i * m_SymmIndexVec.size() ) )
        {
            return true;
        }
    }
    return false;
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

    m_Ixx.Init( "Ixx", "FeaProperty", this, 0.0, -1.0e12, 1.0e12 );
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
    m_FeaMaterialType.Init( "FeaMaterialType", "FeaMaterial", this, vsp::FEA_ISOTROPIC, vsp::FEA_ISOTROPIC, vsp::FEA_NUM_MAT_TYPES - 1 );
    m_FeaMaterialType.SetDescript( "Fea Material Type" );

    m_MassDensity.Init( "MassDensity", "FeaMaterial", this, 1.0, 0.0, 1.0e12 );
    m_MassDensity.SetDescript( "Mass Density of Material" );

    m_ElasticModulus.Init( "ElasticModulus", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_ElasticModulus.SetDescript( "Elastic (Young's) Modulus for Material" );

    m_PoissonRatio.Init( "PoissonRatio", "FeaMaterial", this, 0.0, -1.0, 0.5 );
    m_PoissonRatio.SetDescript( "Poisson's Ratio for Material" );

    m_ThermalExpanCoeff.Init( "ThermalExpanCoeff", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_ThermalExpanCoeff.SetDescript( "Thermal Expansion Coefficient for Material" );

    m_E1.Init( "E1", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_E1.SetDescript( "E1 Elastic (Young's) Modulus for Material" );

    m_E2.Init( "E2", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_E2.SetDescript( "E2 Elastic (Young's) Modulus for Material" );

    m_E3.Init( "E3", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_E3.SetDescript( "E3 Elastic (Young's) Modulus for Material" );

    m_nu12.Init( "nu12", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_nu12.SetDescript( "nu12 Poisson's Ratio for Material" );

    m_nu13.Init( "nu13", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_nu13.SetDescript( "nu13 Poisson's Ratio for Material" );

    m_nu23.Init( "nu23", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_nu23.SetDescript( "nu23 Poisson's Ratio for Material" );

    m_G12.Init( "G12", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_G12.SetDescript( "G12 Shear Modulus for Material" );

    m_G13.Init( "G13", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_G13.SetDescript( "G13 Shear Modulus for Material" );

    m_G23.Init( "G23", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_G23.SetDescript( "G23 Shear Modulus for Material" );

    m_A1.Init( "A1", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_A1.SetDescript( "A1 Thermal Expansion Coefficient for Material" );

    m_A2.Init( "A2", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_A2.SetDescript( "A2 Thermal Expansion Coefficient for Material" );

    m_A3.Init( "A3", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_A3.SetDescript( "A3 Thermal Expansion Coefficient for Material" );

}

FeaMaterial::~FeaMaterial()
{

}

void FeaMaterial::Update()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    if ( !m_UserFeaMaterial )
    {
        // SI Units Used
        // Reference: http://www.matweb.com/search/datasheet.aspx?bassnum=MA0001

        int density_unit = -1;
        int pressure_unit = -1;

        switch ( (int)veh->m_StructUnit() )
        {
        case vsp::SI_UNIT:
        density_unit = vsp::RHO_UNIT_KG_M3;
        pressure_unit = vsp::PRES_UNIT_PA;
        break;

        case vsp::CGS_UNIT:
        density_unit = vsp::RHO_UNIT_G_CM3;
        pressure_unit = vsp::PRES_UNIT_BA;
        break;

        case vsp::MPA_UNIT:
        density_unit = vsp::RHO_UNIT_TONNE_MM3;
        pressure_unit = vsp::PRES_UNIT_MPA;
        break;

        case vsp::BFT_UNIT:
        density_unit = vsp::RHO_UNIT_SLUG_FT3;
        pressure_unit = vsp::PRES_UNIT_PSF;
        break;

        case vsp::BIN_UNIT:
        density_unit = vsp::RHO_UNIT_LBFSEC2_IN4;
        pressure_unit = vsp::PRES_UNIT_PSI;
        break;
        }

        if ( strcmp( m_Name.c_str(), "Aluminum 7075-T6" ) == 0 )
        {
            m_PoissonRatio.Set( 0.33 );
            m_MassDensity.Set( ConvertDensity( 2810, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_ElasticModulus.Set( ConvertPressure( 71.7e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_ThermalExpanCoeff.Set( ConvertThermalExpanCoeff( 23.6e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-�C)
        }
        else if ( strcmp( m_Name.c_str(), "Aluminum 2024-T3" ) == 0 )
        {
            m_PoissonRatio.Set( 0.33 );
            m_MassDensity.Set( ConvertDensity( 2780, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_ElasticModulus.Set( ConvertPressure( 73.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_ThermalExpanCoeff.Set( ConvertThermalExpanCoeff( 23.2e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-�C)
        }
        else if ( strcmp( m_Name.c_str(), "Titanium Ti-6Al-4V" ) == 0 )
        {
            m_PoissonRatio.Set( 0.342 );
            m_MassDensity.Set( ConvertDensity( 4430, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_ElasticModulus.Set( ConvertPressure( 113.8e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_ThermalExpanCoeff.Set( ConvertThermalExpanCoeff( 9.2e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-�C)
        }
        else if ( strcmp( m_Name.c_str(), "AISI 4130 Steel" ) == 0 )
        {
            m_PoissonRatio.Set( 0.29 );
            m_MassDensity.Set( ConvertDensity( 7850, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_ElasticModulus.Set( ConvertPressure( 205e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_ThermalExpanCoeff.Set( ConvertThermalExpanCoeff( 13.7e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-�C)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [0_2/90]s" ) == 0 )
        {
            /*
            AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629
            AS4 fiber density 1.79 g/cm^3
            3501-6 resin density 1.265 g/cm^3
            60.5% fiber volume fraction
            laminateDensity = ( (1-V) * resinDensity) + (V * fiberDensity)
            laminateDensity = 1.582625
            */
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 79.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 44.6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 11.4e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.072 );
            m_nu13.Set( 0.402 );
            m_nu23.Set( 0.465 );
            m_G12.Set( ConvertPressure( 6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 5.38e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 3.47e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [0/90]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 62.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 62.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 11e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.052 );
            m_nu13.Set( 0.438 );
            m_nu23.Set( 0.427 );
            m_G12.Set( ConvertPressure( 6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 4.22e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 3.95e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [0/90/+-45]s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 46e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 46.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 11.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.296 );
            m_nu13.Set( 0.318 );
            m_nu23.Set( 0.317 );
            m_G12.Set( ConvertPressure( 17.7e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 4.32e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 3.58e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [+-30]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 47.2e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 12.2e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 10.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 1.13 );
            m_nu13.Set( -0.197 );
            m_nu23.Set( 0.434 );
            m_G12.Set( ConvertPressure( 23.6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 4.88e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 3.55e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [+-45]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 20.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 20.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 11.8e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.689 );
            m_nu13.Set( 0.211 );
            m_nu23.Set( 0.211 );
            m_G12.Set( ConvertPressure( 29.4e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 4.11e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 4.11e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [+-60]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 12.2e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 47.2e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 12e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.294 );
            m_nu13.Set( 0.434 );
            m_nu23.Set( -0.197 );
            m_G12.Set( ConvertPressure( 23.6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 3.55e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 4.88e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Glass Epoxy S2 3501-6 [0_2/90]s" ) == 0 )
        {
            /*
            S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 629
            S2 fiber density 2.49 g/cm^3
            3501-6 resin density 1.265 g/cm^3
            50% fiber volume fraction
            laminateDensity = ( (1-V) * resinDensity) + (V * fiberDensity)
            laminateDensity = 1.8775
            */
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1878, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 38.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 26.4e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 15.9e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.165 );
            m_nu13.Set( 0.359 );
            m_nu23.Set( 0.427 );
            m_G12.Set( ConvertPressure( 6.76e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 6.33e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 5.2e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Glass Epoxy S2 3501-6 [0/90]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1878, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 32.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 32.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 15.8e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.135 );
            m_nu13.Set( 0.393 );
            m_nu23.Set( 0.392 );
            m_G12.Set( ConvertPressure( 6.76e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 5.72e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 5.59e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Glass Epoxy S2 3501-6 [0/90/+-45]s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1878, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 26.8e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 26.8e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 16e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.28 );
            m_nu13.Set( 0.329 );
            m_nu23.Set( 0.329 );
            m_G12.Set( ConvertPressure( 10.5e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 5.78e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 5.04e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Balsa LTR" ) == 0 )
        {
            // Wood Handbook, Wood as an Engineering Material 2010
            // Forest Products Laboratory. General Technical Report FPL-GTR-190.
            // Madison, WI: U.S. Department of Agriculture, Forest Service, Forest Products Laboratory
            double EL = 1.1 * 3400e6;  // 10% Correction to modulus from bending test.
            double ET = 0.015 * EL;
            double ER = 0.046 * EL;
            double GLR = 0.054 * EL;
            double GLT = 0.037 * EL;
            double GRT = 0.005 * EL;
            double nuLR = 0.229;
            double nuLT = 0.488;
            double nuRT = 0.665;

            // Calculate these using equation to ensure consistent values.
            // Commented value is that given in the reference
            double nuRL = nuLR * ER / EL; // 0.018;
            double nuTL = nuLT * ET / EL; // 0.009;
            double nuTR = nuRT * ET / ER; // 0.231;

            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 160, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( EL, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( ET, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( ER, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( nuLT );
            m_nu13.Set( nuLR );
            m_nu23.Set( nuTR );
            m_G12.Set( ConvertPressure( GLT, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( GLR, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( GRT, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Sitka Spruce LTR" ) == 0 )
        {
            // Elastic properties from NASA TM-104059
            // Structural Integrity of Wind Tunnel Wooden Fan Blades
            // Sitka Spruce 8% Moisture content
            // Density and CTE from MatWeb
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 360, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3

            double EL = 1772000;
            double ET = 87000;
            double ER = 154000;
            double GLT = 117000;
            double GLR = 120000;
            double GRT = 7100;

            double nuLT = 0.441;
            double nuLR = 0.375;
            double nuRT = 0.471;
            // Calculate these using equation to ensure consistent values.
            // Commented value is that given in the reference
            double nuRL = nuLR * ER / EL; // 0.034;
            double nuTL = nuLT * ET / EL; // 0.022;
            double nuTR = nuRT * ET / ER; // 0.248;

            m_E1.Set( ConvertPressure( EL, vsp::PRES_UNIT_PSI, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( ET, vsp::PRES_UNIT_PSI, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( ER, vsp::PRES_UNIT_PSI, pressure_unit ) ); // Pa
            m_nu12.Set( nuLT );
            m_nu13.Set( nuLR );
            m_nu23.Set( nuTR );
            m_G12.Set( ConvertPressure( GLT, vsp::PRES_UNIT_PSI, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( GLR, vsp::PRES_UNIT_PSI, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( GRT, vsp::PRES_UNIT_PSI, pressure_unit ) ); // Pa

            m_A1.Set( ConvertThermalExpanCoeff( 5.4e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 6.3e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 34.1e-6, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        // This group of materials seemed mis-labeled in the handbook.
        // It duplicates another entry above -- but the values are different.
        // It is possible that these values apply to S2/3501-6 for the indicated laminate schedules.
        /*
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [+-30]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 30.7e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 15.6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 14.9e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.545 );
            m_nu13.Set( 0.136 );
            m_nu23.Set( 0.406 );
            m_G12.Set( ConvertPressure( 12.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 6.17e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 5.26e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [+-45]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 19.9e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 19.9e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 16.1e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.467 );
            m_nu13.Set( 0.284 );
            m_nu23.Set( 0.284 );
            m_G12.Set( ConvertPressure( 14.2e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 5.67e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 5.67e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        else if ( strcmp( m_Name.c_str(), "Carbon Epoxy AS4 3501-6 [+-60]_2s" ) == 0 )
        {
            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
            m_MassDensity.Set( ConvertDensity( 1583, vsp::RHO_UNIT_KG_M3, density_unit ) ); // kg/m^3
            m_E1.Set( ConvertPressure( 15.6e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E2.Set( ConvertPressure( 30.7e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_E3.Set( ConvertPressure( 16.8e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_nu12.Set( 0.277 );
            m_nu13.Set( 0.406 );
            m_nu23.Set( 0.136 );
            m_G12.Set( ConvertPressure( 12.3e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G13.Set( ConvertPressure( 5.26e9, vsp::PRES_UNIT_PA, pressure_unit ) ); // Pa
            m_G23.Set( ConvertPressure( 6.17e9, vsp::PRES_UNIT_PA, pressure_unit ) ); //  Pa
            m_A1.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A2.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
            m_A3.Set( ConvertThermalExpanCoeff( 0.0, vsp::SI_UNIT, veh->m_StructUnit() ) ); // m/(m-K)
        }
        */
    }
}

void FeaMaterial::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }

    Update();
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
