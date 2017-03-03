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

//////////////////////////////////////////////////////
//===================== FeaSpar ====================//
//////////////////////////////////////////////////////

FeaSpar::FeaSpar( string geomID, int type ) : FeaPart( geomID, type )
{
    m_PerU.Init( "PerU", "FeaSpar", this, 0.5, 0.0, 1.0 );
    m_PerU.SetDescript( "Precent U Location" );
    m_Alpha.Init( "Alpha", "FeaSpar", this, 0.0, -90.0, 90.0 );
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
        // TODO: Improve planar surface definition and determine intersections with geom.

        for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i] = VspSurf();
            m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );

            Geom* currgeom = veh->FindGeom( m_ParentGeomID );

            vector< VspSurf > surf_vec;
            currgeom->GetSurfVec( surf_vec );
            VspSurf currsurf = surf_vec[m_SymmIndexVec[i]];

            BndBox currsurfbbox;
            currsurf.GetBoundingBox( currsurfbbox );
            //vec3d center = currsurfbbox.GetCenter();

            vec3d pointA = currsurf.CompPnt01( 0.0, m_PerU() );
            vec3d pointB = currsurf.CompPnt01( 1.0, m_PerU() );

            vec3d pointC, pointD;

            int type = currgeom->GetType().m_Type;

            if ( type == MS_WING_GEOM_TYPE )
            {
                // Approximate Leading and Trailing Edge at Wing Midsection
                pointC = currsurf.CompPnt01( 0.5, 0.5 );
                pointD = currsurf.CompPnt01( 0.5, 0.0 );
            }
            else
            {
                if ( m_PerU() >= 0.5 )
                {
                    pointC = currsurf.CompPnt01( 0.5, m_PerU() );
                    pointD = currsurf.CompPnt01( 0.5, m_PerU() - 0.5 );
                }
                else
                {
                    pointC = currsurf.CompPnt01( 0.5, m_PerU() + 0.5 );
                    pointD = currsurf.CompPnt01( 0.5, m_PerU() );
                }
            }

            vec3d AtoBvec = pointB - pointA;
            AtoBvec.normalize();

            vec3d CtoDvec = pointD - pointC;
            CtoDvec.normalize();

            vec3d normalvec = cross( AtoBvec, CtoDvec );
            normalvec.normalize();

            double scale = 0.5 * currsurfbbox.GetSmallestDist();

            vec3d cornerA = pointA + normalvec * scale;
            vec3d cornerB = pointA + normalvec * scale * -1;
            vec3d cornerC = pointB + normalvec * scale;
            vec3d cornerD = pointB + normalvec * scale * -1;

            VspSurf* tempsurf = new VspSurf();
            tempsurf->MakePlaneSurf( cornerA, cornerB, cornerC, cornerD );
            BndBox surfbbox;
            tempsurf->GetBoundingBox( surfbbox );
            vec3d surfcenter = surfbbox.GetCenter();

            // Expand plane to be larger than geom at intersection. The Surface outside of the geometry will be ignored when meshed
            double exmag = currsurfbbox.GetLargestDist() / 100.0;

            vec3d expandA = exmag * ( cornerA - surfcenter ) + cornerA;
            vec3d expandB = exmag * ( cornerB - surfcenter ) + cornerB;
            vec3d expandC = exmag * ( cornerC - surfcenter ) + cornerC;
            vec3d expandD = exmag * ( cornerD - surfcenter ) + cornerD;

            //// Apply Scaling Factors
            //vec3d midAB = ( expandA + expandB ) / 2.0;
            //vec3d midCD = ( expandC + expandD ) / 2.0;

            //expandA = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandA - midAB ) + expandA;
            //expandB = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandB - midAB ) + expandB;
            //expandC = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandC - midCD ) + expandC;
            //expandD = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandD - midCD ) + expandD;

            //vec3d midAC = ( expandA + expandC ) / 2.0;
            //vec3d midBD = ( expandB + expandD ) / 2.0;

            //expandA = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandA - midAC ) + expandA;
            //expandC = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandC - midAC ) + expandC;
            //expandB = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandB - midBD ) + expandB;
            //expandD = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandD - midBD ) + expandD;

            // Make Planar Surface
            m_FeaPartSurfVec[i].MakePlaneSurf( expandA, expandB, expandC, expandD );

            // Match flipnormal
            bool geomflipnormal = currsurf.GetFlipNormal();
            bool surfflipnormal = m_FeaPartSurfVec[i].GetFlipNormal();

            if ( surfflipnormal != geomflipnormal )
            {
                //m_Theta = -1 * m_Theta;
                m_Alpha = -1 * m_Alpha();
                m_FeaPartSurfVec[i].FlipNormal();
            }

            // Build Patches?

            //newsurf->GetPiecewiseSurf().BuildPatches( newsurf );

            // Translate to orgin before rotation and then translate back

            // Rotation Matrix
            Matrix4d transMat1, transMat2, rotMat;
            rotMat.loadIdentity();

            if ( type == MS_WING_GEOM_TYPE )
            {
                rotMat.rotate( DEG_2_RAD * m_Theta(), normalvec );
                rotMat.rotate( DEG_2_RAD * m_Alpha(), CtoDvec );
            }
            else
            {
                rotMat.rotate( DEG_2_RAD * m_Theta(), normalvec );
                rotMat.rotate( DEG_2_RAD * m_Alpha(), CtoDvec );
            }

            transMat1.loadIdentity();
            transMat1.translatef( surfcenter.x() * -1, surfcenter.y() * -1, surfcenter.z() * -1 );
            transMat2.loadIdentity();
            transMat2.translatef( surfcenter.x(), surfcenter.y(), surfcenter.z() );

            m_FeaPartSurfVec[i].Transform( transMat1 );
            m_FeaPartSurfVec[i].Transform( rotMat );
            m_FeaPartSurfVec[i].Transform( transMat2 );

            delete tempsurf;
        }
    }
}

void FeaSpar::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        FeaPart::LoadDrawObjs( draw_obj_vec, id, highlight );
    }
}

//////////////////////////////////////////////////////
//===================== FeaRib =====================//
//////////////////////////////////////////////////////

FeaRib::FeaRib( string geomID, int type ) : FeaPart( geomID, type )
{
    m_PerW.Init( "PerW", "FeaRib", this, 0.5, 0.0, 1.0 );
    m_PerW.SetDescript( "Precent W Location" );
    m_Alpha.Init( "Alpha", "FeaRib", this, 0.0, -90.0, 90.0 );
    m_Theta.Init( "Theta", "FeaRib", this, 0.0, -90.0, 90.0 );
    //m_TrimFlag.Init( "TrimFlag", "FeaRib", this, true, 0, 1 );

    //m_PlaneSurfLengthScale.Init( "PlaneSurfLengthScale", "FeaRib", this, 1.0, 0.0, 1.0e12 );
    //m_PlaneSurfWidthScale.Init( "PlaneSurfWidthScale", "FeaRib", this, 1.0, 0.0, 1.0e12 );
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
        // TODO: Improve planar surface definition and determine intersections with geom.

        for ( unsigned int i = 0; i < m_SymmIndexVec.size(); i++ )
        {
            m_FeaPartSurfVec[i] = VspSurf();
            m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );

            Geom* currgeom = veh->FindGeom( m_ParentGeomID );

            vector< VspSurf > surf_vec;
            currgeom->GetSurfVec( surf_vec );
            VspSurf currsurf = surf_vec[m_SymmIndexVec[i]];

            vec3d pointA = currsurf.CompPnt01( m_PerW(), 0.0 );
            vec3d pointB = currsurf.CompPnt01( m_PerW(), 0.5 );
            vec3d pointC = currsurf.CompPnt01( m_PerW(), 0.25 );
            vec3d pointD = currsurf.CompPnt01( m_PerW(), 0.75 );

            VspSurf* tempsurf = new VspSurf();
            tempsurf->MakePlaneSurf( pointA, pointC, pointD, pointB );
            BndBox surfbbox;
            tempsurf->GetBoundingBox( surfbbox );
            vec3d surfcenter = surfbbox.GetCenter();

            vec3d AtoBvec = pointB - pointA;
            AtoBvec.normalize();

            vec3d CtoDvec = pointD - pointC;
            CtoDvec.normalize();

            vec3d normal_vec = cross( AtoBvec, CtoDvec );

            int type = currgeom->GetType().m_Type;
            double scale;

            if ( type == MS_WING_GEOM_TYPE )
            {
                scale = 0.5 * surfbbox.GetLargestDist(); // Changed from get middle distance
            }
            else
            {
                scale = 0.5 * surfbbox.GetLargestDist();
            }

            vec3d cornerA = pointA + CtoDvec * scale;
            vec3d cornerB = pointA + CtoDvec * scale * -1;
            vec3d cornerC = pointB + CtoDvec * scale;
            vec3d cornerD = pointB + CtoDvec * scale * -1;

            // Expand plane to be larger than geom at intersection. The Surface outside of the geometry will be ignored when meshed
            double exmag = 0.1; // expand plane 10% 

            vec3d expandA = exmag * ( cornerA - surfcenter ) + cornerA;
            vec3d expandB = exmag * ( cornerB - surfcenter ) + cornerB;
            vec3d expandC = exmag * ( cornerC - surfcenter ) + cornerC;
            vec3d expandD = exmag * ( cornerD - surfcenter ) + cornerD;

            //// Apply Scaling Factors
            //vec3d midAB = ( expandA + expandB ) / 2.0;
            //vec3d midCD = ( expandC + expandD ) / 2.0;

            //expandA = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandA - midAB ) + expandA;
            //expandB = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandB - midAB ) + expandB;
            //expandC = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandC - midCD ) + expandC;
            //expandD = ( m_PlaneSurfWidthScale() - 1.0 ) * ( expandD - midCD ) + expandD;

            //vec3d midAC = ( expandA + expandC ) / 2.0;
            //vec3d midBD = ( expandB + expandD ) / 2.0;

            //expandA = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandA - midAC ) + expandA;
            //expandC = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandC - midAC ) + expandC;
            //expandB = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandB - midBD ) + expandB;
            //expandD = ( m_PlaneSurfLengthScale() - 1.0 ) * ( expandD - midBD ) + expandD;

            // Make Planar Surface
            m_FeaPartSurfVec[i].MakePlaneSurf( expandA, expandB, expandC, expandD );

            // Match flipnormal
            bool geomflipnormal = currsurf.GetFlipNormal();
            bool surfflipnormal = m_FeaPartSurfVec[i].GetFlipNormal();

            if ( surfflipnormal != geomflipnormal )
            {
                m_Theta = -1 * m_Theta();
                m_Alpha = -1 * m_Alpha();
                m_FeaPartSurfVec[i].FlipNormal();
            }

            // Build Patches?

            //newsurf->GetPiecewiseSurf().BuildPatches( newsurf );

            // Translate to orgin before rotation and then translate back

            // Rotation Matrix
            Matrix4d transMat1, transMat2, rotMat;
            rotMat.loadIdentity();

            if ( type == MS_WING_GEOM_TYPE )
            {
                rotMat.rotate( DEG_2_RAD * m_Theta(), CtoDvec );
                rotMat.rotate( DEG_2_RAD * m_Alpha(), AtoBvec );
            }
            else
            {
                rotMat.rotate( DEG_2_RAD * m_Theta(), AtoBvec );
                rotMat.rotate( DEG_2_RAD * m_Alpha(), CtoDvec );
            }

            transMat1.loadIdentity();
            transMat1.translatef( surfcenter.x() * -1, surfcenter.y() * -1, surfcenter.z() * -1 );
            transMat2.loadIdentity();
            transMat2.translatef( surfcenter.x(), surfcenter.y(), surfcenter.z() );

            m_FeaPartSurfVec[i].Transform( transMat1 );
            m_FeaPartSurfVec[i].Transform( rotMat );
            m_FeaPartSurfVec[i].Transform( transMat2 );

            delete tempsurf;
        }
    }
}

void FeaRib::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        FeaPart::LoadDrawObjs( draw_obj_vec, id, highlight );
    }
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
//================= FeaStiffener ==================//
////////////////////////////////////////////////////

FeaStiffener::FeaStiffener( string geomID, int type ) : FeaPart( geomID, type )
{
    m_StiffenerConstType.Init( "StiffenerConstType", "FeaNoDepth", this, CONST_U, 0, 1 );
    m_StiffenerConstVal.Init( "StiffenerConstVal", "FeaNoDepth", this, 0.5, 0, 1 );
    m_StiffenerConstVal.SetDescript( "Either the U or V value of the line depending on what constant line type is choosen." );

    m_FeaStiffenerSubSurf = new SSLine( geomID );
    m_LVec.push_back( SSLineSeg() );
}

void FeaStiffener::Update()
{
    m_FeaStiffenerSubSurf->m_ConstType.Set( m_StiffenerConstType() );
    m_FeaStiffenerSubSurf->m_ConstVal.Set( m_StiffenerConstVal() );

    m_FeaStiffenerSubSurf->Update();

    ComputeEndPoints();
}

void FeaStiffener::ComputeEndPoints()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        if ( m_StiffenerConstType() == CONST_U )
        {
            m_LVec[0].SetSP0( vec3d( m_StiffenerConstVal(), 1, 0 ) );
            m_LVec[0].SetSP1( vec3d( m_StiffenerConstVal(), 0, 0 ) );
        }
        else if ( m_StiffenerConstType() == CONST_W )
        {
            m_LVec[0].SetSP0( vec3d( 0, m_StiffenerConstVal(), 0 ) );
            m_LVec[0].SetSP1( vec3d( 1, m_StiffenerConstVal(), 0 ) );
        }

        Geom* currgeom = veh->FindGeom( m_ParentGeomID );

        if ( currgeom )
        {
            m_LVec[0].Update( currgeom );
        }
    }
}

void FeaStiffener::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, int id, bool highlight )
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

                for ( int ls = 0; ls < (int)m_LVec.size(); ls++ )
                {
                    int num_pnts = 100;
                    int *num_pnts_ptr = NULL;
                    if ( num_pnts > 0 )
                    {
                        num_pnts_ptr = &num_pnts;
                    }

                    m_FeaPartDO[i].m_PntVec.clear();

                    m_LVec[ls].UpdateDrawObj( &currsurf, currgeom, m_FeaPartDO[i], num_pnts_ptr );

                    // Overwrite SSLineSeg settings
                    m_FeaPartDO[i].m_GeomID = string( "FeaNoDepth_" + std::to_string( id ) + "_" + std::to_string( i ) );
                    m_FeaPartDO[i].m_LineWidth = 2.0;

                    if ( highlight )
                    {
                        m_FeaPartDO[i].m_LineColor = vec3d( 1.0, 0.0, 0.0 );
                    }
                    else
                    {
                        m_FeaPartDO[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
                    }

                    m_FeaPartDO[i].m_Type = DrawObj::VSP_LINES;
                    m_FeaPartDO[i].m_GeomChanged = true;
                    draw_obj_vec.push_back( &m_FeaPartDO[i] );
                }
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

                m_FeaPartSurfVec[i] = VspSurf( surf_vec[m_SymmIndexVec[i]] );

                m_FeaPartSurfVec[i].SetSurfCfdType( vsp::CFD_STRUCTURE );
            }
        }
    }
}
