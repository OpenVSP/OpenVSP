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
#include "VehicleMgr.h"
#include "ParmMgr.h"
#include "StructureMgr.h"
#include "LinkMgr.h"

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
            int prop_index = XmlUtil::FindInt( part_info, "FeaPropertyIndex", 0 );
            int cap_prop_index = XmlUtil::FindInt( part_info, "CapFeaPropertyIndex", 0 );

            if ( type != vsp::FEA_SKIN )
            {
                FeaPart* feapart = AddFeaPart( type );

                feapart->SetFeaPropertyIndex( prop_index );
                feapart->SetCapFeaPropertyIndex( cap_prop_index );

                feapart->DecodeXml( part_info );
            }
            else
            {
                FeaPart* feaskin = new FeaSkin( m_ParentGeomID );
                feaskin->DecodeXml( part_info );

                feaskin->SetFeaPropertyIndex( prop_index );
                feaskin->SetCapFeaPropertyIndex( cap_prop_index );

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
                int prop_index = XmlUtil::FindInt( ss_info_node, "FeaPropertyIndex", 0 );

                SubSurface* ssurf = AddFeaSubSurf( type );
                if ( ssurf )
                {
                    ssurf->DecodeXml( ss_node );

                    ssurf->SetFeaPropertyIndex( prop_index );
                }
            }
        }
    }

    return node;
}

FeaPart* FeaStructure::AddFeaPart( int type )
{
    FeaPart* feaprt = new FeaPart( m_ParentGeomID, type );

    if ( type == vsp::FEA_SLICE )
    {
        feaprt = new FeaSlice( m_ParentGeomID );
        feaprt->SetName( string( "FEA_SLICE_" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_RIB )
    {
        feaprt = new FeaRib( m_ParentGeomID );
        feaprt->SetName( string( "FEA_RIB_" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_SPAR )
    {
        feaprt = new FeaSpar( m_ParentGeomID );
        feaprt->SetName( string( "FEA_SPAR_" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_FIX_POINT )
    {
        feaprt = new FeaFixPoint( m_ParentGeomID );
        feaprt->SetName( string( "FEA_FIX_POINT_" + std::to_string( m_FeaPartCount ) ) );
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

vector < FeaPart* > FeaStructure::AddEvenlySpacedRibs( const int num_rib )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( current_wing )
        {
            vector< VspSurf > surf_vec;
            current_wing->GetSurfVec( surf_vec );

            VspSurf current_surf = surf_vec[m_MainSurfIndx];

            double u_max = current_surf.GetUMax();

            // TODO: Improve u_start and u_end assumption

            double u_start = 1.0;
            double u_end = u_max - 1.0;

            double u_step = ( ( u_end - u_start ) / ( num_rib - 1 ) );

            for ( unsigned int i = 0; i < num_rib; i++ )
            {
                FeaPart* part = AddFeaPart( vsp::FEA_RIB );

                FeaRib* rib = dynamic_cast< FeaRib* >( part );

                if ( rib )
                {
                    rib->m_PerU.Set( ( u_start + i * u_step ) / u_max );
                    rib->Update();
                }
            }
        }
    }
    return m_FeaPartVec;
}

string FeaStructure::AddSpacedSlices( int orientation )
{
    string message;

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh && GetStructSettingsPtr() )
    {
        Geom* current_geom = veh->FindGeom( m_ParentGeomID );

        if ( current_geom )
        {
            double spacing = GetStructSettingsPtr()->m_MultiSliceSpacing.Get();

            vector< VspSurf > surf_vec;
            current_geom->GetSurfVec( surf_vec );

            VspSurf current_surf = surf_vec[m_MainSurfIndx];

            BndBox bbox;
            current_surf.GetBoundingBox( bbox );

            vec3d diag = bbox.GetMax() - bbox.GetMin();
            double total_length;

            if ( orientation == XY_PLANE )
            {
                total_length = diag.z();
            }
            else if ( orientation == YZ_PLANE )
            {
                total_length = diag.x();
            }
            else if ( orientation == XZ_PLANE )
            {
                total_length = diag.y();
            }

            int num_parts = floor( total_length / spacing );
            double percent_space = spacing / total_length;

            for ( size_t i = 0; i <= num_parts; i++ )
            {
                FeaPart* part = AddFeaPart( vsp::FEA_SLICE );

                FeaSlice* slice = dynamic_cast<FeaSlice*>( part );

                if ( slice )
                {
                    slice->m_OrientationPlane.Set( orientation );
                    slice->m_CenterPerBBoxLocation.Set( i * percent_space );
                    slice->m_IncludedElements.Set( GetStructSettingsPtr()->m_MultSliceIncludedElements.Get() );
                    slice->Update();
                }
            }

            message = std::to_string( num_parts + 1 ) + " slices added\n";
        }
    }
    return message;
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
        ssurf->SetName( string( "FEA_SS_LINE_" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_RECTANGLE )
    {
        ssurf = new SSRectangle( m_ParentGeomID );
        ssurf->SetName( string( "FEA_SS_RECT_" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_ELLIPSE )
    {
        ssurf = new SSEllipse( m_ParentGeomID );
        ssurf->SetName( string( "FEA_SS_ELLIP_" + to_string( m_FeaSubSurfCount ) ) );
    }
    else if ( type == vsp::SS_CONTROL )
    {
        ssurf = new SSControlSurf( m_ParentGeomID );
        ssurf->SetName( string( "FEA_SS_CONT_" + to_string( m_FeaSubSurfCount ) ) );
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
    else
    {
        return false;
    }
}

void FeaStructure::UpdateFeaParts()
{
    for ( unsigned int i = 0; i < m_FeaPartVec.size(); i++ )
    {
        m_FeaPartVec[i]->UpdateSymmetricSurfs();

        if ( FeaPartIsFixPoint( i ) )
        {
            FeaFixPoint* fixpt = dynamic_cast<FeaFixPoint*>( m_FeaPartVec[i] );
            assert( fixpt );

            fixpt->m_HalfMeshFlag = m_StructSettings.GetHalfMeshFlag();
        }

        m_FeaPartVec[i]->Update();
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
                feaskin->SetName( string( "FEA_SKIN_" + std::to_string( m_FeaPartCount ) ) );
                feaskin->m_MainSurfIndx.Set( m_MainSurfIndx );
                
                feaskin->UpdateSymmetricSurfs();
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

bool FeaStructure::FeaPartIsFixPoint( int ind )
{
    bool fixpoint = false;
    FeaPart* fea_part = GetFeaPart( ind );

    if ( fea_part )
    {
        if ( fea_part->GetType() == vsp::FEA_FIX_POINT )
        {
            fixpoint = true;
        }
    }
    return fixpoint;
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

int FeaStructure::GetNumFeaSkin()
{
    int num_skin = 0;

    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( m_FeaPartVec[i]->GetType() == vsp::FEA_SKIN )
        {
            num_skin++;
        }
    }
    return num_skin;
}

//==== Get FeaProperty Index from FeaPart Index =====//
int FeaStructure::GetFeaPropertyIndex( int fea_part_ind )
{
    if ( ValidFeaPartInd( fea_part_ind ) )
    {
        FeaPart* fea_part = GetFeaPart( fea_part_ind );
        if ( fea_part )
        {
            return fea_part->GetFeaPropertyIndex();
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
            return fea_part->GetCapFeaPropertyIndex();
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

//////////////////////////////////////////////////////
//==================== FeaPart =====================//
//////////////////////////////////////////////////////

FeaPart::FeaPart( string geomID, int type )
{
    m_FeaPartType = type;
    m_ParentGeomID = geomID;

    m_MainSurfIndx.Init( "MainSurfIndx", "FeaPart", this, -1, -1, 1e12 );
    m_MainSurfIndx.SetDescript( "Surface Index for FeaPart" );

    m_IncludedElements.Init( "IncludedElements", "FeaPart", this, TRIS, TRIS, BOTH_ELEMENTS );
    m_IncludedElements.SetDescript( "Indicates the FeaElements to be Included for the FeaPart" );

    m_FeaPropertyIndex = 0; // Shell property default
    m_CapFeaPropertyIndex = 1; // Beam property default
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

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

xmlNodePtr FeaPart::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr part_info = xmlNewChild( node, NULL, BAD_CAST "FeaPartInfo", NULL );

    XmlUtil::AddIntNode( part_info, "FeaPartType", m_FeaPartType );
    XmlUtil::AddIntNode( part_info, "FeaPropertyIndex", m_FeaPropertyIndex );
    XmlUtil::AddIntNode( part_info, "CapFeaPropertyIndex", m_CapFeaPropertyIndex );

    return ParmContainer::EncodeXml( part_info );
}

xmlNodePtr FeaPart::DecodeXml( xmlNodePtr & node )
{
    return ParmContainer::DecodeXml( node );
}

void FeaPart::UpdateSymmetricSurfs()
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

vector < Matrix4d > FeaPart::CalculateSymmetricTransform()
{
    vector<Matrix4d> transMats;

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return transMats;
    }

    Geom* current_geom = veh->FindGeom( m_ParentGeomID );
    if ( !current_geom )
    {
        return transMats;
    }

    // Compute Relative Translation Matrix
    Matrix4d symmOriginMat;
    if ( current_geom->m_SymAncestOriginFlag() )
    {
        symmOriginMat = current_geom->GetAncestorAttachMatrix( current_geom->m_SymAncestor() - 1 );
    }
    else
    {
        symmOriginMat = current_geom->GetAncestorModelMatrix( current_geom->m_SymAncestor() - 1 );
    }

    transMats.resize( m_SymmIndexVec.size(), Matrix4d() );

    int symFlag = current_geom->GetSymFlag();
    if ( symFlag != 0 )
    {
        int numShifts = -1;
        Matrix4d Ref; // Reflection Matrix
        Matrix4d Ref_Orig; // Original Reflection Matrix

        double angle = ( 360 ) / (double)current_geom->m_SymRotN();
        int currentIndex = m_MainSurfIndx() + 1;
        bool radial = false;

        for ( int i = 0; i < current_geom->GetNumSymFlags(); i++ ) // Loop through each of the set sym flags
        {
            // Find next set sym flag
            while ( true )
            {
                numShifts++;
                if ( ( ( symFlag >> numShifts ) & ( 1 << 0 ) ) || numShifts > vsp::SYM_NUM_TYPES )
                {
                    break;
                }
            }

            // Create Reflection Matrix
            if ( ( 1 << numShifts ) == vsp::SYM_XY )
            {
                Ref.loadXYRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_XZ )
            {
                Ref.loadXZRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_YZ )
            {
                Ref.loadYZRef();
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_X )
            {
                Ref.loadIdentity();
                Ref.rotateX( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_Y )
            {
                Ref.loadIdentity();
                Ref.rotateY( angle );
                Ref_Orig = Ref;
                radial = true;
            }
            else if ( ( 1 << numShifts ) == vsp::SYM_ROT_Z )
            {
                Ref.loadIdentity();
                Ref.rotateZ( angle );
                Ref_Orig = Ref;
                radial = true;
            }

            int numAddSurfs = currentIndex;
            int addIndex = 0;

            for ( int j = currentIndex; j < currentIndex + numAddSurfs; j++ )
            {
                if ( radial ) // rotational reflection
                {
                    for ( int k = 0; k < current_geom->m_SymRotN() - 1; k++ )
                    {
                        transMats[j + k * numAddSurfs].initMat( transMats[j - currentIndex].data() );
                        transMats[j + k * numAddSurfs].postMult( Ref.data() ); // Apply Reflection

                        // Increment rotation by the angle
                        Ref.postMult( Ref_Orig.data() );
                        addIndex++;
                    }
                    // Reset reflection matrices to the beginning angle
                    Ref = Ref_Orig;
                }
                else
                {
                    transMats[j].initMat( transMats[j - currentIndex].data() );
                    transMats[j].postMult( Ref.data() ); // Apply Reflection
                    addIndex++;
                }
            }

            currentIndex += addIndex;
            radial = false;
        }
    }

    //==== Apply Transformations ====//
    for ( int i = 1; i < m_SymmIndexVec.size(); i++ )
    {
        transMats[i].postMult( symmOriginMat.data() );
    }

    return transMats;
}

string FeaPart::GetTypeName( int type )
{
    if ( type == vsp::FEA_SLICE )
    {
        return string( "FeaSlice" );
    }
    if ( type == vsp::FEA_RIB )
    {
        return string( "FeaRib" );
    }
    if ( type == vsp::FEA_SPAR )
    {
        return string( "FeaSpar" );
    }
    if ( type == vsp::FEA_FIX_POINT )
    {
        return string( "FeaFixPoint" );
    }
    if ( type == vsp::FEA_SUB_SURF )
    {
        return string( "FeaSubSurf" );
    }
    if ( type == vsp::FEA_SKIN )
    {
        return string( "FeaSkin" );
    }

    return string( "NONE" );
}

void FeaPart::FetchFeaXFerSurf( vector< XferSurf > &xfersurfs, int compid )
{
    for ( int p = 0; p < m_FeaPartSurfVec.size(); p++ )
    {
        // CFD_STRUCTURE and CFD_STIFFENER type surfaces have m_CompID starting at -9999
        m_FeaPartSurfVec[p].FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx(), compid, xfersurfs );
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

        m_FeaPartDO[j].m_GeomID = string( "FeaPart_" + std::to_string( id ) + "_" + std::to_string( j ) );
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
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex );

    if ( fea_prop )
    {
        return fea_prop->GetFeaMaterialIndex();
    }
    else
    {
        return -1;
    }
}

void FeaPart::SetFeaMaterialIndex( int index )
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex );

    if ( fea_prop )
    {
        fea_prop->SetFeaMaterialIndex( index );
    }
}

//////////////////////////////////////////////////////
//================== FeaFullDepth ==================//
//////////////////////////////////////////////////////

FeaSlice::FeaSlice( string geomID, int type ) : FeaPart( geomID, type )
{
    m_OrientationPlane.Init( "OrientationPlane", "FeaFullDepth", this, XY_PLANE, XY_PLANE, XZ_PLANE );
    m_OrientationPlane.SetDescript( "Plane the FeaFullDepth Part will be Parallel to" );

    m_CenterPerBBoxLocation.Init( "CenterPerBBoxLocation", "FeaFullDepth", this, 0.5, 0.0, 1.0 );
    m_CenterPerBBoxLocation.SetDescript( "The Location of the Center of the FeaFullDepth Part as a Percentage of the Total Bounding Box" );

    m_Theta.Init( "Theta", "FeaFullDepth", this, 0.0, -90.0, 90.0 );
    m_Alpha.Init( "Alpha", "FeaFullDepth", this, 0.0, -90.0, 90.0 );
}

void FeaSlice::Update()
{
    UpdateSymmetricSurfs();
    ComputePlanarSurf();
}

void FeaSlice::ComputePlanarSurf()
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

        VspSurf current_surf = surf_vec[m_MainSurfIndx()];

        m_FeaPartSurfVec[0] = VspSurf();

        if ( m_IncludedElements() == TRIS || m_IncludedElements() == BOTH_ELEMENTS )
        {
            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STRUCTURE );
        }
        else
        {
            m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STIFFENER );
        }

        BndBox geom_bbox;
        current_surf.GetBoundingBox( geom_bbox );

        vec3d geom_center = geom_bbox.GetCenter();

        double del_x = geom_bbox.GetMax( 0 ) - geom_bbox.GetMin( 0 );
        double del_y = geom_bbox.GetMax( 1 ) - geom_bbox.GetMin( 1 );
        double del_z = geom_bbox.GetMax( 2 ) - geom_bbox.GetMin( 2 );

        // Identify corners of the plane and rotation axis
        vec3d cornerA, cornerB, cornerC, cornerD;
        vec3d theta_rot_axis, alpha_rot_axis;

        vec3d slice_center;

        if ( m_OrientationPlane() == XY_PLANE )
        {
            slice_center = vec3d( geom_center.x(), geom_center.y(), geom_bbox.GetMin( 2 ) + del_z * m_CenterPerBBoxLocation() );
        }
        else if ( m_OrientationPlane() == YZ_PLANE )
        {
            slice_center = vec3d( geom_bbox.GetMin( 0 ) + del_x * m_CenterPerBBoxLocation(), geom_center.y(), geom_center.z() );
        }
        else if ( m_OrientationPlane() == XZ_PLANE )
        {
            slice_center = vec3d( geom_center.x(), geom_bbox.GetMin( 1 ) + del_y * m_CenterPerBBoxLocation(), geom_center.z() );
        }

        vec3d offset_vec = slice_center - geom_center;
        double del_x_plus, del_x_minus, del_y_plus, del_y_minus, del_z_plus, del_z_minus;

        if ( m_OrientationPlane() == XY_PLANE )
        {
            double z_off = offset_vec.z();

            if ( m_Theta() >= 0.0 )
            {
                if ( DEG_2_RAD * m_Theta() > atan( ( del_z + 2 * z_off ) / del_x ) )
                {
                    del_x_minus = abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_minus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }

                if ( DEG_2_RAD * m_Theta() > atan( ( del_z - 2 * z_off ) / del_x ) )
                {
                    del_x_plus = abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_plus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }
            }
            else
            {
                if ( abs( DEG_2_RAD * m_Theta() ) > atan( ( del_z + 2 * z_off ) / del_x ) )
                {
                    del_x_plus = abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_plus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }

                if ( abs( DEG_2_RAD * m_Theta() ) > atan( ( del_z - 2 * z_off ) / del_x ) )
                {
                    del_x_minus = abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_minus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }
            }

            if ( m_Alpha() >= 0.0 )
            {
                if ( DEG_2_RAD * m_Alpha() > atan( ( del_z + 2 * z_off ) / del_y ) )
                {
                    del_y_plus = abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_plus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }

                if ( DEG_2_RAD * m_Alpha() > atan( ( del_z - 2 * z_off ) / del_y ) )
                {
                    del_y_minus = abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_minus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }
            }
            else
            {
                if ( abs( DEG_2_RAD * m_Alpha() ) > atan( ( del_z + 2 * z_off ) / del_y ) )
                {
                    del_y_minus = abs( ( del_z + 2 * z_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_minus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }

                if ( abs( DEG_2_RAD * m_Alpha() ) > atan( ( del_z - 2 * z_off ) / del_y ) )
                {
                    del_y_plus = abs( ( del_z - 2 * z_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_plus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }
            }

            vec3d center_to_A = { -0.5 * del_x_minus, -0.5 * del_y_minus, 0.0 };
            cornerA = slice_center + center_to_A;

            vec3d center_to_B = { -0.5 * del_x_minus, 0.5 * del_y_plus, 0.0 };
            cornerB = slice_center + center_to_B;

            vec3d center_to_C = { 0.5 * del_x_plus, -0.5 * del_y_minus, 0.0 };
            cornerC = slice_center + center_to_C;

            vec3d center_to_D = { 0.5 * del_x_plus, 0.5 * del_y_plus, 0.0 };
            cornerD = slice_center + center_to_D;

            theta_rot_axis.set_y( 1.0 ); // y-axis
            alpha_rot_axis.set_x( 1.0 ); // x-axis
        }
        else if ( m_OrientationPlane() == YZ_PLANE )
        {
            double x_off = offset_vec.x();

            if ( DEG_2_RAD * m_Theta() >= 0.0 )
            {
                if ( DEG_2_RAD * m_Theta() > atan( ( del_x + 2 * x_off ) / del_z ) )
                {
                    del_z_plus = abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_z_plus = abs( del_z / cos( DEG_2_RAD * m_Theta() ) );
                }

                if ( DEG_2_RAD * m_Theta() > atan( ( del_x - 2 * x_off ) / del_z ) )
                {
                    del_z_minus = abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_z_minus = abs( del_z / cos( DEG_2_RAD * m_Theta() ) );
                }
            }
            else
            {
                if ( abs( DEG_2_RAD * m_Theta() ) > atan( ( del_x + 2 * x_off ) / del_z ) )
                {
                    del_z_minus = abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_z_minus = abs( del_z / cos( DEG_2_RAD * m_Theta() ) );
                }

                if ( ( abs( DEG_2_RAD * m_Theta() ) > atan( ( del_x - 2 * x_off ) / del_z ) ) )
                {
                    del_z_plus = abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_z_plus = abs( del_z / cos( DEG_2_RAD * m_Theta() ) );
                }
            }

            if ( m_Alpha() >= 0.0 )
            {
                if ( DEG_2_RAD * m_Alpha() > atan( ( del_x + 2 * x_off ) / del_y ) )
                {
                    del_y_minus = abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_minus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }

                if ( DEG_2_RAD * m_Alpha() > atan( ( del_x - 2 * x_off ) / del_y ) )
                {
                    del_y_plus = abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_plus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }
            }
            else
            {
                if ( abs( DEG_2_RAD * m_Alpha() ) > atan( ( del_x + 2 * x_off ) / del_y ) )
                {
                    del_y_plus = abs( ( del_x + 2 * x_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_plus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }

                if ( abs( DEG_2_RAD * m_Alpha() ) > atan( ( del_x - 2 * x_off ) / del_y ) )
                {
                    del_y_minus = abs( ( del_x - 2 * x_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_y_minus = abs( del_y / cos( DEG_2_RAD * m_Alpha() ) );
                }
            }

            vec3d center_to_A = { 0.0, -0.5 * del_y_minus, -0.5 * del_z_minus };
            cornerA = slice_center + center_to_A;

            vec3d center_to_B = { 0.0, 0.5 * del_y_plus, -0.5 * del_z_minus };
            cornerB = slice_center + center_to_B;

            vec3d center_to_C = { 0.0, -0.5 * del_y_minus, 0.5 * del_z_plus };
            cornerC = slice_center + center_to_C;

            vec3d center_to_D = { 0.0, 0.5 * del_y_plus, 0.5 * del_z_plus };
            cornerD = slice_center + center_to_D;

            theta_rot_axis.set_y( 1.0 ); // y-axis
            alpha_rot_axis.set_z( 1.0 ); // z-axis
        }
        else if ( m_OrientationPlane() == XZ_PLANE )
        {
            double y_off = offset_vec.y();

            if ( DEG_2_RAD * m_Theta() > 0.0 )
            {
                if ( DEG_2_RAD * m_Theta() > atan( ( del_y + 2 * y_off ) / del_x ) )
                {
                    del_x_plus = abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_plus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }

                if ( DEG_2_RAD * m_Theta() > atan( ( del_y - 2 * y_off ) / del_x ) )
                {
                    del_x_minus = abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_minus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }
            }
            else
            {
                if ( abs( DEG_2_RAD * m_Theta() ) > atan( ( del_y + 2 * y_off ) / del_x ) )
                {
                    del_x_minus = abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_minus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }

                if ( abs( DEG_2_RAD * m_Theta() ) > atan( ( del_y - 2 * y_off ) / del_x ) )
                {
                    del_x_plus = abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_Theta() ) );
                }
                else
                {
                    del_x_plus = abs( del_x / cos( DEG_2_RAD * m_Theta() ) );
                }
            }

            if ( m_Alpha() >= 0.0 )
            {
                if ( DEG_2_RAD * m_Alpha() > atan( ( del_y + 2 * y_off ) / del_z ) )
                {
                    del_z_minus = abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_z_minus = abs( del_z / cos( DEG_2_RAD * m_Alpha() ) );
                }

                if ( DEG_2_RAD * m_Alpha() > atan( ( del_y - 2 * y_off ) / del_z ) )
                {
                    del_z_plus = abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_z_plus = abs( del_z / cos( DEG_2_RAD * m_Alpha() ) );
                }
            }
            else
            {
                if ( abs( DEG_2_RAD * m_Alpha() ) > atan( ( del_y + 2 * y_off ) / del_z ) )
                {
                    del_z_plus = abs( ( del_y + 2 * y_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_z_plus = abs( del_z / cos( DEG_2_RAD * m_Alpha() ) );
                }

                if ( abs( DEG_2_RAD * m_Alpha() ) > atan( ( del_y - 2 * y_off ) / del_z ) )
                {
                    del_z_minus = abs( ( del_y - 2 * y_off ) / sin( DEG_2_RAD * m_Alpha() ) );
                }
                else
                {
                    del_z_minus = abs( del_z / cos( DEG_2_RAD * m_Alpha() ) );
                }
            }

            vec3d center_to_A = { -0.5 * del_x_minus, 0.0, -0.5 * del_z_minus };
            cornerA = slice_center + center_to_A;

            vec3d center_to_B = { 0.5 * del_x_plus, 0.0, -0.5 * del_z_minus };
            cornerB = slice_center + center_to_B;

            vec3d center_to_C = { -0.5 * del_x_minus, 0.0, 0.5 * del_z_plus };
            cornerC = slice_center + center_to_C;

            vec3d center_to_D = { 0.5 * del_x_plus, 0.0, 0.5 * del_z_plus };
            cornerD = slice_center + center_to_D;

            theta_rot_axis.set_z( 1.0 ); // z-axis
            alpha_rot_axis.set_x( 1.0 ); // x-axis
        }

        // Make Planar Surface
        m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != current_surf.GetFlipNormal() )
        {
            m_FeaPartSurfVec[0].FlipNormal();
            theta_rot_axis = -1 * theta_rot_axis;
            alpha_rot_axis = -1 * alpha_rot_axis;
        }

        // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
        Matrix4d trans_mat_1, trans_mat_2, rot_mat_theta, rot_mat_alpha;

        trans_mat_1.loadIdentity();
        trans_mat_1.translatef( slice_center.x() * -1, slice_center.y() * -1, slice_center.z() * -1 );
        m_FeaPartSurfVec[0].Transform( trans_mat_1 );

        rot_mat_theta.loadIdentity();
        rot_mat_theta.rotate( DEG_2_RAD * m_Theta(), theta_rot_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat_theta );

        rot_mat_alpha.loadIdentity();
        rot_mat_alpha.rotate( DEG_2_RAD * m_Alpha(), alpha_rot_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat_alpha );

        trans_mat_2.loadIdentity();
        trans_mat_2.translatef( slice_center.x(), slice_center.y(), slice_center.z() );
        m_FeaPartSurfVec[0].Transform( trans_mat_2 );

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, calculate and transform the symmetric copies
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }

        // Compute Symmetric Translation Matrix
        vector<Matrix4d> transMats = CalculateSymmetricTransform();

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

void FeaSlice::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

//////////////////////////////////////////////////////
//===================== FeaSpar ====================//
//////////////////////////////////////////////////////

FeaSpar::FeaSpar( string geomID, int type ) : FeaPart( geomID, type )
{
    m_PerV.Init( "PerV", "FeaSpar", this, 0.5, 0.0, 1.0 );
    m_PerV.SetDescript( "Precent V Location" );
    m_Theta.Init( "Theta", "FeaSpar", this, 0.0, -90.0, 90.0 );

}

void FeaSpar::Update()
{
    UpdateSymmetricSurfs();
    ComputePlanarSurf();
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

        m_FeaPartSurfVec[0] = VspSurf();
        m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STRUCTURE );

        vector< VspSurf > surf_vec;
        current_wing->GetSurfVec( surf_vec );
        VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

        BndBox wing_bbox;
        wing_surf.GetBoundingBox( wing_bbox );

        VspCurve constant_v_curve;
        wing_surf.GetW01ConstCurve( constant_v_curve, m_PerV() );

        piecewise_curve_type v_curve = constant_v_curve.GetCurve();

        double u_min = v_curve.get_parameter_min();
        double u_max = v_curve.get_parameter_max();

        vec3d inside_edge, outside_edge;
        inside_edge = v_curve.f( u_min );
        outside_edge = v_curve.f( u_max );

        VspCurve constant_u_curve;
        wing_surf.GetU01ConstCurve( constant_u_curve, 0.5 );

        piecewise_curve_type u_curve = constant_u_curve.GetCurve();

        double v_min = u_curve.get_parameter_min(); // Really must be 0.0
        double v_max = u_curve.get_parameter_max(); // Really should be 4.0
        double v_leading_edge = ( v_min + v_max ) * 0.5;

        // Find two points slightly above and below the trailing edge
        double v_trail_edge_low = v_min + 2 * TMAGIC;
        double v_trail_edge_up = v_max - 2 * TMAGIC;

        vec3d trail_edge_up, trail_edge_low;
        trail_edge_up = u_curve.f( v_trail_edge_low );
        trail_edge_low = u_curve.f( v_trail_edge_up );

        vec3d wing_z_axis = trail_edge_up - trail_edge_low;
        wing_z_axis.normalize();

        double height = 0.5 * wing_bbox.GetSmallestDist();

        vec3d center = ( inside_edge + outside_edge ) / 2;

        vec3d center_to_inner_edge = center - inside_edge; // flip?
        center_to_inner_edge.normalize();

        vec3d center_to_outer_edge = outside_edge - center;
        center_to_outer_edge.normalize();

        vec3d min_trail_edge = wing_surf.CompPnt( 0.0, 0.0 );
        vec3d min_lead_edge = wing_surf.CompPnt( 0.0, v_leading_edge );
        vec3d max_trail_edge = wing_surf.CompPnt( wing_surf.GetUMax(), 0.0 );
        vec3d max_lead_edge = wing_surf.CompPnt( wing_surf.GetUMax(), v_leading_edge );

        vec3d trail_edge_vec = max_trail_edge - min_trail_edge;
        trail_edge_vec.normalize();

        vec3d lead_edge_vec = max_lead_edge - min_lead_edge;
        lead_edge_vec.normalize();

        vec3d inner_edge_vec = min_trail_edge - min_lead_edge;
        inner_edge_vec.normalize();

        vec3d outer_edge_vec = max_trail_edge - max_lead_edge;
        outer_edge_vec.normalize();

        // Determine angle between center and corner points
        vec3d center_to_le_in_vec = min_lead_edge - center;
        vec3d center_to_te_in_vec = min_trail_edge - center;
        vec3d center_to_le_out_vec = max_lead_edge - center;
        vec3d center_to_te_out_vec = max_trail_edge - center;

        center_to_le_in_vec.normalize();
        center_to_te_in_vec.normalize();
        center_to_le_out_vec.normalize();
        center_to_te_out_vec.normalize();

        double alpha_0 = PI / 2 + signed_angle( inner_edge_vec, center_to_outer_edge, wing_z_axis ); // Initial rotation

        double max_angle_inner_le = PI - signed_angle( center_to_le_in_vec, center_to_inner_edge, wing_z_axis );
        double max_angle_inner_te = -1 * PI - signed_angle( center_to_te_in_vec, center_to_inner_edge, wing_z_axis );
        double max_angle_outer_le = -1 * signed_angle( center_to_le_out_vec, center_to_outer_edge, wing_z_axis );
        double max_angle_outer_te = -1 * signed_angle( center_to_te_out_vec, center_to_outer_edge, wing_z_axis );

        double beta_te = signed_angle( center_to_outer_edge, trail_edge_vec, wing_z_axis );
        double beta_le = signed_angle( lead_edge_vec, center_to_inner_edge, wing_z_axis );

        double theta = DEG_2_RAD * m_Theta();

        double length_spar_in, length_spar_out, perp_dist;

        // Determine if the rib intersects the leading/trailing edge or inner/outer edge
        if ( theta >= 0 )
        {
            if ( theta > max_angle_inner_le )
            {
                perp_dist = cross( ( center - max_lead_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - max_lead_edge ).mag();
                length_spar_in = abs( perp_dist / sin( theta + beta_le ) );
            }
            else
            {
                perp_dist = cross( ( center - min_lead_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - min_lead_edge ).mag();
                length_spar_in = abs( perp_dist / cos( theta + alpha_0 ) );
            }

            if ( theta > max_angle_outer_te )
            {
                perp_dist = cross( ( center - max_trail_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - max_trail_edge ).mag();
                length_spar_out = abs( perp_dist / sin( theta - beta_te ) );
            }
            else
            {
                perp_dist = cross( ( center - max_lead_edge ), ( center - max_trail_edge ) ).mag() / ( max_trail_edge - max_lead_edge ).mag();
                length_spar_out = abs( perp_dist / cos( theta + alpha_0 ) );
            }
        }
        else
        {
            if ( theta < max_angle_inner_te )
            {
                perp_dist = cross( ( center - max_trail_edge ), ( center - min_trail_edge ) ).mag() / ( max_trail_edge - min_trail_edge ).mag();
                length_spar_in = abs( perp_dist / sin( theta - beta_te ) );
            }
            else
            {
                perp_dist = cross( ( center - min_lead_edge ), ( center - min_trail_edge ) ).mag() / ( min_trail_edge - min_lead_edge ).mag();
                length_spar_in = abs( perp_dist / cos( theta + alpha_0 ) );
            }

            if ( theta < max_angle_outer_le )
            {
                perp_dist = cross( ( center - max_lead_edge ), ( center - min_lead_edge ) ).mag() / ( max_lead_edge - min_lead_edge ).mag();
                length_spar_out = abs( perp_dist / sin( theta + beta_le ) );
            }
            else
            {
                perp_dist = cross( ( center - max_lead_edge ), ( center - max_trail_edge ) ).mag() / ( max_trail_edge - max_lead_edge ).mag();
                length_spar_out = abs( perp_dist / cos( theta + alpha_0 ) );
            }
        }

        // Apply Rodrigues' Rotation Formula
        vec3d spar_vec_in = center_to_inner_edge * cos( theta ) + cross( wing_z_axis, center_to_inner_edge ) * sin( theta ) + wing_z_axis * dot( wing_z_axis, center_to_inner_edge ) * ( 1 - cos( theta ) );
        vec3d spar_vec_out = center_to_outer_edge * cos( theta ) + cross( wing_z_axis, center_to_outer_edge ) * sin( theta ) + wing_z_axis * dot( wing_z_axis, center_to_outer_edge ) * ( 1 - cos( theta ) );

        spar_vec_in.normalize();
        spar_vec_out.normalize();

        // Calculate final end points
        vec3d inside_edge_f = center - length_spar_in * spar_vec_in;
        vec3d outside_edge_f = center + length_spar_out * spar_vec_out;

        // Identify corners of the plane
        vec3d cornerA, cornerB, cornerC, cornerD;

        cornerA = inside_edge_f + ( height * wing_z_axis );
        cornerB = inside_edge_f - ( height * wing_z_axis );
        cornerC = outside_edge_f + ( height * wing_z_axis );
        cornerD = outside_edge_f - ( height * wing_z_axis );

        // Make Planar Surface
        m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != wing_surf.GetFlipNormal() )
        {
            m_FeaPartSurfVec[0].FlipNormal();
        }

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, calculate and transform the symmetric copies
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }

        // Compute Symmetric Translation Matrix
        vector<Matrix4d> transMats = CalculateSymmetricTransform();

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

void FeaSpar::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

//////////////////////////////////////////////////////
//===================== FeaRib =====================//
//////////////////////////////////////////////////////

FeaRib::FeaRib( string geomID, int type ) : FeaPart( geomID, type )
{
    m_PerU.Init( "PerU", "FeaRib", this, 0.5, 0.0, 1.0 );
    m_PerU.SetDescript( "Precent U Location" );
    m_Theta.Init( "Theta", "FeaRib", this, 0.0, -90.0, 90.0 );

    m_PerpendicularEdgeIndex = 0;
}

void FeaRib::Update()
{
    UpdateSymmetricSurfs();
    ComputePlanarSurf();
}

void FeaRib::ComputePlanarSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_wing = veh->FindGeom( m_ParentGeomID );

        if ( !current_wing || m_FeaPartSurfVec.size() == 0 )
        {
            return;
        }

        m_FeaPartSurfVec[0] = VspSurf();
        m_FeaPartSurfVec[0].SetSurfCfdType( vsp::CFD_STRUCTURE );

        vector< VspSurf > surf_vec;
        current_wing->GetSurfVec( surf_vec );
        VspSurf wing_surf = surf_vec[m_MainSurfIndx()];

        BndBox wing_bbox;
        wing_surf.GetBoundingBox( wing_bbox );

        VspCurve constant_u_curve;
        wing_surf.GetU01ConstCurve( constant_u_curve, m_PerU() );

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

        vec3d chord_dir_vec = trail_edge - lead_edge;
        chord_dir_vec.normalize();

        vec3d center = ( trail_edge + lead_edge ) / 2;

        double alpha = 0.0;
        double u_edge_out = m_PerU() + 2 * FLT_EPSILON;
        double u_edge_in = m_PerU() - 2 * FLT_EPSILON;

        if ( m_PerpendicularEdgeIndex == PERPENDICULAR_TRAIL_EDGE )
        {
            vec3d trail_edge_out, trail_edge_in;
            trail_edge_out = wing_surf.CompPnt01( u_edge_out, v_min );
            trail_edge_in = wing_surf.CompPnt01( u_edge_in, v_min );

            vec3d lead_edge_dir_vec = trail_edge_out - trail_edge_in;
            lead_edge_dir_vec.normalize();

            alpha = ( PI / 2 ) - acos( dot( lead_edge_dir_vec, chord_dir_vec ) );
        }
        else if ( m_PerpendicularEdgeIndex == PERPENDICULAR_LEAD_EDGE )
        {
            vec3d lead_edge_out, lead_edge_in;
            lead_edge_out = wing_surf.CompPnt01( u_edge_out , v_leading_edge / v_max );
            lead_edge_in = wing_surf.CompPnt01( u_edge_in, v_leading_edge / v_max );

            vec3d trail_edge_dir_vec = lead_edge_out - lead_edge_in;
            trail_edge_dir_vec.normalize();

            alpha = ( PI / 2 ) - acos( dot( trail_edge_dir_vec, chord_dir_vec ) );
        }
        else if ( m_PerpendicularEdgeIndex > 2 )
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

                alpha = ( PI / 2 ) - acos( dot( spar_dir_vec, chord_dir_vec ) );
            }
            else
            {
                m_PerpendicularEdgeIndex = 0;
            }
        }

        vec3d center_to_trail_edge = trail_edge - center;
        center_to_trail_edge.normalize();

        vec3d center_to_lead_edge = center - lead_edge;
        center_to_lead_edge.normalize();

        double length_rib_0 = dist( trail_edge, lead_edge ) / 2; // Initial rib half length

        vec3d min_trail_edge = wing_surf.CompPnt( 0.0, 0.0 );
        vec3d min_lead_edge = wing_surf.CompPnt( 0.0, v_leading_edge );
        vec3d max_trail_edge = wing_surf.CompPnt( wing_surf.GetUMax(), 0.0 );
        vec3d max_lead_edge = wing_surf.CompPnt( wing_surf.GetUMax(), v_leading_edge );

        vec3d trail_edge_vec = trail_edge - min_trail_edge;
        trail_edge_vec.normalize();

        vec3d lead_edge_vec = lead_edge - min_lead_edge;
        lead_edge_vec.normalize();

        vec3d inner_edge_vec = min_trail_edge - min_lead_edge;
        inner_edge_vec.normalize();

        vec3d outer_edge_vec = max_trail_edge - max_lead_edge;
        outer_edge_vec.normalize();

        vec3d x_axis = vec3d( 1, 0, 0 );

        double alpha_0 = signed_angle( x_axis, inner_edge_vec, wing_z_axis ); // Initial rotation

        // Determine angle between center and corner points
        vec3d center_to_le_min_vec = min_lead_edge - center;
        vec3d center_to_te_min_vec = min_trail_edge - center;
        vec3d center_to_le_max_vec = max_lead_edge - center;
        vec3d center_to_te_max_vec = max_trail_edge - center;

        center_to_le_min_vec.normalize();
        center_to_te_min_vec.normalize();
        center_to_le_max_vec.normalize();
        center_to_te_max_vec.normalize();

        double max_angle_inner_le = -1 * PI - signed_angle( center_to_le_min_vec, center_to_lead_edge, wing_z_axis );
        double max_angle_inner_te = -1 * signed_angle( center_to_te_min_vec, center_to_trail_edge, wing_z_axis );
        double max_angle_outer_le = PI - signed_angle( center_to_le_max_vec, center_to_lead_edge, wing_z_axis );
        double max_angle_outer_te = -1 * signed_angle( center_to_te_max_vec, center_to_trail_edge, wing_z_axis );

        double theta = DEG_2_RAD * m_Theta();

        double beta_te = signed_angle( trail_edge_vec, center_to_trail_edge, wing_z_axis );
        double beta_le = signed_angle( lead_edge_vec, center_to_lead_edge, wing_z_axis );

        double phi_te = PI - ( theta + alpha + beta_te );
        double phi_le = PI - ( theta + alpha + beta_le );

        double length_rib_te, length_rib_le, perp_dist;

        // Determine if the rib intersects the leading/trailing edge or inner/outer edge
        if ( theta + alpha <= 0 )
        {
            if ( theta + alpha <= max_angle_inner_le )
            {
                perp_dist = cross( ( center - min_trail_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - min_trail_edge ).mag();
                length_rib_le = abs( perp_dist / sin( theta + alpha ) );
            }
            else
            {
                length_rib_le = abs( length_rib_0 * sin( beta_le ) / sin( phi_le ) );
            }

            if ( theta + alpha <= max_angle_outer_te )
            {
                perp_dist = cross( ( center - max_trail_edge ), ( center - max_lead_edge ) ).mag() / ( max_lead_edge - max_trail_edge ).mag();
                length_rib_te = abs( perp_dist / sin( theta + alpha ) );
            }
            else
            {
                length_rib_te = abs( length_rib_0 * sin( beta_te ) / sin( phi_te ) );
            }
        }
        else
        {
            if ( theta + alpha >= max_angle_inner_te )
            {
                perp_dist = cross( ( center - min_trail_edge ), ( center - min_lead_edge ) ).mag() / ( min_lead_edge - min_trail_edge ).mag();
                length_rib_te = abs( perp_dist / sin( theta + alpha ) );
            }
            else
            {
                length_rib_te = abs( length_rib_0 * sin( beta_te ) / sin( phi_te ) );
            }

            if ( theta + alpha >= max_angle_outer_le )
            {
                perp_dist = cross( ( center - max_trail_edge ), ( center - max_lead_edge ) ).mag() / ( max_lead_edge - max_trail_edge ).mag();
                length_rib_le = abs( perp_dist / sin( theta + alpha ) );
            }
            else
            {
                length_rib_le = abs( length_rib_0 * sin( beta_le ) / sin( phi_le ) );
            }
        }

        // Apply Rodrigues' Rotation Formula
        vec3d rib_vec_te = center_to_trail_edge * cos( theta + alpha  ) + cross( wing_z_axis, center_to_trail_edge ) * sin( theta + alpha ) + wing_z_axis * dot( wing_z_axis, center_to_trail_edge ) * ( 1 - cos( theta + alpha ) );
        vec3d rib_vec_le = center_to_lead_edge * cos( theta + alpha  ) + cross( wing_z_axis, center_to_lead_edge ) * sin( theta + alpha ) + wing_z_axis * dot( wing_z_axis, center_to_lead_edge ) * ( 1 - cos( theta + alpha ) );
        
        rib_vec_te.normalize();
        rib_vec_le.normalize();

        // Calculate final end points
        vec3d trail_edge_f = center + length_rib_te * rib_vec_te;
        vec3d lead_edge_f = center - length_rib_le * rib_vec_le;

        // Identify corners of the plane
        vec3d cornerA, cornerB, cornerC, cornerD;

        double height = 0.5 * wing_bbox.GetSmallestDist();

        cornerA = trail_edge_f + ( height * wing_z_axis );
        cornerB = trail_edge_f - ( height * wing_z_axis );
        cornerC = lead_edge_f + ( height * wing_z_axis );
        cornerD = lead_edge_f - ( height * wing_z_axis );

        // Make Planar Surface
        m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != wing_surf.GetFlipNormal() )
        {
            m_FeaPartSurfVec[0].FlipNormal();
        }

        // Using the primary m_FeaPartSurfVec (index 0) as a reference, calculate and transform the symmetric copies
        for ( unsigned int j = 1; j < m_SymmIndexVec.size(); j++ )
        {
            m_FeaPartSurfVec[j] = m_FeaPartSurfVec[j - 1];
        }

        // Compute Symmetric Translation Matrix
        vector<Matrix4d> transMats = CalculateSymmetricTransform();

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

void FeaRib::UpdateDrawObjs( int id, bool highlight )
{
    FeaPart::UpdateDrawObjs( id, highlight );
}

////////////////////////////////////////////////////
//================= FeaFixPoint ==================//
////////////////////////////////////////////////////

FeaFixPoint::FeaFixPoint( string compID, int type ) : FeaPart( compID, type )
{
    m_PosU.Init( "PosU", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosU.SetDescript( "Precent U Location" );
    m_PosW.Init( "PosW", "FeaFixPoint", this, 0.0, 0.0, 1.0 );
    m_PosW.SetDescript( "Precent W Location" );

    m_FeaPropertyIndex = -1; // No property
    m_CapFeaPropertyIndex = -1; // No property
    m_BorderFlag = false;
    m_HalfMeshFlag = false;
}

void FeaFixPoint::Update()
{
    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        parent_part->Update(); // Update Parent FeaPart Surface
    }

    UpdateSymmetricSurfs();
    IdentifySplitSurfIndex();

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

void FeaFixPoint::IdentifySplitSurfIndex()
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

        // Check if the UW point is on a valid patch (invalid patches are discarded in FetchXFerSurf)
        bool on_valid_patch = parent_surf_vec[i].FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx(), 0, tempxfersurfs, uw[0], uw[1] );

        int num_split_surfs = tempxfersurfs.size();

        for ( size_t j = 0; j < num_split_surfs; j++ )
        {
            bool addSurfFlag = true;
            if ( m_HalfMeshFlag && LessThanY( tempxfersurfs[j].m_Surface, 1e-6 ) )
            {
                addSurfFlag = false;
            }

            if ( m_HalfMeshFlag && PlaneAtYZero( tempxfersurfs[j].m_Surface ) )
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
                else if ( ( closedU && umax == parent_Umax && uw.x() == parent_Umin ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant U border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( uw.x() > umin && uw.x() < umax ) && ( closedW && vmax == parent_Wmax && uw.y() == parent_Wmin ) ) // FeaFixPoint on constant W border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( uw.x() > umin && uw.x() < umax ) && ( closedW && vmin == parent_Wmin && uw.y() == parent_Wmax ) ) // FeaFixPoint on constant W border
                {
                    m_SplitSurfIndex[i].push_back( j + i * num_split_surfs );
                    m_BorderFlag = true;
                }
                else if ( ( closedU && umin == parent_Umin && uw.y() == parent_Umax ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant W border
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

////////////////////////////////////////////////////
//=================== FeaSkin ====================//
////////////////////////////////////////////////////

FeaSkin::FeaSkin( string geomID, int type ) : FeaPart( geomID, type )
{

}

void FeaSkin::Update()
{
    UpdateSymmetricSurfs();
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
            for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
            {
                vector< VspSurf > surf_vec;
                currgeom->GetSurfVec( surf_vec );

                m_FeaPartSurfVec[i] = surf_vec[m_SymmIndexVec[i]];

                m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_NORMAL );
            }
        }
    }
}

////////////////////////////////////////////////////
//================= FeaProperty ==================//
////////////////////////////////////////////////////

FeaProperty::FeaProperty() : ParmContainer()
{
    m_FeaPropertyType.Init( "FeaPropertyType", "FeaProperty", this, SHELL_PROPERTY, SHELL_PROPERTY, BEAM_PROPERTY );
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

    m_FeaMaterialIndex = 0;
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

    XmlUtil::AddIntNode( prop_info, "FeaMaterialIndex", m_FeaMaterialIndex );

    return prop_info;
}

xmlNodePtr FeaProperty::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    m_FeaMaterialIndex = XmlUtil::FindInt( node, "FeaMaterialIndex", m_FeaMaterialIndex );

    return node;
}

string FeaProperty::GetTypeName( )
{
    if ( m_FeaPropertyType() == SHELL_PROPERTY )
    {
        return string( "Shell" );
    }
    if ( m_FeaPropertyType() == BEAM_PROPERTY )
    {
        return string( "Beam" );
    }

    return string( "NONE" );
}

void FeaProperty::WriteNASTRAN( FILE* fp, int prop_id )
{
    if ( m_FeaPropertyType() == SHELL_PROPERTY )
    {
        fprintf( fp, "PSHELL,%d,%d,%f\n", prop_id, m_FeaMaterialIndex + 1, m_Thickness() );
    }
    if ( m_FeaPropertyType() == BEAM_PROPERTY )
    {
        fprintf( fp, "PBEAM,%d,%d,%f,%f,%f,%f,%f\n", prop_id, m_FeaMaterialIndex + 1, m_CrossSecArea(), m_Izz(), m_Iyy(), m_Izy(), m_Ixx() );
    }
}

void FeaProperty::WriteCalculix( FILE* fp, string ELSET )
{
    FeaMaterial* fea_mat = StructureMgr.GetFeaMaterial( m_FeaMaterialIndex );

    if ( fea_mat )
    {
        if ( m_FeaPropertyType() == SHELL_PROPERTY )
        {
            fprintf( fp, "*SHELL SECTION, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), fea_mat->GetName().c_str() );
            fprintf( fp, "%g\n", m_Thickness() );
        }
        if ( m_FeaPropertyType() == BEAM_PROPERTY )
        {
            // Note: *BEAM GENERAL SECTION is supported by Abaqus but not Calculix. Calculix depends on BEAM SECTION properties
            //  where the cross-section dimensions must be explicitly defined. 
            fprintf( fp, "*BEAM GENERAL SECTION, SECTION=GENERAL, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), fea_mat->GetName().c_str() );
            fprintf( fp, "%g,%g,%g,%g,%g\n", m_CrossSecArea(), m_Izz(), m_Izy(), m_Iyy(), m_Ixx() );
        }
    }
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

void FeaMaterial::WriteNASTRAN( FILE* fp, int mat_id )
{
    fprintf( fp, "MAT1,%d,%g,%g,%g,%g,%g\n", mat_id, m_ElasticModulus(), GetShearModulus(), m_PoissonRatio(), m_MassDensity(), m_ThermalExpanCoeff() );
}

void FeaMaterial::WriteCalculix( FILE* fp, int mat_id )
{
    fprintf( fp, "*MATERIAL, NAME=%s\n", GetName().c_str() );
    fprintf( fp, "*DENSITY\n" );
    fprintf( fp, "%g\n", m_MassDensity() );
    fprintf( fp, "*ELASTIC, TYPE=ISO\n" );
    fprintf( fp, "%g,%g\n", m_ElasticModulus(), m_PoissonRatio() );
    fprintf( fp, "*EXPANSION, TYPE=ISO\n" );
    fprintf( fp, "%g\n", m_ThermalExpanCoeff() );
}

double FeaMaterial::GetShearModulus()
{
    return ( m_ElasticModulus() / ( 2 * ( m_PoissonRatio() + 1 ) ) );
}
