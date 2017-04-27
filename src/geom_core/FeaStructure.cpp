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

//==== Highlight Active Subsurface ====//
void FeaStructure::RecolorFeaSubSurfs( int active_ind )
{
    for ( int i = 0; i < (int)m_FeaSubSurfVec.size(); i++ )
    {
        if ( i == active_ind )
        {
            m_FeaSubSurfVec[i]->SetLineColor( vec3d( 1, 0, 0 ) );
        }
        else
        {
            m_FeaSubSurfVec[i]->SetLineColor( vec3d( 0, 0, 0 ) );
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
        m_FeaPartVec[i]->Update();
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

    m_IntersectionCapFlag.Init( "IntersectionCapFlag", "FeaPart", this, false, false, true );
    m_IntersectionCapFlag.SetDescript( "Flag to Identify FeaElements at Intersections" );

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

    ParmContainer::EncodeXml( part_info );

    XmlUtil::AddIntNode( part_info, "FeaPartType", m_FeaPartType );
    XmlUtil::AddIntNode( part_info, "FeaPropertyIndex", m_FeaPropertyIndex );
    XmlUtil::AddIntNode( part_info, "CapFeaPropertyIndex", m_CapFeaPropertyIndex );

    return part_info;
}

xmlNodePtr FeaPart::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );

    return node;
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

void FeaPart::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    m_FeaPartDO.clear();
    m_FeaPartDO.resize( m_FeaPartSurfVec.size() );

    for ( unsigned int j = 0; j < m_FeaPartSurfVec.size(); j++ )
    {
        m_FeaPartDO[j].m_PntVec.clear();

        m_FeaPartDO[j].m_GeomID = string( "FeaPart_" + std::to_string( id ) + "_" + std::to_string( j ) );
        m_FeaPartDO[j].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_FeaPartDO[j].m_LineWidth = 1.0;

        if ( highlight )
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
        }
        else
        {
            m_FeaPartDO[j].m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
        }

        m_FeaPartDO[j].m_Type = DrawObj::VSP_LINES;

        vec3d p00 = m_FeaPartSurfVec[j].CompPnt01( 0, 0 );
        vec3d p10 = m_FeaPartSurfVec[j].CompPnt01( 1, 0 );
        vec3d p11 = m_FeaPartSurfVec[j].CompPnt01( 1, 1 );
        vec3d p01 = m_FeaPartSurfVec[j].CompPnt01( 0, 1 );

        for ( int i = 0; i < 4; i++ )
        {
            double fu = (double)i / 3.0;
            vec3d p0 = p00 + ( p10 - p00 )*fu;
            vec3d p1 = p01 + ( p11 - p01 )*fu;

            m_FeaPartDO[j].m_PntVec.push_back( p0 );
            m_FeaPartDO[j].m_PntVec.push_back( p1 );
        }
        for ( int i = 0; i < 4; i++ )
        {
            double fw = (double)i / 3.0;
            vec3d p0 = p00 + ( p01 - p00 )*fw;
            vec3d p1 = p10 + ( p11 - p10 )*fw;

            m_FeaPartDO[j].m_PntVec.push_back( p0 );
            m_FeaPartDO[j].m_PntVec.push_back( p1 );
        }

        m_FeaPartDO[j].m_GeomChanged = true;

        draw_obj_vec.push_back( &m_FeaPartDO[j] );
    }
}

vector < VspSurf* > FeaPart::GetFeaPartSurfPtrVec()
{
    vector < VspSurf* > ptr_vec;
    ptr_vec.resize( m_FeaPartSurfVec.size() );

    for ( size_t i = 0; i < m_FeaPartSurfVec.size(); i++ )
    {
        ptr_vec[i] = &m_FeaPartSurfVec[i];
    }

    return ptr_vec;
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
    m_IncludeTrisFlag.Init( "IncludeTrisFlag", "FeaFullDepth", this, true, false, true );
    m_IncludeTrisFlag.SetDescript( "Flag to Include Interior Tris" );

    m_OrientationPlane.Init( "OrientationPlane", "FeaFullDepth", this, XY_PLANE, XY_PLANE, XZ_PLANE );
    m_OrientationPlane.SetDescript( "Plane the FeaFullDepth Part will be Parallel to" );

    m_CenterPerBBoxLocation.Init( "CenterPerBBoxLocation", "FeaFullDepth", this, 0.5, 0.0, 1.0 );
    m_CenterPerBBoxLocation.SetDescript( "The Location of the Center of the FeaFullDepth Part as a Percentage of the Total Bounding Box" );

    m_Theta.Init( "Theta", "FeaFullDepth", this, 0.0, -90.0, 90.0 );
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

        if ( m_IncludeTrisFlag() )
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
        vec3d rot_axis;

        if ( m_OrientationPlane() == XY_PLANE )
        {
            vec3d center_to_A = { -0.5 * del_x, -0.5 * del_y, 0.0 };
            cornerA = geom_center + center_to_A;

            vec3d center_to_B = { -0.5 * del_x, 0.5 * del_y, 0.0 };
            cornerB = geom_center + center_to_B;

            vec3d center_to_C = { 0.5 * del_x, -0.5 * del_y, 0.0 };
            cornerC = geom_center + center_to_C;

            vec3d center_to_D = { 0.5 * del_x, 0.5 * del_y, 0.0 };
            cornerD = geom_center + center_to_D;

            rot_axis.set_y( 1.0 ); // y-axis
        }
        else if ( m_OrientationPlane() == YZ_PLANE )
        {
            vec3d center_to_A = { 0.0, -0.5 * del_y, -0.5 * del_z };
            cornerA = geom_center + center_to_A;

            vec3d center_to_B = { 0.0, 0.5 * del_y, -0.5 * del_z };
            cornerB = geom_center + center_to_B;

            vec3d center_to_C = { 0.0, -0.5 * del_y, 0.5 * del_z };
            cornerC = geom_center + center_to_C;

            vec3d center_to_D = { 0.0, 0.5 * del_y, 0.5 * del_z };
            cornerD = geom_center + center_to_D;

            rot_axis.set_y( 1.0 ); // y-axis
        }
        else if ( m_OrientationPlane() == XZ_PLANE )
        {
            vec3d center_to_A = { -0.5 * del_x, 0.0, -0.5 * del_z };
            cornerA = geom_center + center_to_A;

            vec3d center_to_B = { 0.5 * del_x, 0.0, -0.5 * del_z };
            cornerB = geom_center + center_to_B;

            vec3d center_to_C = { -0.5 * del_x, 0.0, 0.5 * del_z };
            cornerC = geom_center + center_to_C;

            vec3d center_to_D = { 0.5 * del_x, 0.0, 0.5 * del_z };
            cornerD = geom_center + center_to_D;

            rot_axis.set_z( 1.0 ); // z-axis
        }

        // Make Planar Surface
        m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != current_surf.GetFlipNormal() )
        {
            m_FeaPartSurfVec[0].FlipNormal();
            rot_axis = -1 * rot_axis;
        }

        // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
        Matrix4d trans_mat_1, trans_mat_2, rot_mat;

        trans_mat_1.loadIdentity();
        trans_mat_1.translatef( geom_center.x() * -1, geom_center.y() * -1, geom_center.z() * -1 );
        m_FeaPartSurfVec[0].Transform( trans_mat_1 );

        rot_mat.loadIdentity();
        rot_mat.rotate( DEG_2_RAD * m_Theta(), rot_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat );

        trans_mat_2.loadIdentity();

        if ( m_OrientationPlane() == XY_PLANE )
        {
            trans_mat_2.translatef( geom_center.x(), geom_center.y(), geom_bbox.GetMin( 2 ) + del_z * m_CenterPerBBoxLocation() );
        }
        else if ( m_OrientationPlane() == YZ_PLANE )
        {
            trans_mat_2.translatef( geom_bbox.GetMin( 0 ) + del_x * m_CenterPerBBoxLocation(), geom_center.y(), geom_center.z() );
        }
        else if ( m_OrientationPlane() == XZ_PLANE )
        {
            trans_mat_2.translatef( geom_center.x(), geom_bbox.GetMin( 1 ) + del_y * m_CenterPerBBoxLocation(), geom_center.z() );
        }

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

void FeaSlice::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    FeaPart::LoadDrawObjs( draw_obj_vec, id, highlight );
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

        // Find two points slightly above and below the trailing edge
        double v_trail_edge_low = v_min + 2 * TMAGIC;
        double v_trail_edge_up = v_max - 2 * TMAGIC;

        vec3d trail_edge_up, trail_edge_low;
        trail_edge_up = u_curve.f( v_trail_edge_low );
        trail_edge_low = u_curve.f( v_trail_edge_up );

        vec3d z_axis = trail_edge_up - trail_edge_low;
        z_axis.normalize();

        // Identify corners of the plane and rotation axis
        vec3d cornerA, cornerB, cornerC, cornerD;

        double height = 0.5 * wing_bbox.GetSmallestDist();

        cornerA = inside_edge + ( height * z_axis );
        cornerB = inside_edge - ( height * z_axis );
        cornerC = outside_edge + ( height * z_axis );
        cornerD = outside_edge - ( height * z_axis );

        // Make Planar Surface
        m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != wing_surf.GetFlipNormal() )
        {
            z_axis = -1 * z_axis;
            m_FeaPartSurfVec[0].FlipNormal();
        }

        vec3d center = ( trail_edge_up + trail_edge_low ) / 2;

        // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
        Matrix4d trans_mat_1, trans_mat_2, rot_mat;

        trans_mat_1.loadIdentity();
        trans_mat_1.translatef( center.x() * -1, center.y() * -1, center.z() * -1 );
        m_FeaPartSurfVec[0].Transform( trans_mat_1 );

        rot_mat.loadIdentity();
        rot_mat.rotate( DEG_2_RAD * m_Theta(), z_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat );

        trans_mat_2.loadIdentity();
        trans_mat_2.translatef( center.x(), center.y(), center.z() );
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

void FeaSpar::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    FeaPart::LoadDrawObjs( draw_obj_vec, id, highlight );
}

//////////////////////////////////////////////////////
//===================== FeaRib =====================//
//////////////////////////////////////////////////////

FeaRib::FeaRib( string geomID, int type ) : FeaPart( geomID, type )
{
    m_PerU.Init( "PerU", "FeaRib", this, 0.5, 0.0, 1.0 );
    m_PerU.SetDescript( "Precent U Location" );
    m_Theta.Init( "Theta", "FeaRib", this, 0.0, -90.0, 90.0 );

    m_PerpendicularEdgeFlag.Init( "PerpendicularEdgeFlag", "FeaRib", this, PERPENDICULAR_NONE, PERPENDICULAR_NONE, PERPENDICULAR_TRAIL_EDGE );
    m_PerpendicularEdgeFlag.SetDescript( "Flag Indicating Perpendicular Alignment of FeaRib" );
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

        vec3d z_axis = trail_edge_up - trail_edge_low;
        z_axis.normalize();

        vec3d chord_dir_vec = trail_edge - lead_edge;
        chord_dir_vec.normalize();

        // Identify corners of the plane and rotation axis
        vec3d cornerA, cornerB, cornerC, cornerD;

        double height = 0.5 * wing_bbox.GetSmallestDist();

        cornerA = trail_edge + ( height * z_axis );
        cornerB = trail_edge - ( height * z_axis );
        cornerC = lead_edge + ( height * z_axis );
        cornerD = lead_edge - ( height * z_axis );

        // Make Planar Surface
        m_FeaPartSurfVec[0].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

        if ( m_FeaPartSurfVec[0].GetFlipNormal() != wing_surf.GetFlipNormal() )
        {
            z_axis = -1 * z_axis;
            m_FeaPartSurfVec[0].FlipNormal();
        }

        vec3d center = ( trail_edge + lead_edge ) / 2;

        double alpha = 0.0;
        double u_edge_out = m_PerU() + 2 * FLT_EPSILON;
        double u_edge_in = m_PerU() - 2 * FLT_EPSILON;

        if ( m_PerpendicularEdgeFlag() == PERPENDICULAR_TRAIL_EDGE )
        {
            vec3d trail_edge_out, trail_edge_in;
            trail_edge_out = wing_surf.CompPnt01( u_edge_out, v_min );
            trail_edge_in = wing_surf.CompPnt01( u_edge_in, v_min );

            vec3d lead_edge_dir_vec = trail_edge_out - trail_edge_in;
            lead_edge_dir_vec.normalize();

            alpha = ( PI / 2 ) - acos( dot( lead_edge_dir_vec, chord_dir_vec ) );
        }
        else if ( m_PerpendicularEdgeFlag() == PERPENDICULAR_LEAD_EDGE )
        {
            vec3d lead_edge_out, lead_edge_in;
            lead_edge_out = wing_surf.CompPnt01( u_edge_out , v_leading_edge / v_max );
            lead_edge_in = wing_surf.CompPnt01( u_edge_in, v_leading_edge / v_max );

            vec3d trail_edge_dir_vec = lead_edge_out - lead_edge_in;
            trail_edge_dir_vec.normalize();

            alpha = ( PI / 2 ) - acos( dot( trail_edge_dir_vec, chord_dir_vec ) );
        }

        // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
        Matrix4d trans_mat_1, trans_mat_2, rot_mat;

        trans_mat_1.loadIdentity();
        trans_mat_1.translatef( center.x() * -1, center.y() * -1, center.z() * -1 );
        m_FeaPartSurfVec[0].Transform( trans_mat_1 );

        rot_mat.loadIdentity();
        rot_mat.rotate( DEG_2_RAD * m_Theta() + alpha, z_axis );
        m_FeaPartSurfVec[0].Transform( rot_mat );

        trans_mat_2.loadIdentity();
        trans_mat_2.translatef( center.x(), center.y(), center.z() );
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

void FeaRib::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    FeaPart::LoadDrawObjs( draw_obj_vec, id, highlight );
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
}

void FeaFixPoint::Update()
{
    UpdateSymmetricSurfs();
    IdentifySplitSurfIndex();

    m_FeaPartSurfVec.clear(); // FeaFixPoints are not a VspSurf
}

void FeaFixPoint::IdentifySplitSurfIndex()
{
    // This function is called instead of FeaPart::FetchFeaXFerSurf when the FeaPart type is FEA_FIX_POINT, since 
    //  FeaFixPoints are not surfaces. This function determines the number of split surfaces for the FeaFixPoint 
    //  Parent Surface, and determines which split surface the FeaFixPoint lies on.

    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( !parent_part )
    {
        return;
    }

    vector < VspSurf* > parent_surf_vec = parent_part->GetFeaPartSurfPtrVec();

    m_SplitSurfIndex.clear();
    m_BorderFlag = false;

    if ( parent_surf_vec.size() > 0 )
    {
        // Get U/W values
        vec2d uw = GetUW();

        double parent_Umax = parent_surf_vec[0]->GetUMax();
        double parent_Wmax = parent_surf_vec[0]->GetWMax();

        // Check if U/W is closed, in which case the minimum and maximum U/W will be at the same point
        bool closedU = parent_surf_vec[0]->IsClosedU();
        bool closedW = parent_surf_vec[0]->IsClosedW();

        // Split the parent surface
        vector< XferSurf > tempxfersurfs;
        parent_surf_vec[0]->FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx(), 0, tempxfersurfs );

        for ( size_t j = 0; j < tempxfersurfs.size(); j++ )
        {
            double umax = tempxfersurfs[j].m_Surface.get_umax();
            double umin = tempxfersurfs[j].m_Surface.get_u0();
            double vmax = tempxfersurfs[j].m_Surface.get_vmax();
            double vmin = tempxfersurfs[j].m_Surface.get_v0();

            // Check if FeaFixPoint is on XferSurf or border curve
            if ( uw.x() > umin && uw.x() < umax && uw.y() > vmin && uw.y() < vmax ) // FeaFixPoint on surface
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = false;
            }
            else if ( ( uw.x() > umin && uw.x() < umax ) && ( uw.y() == vmin || uw.y() == vmax ) ) // FeaFixPoint on constant W border
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
            }
            else if ( ( uw.x() == umin || uw.x() == umax ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant U border
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
            }
            else if ( ( closedU && umax == parent_Umax && uw.x() == 0.0 ) && ( uw.y() > vmin && uw.y() < vmax ) ) // FeaFixPoint on constant U border
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
            }
            else if ( ( uw.x() > umin && uw.x() < umax ) && ( closedW && vmax == parent_Wmax && uw.y() == 0.0 ) ) // FeaFixPoint on constant W border
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
            }
            else if ( ( uw.x() == umin || uw.x() == umax ) && ( uw.y() == vmin || uw.y() == vmax ) ) // FeaFixPoint on constant UW intersection (already a node)
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
            }
            else if ( ( closedU && umax == parent_Umax && uw.x() == 0.0 ) && ( uw.y() == vmin || uw.y() == vmax ) ) // FeaFixPoint on constant UW intersection (already a node)
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
            }
            else if ( ( uw.x() == umin || uw.x() == umax ) && ( closedW && vmax == parent_Wmax && uw.y() == 0.0 ) ) // FeaFixPoint on constant UW intersection (already a node)
            {
                m_SplitSurfIndex.push_back( j );
                m_BorderFlag = true;
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
        vector < VspSurf* > parent_surf_vec = parent_part->GetFeaPartSurfPtrVec();
        pnt_vec.resize( parent_surf_vec.size() );

        for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
        {
            if ( parent_surf_vec[i] )
            {
                pnt_vec[i] = parent_surf_vec[i]->CompPnt01( m_PosU(), m_PosW() );
            }
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
        vector < VspSurf* > parent_surf_vec = parent_part->GetFeaPartSurfPtrVec();

        if ( parent_surf_vec[0] ) // Only consider main parent surface (same UW for symmetric copies)
        { 
            uw.set_x( parent_surf_vec[0]->GetUMax() * m_PosU() );
            uw.set_y( parent_surf_vec[0]->GetWMax() * m_PosW() );
        }
    }
    return uw;
}

void FeaFixPoint::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    FeaPart* parent_part = StructureMgr.GetFeaPart( m_ParentFeaPartID );

    if ( parent_part )
    {
        vector < VspSurf* > parent_surf_vec = parent_part->GetFeaPartSurfPtrVec();

        for ( size_t i = 0; i < parent_surf_vec.size(); i++ )
        {
            m_FeaPartDO.resize( parent_surf_vec.size() );

            if ( parent_surf_vec[i] )
            {
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

                vec3d fixpt = parent_surf_vec[i]->CompPnt01( m_PosU(), m_PosW() );
                m_FeaPartDO[i].m_PntVec.push_back( fixpt );

                m_FeaPartDO[i].m_GeomChanged = true;

                draw_obj_vec.push_back( &m_FeaPartDO[i] );
            }
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
