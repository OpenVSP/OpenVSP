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

//==== Generate Unique ID ====//
string GenerateID()
{
    return ParmMgr.GenerateID( 8 );
}

//////////////////////////////////////////////////////
//================== FeaStructure ==================//
//////////////////////////////////////////////////////

FeaStructure::FeaStructure( string geomID, int surf_index )
{
    m_ParentGeomID = geomID;
    m_MainSurfIndx = surf_index;

    m_FeaPartCount = 0;
    m_FeaSubSurfCount = 0;

    m_FeaStructID = GenerateID();
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
    XmlUtil::AddStringNode( fea_info, "FeaStructID", m_FeaStructID );
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

            if ( type != vsp::FEA_SKIN )
            {
                FeaPart* feapart = AddFeaPart( type );

                feapart->SetFeaPropertyIndex( prop_index );

                feapart->DecodeXml( part_info );
            }
            else
            {
                FeaPart* feaskin = new FeaSkin( m_ParentGeomID );
                feaskin->DecodeXml( part_info );

                feaskin->SetFeaPropertyIndex( prop_index );

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

    if ( type == vsp::FEA_FULL_DEPTH )
    {
        feaprt = new FeaFullDepth( m_ParentGeomID );
        feaprt->SetName( string( "FEA_FULL_DEPTH_" + std::to_string( m_FeaPartCount ) ) );
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
    else if ( type == vsp::FEA_STIFFENER_PLANE )
    {
        feaprt = new FeaStiffenerPlane( m_ParentGeomID );
        feaprt->SetName( string( "FEA_STIFFENER_PLANE_" + std::to_string( m_FeaPartCount ) ) );
    }
    else if ( type == vsp::FEA_STIFFENER_SUB_SURF )
    {
        feaprt = new FeaStiffenerSubSurf( m_ParentGeomID );
        feaprt->SetName( string( "FEA_STIFFENER_SUB_SURF_" + std::to_string( m_FeaPartCount ) ) );
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

                m_FeaPartVec.push_back( feaskin );
                //m_FeaPartCount++; // ?
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

FeaPart* FeaStructure::GetFeaPart( const string & id )
{
    for ( int i = 0; i < (int)m_FeaPartVec.size(); i++ )
    {
        if ( m_FeaPartVec[i]->GetID() == id )
        {
            if ( ValidFeaPartInd( i ) )
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
            if ( ValidFeaPartInd( i ) )
                return i;
        }
    }
    return -1;
}

//////////////////////////////////////////////////////
//==================== FeaPart =====================//
//////////////////////////////////////////////////////

FeaPart::FeaPart( string geomID, int type )
{
    m_FeaPartType = type;
    m_ParentGeomID = geomID;

    m_MainSurfIndx.Init( "MainSurfIndx", "FeaStructure", this, -1, -1, 1e12 );
    m_MainSurfIndx.SetDescript( "Surface Index for FeaStructure" );

    m_FeaPropertyIndex = 0; // Shell property default
}

FeaPart::~FeaPart()
{

}

void FeaPart::Update()
{
    UpdateSymmetricSurfs();
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

string FeaPart::GetTypeName( int type )
{
    if ( type == vsp::FEA_FULL_DEPTH )
    {
        return string( "FeaFullDepth" );
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
    if ( type == vsp::FEA_STIFFENER_PLANE )
    {
        return string( "FeaStiffenerPlane" );
    }
    if ( type == vsp::FEA_STIFFENER_SUB_SURF )
    {
        return string( "FeaStiffenerSubSurf" );
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
        // CFD_STRUCTURE type surfaces have m_CompID == -9999
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

int FeaPart::GetFeaMaterialIndex()
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex );

    return fea_prop->GetFeaMaterialIndex();

}

void FeaPart::SetFeaMaterialIndex( int index )
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex );

    fea_prop->SetFeaMaterialIndex( index );
}

//////////////////////////////////////////////////////
//================== FeaFullDepth ==================//
//////////////////////////////////////////////////////

FeaFullDepth::FeaFullDepth( string geomID, int type ) : FeaPart( geomID, type )
{
    m_OrientationPlane.Init( "OrientationPlane", "FeaFullDepth", this, XY_PLANE, XY_PLANE, XZ_PLANE );
    m_OrientationPlane.SetDescript( "Plane the FeaFullDepth Part will be Parallel to" );

    m_CenterPerBBoxLocation.Init( "CenterPerBBoxLocation", "FeaFullDepth", this, 0.5, 0.0, 1.0 );
    m_CenterPerBBoxLocation.SetDescript( "The Location of the Center of the FeaFullDepth Part as a Percentage of the Total Bounding Box" );

    m_Theta.Init( "Theta", "FeaFullDepth", this, 0.0, -90.0, 90.0 );
}

void FeaFullDepth::Update()
{
    ComputePlanarSurf();
}

void FeaFullDepth::ComputePlanarSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i] = VspSurf();
            m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );

            Geom* current_geom = veh->FindGeom( m_ParentGeomID );

            vector< VspSurf > surf_vec;
            current_geom->GetSurfVec( surf_vec );
            VspSurf current_surf = surf_vec[m_SymmIndexVec[i]];

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
            m_FeaPartSurfVec[i].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
            Matrix4d trans_mat_1, trans_mat_2, rot_mat;

            trans_mat_1.loadIdentity();
            trans_mat_1.translatef( geom_center.x() * -1, geom_center.y() * -1, geom_center.z() * -1 );
            m_FeaPartSurfVec[i].Transform( trans_mat_1 );

            rot_mat.loadIdentity();
            rot_mat.rotate( DEG_2_RAD * m_Theta(), rot_axis );
            m_FeaPartSurfVec[i].Transform( rot_mat );

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

            m_FeaPartSurfVec[i].Transform( trans_mat_2 );
        }
    }
}

void FeaFullDepth::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
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
    //m_Alpha.Init( "Alpha", "FeaSpar", this, 0.0, -90.0, 90.0 );
    m_Theta.Init( "Theta", "FeaSpar", this, 0.0, -90.0, 90.0 );
    //m_TrimFlag.Init( "TrimFlag", "FeaSpar", this, true, 0, 1 );

    //m_PlaneSurfLengthScale.Init( "PlaneSurfLengthScale", "FeaSpar", this, 1.0, 0.0, 1.0e12 );
    //m_PlaneSurfWidthScale.Init( "PlaneSurfWidthScale", "FeaSpar", this, 1.0, 0.0, 1.0e12 );

}

void FeaSpar::Update()
{
    ComputePlanarSurf();
}

void FeaSpar::ComputePlanarSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i] = VspSurf();
            m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );

            Geom* current_wing = veh->FindGeom( m_ParentGeomID );

            vector< VspSurf > surf_vec;
            current_wing->GetSurfVec( surf_vec );
            VspSurf wing_surf = surf_vec[m_SymmIndexVec[i]];

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
            m_FeaPartSurfVec[i].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            if ( m_FeaPartSurfVec[i].GetFlipNormal() != wing_surf.GetFlipNormal() )
            {
                z_axis = -1 * z_axis;
                m_FeaPartSurfVec[i].FlipNormal();
            }

            vec3d center = ( trail_edge_up + trail_edge_low ) / 2;

            // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
            Matrix4d trans_mat_1, trans_mat_2, rot_mat;

            trans_mat_1.loadIdentity();
            trans_mat_1.translatef( center.x() * -1, center.y() * -1, center.z() * -1 );
            m_FeaPartSurfVec[i].Transform( trans_mat_1 );

            rot_mat.loadIdentity();
            rot_mat.rotate( DEG_2_RAD * m_Theta(), z_axis );
            m_FeaPartSurfVec[i].Transform( rot_mat );

            trans_mat_2.loadIdentity();
            trans_mat_2.translatef( center.x(), center.y(), center.z() );
            m_FeaPartSurfVec[i].Transform( trans_mat_2 );
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
    //m_Alpha.Init( "Alpha", "FeaRib", this, 0.0, -90.0, 90.0 );
    m_Theta.Init( "Theta", "FeaRib", this, 0.0, -90.0, 90.0 );
    //m_TrimFlag.Init( "TrimFlag", "FeaRib", this, true, 0, 1 );

    //m_PlaneSurfLengthScale.Init( "PlaneSurfLengthScale", "FeaRib", this, 1.0, 0.0, 1.0e12 );
    //m_PlaneSurfWidthScale.Init( "PlaneSurfWidthScale", "FeaRib", this, 1.0, 0.0, 1.0e12 );

    m_PerpendicularEdgeFlag.Init( "PerpendicularEdgeFlag", "FeaRib", this, PERPENDICULAR_NONE, PERPENDICULAR_NONE, PERPENDICULAR_TRAIL_EDGE );
    m_PerpendicularEdgeFlag.SetDescript( "Flag Indicating Perpendicular Alignment of FeaRib" );
}

void FeaRib::Update()
{
    ComputePlanarSurf();
}

void FeaRib::ComputePlanarSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i] = VspSurf();
            m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );

            Geom* current_wing = veh->FindGeom( m_ParentGeomID );

            vector< VspSurf > surf_vec;
            current_wing->GetSurfVec( surf_vec );
            VspSurf wing_surf = surf_vec[m_SymmIndexVec[i]];

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
            m_FeaPartSurfVec[i].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            if ( m_FeaPartSurfVec[i].GetFlipNormal() != wing_surf.GetFlipNormal() )
            {
                z_axis = -1 * z_axis;
                m_FeaPartSurfVec[i].FlipNormal();
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
            m_FeaPartSurfVec[i].Transform( trans_mat_1 );

            rot_mat.loadIdentity();
            rot_mat.rotate( DEG_2_RAD * m_Theta() + alpha, z_axis );
            m_FeaPartSurfVec[i].Transform( rot_mat );

            trans_mat_2.loadIdentity();
            trans_mat_2.translatef( center.x(), center.y(), center.z() );
            m_FeaPartSurfVec[i].Transform( trans_mat_2 );
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

}

void FeaFixPoint::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* currgeom = veh->FindGeom( m_ParentGeomID );

        if ( currgeom )
        {
            m_FeaPartDO.resize( m_SymmIndexVec.size() );

            for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
            {
                vector< VspSurf > surf_vec;
                currgeom->GetSurfVec( surf_vec );
                VspSurf currsurf = surf_vec[m_SymmIndexVec[i]];

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

                vec3d fixpt = currsurf.CompPnt01( m_PosU(), m_PosW() );
                m_FeaPartDO[i].m_PntVec.push_back( fixpt );

                m_FeaPartDO[i].m_GeomChanged = true;

                draw_obj_vec.push_back( &m_FeaPartDO[i] );
            }
        }
    }
}

////////////////////////////////////////////////////
//============== FeaStiffenerPlane ===============//
////////////////////////////////////////////////////
FeaStiffenerPlane::FeaStiffenerPlane( string geomID, int type ) : FeaPart( geomID, type )
{
    m_OrientationPlane.Init( "OrientationPlane", "FeaFullDepth", this, XY_PLANE, XY_PLANE, XZ_PLANE );
    m_OrientationPlane.SetDescript( "Plane the FeaFullDepth Part will be Parallel to" );

    m_CenterPerBBoxLocation.Init( "CenterPerBBoxLocation", "FeaFullDepth", this, 0.5, 0.0, 1.0 );
    m_CenterPerBBoxLocation.SetDescript( "The Location of the Center of the FeaFullDepth Part as a Percentage of the Total Bounding Box" );

    m_Theta.Init( "Theta", "FeaFullDepth", this, 0.0, -90.0, 90.0 );

    m_FeaPropertyIndex = 1; // Default beam property
}

void FeaStiffenerPlane::Update()
{
    ComputePlanarSurf();
}

void FeaStiffenerPlane::ComputePlanarSurf()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i] = VspSurf();
            m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );

            Geom* current_geom = veh->FindGeom( m_ParentGeomID );

            vector< VspSurf > surf_vec;
            current_geom->GetSurfVec( surf_vec );
            VspSurf current_surf = surf_vec[m_SymmIndexVec[i]];

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
            m_FeaPartSurfVec[i].MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );

            // Translate to the origin, rotate, and translate back to m_CenterPerBBoxLocation
            Matrix4d trans_mat_1, trans_mat_2, rot_mat;

            trans_mat_1.loadIdentity();
            trans_mat_1.translatef( geom_center.x() * -1, geom_center.y() * -1, geom_center.z() * -1 );
            m_FeaPartSurfVec[i].Transform( trans_mat_1 );

            rot_mat.loadIdentity();
            rot_mat.rotate( DEG_2_RAD * m_Theta(), rot_axis );
            m_FeaPartSurfVec[i].Transform( rot_mat );

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

            m_FeaPartSurfVec[i].Transform( trans_mat_2 );
        }
    }
}

void FeaStiffenerPlane::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    FeaPart::LoadDrawObjs( draw_obj_vec, id, highlight );
}

////////////////////////////////////////////////////
//============= FeaStiffenerSubSurf ==============//
////////////////////////////////////////////////////
FeaStiffenerSubSurf::FeaStiffenerSubSurf( string geomID, int type ) : FeaPart( geomID, type )
{
    m_StiffenerConstType.Init( "StiffenerConstType", "FeaStiffener", this, CONST_U, CONST_U, CONST_V );

    m_StiffenerConstVal.Init( "StiffenerConstVal", "FeaStiffener", this, 0.5, 0, 1 );
    m_StiffenerConstVal.SetDescript( "Either the U or V value depending on what constant type is choosen." );

    m_FeaPropertyIndex = 1; // Default beam property

    m_FeaStiffenerSubSurf = new SSLine( m_ParentGeomID );
}

void FeaStiffenerSubSurf::Update()
{
    m_FeaStiffenerSubSurf->m_ConstType.Set( m_StiffenerConstType() );
    m_FeaStiffenerSubSurf->m_ConstVal.Set( m_StiffenerConstVal() );

    m_FeaStiffenerSubSurf->Update();
}

void FeaStiffenerSubSurf::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    int begin_size = draw_obj_vec.size();

    m_FeaStiffenerSubSurf->LoadDrawObjs( draw_obj_vec );

    int end_size = draw_obj_vec.size();

    for ( unsigned int i = begin_size; i < end_size; i++ )
    {
        if ( highlight )
        {
            draw_obj_vec[i]->m_LineColor = vec3d( 1.0, 0.0, 0.0 );
        }
        else
        {
            draw_obj_vec[i]->m_LineColor = vec3d( 96.0 / 255.0, 96.0 / 255.0, 96.0 / 255.0 );
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
        fprintf( fp, "PSHELL,%d,%d,%f\n", prop_id, m_FeaMaterialIndex, m_Thickness() );
    }
    if ( m_FeaPropertyType() == BEAM_PROPERTY )
    {
        fprintf( fp, "PBEAM,%d,%d,%f,%f,%f,%f,%f\n", prop_id, m_FeaMaterialIndex, m_CrossSecArea(), m_Izz(), m_Iyy(), m_Izy(), m_Ixx() );
    }
}

void FeaProperty::WriteCalculix( FILE* fp, string ELSET )
{
    FeaMaterial* fea_mat = StructureMgr.GetFeaMaterial( m_FeaMaterialIndex );

    if ( m_FeaPropertyType() == SHELL_PROPERTY )
    {
        fprintf( fp, "*SHELL GENERAL SECTION, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), fea_mat->GetName().c_str() );
        fprintf( fp, "%g,\n", m_Thickness() );
    }
    if ( m_FeaPropertyType() == BEAM_PROPERTY )
    {
        fprintf( fp, "*BEAM GENERAL SECTION, ELSET=%s, MATERIAL=%s\n", ELSET.c_str(), fea_mat->GetName().c_str() );
        fprintf( fp, "%g,%g,%g,%g,%g\n", m_CrossSecArea(), m_Izz(), m_Izy(), m_Iyy(), m_Ixx() );
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
    fprintf( fp, "%g,\n", m_MassDensity() );
    fprintf( fp, "*ELASTIC, TYPE=ISO\n" );
    fprintf( fp, "%g,%g\n", m_ElasticModulus(), m_PoissonRatio() );
    fprintf( fp, "*EXPANSION, TYPE=ISO\n" );
    fprintf( fp, "%g,\n", m_ThermalExpanCoeff() );
}

double FeaMaterial::GetShearModulus()
{
    return ( m_ElasticModulus() / ( 2 * ( m_PoissonRatio() + 1 ) ) );
}
