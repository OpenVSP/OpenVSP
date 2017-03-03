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
