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
#include "StlHelper.h"

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

    m_FeaGridDensity.SetParentStruct( this );
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

    // Delete Boundary Conditions
    for ( int i = 0; i < (int)m_FeaBCVec.size(); i++ )
    {
        delete m_FeaBCVec[i];
    }
    m_FeaBCVec.clear();
}

void FeaStructure::Update()
{
    UpdateFeaParts();
    UpdateFeaSubSurfs();
    UpdateFeaBCs();
    m_FeaGridDensity.Update();
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

    for ( unsigned int i = 0; i < m_FeaBCVec.size(); i++ )
    {
        m_FeaBCVec[i]->EncodeXml( fea_info );
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
                FeaPart* feaskin = new FeaSkin( m_ParentGeomID, GetID() );
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

    int numBCs = XmlUtil::GetNumNames( node, "FeaBCInfo" );

    for ( unsigned int i = 0; i < numBCs; i++ )
    {
        xmlNodePtr bc_info = XmlUtil::GetNode( node, "FeaBCInfo", i );

        if ( bc_info )
        {
            FeaBC* feabc = AddFeaBC();
            feabc->DecodeXml( bc_info );
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

    for ( size_t i = 0; i < (int)m_FeaBCVec.size(); i++ )
    {
        m_FeaBCVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
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
        feaprt = new FeaSlice( m_ParentGeomID, GetID() );
        feaprt->SetName( string( "Slice" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_RIB )
    {
        feaprt = new FeaRib( m_ParentGeomID, GetID() );
        feaprt->SetName( string( "Rib" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_SPAR )
    {
        feaprt = new FeaSpar( m_ParentGeomID, GetID() );
        feaprt->SetName( string( "Spar" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_FIX_POINT )
    {
        // Initially define the FeaFixPoint on the skin surface
        FeaPart* skin = GetFeaSkin();

        if ( skin )
        {
            feaprt = new FeaFixPoint( m_ParentGeomID, GetID(), skin->GetID() );
            feaprt->SetName( string( "FixPoint" + std::to_string( m_FeaPartCount ) ) );
        }
    }
    else if ( type == vsp::FEA_DOME )
    {
        feaprt = new FeaDome( m_ParentGeomID, GetID() );
        feaprt->SetName( string( "Dome" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_RIB_ARRAY )
    {
        feaprt = new FeaRibArray( m_ParentGeomID, GetID() );
        feaprt->SetName( string( "RibArray" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_SLICE_ARRAY )
    {
        feaprt = new FeaSliceArray( m_ParentGeomID, GetID() );
        feaprt->SetName( string( "SliceArray" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_TRIM )
    {
        feaprt = new FeaPartTrim( m_ParentGeomID, GetID() );
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

    ReorderVectorIndex( m_FeaPartVec, ind, action );
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

SubSurface* FeaStructure::GetFeaSubSurf( const string &id )
{
    for ( int i = 0; i < m_FeaSubSurfVec.size(); i++ )
    {
        if ( m_FeaSubSurfVec[i]->GetID() == id )
        {
            return m_FeaSubSurfVec[i];
        }
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

    ReorderVectorIndex( m_FeaSubSurfVec, ind, action );
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

void FeaStructure::UpdateFeaBCs()
{
    for ( unsigned int i = 0; i < m_FeaBCVec.size(); i++ )
    {
        m_FeaBCVec[i]->Update();
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
            feaskin = new FeaSkin( m_ParentGeomID, GetID() );

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

int FeaStructure::GetFeaPartIndex( const string &id )
{
    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( id == m_FeaPartVec[i]->GetID() )
        {
            return i;
        }
    }
    return -1;
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

vector< FeaPart* > FeaStructure::GetFeaPartVecType( int type )
{
    vector< FeaPart* > retvec;

    for ( int i = 0; i < m_FeaPartVec.size(); i++ )
    {
        FeaPart* fea_part = m_FeaPartVec[i];
        if ( fea_part )
        {
            if ( fea_part->GetType() == type )
            {
                retvec.push_back( fea_part );
            }
        }
    }
    return retvec;
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

void FeaStructure::ResetExportFileNames()
{
    m_StructSettings.ResetExportFileNames( GetName() );
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

FeaBC* FeaStructure::AddFeaBC( int type )
{
    FeaBC* feabc = new FeaBC( GetID() );

    if ( feabc )
    {
        feabc->m_FeaBCType.Set( type );
        AddFeaBC( feabc );
    }

    return feabc;
}

void FeaStructure::DelFeaBC( int ind )
{
    if ( ValidFeaBCInd( ind ) )
    {
        delete m_FeaBCVec[ind];
        m_FeaBCVec.erase( m_FeaBCVec.begin() + ind );
    }
}

bool FeaStructure::ValidFeaBCInd( int ind )
{
    if ( (int)m_FeaBCVec.size() > 0 && ind >= 0 && ind < (int)m_FeaBCVec.size() )
    {
        return true;
    }
    return false;
}

FeaBC* FeaStructure::GetFeaBC( int ind )
{
    if ( ValidFeaBCInd( ind ) )
    {
        return m_FeaBCVec[ind];
    }
    return NULL;
}

int FeaStructure::GetFeaBCIndex( const string &id )
{
    for ( int i = 0; i < (int)m_FeaBCVec.size(); i++ )
    {
        if ( id == m_FeaBCVec[i]->GetID() )
        {
            return i;
        }
    }
    return -1;
}

int FeaStructure::GetFeaBCIndex( FeaBC* fea_bc )
{
    for ( int i = 0; i < (int)m_FeaBCVec.size(); i++ )
    {
        if ( m_FeaBCVec[i] == fea_bc )
        {
            return i;
        }
    }
    return -1; // indicates an error
}

//////////////////////////////////////////////////////
//==================== FeaPart =====================//
//////////////////////////////////////////////////////

FeaPart::FeaPart( const string &geomID, const string &structID, int type )
{
    m_FeaPartType = type;
    m_ParentGeomID = geomID;
    m_StructID = structID;

    m_MainSurfIndx = 0;

    m_IncludedElements.Init( "IncludedElements", "FeaPart", this, vsp::FEA_DEPRECATED, vsp::FEA_DEPRECATED, vsp::FEA_NUM_ELEMENT_TYPES - 1 );
    m_IncludedElements.SetDescript( "Indicates the FeaElements to be Included for the FeaPart" );

    m_CreateBeamElements.Init( "CreateBeamElements","FeaPart", this, false, false, true );
    m_CreateBeamElements.SetDescript( "Flag to indicate whether to create beam elements for this part" );

    m_KeepDelShellElements.Init( "MarkDelShellElements", "FeaPart", this, vsp::FEA_KEEP, vsp::FEA_KEEP, vsp::FEA_NUM_SHELL_TREATMENT_TYPES - 1 );
    m_KeepDelShellElements.SetDescript( "Indicates whether to mark or delete shell elements from this part" );

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

    m_FeaPropertyIndex.Init( "FeaPropertyIndex", "FeaPart", this, -1, -1, 1e12 ); // Shell property default
    m_FeaPropertyIndex.SetDescript( "FeaPropertyIndex for Shell Elements" );

    m_CapFeaPropertyIndex.Init( "CapFeaPropertyIndex", "FeaPart", this, -1, -1, 1e12 ); // Beam property default
    m_CapFeaPropertyIndex.SetDescript( "FeaPropertyIndex for Beam (Cap) Elements" );
}

FeaPart::~FeaPart()
{

}

void FeaPart::Update()
{
    m_LateUpdateFlag = false;

    if ( m_IncludedElements() != vsp::FEA_DEPRECATED )
    {
        if ( m_IncludedElements() == vsp::FEA_SHELL )
        {
            m_CreateBeamElements.Set( false );
            m_KeepDelShellElements.Set( vsp::FEA_KEEP );
        }
        else if ( m_IncludedElements() == vsp::FEA_BEAM )
        {
            m_CreateBeamElements.Set( true );
            m_KeepDelShellElements.Set( vsp::FEA_DELETE );
        }
        else if ( m_IncludedElements() == vsp::FEA_SHELL_AND_BEAM )
        {
            m_CreateBeamElements.Set( true );
            m_KeepDelShellElements.Set( vsp::FEA_KEEP );
        }

        m_IncludedElements.Set( vsp::FEA_DEPRECATED );
    }

    if ( m_FeaPropertyIndex() != -1 )
    {
        vector < FeaProperty* > prop_vec = StructureMgr.GetFeaPropertyVec();
        if ( m_FeaPropertyIndex() < prop_vec.size() )
        {
            m_FeaPropertyID = prop_vec[ m_FeaPropertyIndex() ]->GetID();
            m_FeaPropertyIndex = -1;
        }
    }

    if ( m_CapFeaPropertyIndex() != -1 )
    {
        vector < FeaProperty* > prop_vec = StructureMgr.GetFeaPropertyVec();
        if ( m_CapFeaPropertyIndex() < prop_vec.size() )
        {
            m_CapFeaPropertyID = prop_vec[ m_CapFeaPropertyIndex() ]->GetID();
            m_CapFeaPropertyIndex = -1;
        }
    }

    if ( m_KeepDelShellElements() == vsp::FEA_KEEP )
    {
        FeaProperty *shell_prop = StructureMgr.GetFeaProperty( m_FeaPropertyID );
        if ( !shell_prop )
        {
            m_FeaPropertyID = StructureMgr.GetSomeShellProperty();
        }
    }

    if ( m_CreateBeamElements() )
    {
        FeaProperty *cap_prop = StructureMgr.GetFeaProperty( m_CapFeaPropertyID );
        if ( !cap_prop )
        {
            m_CapFeaPropertyID = StructureMgr.GetSomeBeamProperty();
        }
    }

    UpdateSurface();

    for ( int i = 0; i < m_MainFeaPartSurfVec.size(); i++ )
    {
        m_MainFeaPartSurfVec[i].InitUMapping();
    }

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
            if ( m_KeepDelShellElements() == vsp::FEA_KEEP )
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

void FeaPart::ParmChanged( Parm* parm_ptr, int type )
{
    FeaStructure *pstruct = StructureMgr.GetFeaStruct( m_StructID );

    if ( pstruct )
    {
        pstruct->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr FeaPart::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr part_info = xmlNewChild( node, NULL, BAD_CAST "FeaPartInfo", NULL );

    XmlUtil::AddIntNode( part_info, "FeaPartType", m_FeaPartType );

    if ( part_info )
    {
        XmlUtil::AddStringNode( part_info, "FeaPropertyID", m_FeaPropertyID );
        XmlUtil::AddStringNode( part_info, "CapFeaPropertyID", m_CapFeaPropertyID );
    }

    return ParmContainer::EncodeXml( part_info );
}

xmlNodePtr FeaPart::DecodeXml( xmlNodePtr & node )
{
    if ( node )
    {
        m_FeaPropertyID = ParmMgr.RemapID( XmlUtil::FindString( node, "FeaPropertyID", m_FeaPropertyID ) );
        m_CapFeaPropertyID = ParmMgr.RemapID( XmlUtil::FindString( node, "CapFeaPropertyID", m_CapFeaPropertyID ) );
    }

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

FeaSlice::FeaSlice( const string &geomID, const string &structID, int type ) : FeaPart( geomID, structID, type )
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

FeaSpar::FeaSpar( const string &geomID, const string &structID, int type ) : FeaSlice( geomID, structID, type )
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
            temp_slice = new FeaSlice( m_ParentGeomID, m_StructID );

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

FeaRib::FeaRib( const string &geomID, const string &structID, int type ) : FeaSlice( geomID, structID, type )
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
            temp_slice = new FeaSlice( m_ParentGeomID, m_StructID );

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

FeaFixPoint::FeaFixPoint( const string &compID, const string &structID, const string &partID, int type ) : FeaPart( compID, structID, type )
{
    m_ParentFeaPartID = partID;

    m_PosU.Init( "PosU", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosU.SetDescript( "Percent U Location" );

    m_PosW.Init( "PosW", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosW.SetDescript( "Percent W Location" );

    m_FixPointMassFlag.Init( "FixPointMassFlag", "FeaFixPoint", this, false, false, true );
    m_FixPointMassFlag.SetDescript( "Flag to Include Mass of FeaFixPoint" );

    m_FixPointMass.Init( "FixPointMass", "FeaFixPoint", this, 1.0, 0.0, 1e12 );
    m_FixPointMass.SetDescript( "FeaFixPoint Mass Value" );

    m_FixPointMass_FEM.Init( "FixPointMass_FEM", "FeaFixPoint", this, 0.0, 0.0, 1e12 );
    m_FixPointMass_FEM.SetDescript( "FeaFixPoint Mass Value in FEM units." );

    m_MassUnit.Init( "MassUnit", "FeaFixPoint", this, vsp::MASS_UNIT_LBM, vsp::MASS_UNIT_G, vsp::NUM_MASS_UNIT - 1 );
    m_MassUnit.SetDescript( "Mass units used to specify point mass" );

    m_FeaPropertyIndex = -1; // No property
    m_CapFeaPropertyIndex = -1; // No property
    m_FeaPropertyID = "";
    m_CapFeaPropertyID = "";
}

void FeaFixPoint::UpdateSurface()
{
    m_MainFeaPartSurfVec.clear(); // FeaFixPoints are not a VspSurf

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    int mass_unit = -1;

    switch ( ( int ) veh->m_StructUnit() )
    {
        case vsp::SI_UNIT:
            mass_unit = vsp::MASS_UNIT_KG;
            break;

        case vsp::CGS_UNIT:
            mass_unit = vsp::MASS_UNIT_G;
            break;

        case vsp::MPA_UNIT:
            mass_unit = vsp::MASS_UNIT_TONNE;
            break;

        case vsp::BFT_UNIT:
            mass_unit = vsp::MASS_UNIT_SLUG;
            break;

        case vsp::BIN_UNIT:
            mass_unit = vsp::MASS_LBFSEC2IN;
            break;
    }

    m_FixPointMass_FEM = ConvertMass( m_FixPointMass(), m_MassUnit(), mass_unit );
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
            double umapmax = parent_surf_vec[i].GetUMapMax();
            double umax = parent_surf_vec[i].GetUMax();
            double u = parent_surf_vec[i].InvertUMapping( m_PosU() * umapmax ) / umax;
            if ( u < 0 )
            {
                u = m_PosU();
            }

            pnt_vec[i] = parent_surf_vec[i].CompPnt01( u, m_PosW() );
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
            double umapmax = parent_surf_vec[0].GetUMapMax();
            double u = parent_surf_vec[0].InvertUMapping( m_PosU() * umapmax );
            if ( u < 0 )
            {
                u = m_PosU();
            }

            uw.set_x( u );
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
        m_ParentFeaPartID = ParmMgr.RemapID( XmlUtil::FindString( fea_prt_node, "ParentFeaPartID", m_ParentFeaPartID ) );
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

            double umapmax = parent_surf_vec[i].GetUMapMax();
            double umax = parent_surf_vec[i].GetUMax();
            double u = parent_surf_vec[i].InvertUMapping( m_PosU() * umapmax ) / umax;

            if ( u < 0 )
            {
                u = m_PosU();
            }

            vec3d fixpt = parent_surf_vec[i].CompPnt01( u, m_PosW() );
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

int FeaFixPoint::NumFeaPartSurfs()
{
    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        return parent_part->NumFeaPartSurfs();
    }
    return 0;
}

////////////////////////////////////////////////////
//================= FeaPartTrim ==================//
////////////////////////////////////////////////////

FeaPartTrim::FeaPartTrim( const string &geomID, const string &structID, int type ) : FeaPart( geomID, structID, type )
{
    m_CreateBeamElements.Set( false );
    m_KeepDelShellElements.Set( vsp::FEA_DELETE );

    m_FeaPropertyIndex = -1; // No property
    m_CapFeaPropertyIndex = -1; // No property
    m_FeaPropertyID = "";
    m_CapFeaPropertyID = "";
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
        snprintf( str, sizeof( str ),  "FlipFlag_%d", i );
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
        snprintf( str, sizeof( str ),  "FlipFlag_%d", i );
        m_FlipFlagVec[i]->SetName( string( str ) );
    }
}

////////////////////////////////////////////////////
//=================== FeaSkin ====================//
////////////////////////////////////////////////////

FeaSkin::FeaSkin( const string &geomID, const string &structID, int type ) : FeaPart( geomID, structID, type )
{
    m_CreateBeamElements.Set( false );
    m_KeepDelShellElements.Set( vsp::FEA_KEEP );

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

FeaDome::FeaDome( const string &geomID, const string &structID, int type ) : FeaPart( geomID, structID, type )
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
    // Two DrawObjs per Dome surface: index j corresponds to the surface (quads) and 
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
        m_FeaPartSurfVec[j].Tesselate( 10, 18, pnts, norms, uw, 3, 10, false );

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

FeaRibArray::FeaRibArray( const string &geomID, const string &structID, int type ) : FeaPart( geomID, structID, type )
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
            rib = new FeaRib( m_ParentGeomID, m_StructID );
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
    FeaRib* fearib = new FeaRib( m_ParentGeomID, m_StructID );

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
        fearib->m_CreateBeamElements.Set( m_CreateBeamElements() );
        fearib->m_KeepDelShellElements.Set( m_KeepDelShellElements() );
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
        fearib->m_FeaPropertyID = m_FeaPropertyID;
        fearib->m_CapFeaPropertyID = m_CapFeaPropertyID;

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

FeaSliceArray::FeaSliceArray( const string &geomID, const string &structID, int type ) : FeaPart( geomID, structID, type )
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
            slice = new FeaSlice( m_ParentGeomID, m_StructID );
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
    FeaSlice* slice = new FeaSlice( m_ParentGeomID, m_StructID );

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
        slice->m_CreateBeamElements.Set( m_CreateBeamElements() );
        slice->m_KeepDelShellElements.Set( m_KeepDelShellElements() );
        slice->m_FeaPropertyIndex.Set( m_FeaPropertyIndex() );
        slice->m_CapFeaPropertyIndex.Set( m_CapFeaPropertyIndex() );
        slice->m_FeaPropertyID = m_FeaPropertyID;
        slice->m_CapFeaPropertyID = m_CapFeaPropertyID;

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

    m_Izz.Init( "Izz", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Izz.SetDescript( "Area Moment of Inertia for Bending in XY Plane of FeaElement Neutral Axis (I1)" );

    m_Iyy.Init( "Iyy", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Iyy.SetDescript( "Area Moment of Inertia for Bending in XZ Plane of FeaElement Neutral Axis (I2)" );

    m_Izy.Init( "Izy", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Izy.SetDescript( "Area Product of Inertia of FeaElement (I12)" );

    m_Ixx.Init( "Ixx", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Ixx.SetDescript( "Torsional Constant About FeaElement Neutral Axis (J)" );

    m_Dim1.Init( "Dim1", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim1.SetDescript( "First Dimension of the Cross Section" );

    m_Dim2.Init( "Dim2", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim2.SetDescript( "Second Dimension of the Cross Section" );

    m_Dim3.Init( "Dim3", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim3.SetDescript( "Third Dimension of the Cross Section" );

    m_Dim4.Init( "Dim4", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim4.SetDescript( "Fourth Dimension of the Cross Section" );

    m_Dim5.Init( "Dim5", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim5.SetDescript( "Fifth Dimension of the Cross Section" );

    m_Dim6.Init( "Dim6", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim6.SetDescript( "Sixth Dimension of the Cross Section" );

    m_LengthUnit.Init( "LengthUnit", "FeaProperty", this, vsp::LEN_IN, vsp::LEN_MM, vsp::NUM_LEN_UNIT - 1 );
    m_LengthUnit.SetDescript( "Length units used to specify property information" );

    m_Thickness_FEM.Init( "Thickness_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Thickness_FEM.SetDescript( "Thickness of FeaElement in FEM units" );

    m_CrossSecArea_FEM.Init( "CrossSecArea_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_CrossSecArea_FEM.SetDescript( "Cross-Sectional Area of FeaElement in FEM units" );

    m_Izz_FEM.Init( "Izz_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Izz_FEM.SetDescript( "Area Moment of Inertia for Bending in XY Plane of FeaElement Neutral Axis (I1) in FEM units" );

    m_Iyy_FEM.Init( "Iyy_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Iyy_FEM.SetDescript( "Area Moment of Inertia for Bending in XZ Plane of FeaElement Neutral Axis (I2) in FEM units" );

    m_Izy_FEM.Init( "Izy_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Izy_FEM.SetDescript( "Area Product of Inertia of FeaElement (I12) in FEM units" );

    m_Ixx_FEM.Init( "Ixx_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Ixx_FEM.SetDescript( "Torsional Constant About FeaElement Neutral Axis (J) in FEM units" );

    m_Dim1_FEM.Init( "Dim1_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim1_FEM.SetDescript( "First Dimension of the Cross Section in FEM units" );

    m_Dim2_FEM.Init( "Dim2_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim2_FEM.SetDescript( "Second Dimension of the Cross Section in FEM units" );

    m_Dim3_FEM.Init( "Dim3_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim3_FEM.SetDescript( "Third Dimension of the Cross Section in FEM units" );

    m_Dim4_FEM.Init( "Dim4_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim4_FEM.SetDescript( "Fourth Dimension of the Cross Section in FEM units" );

    m_Dim5_FEM.Init( "Dim5_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim5_FEM.SetDescript( "Fifth Dimension of the Cross Section in FEM units" );

    m_Dim6_FEM.Init( "Dim6_FEM", "FeaProperty", this, 0.1, 0.0, 1.0e12 );
    m_Dim6_FEM.SetDescript( "Sixth Dimension of the Cross Section in FEM units" );

    m_CrossSectType.Init( "CrossSectType", "FeaProperty", this, vsp::FEA_XSEC_GENERAL, vsp::FEA_XSEC_GENERAL, vsp::FEA_XSEC_BOX );
    m_CrossSectType.SetDescript( "Cross Section Type" );

    m_FeaMaterialIndex.Init( "FeaMaterialIndex", "FeaProperty", this, -1, -1, 1e12 );
    m_FeaMaterialIndex.SetDescript( "Deprecated FeaMaterial Index for FeaProperty" );

    m_FeaMaterialID = "_Al6061T6";
}

FeaProperty::~FeaProperty()
{

}

void FeaProperty::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    // Attempt to convert legacy Index based material association to new ID approach.
    if ( m_FeaMaterialIndex() != -1 )
    {
        // Legacy matid's
        int nlegacy = 15;
        const char *matids[] = {"_Al7075T6",
                                "_Al2024T3",
                                "_Ti6Al4V",
                                "_CrMo4130",
                                "_AS4-1",
                                "_AS4-2",
                                "_AS4-3",
                                "_AS4-4",
                                "_AS4-5",
                                "_AS4-6",
                                "_S2-1",
                                "_S2-2",
                                "_S2-3",
                                "_Balsa",
                                "_Spruce",
        };

        if ( m_FeaMaterialIndex() < nlegacy ) // Legacy built-in material.
        {
            m_FeaMaterialID = matids[ m_FeaMaterialIndex() ];
        }
        else // User-defined material.
        {
            vector < FeaMaterial* > mat_vec = StructureMgr.GetFeaMaterialVec();

            int ilastbuiltin = -1;
            for ( int i = 0; i < mat_vec.size(); i++ )
            {
                if ( mat_vec[i]->m_UserFeaMaterial == false )
                {
                    ilastbuiltin = i;
                }
            }

            int index = m_FeaMaterialIndex() - nlegacy + ilastbuiltin;

            if ( index >= 0 && index < mat_vec.size() )
            {
                m_FeaMaterialID = mat_vec[ index ]->GetID();
            }
            else
            {
                m_FeaMaterialID = "_Al7075T6";
            }
        }

        switch ( veh->m_StructUnit() )
        {
            case vsp::SI_UNIT:
                m_LengthUnit.Set( vsp::LEN_M );
                break;

            case vsp::CGS_UNIT:
                m_LengthUnit.Set( vsp::LEN_CM );
                break;

            case vsp::MPA_UNIT:
                m_LengthUnit.Set( vsp::LEN_MM );
                break;

            case vsp::BFT_UNIT:
                m_LengthUnit.Set( vsp::LEN_FT );
                break;

            case vsp::BIN_UNIT:
                m_LengthUnit.Set( vsp::LEN_IN );
                break;
        }

        // Mark index as -1 to deprecate its use.
        m_FeaMaterialIndex = -1;
    }

    FeaMaterial *fea_mat = StructureMgr.GetFeaMaterial( m_FeaMaterialID );
    if ( fea_mat )
    {
        if ( fea_mat->m_FeaMaterialType() == vsp::FEA_LAMINATE )
        {
            m_Thickness = fea_mat->m_Thickness();
            m_Thickness_FEM = fea_mat->m_Thickness_FEM();
            m_LengthUnit = fea_mat->m_LengthUnit();
        }
    }

    if ( m_LengthUnit() == vsp::LEN_UNITLESS )
    {
        m_Thickness_FEM = m_Thickness.Get();
        m_CrossSecArea_FEM = m_CrossSecArea.Get();
        m_Ixx_FEM = m_Ixx.Get();
        m_Iyy_FEM = m_Iyy.Get();
        m_Izy_FEM = m_Izy.Get();
        m_Izz_FEM = m_Izz.Get();
        m_Dim2_FEM = m_Dim2.Get();
        m_Dim1_FEM = m_Dim1.Get();
        m_Dim3_FEM = m_Dim3.Get();
        m_Dim4_FEM = m_Dim4.Get();
        m_Dim5_FEM = m_Dim5.Get();
        m_Dim6_FEM = m_Dim6.Get();
    }
    else
    {
        int length_unit = -1;

        switch (( int ) veh->m_StructUnit())
        {
            case vsp::SI_UNIT:
                length_unit = vsp::LEN_M;
                break;

            case vsp::CGS_UNIT:
                length_unit = vsp::LEN_CM;
                break;

            case vsp::MPA_UNIT:
                length_unit = vsp::LEN_MM;
                break;

            case vsp::BFT_UNIT:
                length_unit = vsp::LEN_FT;
                break;

            case vsp::BIN_UNIT:
                length_unit = vsp::LEN_IN;
                break;
        }

        m_Thickness_FEM = ConvertLength( m_Thickness.Get(), m_LengthUnit(), length_unit );
        m_CrossSecArea_FEM = ConvertLength2( m_CrossSecArea.Get(), m_LengthUnit(), length_unit );
        m_Ixx_FEM = ConvertLength4( m_Ixx.Get(), m_LengthUnit(), length_unit );
        m_Iyy_FEM = ConvertLength4( m_Iyy.Get(), m_LengthUnit(), length_unit );
        m_Izy_FEM = ConvertLength4( m_Izy.Get(), m_LengthUnit(), length_unit );
        m_Izz_FEM = ConvertLength4( m_Izz.Get(), m_LengthUnit(), length_unit );
        m_Dim1_FEM = ConvertLength( m_Dim1.Get(), m_LengthUnit(), length_unit );
        m_Dim2_FEM = ConvertLength( m_Dim2.Get(), m_LengthUnit(), length_unit );
        m_Dim3_FEM = ConvertLength( m_Dim3.Get(), m_LengthUnit(), length_unit );
        m_Dim4_FEM = ConvertLength( m_Dim4.Get(), m_LengthUnit(), length_unit );
        m_Dim5_FEM = ConvertLength( m_Dim5.Get(), m_LengthUnit(), length_unit );
        m_Dim6_FEM = ConvertLength( m_Dim6.Get(), m_LengthUnit(), length_unit );
    }
}

xmlNodePtr FeaProperty::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr prop_info = xmlNewChild( node, NULL, BAD_CAST "FeaPropertyInfo", NULL );

    ParmContainer::EncodeXml( prop_info );

    if ( prop_info )
    {
        XmlUtil::AddStringNode( prop_info, "FeaMaterialID", m_FeaMaterialID );
    }

    return prop_info;
}

xmlNodePtr FeaProperty::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    if ( node )
    {
        m_FeaMaterialID = ParmMgr.RemapID( XmlUtil::FindString( node, "FeaMaterialID", m_FeaMaterialID ) );
    }

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
//================== FeaLayer ====================//
////////////////////////////////////////////////////

FeaLayer::FeaLayer() : ParmContainer()
{
    m_Thickness.Init( "Thickness", "FeaLayer", this, 0.1, 0.0, 1.0e12 );
    m_Thickness.SetDescript( "Thickness of layer" );

    m_Theta.Init( "Theta", "FeaLayer", this, 0.0, -360.0, 360.0 );
    m_Theta.SetDescript( "Lamina orientation angle (degrees)" );

    m_Thickness_FEM.Init( "Thickness_FEM", "FeaLayer", this, 0.1, 0.0, 1.0e12 );
    m_Thickness_FEM.SetDescript( "Thickness of layer in FEM units" );

    m_FeaMaterialID = "_AS4-Uni";
}

FeaLayer::~FeaLayer()
{
}

void FeaLayer::Update( int cur_len_unit )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    int length_unit = -1;

    switch ( ( int ) veh->m_StructUnit() )
    {
        case vsp::SI_UNIT:
            length_unit = vsp::LEN_M;
            break;

        case vsp::CGS_UNIT:
            length_unit = vsp::LEN_CM;
            break;

        case vsp::MPA_UNIT:
            length_unit = vsp::LEN_MM;
            break;

        case vsp::BFT_UNIT:
            length_unit = vsp::LEN_FT;
            break;

        case vsp::BIN_UNIT:
            length_unit = vsp::LEN_IN;
            break;
    }

    m_Thickness_FEM = ConvertLength( m_Thickness.Get(), cur_len_unit, length_unit );
}

xmlNodePtr FeaLayer::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr prop_info = xmlNewChild( node, NULL, BAD_CAST "FeaLayerInfo", NULL );

    ParmContainer::EncodeXml( prop_info );

    if ( prop_info )
    {
        XmlUtil::AddStringNode( prop_info, "FeaLaminaID", m_FeaMaterialID );
    }

    return prop_info;
}

xmlNodePtr FeaLayer::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    if ( node )
    {
        m_FeaMaterialID = ParmMgr.RemapID( XmlUtil::FindString( node, "FeaLaminaID", m_FeaMaterialID ) );
    }

    return node;
}

void FeaLayer::GetTransMat( mat3 & T )
{
    long double ttheta = m_Theta() * PI / 180.0;
    long double c( std::cos( ttheta ) ), s(std::sin( ttheta ) );
    long double c2( c * c ), s2( s * s ), sc( c * s );

    T << c2,  s2,   2.*sc,
         s2,  c2,  -2.*sc,
        -sc,  sc, c2 - s2;
}

void FeaLayer::GetInvTransMat( mat3 & Tinv )
{
    long double ttheta = m_Theta() * PI / 180.0;
    long double c( std::cos( ttheta ) ), s(std::sin( ttheta ) );
    long double c2( c * c ), s2( s * s ), sc( c * s );

    Tinv << c2,  s2,  -2.*sc,
            s2,  c2,   2.*sc,
            sc, -sc, c2 - s2;
}

////////////////////////////////////////////////////
//================= FeaMaterial ==================//
////////////////////////////////////////////////////

FeaMaterial::FeaMaterial() : ParmContainer()
{
    m_FeaMaterialType.Init( "FeaMaterialType", "FeaMaterial", this, vsp::FEA_ISOTROPIC, vsp::FEA_ISOTROPIC, vsp::FEA_NUM_MAT_TYPES - 1 );
    m_FeaMaterialType.SetDescript( "Fea Material Type" );

    m_MassDensity.Init( "MassDensity", "FeaMaterial", this, 0.098, 0.0, 1.0e12 );
    m_MassDensity.SetDescript( "Mass Density of Material" );

    m_ElasticModulus.Init( "ElasticModulus", "FeaMaterial", this, 9900000, 0.0, 1.0e12 );
    m_ElasticModulus.SetDescript( "Elastic (Young's) Modulus for Material" );

    m_PoissonRatio.Init( "PoissonRatio", "FeaMaterial", this, 0.33, -1.0, 0.5 );
    m_PoissonRatio.SetDescript( "Poisson's Ratio for Material" );

    m_ThermalExpanCoeff.Init( "ThermalExpanCoeff", "FeaMaterial", this, 0.000013, -1.0e12, 1.0e12 );
    m_ThermalExpanCoeff.SetDescript( "Thermal Expansion Coefficient for Material" );

    m_Thickness.Init( "Thickness", "FeaMaterial", this, 0.1, 0.0, 1.0e12 );
    m_Thickness.SetDescript( "Thickness of laminate" );

    m_LengthUnit.Init( "LengthUnit", "FeaMaterial", this, vsp::LEN_IN, vsp::LEN_MM, vsp::NUM_LEN_UNIT - 1 );
    m_LengthUnit.SetDescript( "Length units used to specify property information" );

    m_DensityUnit.Init( "DensityUnit", "FeaMaterial", this, vsp::RHO_UNIT_LBM_IN3, vsp::RHO_UNIT_SLUG_FT3, vsp::NUM_RHO_UNIT - 1 );
    m_DensityUnit.SetDescript( "Density units used to specify material properties." );

    m_ModulusUnit.Init( "ModulusUnit", "FeaMaterial", this, vsp::PRES_UNIT_PSI, vsp::PRES_UNIT_PSF, vsp::NUM_PRES_UNIT - 1 );
    m_ModulusUnit.SetDescript( "Modulus units used to specify material properties." );

    m_TemperatureUnit.Init( "TemperatureUnit", "FeaMaterial", this, vsp::TEMP_UNIT_F, vsp::TEMP_UNIT_K, vsp::NUM_TEMP_UNIT - 1 );
    m_TemperatureUnit.SetDescript( "Temperature units used to specify material properties." );

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

    m_A1.Init( "A1", "FeaMaterial", this, 0.0, -1.0e12, 1.0e12 );
    m_A1.SetDescript( "A1 Thermal Expansion Coefficient for Material" );

    m_A2.Init( "A2", "FeaMaterial", this, 0.0, -1.0e12, 1.0e12 );
    m_A2.SetDescript( "A2 Thermal Expansion Coefficient for Material" );

    m_A3.Init( "A3", "FeaMaterial", this, 0.0, -1.0e12, 1.0e12 );
    m_A3.SetDescript( "A3 Thermal Expansion Coefficient for Material" );

    // Properties in FEA units -- auto-computed.

    m_MassDensity_FEM.Init( "MassDensity_FEM", "FeaMaterial", this, 1.0, 0.0, 1.0e12 );
    m_MassDensity_FEM.SetDescript( "Mass Density of Material in FEM units" );

    m_ElasticModulus_FEM.Init( "ElasticModulus_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_ElasticModulus_FEM.SetDescript( "Elastic (Young's) Modulus for Material in FEM units" );

    m_ThermalExpanCoeff_FEM.Init( "ThermalExpanCoeff_FEM", "FeaMaterial", this, 0.0, -1.0, 1.0 );
    m_ThermalExpanCoeff_FEM.SetDescript( "Thermal Expansion Coefficient for Material in FEM units" );

    m_Thickness_FEM.Init( "Thickness_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_Thickness_FEM.SetDescript( "Thickness of laminate in FEM units" );

    m_E1_FEM.Init( "E1_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_E1_FEM.SetDescript( "E1 Elastic (Young's) Modulus for Material in FEM units" );

    m_E2_FEM.Init( "E2_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_E2_FEM.SetDescript( "E2 Elastic (Young's) Modulus for Material in FEM units" );

    m_E3_FEM.Init( "E3_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_E3_FEM.SetDescript( "E3 Elastic (Young's) Modulus for Material in FEM units" );

    m_G12_FEM.Init( "G12_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_G12_FEM.SetDescript( "G12 Shear Modulus for Material in FEM units" );

    m_G13_FEM.Init( "G13_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_G13_FEM.SetDescript( "G13 Shear Modulus for Material in FEM units" );

    m_G23_FEM.Init( "G23_FEM", "FeaMaterial", this, 0.0, 0.0, 1.0e12 );
    m_G23_FEM.SetDescript( "G23 Shear Modulus for Material in FEM units" );

    m_A1_FEM.Init( "A1_FEM", "FeaMaterial", this, 0.0, -1.0e12, 1.0e12 );
    m_A1_FEM.SetDescript( "A1 Thermal Expansion Coefficient for Material in FEM units" );

    m_A2_FEM.Init( "A2_FEM", "FeaMaterial", this, 0.0, -1.0e12, 1.0e12 );
    m_A2_FEM.SetDescript( "A2 Thermal Expansion Coefficient for Material in FEM units" );

    m_A3_FEM.Init( "A3_FEM", "FeaMaterial", this, 0.0, -1.0e12, 1.0e12 );
    m_A3_FEM.SetDescript( "A3 Thermal Expansion Coefficient for Material in FEM units" );

    m_CurrentLayerIndex = 0;
    m_FeaLayerCount = 0;
}

FeaMaterial::~FeaMaterial()
{

}

void FeaMaterial::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    if ( m_FeaMaterialType() == vsp::FEA_ISOTROPIC )
    {
        m_E1 = m_ElasticModulus();
        m_E2 = m_ElasticModulus();
        m_E3 = m_ElasticModulus();

        m_nu12 = m_PoissonRatio();
        m_nu13 = m_PoissonRatio();
        m_nu23 = m_PoissonRatio();

        m_G12 = GetShearModulus();
        m_G13 = m_G12();
        m_G23 = m_G12();

        m_A1 = m_ThermalExpanCoeff();
        m_A2 = m_ThermalExpanCoeff();
        m_A3 = m_ThermalExpanCoeff();
    }
    else if ( m_FeaMaterialType() == vsp::FEA_LAMINATE )
    {
        for ( int i = 0; i < m_LayerVec.size(); i++ )
        {
            m_LayerVec[i]->Update( m_LengthUnit() );
        }

        LaminateTheory();
    }
    else
    {
        m_ElasticModulus = m_E1();
        m_PoissonRatio = m_nu12();
        m_ThermalExpanCoeff = m_A1();
    }

    if ( m_FeaMaterialType() == vsp::FEA_ENG_ORTHO_TRANS_ISO )
    {
        // Apply assumption of transverse isotropy according to MIL-HDBK-17-3F
        // Eq. 10.2.4.1(b)

        m_E3 = m_E2();
        m_G13 = m_G12();
        m_nu13 = m_nu12();
        m_G23 = m_E2() / ( 2.0 * (1.0 + m_nu23() ) );

        // Transverse isotropy applied to CTE.  Not explicitly stated in MIL-HDBK-17-3F, but
        // it carries from the same arguments.
        m_A3 = m_A2();
    }

    if ( m_FeaMaterialType() != vsp::FEA_LAMINATE )
    {
        int density_unit = -1;
        int pressure_unit = -1;
        int temp_unit = -1;

        switch ( ( int ) veh->m_StructUnit() )
        {
            case vsp::SI_UNIT:
                density_unit = vsp::RHO_UNIT_KG_M3;
                pressure_unit = vsp::PRES_UNIT_PA;
                temp_unit = vsp::TEMP_UNIT_K;
                break;

            case vsp::CGS_UNIT:
                density_unit = vsp::RHO_UNIT_G_CM3;
                pressure_unit = vsp::PRES_UNIT_BA;
                temp_unit = vsp::TEMP_UNIT_K;
                break;

            case vsp::MPA_UNIT:
                density_unit = vsp::RHO_UNIT_TONNE_MM3;
                pressure_unit = vsp::PRES_UNIT_MPA;
                temp_unit = vsp::TEMP_UNIT_K;
                break;

            case vsp::BFT_UNIT:
                density_unit = vsp::RHO_UNIT_SLUG_FT3;
                pressure_unit = vsp::PRES_UNIT_PSF;
                temp_unit = vsp::TEMP_UNIT_R;
                break;

            case vsp::BIN_UNIT:
                density_unit = vsp::RHO_UNIT_LBFSEC2_IN4;
                pressure_unit = vsp::PRES_UNIT_PSI;
                temp_unit = vsp::TEMP_UNIT_R;
                break;
        }

        m_MassDensity_FEM = ConvertDensity( m_MassDensity(), m_DensityUnit(), density_unit );
        m_ElasticModulus_FEM = ConvertPressure( m_ElasticModulus(), m_ModulusUnit(), pressure_unit );
        m_ThermalExpanCoeff_FEM = ConvertThermalExpanCoeff( m_ThermalExpanCoeff(), m_TemperatureUnit(), temp_unit );

        m_E1_FEM = ConvertPressure( m_E1(), m_ModulusUnit(), pressure_unit );
        m_E2_FEM = ConvertPressure( m_E2(), m_ModulusUnit(), pressure_unit );
        m_E3_FEM = ConvertPressure( m_E3(), m_ModulusUnit(), pressure_unit );

        m_G12_FEM = ConvertPressure( m_G12(), m_ModulusUnit(), pressure_unit );
        m_G13_FEM = ConvertPressure( m_G13(), m_ModulusUnit(), pressure_unit );
        m_G23_FEM = ConvertPressure( m_G23(), m_ModulusUnit(), pressure_unit );

        m_A1_FEM = ConvertThermalExpanCoeff( m_A1(), m_TemperatureUnit(), temp_unit );
        m_A2_FEM = ConvertThermalExpanCoeff( m_A2(), m_TemperatureUnit(), temp_unit );
        m_A3_FEM = ConvertThermalExpanCoeff( m_A3(), m_TemperatureUnit(), temp_unit );
    }
}

void FeaMaterial::MakeMaterial( string id )
{
    if ( id == "_Al7075T6" )
    {
        m_Name = "Aluminum 7075-T6";
        m_Description = "Valid for T-6 sheet / -T651 plate per MIL-HDBK-5J/MMPDS";
        m_FeaMaterialType.Set( vsp::FEA_ISOTROPIC );

        m_PoissonRatio.Set( 0.33 );

        m_MassDensity.Set( 0.101 ); // lbm/in^3
        m_DensityUnit.Set( vsp::RHO_UNIT_LBM_IN3 );

        m_ElasticModulus.Set( 10300000 ); // psi
        m_ModulusUnit.Set( vsp::PRES_UNIT_PSI );

        m_ThermalExpanCoeff.Set( 0.0000128 ); // in/(in-�F)
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_F );
    }
    else if ( id == "_Al6061T6" )
    {
        m_Name = "Aluminum 6061-T6";
        m_Description = "Per MIL-HDBK-5J/MMPDS";
        m_FeaMaterialType.Set( vsp::FEA_ISOTROPIC );

        m_PoissonRatio.Set( 0.33 );

        m_MassDensity.Set( 0.098 ); // lbm/in^3
        m_DensityUnit.Set( vsp::RHO_UNIT_LBM_IN3 );

        m_ElasticModulus.Set( 9900000 ); // psi
        m_ModulusUnit.Set( vsp::PRES_UNIT_PSI );

        m_ThermalExpanCoeff.Set( 0.000013 ); // in/(in-�F)
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_F );
    }
    if ( id == "_Al2024T3" )
    {
        m_Name = "Aluminum 2024-T3";
        m_Description = "Per MIL-HDBK-5J/MMPDS";
        m_FeaMaterialType.Set( vsp::FEA_ISOTROPIC );

        m_PoissonRatio.Set( 0.33 );

        m_MassDensity.Set( 0.1 ); // lbm/in^3
        m_DensityUnit.Set( vsp::RHO_UNIT_LBM_IN3 );

        m_ElasticModulus.Set( 10500000 ); // psi
        m_ModulusUnit.Set( vsp::PRES_UNIT_PSI );

        m_ThermalExpanCoeff.Set( 0.000013 ); // in/(in-�F)
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_F );
    }
    if ( id == "_Ti6Al4V" )
    {
        m_Name = "Titanium Ti-6Al-4V";
        m_Description = "Valid for annealed (AMS 4911) and solution treated and aged condition (T-9046) per MIL-HDBK-5J/MMPDS";
        m_FeaMaterialType.Set( vsp::FEA_ISOTROPIC );

        m_PoissonRatio.Set( 0.31 );

        m_MassDensity.Set( 0.16 ); // lbm/in^3
        m_DensityUnit.Set( vsp::RHO_UNIT_LBM_IN3 );

        m_ElasticModulus.Set( 16000000 ); // psi
        m_ModulusUnit.Set( vsp::PRES_UNIT_PSI );

        m_ThermalExpanCoeff.Set( 0.00000895 ); // in/(in-�F)
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_F );
    }
    if ( id == "_CrMo4130" )
    {
        m_Name = "Steel AISI 4130";
        m_Description = "Valid for tube (AMS 6371) and sheet (6345) per MIL-HDBK-5J/MMPDS";
        m_FeaMaterialType.Set( vsp::FEA_ISOTROPIC );

        m_PoissonRatio.Set( 0.33 );

        m_MassDensity.Set( 0.283 ); // lbm/in^3
        m_DensityUnit.Set( vsp::RHO_UNIT_LBM_IN3 );

        m_ElasticModulus.Set( 29000000 ); // psi
        m_ModulusUnit.Set( vsp::PRES_UNIT_PSI );

        m_ThermalExpanCoeff.Set( 0.000007 ); // in/(in-�F)
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_F );
    }
    // AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 627
    // AS4 fiber density 1.79 g/cm^3
    // 3501-6 resin density 1.265 g/cm^3
    // 59.5% fiber volume fraction
    else if ( id == "_AS4-Uni" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 627.  59.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO_TRANS_ISO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        double v = 0.595;
        double rhof= 1.79;
        double rhor = 1.265;
        double rho = ( ( 1.0 - v ) * rhor ) + ( v * rhof );

        m_MassDensity.Set( rho );
        m_E1.Set( 113.6e9 );
        m_E2.Set( 9.65e9 );
        // m_E3.Set( 9.65e9 );
        m_nu12.Set( 0.334 );
        // m_nu13.Set( 0.328 );
        m_nu23.Set( 0.540 );
        m_G12.Set( 6.0e9 );
        // m_G13.Set( 6.0e9 );
        // m_G23.Set( 3.1e9 );
        m_A1.Set( 0.018e-6 ); // -0.9e-6   0.72e-6
        m_A2.Set( 28.8e-6 );  // 27.0e-6   29.5e-6
        // m_A3.Set( 28.8e-6 );
    }
    // AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629
    // AS4 fiber density 1.79 g/cm^3
    // 3501-6 resin density 1.265 g/cm^3
    // 60.5% fiber volume fraction
    // laminateDensity = ( (1-V) * resinDensity) + (V * fiberDensity)
    // laminateDensity = 1.582625
    else if ( id == "_AS4-1" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6 [0_2/90]s";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.582625 );
        m_E1.Set( 79.3e9 );
        m_E2.Set( 44.6e9 );
        m_E3.Set( 11.4e9 );
        m_nu12.Set( 0.072 );
        m_nu13.Set( 0.402 );
        m_nu23.Set( 0.465 );
        m_G12.Set( 6e9 );
        m_G13.Set( 5.38e9 );
        m_G23.Set( 3.47e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_AS4-2" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6 [0/90]_2s";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.582625 );
        m_E1.Set( 62.1e9 );
        m_E2.Set( 62.1e9 );
        m_E3.Set( 11e9 );
        m_nu12.Set( 0.052 );
        m_nu13.Set( 0.438 );
        m_nu23.Set( 0.427 );
        m_G12.Set( 6e9 );
        m_G13.Set( 4.22e9 );
        m_G23.Set( 3.95e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_AS4-3" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6 [0/90/+-45]s";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.582625 );
        m_E1.Set( 46e9 );
        m_E2.Set( 46.1e9 );
        m_E3.Set( 11.1e9 );
        m_nu12.Set( 0.296 );
        m_nu13.Set( 0.318 );
        m_nu23.Set( 0.317 );
        m_G12.Set( 17.7e9 );
        m_G13.Set( 4.32e9 );
        m_G23.Set( 3.58e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_AS4-4" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6 [+-30]_2s";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.582625 );
        m_E1.Set( 47.2e9 );
        m_E2.Set( 12.2e9 );
        m_E3.Set( 10.3e9 );
        m_nu12.Set( 1.13 );
        m_nu13.Set( -0.197 );
        m_nu23.Set( 0.434 );
        m_G12.Set( 23.6e9 );
        m_G13.Set( 4.88e9 );
        m_G23.Set( 3.55e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_AS4-5" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6 [+-45]_2s";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.582625 );
        m_E1.Set( 20.3e9 );
        m_E2.Set( 20.3e9 );
        m_E3.Set( 11.8e9 );
        m_nu12.Set( 0.689 );
        m_nu13.Set( 0.211 );
        m_nu23.Set( 0.211 );
        m_G12.Set( 29.4e9 );
        m_G13.Set( 4.11e9 );
        m_G23.Set( 4.11e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_AS4-6" )
    {
        m_Name = "Carbon Epoxy AS4 3501-6 [+-60]_2s";
        m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.582625 );
        m_E1.Set( 12.2e9 );
        m_E2.Set( 47.2e9 );
        m_E3.Set( 12e9 );
        m_nu12.Set( 0.294 );
        m_nu13.Set( 0.434 );
        m_nu23.Set( -0.197 );
        m_G12.Set( 23.6e9 );
        m_G13.Set( 3.55e9 );
        m_G23.Set( 4.88e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    // S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 627
    // S2 fiber density 2.49 g/cm^3
    // 3501-6 resin density 1.265 g/cm^3
    // 59.5% fiber volume fraction
    else if ( id == "_S2-Uni" )
    {
        m_Name = "Glass Epoxy S2 3501-6";
        m_Description = "S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 627.  56.5% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO_TRANS_ISO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        double v = 0.565;
        double rhof = 2.49;
        double rhor = 1.265;
        double rho = ( ( 1.0 - v ) * rhor ) + ( v * rhof );

        m_MassDensity.Set( rho );
        m_E1.Set( 49.3e9 );
        m_E2.Set( 14.7e9 );
        // m_E3.Set( 14.7e9 );
        m_nu12.Set( 0.296 );
        // m_nu13.Set( 0.306 );
        m_nu23.Set( 0.499 );
        m_G12.Set( 6.8e9 );
        // m_G13.Set( 6.8e9 );
        // m_G23.Set( 4.9e9 );
        m_A1.Set( 0.0e-6 );  // 3.78e-6
        m_A2.Set( 0.0e-6 );  // 16.7e-6
        // m_A3.Set( 0.0e-6 );
    }
    // S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 629
    // S2 fiber density 2.49 g/cm^3
    // 3501-6 resin density 1.265 g/cm^3
    // 50% fiber volume fraction
    // laminateDensity = ( (1-V) * resinDensity) + (V * fiberDensity)
    // laminateDensity = 1.8775
    else if ( id == "_S2-1" )
    {
        m_Name = "Glass Epoxy S2 3501-6 [0_2/90]s";
        m_Description = "S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  50% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.8775 );
        m_E1.Set( 38.1e9 );
        m_E2.Set( 26.4e9 );
        m_E3.Set( 15.9e9 );
        m_nu12.Set( 0.165 );
        m_nu13.Set( 0.359 );
        m_nu23.Set( 0.427 );
        m_G12.Set( 6.76e9 );
        m_G13.Set( 6.33e9 );
        m_G23.Set( 5.2e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_S2-2" )
    {
        m_Name = "Glass Epoxy S2 3501-6 [0/90]_2s";
        m_Description = "S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  50% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.8775 );
        m_E1.Set( 32.3e9 );
        m_E2.Set( 32.3e9 );
        m_E3.Set( 15.8e9 );
        m_nu12.Set( 0.135 );
        m_nu13.Set( 0.393 );
        m_nu23.Set( 0.392 );
        m_G12.Set( 6.76e9 );
        m_G13.Set( 5.72e9 );
        m_G23.Set( 5.59e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_S2-3" )
    {
        m_Name = "Glass Epoxy S2 3501-6 [0/90/+-45]s";
        m_Description = "S2 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  50% fiber volume fraction";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.8775 );
        m_E1.Set( 26.8e9 );
        m_E2.Set( 26.8e9 );
        m_E3.Set( 16e9 );
        m_nu12.Set( 0.28 );
        m_nu13.Set( 0.329 );
        m_nu23.Set( 0.329 );
        m_G12.Set( 10.5e9 );
        m_G13.Set( 5.78e9 );
        m_G23.Set( 5.04e9 );
        m_A1.Set( 0.0 );
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
    }
    else if ( id == "_Balsa" )
    {
        m_Name = "Balsa LTR";
        m_Description = "From FPL-GTR-190";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
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

        m_MassDensity.Set( 160 ); // kg/m^3
        m_DensityUnit.Set( vsp::RHO_UNIT_KG_M3 );

        m_E1.Set( EL ); // Pa
        m_E2.Set( ET );
        m_E3.Set( ER );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );

        m_nu12.Set( nuLT );
        m_nu13.Set( nuLR );
        m_nu23.Set( nuTR );

        m_G12.Set( GLT );
        m_G13.Set( GLR );
        m_G23.Set( GRT );

        m_A1.Set( 0.0 ); // 1/K
        m_A2.Set( 0.0 );
        m_A3.Set( 0.0 );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );
    }
    else if ( id == "_Spruce" )
    {
        m_Name = "Sitka Spruce LTR";
        m_Description = "From NASA TM-104059.  Sitka Spruce 8% moisture content.  Density and CTE from MatWeb.";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
        // Elastic properties from NASA TM-104059
        // Structural Integrity of Wind Tunnel Wooden Fan Blades
        // Sitka Spruce 8% Moisture content
        // Density and CTE from MatWeb
        m_MassDensity.Set( 360); // kg/m^3
        m_DensityUnit.Set( vsp::RHO_UNIT_KG_M3 );

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

        m_E1.Set( EL ); // psi
        m_E2.Set( ET );
        m_E3.Set( ER );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PSI );

        m_nu12.Set( nuLT );
        m_nu13.Set( nuLR );
        m_nu23.Set( nuTR );

        m_G12.Set( GLT );
        m_G13.Set( GLR );
        m_G23.Set( GRT );

        m_A1.Set( 5.4e-6 ); // 1/K
        m_A2.Set( 6.3e-6 );
        m_A3.Set( 34.1e-6 );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );
    }
    else if ( id == "_hmce" )
    {
        m_Name = "HM Carbon Epoxy";
        m_Description = "Generic high modulus carbon";
        m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );

        m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
        m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
        m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );

        m_MassDensity.Set( 1.63 );
        m_E1.Set( 230.0e9 );
        m_E2.Set( 6.6e9 );
        m_nu12.Set( 0.25 );
        m_G12.Set( 4.8e9 );
        m_A1.Set( -0.7e-6 );
        m_A2.Set( 28.0e-6 );
    }
    m_UserFeaMaterial = false;

    // Change built-in material property ID's to hard-coded values based on base material ID.
    // This way, they will always be the same for all models and all files.

    ChangeID( id );

    m_FeaMaterialType.ChangeID( id + "t" );

    m_MassDensity.ChangeID( id + "rho" );
    m_ElasticModulus.ChangeID( id + "E" );
    m_PoissonRatio.ChangeID( id + "nu" );
    m_ThermalExpanCoeff.ChangeID( id + "a" );

    m_DensityUnit.ChangeID( id + "rhou" );
    m_ModulusUnit.ChangeID( id + "Eu" );
    m_TemperatureUnit.ChangeID( id + "au" );

    m_E1.ChangeID( id + "E1" );
    m_E2.ChangeID( id + "E2" );
    m_E3.ChangeID( id + "E3" );
    m_nu12.ChangeID( id + "nu12" );
    m_nu13.ChangeID( id + "nu13" );
    m_nu23.ChangeID( id + "nu23" );
    m_G12.ChangeID( id + "G12" );
    m_G13.ChangeID( id + "G13" );
    m_G23.ChangeID( id + "G23" );
    m_A1.ChangeID( id + "A1" );
    m_A2.ChangeID( id + "A2" );
    m_A3.ChangeID( id + "A3" );

    m_MassDensity_FEM.ChangeID( id + "rho_FEM" );
    m_ElasticModulus_FEM.ChangeID( id + "E_FEM" );
    m_ThermalExpanCoeff_FEM.ChangeID( id + "a_FEM" );

    m_E1_FEM.ChangeID( id + "E1_FEM" );
    m_E2_FEM.ChangeID( id + "E2_FEM" );
    m_E3_FEM.ChangeID( id + "E3_FEM" );
    m_G12_FEM.ChangeID( id + "G12_FEM" );
    m_G13_FEM.ChangeID( id + "G13_FEM" );
    m_G23_FEM.ChangeID( id + "G23_FEM" );
    m_A1_FEM.ChangeID( id + "A1_FEM" );
    m_A2_FEM.ChangeID( id + "A2_FEM" );
    m_A3_FEM.ChangeID( id + "A3_FEM" );



// This group of materials seemed mis-labeled in the handbook.
// It duplicates another entry above -- but the values are different.
// It is possible that these values apply to S2/3501-6 for the indicated laminate schedules.

//        else if ( id == "_AS4-7" )
//        {
//            m_Name = "Carbon Epoxy AS4 3501-6 [+-30]_2s";
//            m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
//            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
//
//            m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
//            m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
//            m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );
//
//            m_MassDensity.Set( 1.582625 );
//            m_E1.Set( 30.7e9 );
//            m_E2.Set( 15.6e9 );
//            m_E3.Set( 14.9e9 );
//            m_nu12.Set( 0.545 );
//            m_nu13.Set( 0.136 );
//            m_nu23.Set( 0.406 );
//            m_G12.Set( 12.3e9 );
//            m_G13.Set( 6.17e9 );
//            m_G23.Set( 5.26e9 );
//            m_A1.Set( 0.0 );
//            m_A2.Set( 0.0 );
//            m_A3.Set( 0.0 );
//        }
//        else if ( id == "_AS4-8" )
//        {
//            m_Name = "Carbon Epoxy AS4 3501-6 [+-45]_2s";
//            m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
//            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
//
//            m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
//            m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
//            m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );
//
//            m_MassDensity.Set( 1.582625 );
//            m_E1.Set( 19.9e9 );
//            m_E2.Set( 19.9e9 );
//            m_E3.Set( 16.1e9 );
//            m_nu12.Set( 0.467 );
//            m_nu13.Set( 0.284 );
//            m_nu23.Set( 0.284 );
//            m_G12.Set( 14.2e9 );
//            m_G13.Set( 5.67e9 );
//            m_G23.Set( 5.67e9 );
//            m_A1.Set( 0.0 );
//            m_A2.Set( 0.0 );
//            m_A3.Set( 0.0 );
//        }
//        else if ( id == "_AS4-9" )
//        {
//            m_Name = "Carbon Epoxy AS4 3501-6 [+-60]_2s";
//            m_Description = "AS4 3501-6 elasticity data from MIL-HDBK-17-3F p. 629.  60.5% fiber volume fraction";
//            m_FeaMaterialType.Set( vsp::FEA_ENG_ORTHO );
//
//            m_DensityUnit.Set( vsp::RHO_UNIT_G_CM3 );
//            m_ModulusUnit.Set( vsp::PRES_UNIT_PA );
//            m_TemperatureUnit.Set( vsp::TEMP_UNIT_K );
//
//            m_MassDensity.Set( 1.582625 );
//            m_E1.Set( 15.6e9 );
//            m_E2.Set( 30.7e9 );
//            m_E3.Set( 16.8e9 );
//            m_nu12.Set( 0.277 );
//            m_nu13.Set( 0.406 );
//            m_nu23.Set( 0.136 );
//            m_G12.Set( 12.3e9 );
//            m_G13.Set( 5.26e9 );
//            m_G23.Set( 6.17e9 );
//            m_A1.Set( 0.0 );
//            m_A2.Set( 0.0 );
//            m_A3.Set( 0.0 );
//        }
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

    if ( mat_info )
    {
        XmlUtil::AddStringNode( mat_info, "Description", m_Description );

        for ( unsigned int i = 0; i < m_LayerVec.size(); i++ )
        {
            m_LayerVec[i]->EncodeXml( mat_info );
        }
    }

    return mat_info;
}

xmlNodePtr FeaMaterial::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    if ( node )
    {
        m_Description = ParmMgr.RemapID( XmlUtil::FindString( node, "Description", m_Description ) );

        int numlayers = XmlUtil::GetNumNames( node, "FeaLayerInfo" );

        for ( unsigned int i = 0; i < numlayers; i++ )
        {
            xmlNodePtr layer_info = XmlUtil::GetNode( node, "FeaLayerInfo", i );

            if ( layer_info )
            {
                FeaLayer* fealayer = AddLayer();
                fealayer->DecodeXml( layer_info );
            }
        }
    }

    return node;
}

double FeaMaterial::GetShearModulus()
{
    return ( m_ElasticModulus() / ( 2.0 * ( m_PoissonRatio() + 1.0 ) ) );
}

double FeaMaterial::GetShearModulus_FEM()
{
    return ( m_ElasticModulus_FEM() / ( 2.0 * ( m_PoissonRatio() + 1.0 ) ) );
}

FeaLayer* FeaMaterial::AddLayer()
{
    FeaLayer* fealayer = new FeaLayer();

    if ( fealayer )
    {
        fealayer->SetName( string( "Layer" + std::to_string( m_FeaLayerCount ) ), false ); // false is for removeslashes
        m_LayerVec.push_back( fealayer );
        m_FeaLayerCount++;
    }

    return fealayer;
}

FeaLayer * FeaMaterial::GetCurrLayer()
{
    if ( ValidLayerInd( GetCurrLayerIndex() ) )
    {
        return m_LayerVec[ GetCurrLayerIndex() ];
    }

    return NULL;
}

bool FeaMaterial::DeleteLayer( const string &id )
{
    bool delsuccess = false;
    vector < FeaLayer* > newlayervec;

    for ( size_t i = 0; i < m_LayerVec.size(); i++ )
    {
        if ( m_LayerVec[i]->GetID() == id )
        {
            delete m_LayerVec[i];
            delsuccess = true;
        }
        else
        {
            newlayervec.push_back( m_LayerVec[i] );
        }
    }
    m_LayerVec = newlayervec;
    return delsuccess;
}

bool FeaMaterial::ValidLayerInd( int index )
{
    if ( (int)m_LayerVec.size() > 0 && index >= 0 && index < (int)m_LayerVec.size() )
    {
        return true;
    }
    return false;
}

void FeaMaterial::ReorderCurrentLayer( int action )
{
    int index = GetCurrLayerIndex();
    index = ReorderVectorIndex( m_LayerVec, index, action );

    SetCurrLayerIndex( index );
}

FeaLayer* FeaMaterial::GetFeaLayer( string id )
{
    if ( id == string( "NONE" ) )
    {
        return NULL;
    }
    for ( int i = 0 ; i < ( int )m_LayerVec.size() ; i++ )
    {
        if ( m_LayerVec[i]->GetID() == id )
        {
            return m_LayerVec[i];
        }
    }
    return NULL;
}

int FeaMaterial::GetCurrLayerIndex()
{
    return m_CurrentLayerIndex;
}

void FeaMaterial::SetCurrLayerIndex( int index )
{
    m_CurrentLayerIndex = index;
}

void FeaMaterial::LaminateTheory()
{
    // printf( "\nCLT Calculations for %s\n", m_Name.c_str() );

    m_Thickness = 0;
    m_MassDensity = 0;
    m_E1 = 0;
    m_E2 = 0;
    m_E3 = 0;
    m_nu12 = 0;
    m_nu13 = 0;
    m_nu23 = 0;
    m_G12 = 0;
    m_G13 = 0;
    m_G23 = 0;
    m_A1 = 0;
    m_A2 = 0;
    m_A3 = 0;

    m_Thickness_FEM = 0;
    m_MassDensity_FEM = 0;
    m_E1_FEM = 0;
    m_E2_FEM = 0;
    m_E3_FEM = 0;
    m_G12_FEM = 0;
    m_G13_FEM = 0;
    m_G23_FEM = 0;
    m_A1_FEM = 0;
    m_A2_FEM = 0;
    m_A3_FEM = 0;


    mat3 R, Rinv;
    R << 1, 0, 0,
         0, 1, 0,
         0, 0, 2;

    Rinv << 1, 0, 0,
            0, 1, 0,
            0, 0, 0.5;

    int nlayer = m_LayerVec.size();

    // Build laminate coordinate system.
    vector < long double > z( nlayer + 1, 0.0 );
    for ( int ilay = 0; ilay < nlayer; ilay++ )
    {
        FeaLayer* lay = m_LayerVec[ ilay ];
        if ( lay )
        {
            z[ilay + 1] = z[ilay] + lay->m_Thickness_FEM();
            m_Thickness = m_Thickness() + lay->m_Thickness();
        }
    }

    long double thickness = z[ nlayer ];

    // Shift to laminate geometric mid-plane.
    long double z0 = 0.5 * thickness;
    for ( int ilay = 0; ilay <= nlayer; ilay++ )
    {
        z[ilay] = z[ilay] - z0;
    }

    mat3 A = mat3::Zero();
    mat3 B = mat3::Zero();
    mat3 D = mat3::Zero();
    vec3 E = vec3::Zero();

    double m = 0.0;
    bool findlayer = true;
    for ( int ilay = 0; ilay < nlayer; ilay++ )
    {
        FeaLayer *lay = m_LayerVec[ ilay ];

        if ( lay )
        {
            FeaMaterial *mat = StructureMgr.GetFeaMaterial( lay->m_FeaMaterialID );
            if ( mat )
            {
                // Sum mass per square of laminate.
                m += lay->m_Thickness_FEM() * mat->m_MassDensity_FEM();

                // Get lamina transformation matrices.
                mat3 T, Tinv;
                lay->GetTransMat( T );
                lay->GetInvTransMat( Tinv );

                // Get lamina compliance matrix
                mat3 S;
                mat->GetCompliance( S );

                // Get lamina CTE vector
                vec3 alpha;
                mat->GetCTEVec( alpha );

                mat3 Sbar;
                Sbar = R * Tinv * Rinv * S * T;

                vec3 alphabar;
                alphabar = T * alpha;

                // Lamina stiffness
                mat3 Qbar;
                Qbar = Sbar.inverse();

                // Assemble laminate stiffness contributions
                long double z1, z2, z3;
                long double zip1, zi;
                zi = z[ ilay ];
                zip1 = z[ ilay + 1 ];
                z1 = zip1 - zi;
                z2 = ( zip1 * zip1 - zi * zi ) * 0.5;
                z3 = ( zip1 * zip1 * zip1 - zi * zi * zi ) / 3.0;

                A += z1 * Qbar;
                B += z2 * Qbar;
                D += z3 * Qbar;
                E += z1 * Qbar * alphabar;
            }
            else
            {
                findlayer = false;
            }
        }
        else
        {
            findlayer = false;
        }
    }

    if ( findlayer )
    {
        // Assemble laminate stiffness matrix.
        mat6 ABD = mat6::Zero();
        ABD.topLeftCorner( 3, 3 ) = A;
        ABD.topRightCorner( 3, 3 ) = B;
        ABD.bottomLeftCorner( 3, 3 ) = B;
        ABD.bottomRightCorner( 3, 3 ) = D;

        // printf( "ABD Matrix\n" );
        // std::cout << ABD << "\n\n\n";

        mat6 Slam;
        Slam = ABD.inverse();

        // printf( "ABD Inverse\n" );
        // std::cout << Slam << "\n\n\n";

        vec3 alpha = A.inverse() * E;

        // Extract laminate properties from laminate compliance matrix.
        long double E1, E2, G12, nu12, nu21, a1, a2, a12;
        E1 = ( 1.0 / Slam( 0, 0 )) / thickness;
        E2 = ( 1.0 / Slam( 1, 1 )) / thickness;
        G12 = ( 1.0 / Slam( 2, 2 )) / thickness;
        nu12 = -Slam( 1, 0 ) / Slam( 0, 0 );
        nu21 = -Slam( 0, 1 ) / Slam( 1, 1 );
        a1 = alpha( 0 );
        a2 = alpha( 1 );
        a12 = alpha( 2 );


        // printf( "Laminate Properties\n" );
        // printf( "E1:  %Lg\n", E1 );
        // printf( "E2:  %Lg\n", E2 );
        // printf( "nu:  %Lf\n", nu12 );
        // printf( "G12: %Lg\n\n\n", G12 );
        // printf( "a1:  %Lg\n", a1 );
        // printf( "a2:  %Lg\n", a2 );
        // printf( "a12:  %Lg\n", a12 );

        m_E1_FEM = E1;
        m_E2_FEM = E2;
        m_nu12 = nu12;
        m_G12_FEM = G12;
        m_Thickness_FEM = thickness;
        m_MassDensity_FEM = m / thickness;

        m_A1_FEM = a1;
        m_A2_FEM = a2;
    }
}

void FeaMaterial::GetCompliance( mat3 & S )
{
    long double nu( m_nu12() );
    long double E1( m_E1_FEM() ), E2( m_E2_FEM() );
    long double G12( m_G12_FEM() );

    S <<  1. / E1, -nu / E1,        0.,
         -nu / E1,  1. / E2,        0.,
               0.,        0., 1. / G12;
}

void FeaMaterial::GetCTEVec( vec3 & alpha )
{
    long double a1( m_A1_FEM() );
    long double a2( m_A2_FEM() );

    alpha << a1, a2, 0.0;
}

//////////////////////////////////////////////////////
//================= FeaConnection ==================//
//////////////////////////////////////////////////////

FeaConnection::FeaConnection( )
{
    m_StartFixPtSurfIndex.Init( "StartFixPtSurfIndex", "Connection", this, -1, -1, 1e12 );
    m_EndFixPtSurfIndex.Init( "EndFixPtSurfIndex", "Connection", this, -1, -1, 1e12 );

    m_ConMode.Init( "ConMode", "Connection", this, vsp::FEA_BCM_ALL, vsp::FEA_BCM_USER, vsp::FEA_BCM_PIN ); // not all possible.
    m_Constraints.Init( "Constraints", "Connection", this, 0, 0, 63 );

    m_ConnLineDO.m_Type = DrawObj::VSP_LINES;
    m_ConnLineDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_ConnLineDO.m_LineWidth = 2.0;
    m_ConnLineDO.m_GeomID = GetID() + "Line";

    m_ConnPtsDO.m_Type = DrawObj::VSP_POINTS;
    m_ConnPtsDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_ConnPtsDO.m_PointSize = 7.0;
    m_ConnPtsDO.m_GeomID = GetID() + "Pts";
}

BitMask FeaConnection::GetAsBitMask()
{
    BitMask bm( m_Constraints() );

    return bm;
}

void FeaConnection::Update( )
{
    m_Constraints.Deactivate();

    if ( m_ConMode() == vsp::FEA_BCM_USER )
    {
        m_Constraints.Activate();
    }
    else if ( m_ConMode() == vsp::FEA_BCM_ALL )
    {
        std::vector < bool > bv( 6, true );
        BitMask bm( bv );
        m_Constraints.Set( bm.AsNum() );
    }
    else if ( m_ConMode() == vsp::FEA_BCM_PIN )
    {
        std::vector < bool > bv = {true, true, true, false, false, false};
        BitMask bm( bv );
        m_Constraints.Set( bm.AsNum() );
    }

    UpdateDrawObjs();
}

xmlNodePtr FeaConnection::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr conn_node = xmlNewChild( node, NULL, BAD_CAST "Connection", NULL );

    if ( conn_node )
    {
        ParmContainer::EncodeXml( conn_node );

        XmlUtil::AddStringNode( conn_node, "StartFixPtID", m_StartFixPtID );
        XmlUtil::AddStringNode( conn_node, "StartStructID", m_StartStructID );
        XmlUtil::AddStringNode( conn_node, "EndFixPtID", m_EndFixPtID );
        XmlUtil::AddStringNode( conn_node, "EndStructID", m_EndStructID );
    }

    return conn_node;
}

xmlNodePtr FeaConnection::DecodeXml( xmlNodePtr & conn_node )
{
    if ( conn_node )
    {
        ParmContainer::DecodeXml( conn_node );

        m_StartFixPtID = ParmMgr.RemapID( XmlUtil::FindString( conn_node, "StartFixPtID", m_StartFixPtID ) );
        m_StartStructID = ParmMgr.RemapID( XmlUtil::FindString( conn_node, "StartStructID", m_StartStructID ) );
        m_EndFixPtID = ParmMgr.RemapID( XmlUtil::FindString( conn_node, "EndFixPtID", m_EndFixPtID ) );
        m_EndStructID = ParmMgr.RemapID( XmlUtil::FindString( conn_node, "EndStructID", m_EndStructID ) );
    }

    return conn_node;
}

string FeaConnection::MakeLabel()
{
    string lbl;

    FeaStructure* start_struct = StructureMgr.GetFeaStruct( m_StartStructID );
    FeaPart* start_pt = StructureMgr.GetFeaPart( m_StartFixPtID );
    FeaStructure* end_struct = StructureMgr.GetFeaStruct( m_EndStructID );
    FeaPart* end_pt = StructureMgr.GetFeaPart( m_EndFixPtID );

    if ( start_struct && start_pt && end_struct && end_pt )
    {
        char str[512];
        snprintf( str, sizeof( str ),  "%s:%s:%d:%s:%s:%d:", start_struct->GetName().c_str(), start_pt->GetName().c_str(), m_StartFixPtSurfIndex(),
                                      end_struct->GetName().c_str(), end_pt->GetName().c_str(), m_EndFixPtSurfIndex() );
        lbl = string( str );
    }
    return lbl;
}

string FeaConnection::MakeName()
{
    string name;

    FeaStructure* start_struct = StructureMgr.GetFeaStruct( m_StartStructID );
    FeaPart* start_pt = StructureMgr.GetFeaPart( m_StartFixPtID );
    FeaStructure* end_struct = StructureMgr.GetFeaStruct( m_EndStructID );
    FeaPart* end_pt = StructureMgr.GetFeaPart( m_EndFixPtID );

    if ( start_struct && start_pt && end_struct && end_pt )
    {
        char str[512];
        snprintf( str, sizeof( str ),  "%s_%s_%d_to_%s_%s_%d", start_struct->GetName().c_str(), start_pt->GetName().c_str(), m_StartFixPtSurfIndex(),
                 end_struct->GetName().c_str(), end_pt->GetName().c_str(), m_EndFixPtSurfIndex() );
        name = string( str );
    }
    return name;
}

void FeaConnection::LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_ConnLineDO );
    draw_obj_vec.push_back( &m_ConnPtsDO );
}

void FeaConnection::UpdateDrawObjs()
{
    FeaFixPoint* start_pt = dynamic_cast< FeaFixPoint* > ( StructureMgr.GetFeaPart( m_StartFixPtID ) );
    FeaFixPoint* end_pt = dynamic_cast< FeaFixPoint* > ( StructureMgr.GetFeaPart( m_EndFixPtID ) );

    if ( start_pt && end_pt )
    {
        vector <vec3d> sp = start_pt->GetPntVec();
        vector <vec3d> ep = end_pt->GetPntVec();

        vector <vec3d> pv(2);
        pv[0] = sp[ m_StartFixPtSurfIndex() ];
        pv[1] = ep[ m_EndFixPtSurfIndex() ];

        m_ConnLineDO.m_GeomChanged = true;
        m_ConnLineDO.m_PntVec = pv;

        m_ConnPtsDO.m_GeomChanged = true;
        m_ConnPtsDO.m_PntVec = pv;
    }
}

void FeaConnection::SetDrawObjHighlight ( bool highlight )
{
    if ( highlight )
    {
        m_ConnLineDO.m_LineColor = vec3d( 0.0, 0.0, 1.0 );

        m_ConnPtsDO.m_PointColor = vec3d( 0.0, 0.0, 1.0 );
        m_ConnPtsDO.m_Visible = true;

    }
    else
    {
        m_ConnLineDO.m_LineColor = vec3d( 0.0, 0.0, 0.0 );

        m_ConnPtsDO.m_Visible = false;
    }
}


//////////////////////////////////////////////////////
//================= FeaConnection ==================//
//////////////////////////////////////////////////////

FeaAssembly::FeaAssembly( )
{
}

FeaAssembly::~FeaAssembly()
{
    for ( int i = 0 ; i < ( int )m_ConnectionVec.size() ; i++ )
    {
        delete m_ConnectionVec[i];
    }
    m_ConnectionVec.clear();
}

void FeaAssembly::Update()
{
    for ( int i = 0; i < m_StructIDVec.size(); i++ )
    {
        FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_StructIDVec[i] );
        if ( !fea_struct )
        {
            DelStructure( m_StructIDVec[i] );
        }
    }

    for ( int i = 0 ; i < ( int )m_ConnectionVec.size() ; i++ )
    {
        FeaConnection* conn = m_ConnectionVec[i];
        if ( conn )
        {
            conn->Update();
        }
    }
}

xmlNodePtr FeaAssembly::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr assy_node = xmlNewChild( node, NULL, BAD_CAST "FeaAssembly", NULL );

    if ( assy_node )
    {
        ParmContainer::EncodeXml( assy_node );

        xmlNodePtr structlist_node = xmlNewChild( assy_node, NULL, BAD_CAST "Structure_List", NULL );
        for ( int i = 0 ; i < ( int )m_StructIDVec.size() ; i++ )
        {
            xmlNodePtr struct_node = xmlNewChild( structlist_node, NULL, BAD_CAST "Structure", NULL );
            XmlUtil::AddStringNode( struct_node, "ID", m_StructIDVec[i] );
        }

        xmlNodePtr conlist_node = xmlNewChild( assy_node, NULL, BAD_CAST "Connection_List", NULL );
        for ( int i = 0 ; i < ( int )m_ConnectionVec.size() ; i++ )
        {
            FeaConnection* conn = m_ConnectionVec[i];
            if ( conn )
            {
                conn->EncodeXml( conlist_node );
            }
        }
    }

    m_AssemblySettings.EncodeXml( assy_node );

    return assy_node;
}

xmlNodePtr FeaAssembly::DecodeXml( xmlNodePtr & assy_node )
{
    ParmContainer::DecodeXml( assy_node );

    if ( assy_node )
    {
        m_StructIDVec.clear();

        xmlNodePtr structlist_node = XmlUtil::GetNode( assy_node, "Structure_List", 0 );
        if ( structlist_node )
        {
            int num_struct = XmlUtil::GetNumNames( structlist_node, "Structure" );

            for ( int i = 0; i < num_struct; i++ )
            {
                xmlNodePtr n = XmlUtil::GetNode( structlist_node, "Structure", i );
                m_StructIDVec.push_back( ParmMgr.RemapID( XmlUtil::FindString( n, "ID", string() ) ) );
            }
        }

        xmlNodePtr conlist_node = XmlUtil::GetNode( assy_node, "Connection_List", 0 );
        if ( conlist_node )
        {
            int num = XmlUtil::GetNumNames( conlist_node, "Connection" );

            for ( int i = 0; i < num; i++ )
            {
                xmlNodePtr n = XmlUtil::GetNode( conlist_node, "Connection", i );
                if ( n )
                {
                    FeaConnection *conn = new FeaConnection();
                    conn->DecodeXml( n );
                    m_ConnectionVec.push_back( conn );
                }
            }
        }
    }

    m_AssemblySettings.DecodeXml( assy_node );
    ResetExportFileNames();

    return assy_node;
}

void FeaAssembly::AddStructure( const string &id )
{
    if ( !vector_contains_val( m_StructIDVec, id ) )
    {
        m_StructIDVec.push_back( id );
    }
}

void FeaAssembly::DelStructure( const string &id )
{
    if ( vector_contains_val( m_StructIDVec, id ) )
    {
        vector_remove_val( m_StructIDVec, id );
    }
}

void FeaAssembly::GetAllFixPts( vector< FeaPart* > & fixpts, vector <string> &structids )
{
    fixpts.clear();
    structids.clear();

    for ( int i = 0; i < m_StructIDVec.size(); i++ )
    {
        FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_StructIDVec[i] );
        if ( fea_struct )
        {
            vector < FeaPart * > st_pts = fea_struct->GetFeaPartVecType( vsp::FEA_FIX_POINT );
            if ( st_pts.size() > 0 )
            {
                fixpts.insert( fixpts.end(), st_pts.begin(), st_pts.end() );
                vector < string > ids( st_pts.size(), m_StructIDVec[ i ] );
                structids.insert( structids.end(), ids.begin(), ids.end() );
            }
        }
    }
}

void FeaAssembly::AddConnection( const string &startid, const string &startstructid, int startindx,
                                 const string &endid, const string &endstructid, int endindx )
{
    if ( startid != endid )
    {
        FeaConnection *conn = new FeaConnection();

        conn->m_StartFixPtID = startid;
        conn->m_StartStructID = startstructid;
        conn->m_StartFixPtSurfIndex = startindx;

        conn->m_EndFixPtID = endid;
        conn->m_EndStructID = endstructid;
        conn->m_EndFixPtSurfIndex = endindx;

        m_ConnectionVec.push_back( conn );
    }
}

void FeaAssembly::DelConnection( int index )
{
    if ( index >= 0 && index < m_ConnectionVec.size() && m_ConnectionVec.size() > 0 )
    {
        FeaConnection* con = m_ConnectionVec[index];
        if ( con )
        {
            delete con;
        }
        m_ConnectionVec.erase( m_ConnectionVec.begin() + index );
    }
}

void FeaAssembly::ResetExportFileNames()
{
    m_AssemblySettings.ResetExportFileNames( GetName() );
}

void FeaAssembly::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    m_AssemblySettings.AddLinkableParms( linkable_parm_vec, m_ID );
}

//////////////////////////////////////////////////////
//===================== FeaBC ======================//
//////////////////////////////////////////////////////

FeaBC::FeaBC( const string &structID )
{
    m_StructID = structID;
    m_FeaBCType.Init( "Type", "FeaBC", this, vsp::FEA_BC_STRUCTURE, vsp::FEA_BC_STRUCTURE, vsp::FEA_NUM_BC_TYPES - 1 );

    m_ConMode.Init( "ConMode", "FeaBC", this, vsp::FEA_BCM_USER, vsp::FEA_BCM_USER, vsp::FEA_NUM_BCM_MODES - 1 );
    m_Constraints.Init( "Constraints", "FeaBC", this, 0, 0, 63 );

    m_XLTFlag.Init( "XLTFlag", "FeaBC", this, false, false, true );
    m_XGTFlag.Init( "XLGFlag", "FeaBC", this, false, false, true );

    m_YLTFlag.Init( "YLTFlag", "FeaBC", this, false, false, true );
    m_YGTFlag.Init( "YLGFlag", "FeaBC", this, false, false, true );

    m_ZLTFlag.Init( "ZLTFlag", "FeaBC", this, false, false, true );
    m_ZGTFlag.Init( "ZLGFlag", "FeaBC", this, false, false, true );

    m_XLTVal.Init( "XLTVal", "FeaBC", this, 0.0, -1.0e12, 1.0e12 );
    m_XGTVal.Init( "XGTVal", "FeaBC", this, 0.0, -1.0e12, 1.0e12 );

    m_YLTVal.Init( "YLTVal", "FeaBC", this, 0.0, -1.0e12, 1.0e12 );
    m_YGTVal.Init( "YGTVal", "FeaBC", this, 0.0, -1.0e12, 1.0e12 );

    m_ZLTVal.Init( "ZLTVal", "FeaBC", this, 0.0, -1.0e12, 1.0e12 );
    m_ZGTVal.Init( "ZGTVal", "FeaBC", this, 0.0, -1.0e12, 1.0e12 );
}

void FeaBC::ParmChanged( Parm* parm_ptr, int type )
{
    FeaStructure *pstruct = StructureMgr.GetFeaStruct( m_StructID );

    if ( pstruct )
    {
        pstruct->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr FeaBC::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr bc_info = xmlNewChild( node, NULL, BAD_CAST "FeaBCInfo", NULL );

    xmlNodePtr conn_node = ParmContainer::EncodeXml( bc_info );

    if ( conn_node )
    {
        XmlUtil::AddStringNode( conn_node, "PartID", m_PartID );
        XmlUtil::AddStringNode( conn_node, "SubSurfID", m_SubSurfID );
    }

    return conn_node;
}

xmlNodePtr FeaBC::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr conn_node = ParmContainer::DecodeXml( node );

    if ( conn_node )
    {
        m_PartID = ParmMgr.RemapID( XmlUtil::FindString( conn_node, "PartID", m_PartID ) );
        m_SubSurfID = ParmMgr.RemapID( XmlUtil::FindString( conn_node, "SubSurfID", m_SubSurfID ) );

    }

    return conn_node;
}

string FeaBC::GetDescription()
{
    string desc = GetDescriptionDOF();

    if ( m_FeaBCType() == vsp::FEA_BC_PART )
    {
        FeaPart* part = StructureMgr.GetFeaPart( m_PartID );

        if ( part )
        {
            desc += part->GetName() + " ";
        }
        else
        {
            desc += string( "Not Found " );
        }
    }
    else if ( m_FeaBCType() == vsp::FEA_BC_SUBSURF )
    {
        SubSurface* subsurf = StructureMgr.GetFeaSubSurf( m_SubSurfID );

        if ( subsurf )
        {
            desc += subsurf->GetName() + " ";
        }
        else
        {
            desc += string( "Not Found " );
        }
    }

    char str[256];

    if ( m_XGTFlag() && m_XLTFlag() )
    {
        snprintf( str, sizeof( str ),  "%g<=X<=%g ", m_XGTVal(), m_XLTVal() );
        desc += string( str );
    }
    else if ( m_XGTFlag() )
    {
        snprintf( str, sizeof( str ),  "X>=%g ", m_XGTVal() );
        desc += string( str );
    }
    else if ( m_XLTFlag() )
    {
        snprintf( str, sizeof( str ),  "X<=%g ", m_XLTVal() );
        desc += string( str );
    }

    if ( m_YGTFlag() && m_YLTFlag() )
    {
        snprintf( str, sizeof( str ),  "%g<=Y<=%g ", m_YGTVal(), m_YLTVal() );
        desc += string( str );
    }
    else if ( m_YGTFlag() )
    {
        snprintf( str, sizeof( str ),  "Y>=%g ", m_YGTVal() );
        desc += string( str );
    }
    else if ( m_YLTFlag() )
    {
        snprintf( str, sizeof( str ),  "Y<=%g ", m_YLTVal() );
        desc += string( str );
    }

    if ( m_ZGTFlag() && m_ZLTFlag() )
    {
        snprintf( str, sizeof( str ),  "%g<=Z<=%g ", m_ZGTVal(), m_ZLTVal() );
        desc += string( str );
    }
    else if ( m_ZGTFlag() )
    {
        snprintf( str, sizeof( str ),  "Z>=%g ", m_ZGTVal() );
        desc += string( str );
    }
    else if ( m_ZLTFlag() )
    {
        snprintf( str, sizeof( str ),  "Z<=%g ", m_ZLTVal() );
        desc += string( str );
    }

    return desc;
}

string FeaBC::GetDescriptionDOF()
{
    char labels[] = "XYZPQR";
    char str[13];

    BitMask bm = GetAsBitMask();

    for ( int i = 0; i < 6; i++ )
    {
        if ( bm.CheckBit( i ) )
        {
            str[ 2 * i ] = labels[ i ];
        }
        else
        {
            str[ 2 * i ] = ' ';
        }

        str[ 2 * i + 1 ] = ':';
    }
    str[12] = '\0';

    return string( str );
}

BitMask FeaBC::GetAsBitMask()
{
    BitMask bm( m_Constraints() );

    return bm;
}

void FeaBC::Update()
{
    m_Constraints.Deactivate();

    if ( m_ConMode() == vsp::FEA_BCM_USER )
    {
        m_Constraints.Activate();
    }
    else if ( m_ConMode() == vsp::FEA_BCM_ALL )
    {
        std::vector < bool > bv( 6, true );
        BitMask bm( bv );
        m_Constraints.Set( bm.AsNum() );
    }
    else if ( m_ConMode() == vsp::FEA_BCM_PIN )
    {
        std::vector < bool > bv = {true, true, true, false, false, false};
        BitMask bm( bv );
        m_Constraints.Set( bm.AsNum() );
    }
    else if ( m_ConMode() == vsp::FEA_BCM_SYMM )
    {
        std::vector < bool > bv = {false, true, false, true, false, true};
        BitMask bm( bv );
        m_Constraints.Set( bm.AsNum() );
    }
    else if ( m_ConMode() == vsp::FEA_BCM_ASYMM )
    {
        std::vector < bool > bv = {true, false, true, false, true, false};
        BitMask bm( bv );
        m_Constraints.Set( bm.AsNum() );
    }
}
