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

    if ( type == vsp::FEA_RIB )
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
    else if ( type == vsp::FEA_STIFFENER )
    {
        feaprt = new FeaStiffener( m_ParentGeomID );
        feaprt->SetName( string( "FEA_STIFFENER_" + std::to_string( m_FeaPartCount ) ) );
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
    return NULL;
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

    m_FeaPropertyIndex = -1;
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
    if ( type == vsp::FEA_STIFFENER )
    {
        return string( "FeaStiffener" );
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

void FeaPart::FetchFeaXFerSurf( vector< XferSurf > &xfersurfs )
{
    for ( int p = 0; p < m_FeaPartSurfVec.size(); p++ )
    {
        // CFD_STRUCTURE type surfaces have m_CompID == -9999
        m_FeaPartSurfVec[p].FetchXFerSurf( m_ParentGeomID, m_MainSurfIndx(), -9999, xfersurfs );
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
