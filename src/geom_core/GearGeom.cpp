//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GearGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspCurve.h"
#include "VspSurf.h"
#include "Vehicle.h"

Bogie::Bogie()
{

    m_Symmetrical.Init( "Symmetrical", "Tire", this, false, false, true );

    m_NAcross.Init( "NumAcross", "Tire", this, 1, 1, 100 );
    m_NTandem.Init( "NumTandem", "Tire", this, 1, 1, 100 );

    m_SpacingType.Init( "SpacingType", "Tire", this, vsp::BOGIE_GAP_FRAC, vsp::BOGIE_CENTER_DIST, vsp::NUM_BOGIE_SPACING_TYPE - 1 );
    m_Spacing.Init( "Spacing", "Tire", this, 1.1, 0.0, 1e12 );
    m_SpacingFrac.Init( "SpacingFrac", "Tire", this, 1.1, 1.0, 100 );
    m_SpacingGap.Init( "SpacingGap", "Tire", this, 1.1, 0.0, 1e12 );
    m_SpacingGapFrac.Init( "SpacingGapFrac", "Tire", this, 0.1, 0.0, 99 );

    m_PitchType.Init( "PitchType", "Tire", this, vsp::BOGIE_GAP_FRAC, vsp::BOGIE_CENTER_DIST, vsp::NUM_BOGIE_SPACING_TYPE - 1 );
    m_Pitch.Init( "Pitch", "Tire", this, 1.1, 0.0, 1e12 );
    m_PitchFrac.Init( "PitchFrac", "Tire", this, 1.1, 1.0, 100 );
    m_PitchGap.Init( "PitchGap", "Tire", this, 1.1, 0.0, 1e12 );
    m_PitchGapFrac.Init( "PitchGapFrac", "Tire", this, 0.1, 0.0, 99 );

    m_XContactPt.Init( "XContactPt", "Tire", this, 0.0, -1e12, 1e12 );
    m_YContactPt.Init( "YContactPt", "Tire", this, 0.0, -1e12, 1e12 );
    m_ZAboveGround.Init( "ZAboveGround", "Tire", this, 0.0, -1e12, 1e12 );

    m_Diameter.Init( "Diameter", "Tire", this, 2.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of the tire" );

    m_Width.Init( "Width", "Tire", this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the tire" );

    m_SLRFlag.Init( "SLRFlag", "Tire", this, false, false, true );
    m_SLRFlag.SetDescript( "Flag to use dimensional static loaded radius" );
    m_DeflectionPct.Init( "DeflectionPct", "Tire", this, 0.35, 0.0, 1.0 );
    m_DeflectionPct.SetDescript( "Static loaded deflection fraction" );
    m_StaticRadius.Init( "StaticRadius", "Tire", this, 0.75, 0.0, 1.0e12 );
    m_StaticRadius.SetDescript( "Static loaded radius" );

    m_DrimFlag.Init( "DrimFlag", "Tire", this, true, false, true );
    m_DrimFlag.SetDescript( "Flag to use dimensional rim diameter" );
    m_DrimFrac.Init( "DrimFrac", "Tire", this, 0.45, 0.0, 1.0 );
    m_DrimFrac.SetDescript( "Wheel rim diameter fraction of wheel diameter" );
    m_Drim.Init( "Drim", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_Drim.SetDescript( "Wheel rim diameter" );

    m_WrimFlag.Init( "WrimFlag", "Tire", this, false, false, true );
    m_WrimFlag.SetDescript( "Flag to use dimensional wheel rim width" );
    m_WrimFrac.Init( "WrimFrac", "Tire", this, 0.77, 0.0, 1.0 );
    m_WrimFrac.SetDescript( "Wheel rim fraction of tire width" );
    m_Wrim.Init( "Wrim", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_Wrim.SetDescript( "Wheel rim width" );

    m_WsFlag.Init( "WsFlag", "Tire", this, false, false, true );
    m_WsFlag.SetDescript( "Flag to use dimensional shoulder width" );
    m_WsFrac.Init( "WsFrac", "Tire", this, 0.88, 0.0, 1.0 );
    m_WsFrac.SetDescript( "Tire shoulder fraction of width" );
    m_Ws.Init( "Ws", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_Ws.SetDescript( "Tire shoulder width" );

    m_HsFlag.Init( "HsFlag", "Tire", this, false, false, true );
    m_HsFlag.SetDescript( "Flag to use dimensional shoulder height" );
    m_HsFrac.Init( "HsFrac", "Tire", this, 0.82, 0.0, 1.0 );
    m_HsFrac.SetDescript( "Tire shoulder height fraction of tire height" );
    m_Hs.Init( "Hs", "Tire", this, 0.88, 0.0, 1.0e12 );
    m_Hs.SetDescript( "Tire shoulder height" );
}

//==== Parm Changed ====//
void Bogie::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
    }
    else
    {
        Update();
    }

    //==== Notify Parent Container (XSec) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

int Bogie::GetNumSurf() const
{
    int sym = 1;
    if ( m_Symmetrical() )
    {
        sym = 2;
    }

    return m_NAcross() * m_NTandem() * sym;
}

void Bogie:: UpdateParms()
{
    // Tire dimensions
    double Do = m_Diameter(); // 13.5;
    double W = m_Width(); // 6.0;

    // Rim
    if ( m_WrimFlag() ) // Use dimensional Wrim.
    {
        m_WrimFrac = m_Wrim() / W;
    }
    else
    {
        m_Wrim = m_WrimFrac() * W;
    }

    if ( m_DrimFlag() ) // Use dimensional Drim.
    {
        m_DrimFrac = m_Drim() / Do;
    }
    else
    {
        m_Drim = m_DrimFrac() * Do;
    }

    // Rim dimensions
    double Drim = m_Drim();
    double Wrim = m_Wrim();
    // double Hflange = 0.55;

    double H = 0.5 * ( Do - Drim );



    if ( m_WsFlag() ) // Use dimensional Ws.
    {
        m_WsFrac = m_Ws() / W;
    }
    else
    {
        m_Ws = m_WsFrac() * W;
    }

    if ( m_HsFlag() ) // Use dimensional Ws.
    {
        m_HsFrac = m_Hs() / H;
    }
    else
    {
        m_Hs = m_HsFrac() * H;
    }

    m_StaticRadius.SetLowerUpperLimits( 0.5 * Drim, 0.5 * Do );
    if ( m_SLRFlag() )
    {
        m_DeflectionPct = ( 0.5 * Do - m_StaticRadius() ) / H ;
    }
    else
    {
        m_StaticRadius = 0.5 * Do - m_DeflectionPct() * H;
    }

    switch ( m_SpacingType() )
    {
        case vsp::BOGIE_CENTER_DIST:
            m_SpacingFrac = m_Spacing() / W;
            m_SpacingGap = m_Spacing() - W;
            m_SpacingGapFrac = m_SpacingFrac() - 1;
            break;
        case vsp::BOGIE_CENTER_DIST_FRAC:
            m_Spacing = m_SpacingFrac() * W;
            m_SpacingGap = m_Spacing() - W;
            m_SpacingGapFrac = m_SpacingFrac() - 1;
            break;
        case vsp::BOGIE_GAP:
            m_Spacing = m_SpacingGap() + W;
            m_SpacingFrac = m_Spacing() / W;
            m_SpacingGapFrac = m_SpacingFrac() - 1;
            break;
        case vsp::BOGIE_GAP_FRAC:
        default:
            m_SpacingFrac = m_SpacingGapFrac() + 1;
            m_Spacing = m_SpacingFrac() * W;
            m_SpacingGap = m_SpacingGapFrac() * W;
            break;
    }

    switch ( m_PitchType() )
    {
        case vsp::BOGIE_CENTER_DIST:
            m_PitchFrac = m_Pitch() / Do;
            m_PitchGap = m_Pitch() - Do;
            m_PitchGapFrac = m_PitchFrac() - 1;
            break;
        case vsp::BOGIE_CENTER_DIST_FRAC:
            m_Pitch = m_PitchFrac() * Do;
            m_PitchGap = m_Pitch() - Do;
            m_PitchGapFrac = m_PitchFrac() - 1;
            break;
        case vsp::BOGIE_GAP:
            m_Pitch = m_PitchGap() + Do;
            m_PitchFrac = m_Pitch() / Do;
            m_PitchGapFrac = m_PitchFrac() - 1;
            break;
        case vsp::BOGIE_GAP_FRAC:
        default:
            m_PitchFrac = m_PitchGapFrac() + 1;
            m_Pitch = m_PitchFrac() * Do;
            m_PitchGap = m_PitchGapFrac() * Do;
            break;
    }


}

void Bogie::UpdateTireCurve()
{
    // Tire dimensions
    double Do = m_Diameter();
    double W = m_Width();

    // Rim dimensions
    double Drim = m_Drim();
    double Wrim = m_Wrim();
    // double Hflange = 0.55;

    double H = 0.5 * ( Do - Drim );


    // Tire shoulder
    double Ws = m_Ws();
    double Ds = 2 * m_Hs() + Drim;


    double Cr = 0;
    double Cw = 0;
    double Cside = 0.25;

    m_TireProfile.CreateTire( Do, W, Ds, Ws, Drim, Wrim );
}

void Bogie::Update()
{
    UpdateParms();
    UpdateTireCurve();

    m_TireSurface.CreateBodyRevolution( m_TireProfile, true, 1 );
    m_TireSurface.SetMagicVParm( false );
    m_TireSurface.SetHalfBOR( true );
    m_TireSurface.FlipNormal();

}

void Bogie::AppendMainSurf( vector < VspSurf > &surfvec ) const
{
    Matrix4d xform;
    xform.translatef( m_XContactPt(), m_YContactPt(), m_ZAboveGround() + m_StaticRadius() );

    int nsymm = 1;
    vector < double > smult = { 1.0 };
    if ( m_Symmetrical() )
    {
        nsymm = 2;
        smult.push_back( -1.0 );
    }
    int na = m_NAcross();
    int nt = m_NTandem();

    double s = m_Spacing();
    double p = m_Pitch();


    double cenAcross = 0.5 * ( na - 1 ) * s;
    double cenTandem = 0.5 * ( nt - 1 ) * p;

    for ( int i = 0; i < na; i++ )
    {
        Matrix4d col = xform;
        col.translatef( 0, i * s - cenAcross, 0 );

        for ( int j = 0; j < nt; j++ )
        {
            Matrix4d row = col;
            row.translatef( j * p - cenTandem, 0, 0 );

            for ( int isymm = 0; isymm < nsymm; isymm++ )
            {
                if ( isymm > 0 )
                {
                    row.mirrory();
                }

                surfvec.push_back( m_TireSurface );
                surfvec.back().Transform( row );

                if ( isymm > 0 )
                {
                    surfvec.back().FlipNormal();
                }
            }
        }
    }

}


//===============================================================================//
//===============================================================================//
//===============================================================================//

//==== Constructor ====//
GearGeom::GearGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "GearGeom";
    m_Type.m_Name = "Gear";
    m_Type.m_Type = GEAR_GEOM_TYPE;

    //==== Init Parms ====//
    m_TessU = 10;
    m_TessW = 8;

    Bogie * mg =  CreateAndAddBogie();


    Update();
}

//==== Destructor ====//
GearGeom::~GearGeom()
{

}

void GearGeom::UpdateSurf()
{
    int nbogies = m_Bogies.size();

    int nsurf = 1;
    for ( int i = 0; i < nbogies; i++ )
    {
        if ( m_Bogies[i] )
        {
            m_Bogies[i]->Update();
            nsurf += m_Bogies[i]->GetNumSurf();
        }
    }


    m_MainSurfVec.clear();
    m_MainSurfVec.reserve( nsurf );

    m_MainSurfVec.resize( 1 );


    for ( int i = 0; i < nbogies; i++ )
    {
        if ( m_Bogies[i] )
        {
            m_Bogies[i]->AppendMainSurf( m_MainSurfVec );
        }
    }


    BndBox bbox;
    if ( VehicleMgr.GetVehicle()->GetVisibleBndBox( bbox ) )
    {
        bbox.Expand( 5.0 * bbox.DiagDist() );
        m_MainSurfVec[0].CreatePlane( bbox.GetMin( 0 ), bbox.GetMax( 0 ), bbox.GetMin( 1 ), bbox.GetMax( 1 ) );
    }
    else
    {
        m_MainSurfVec[0].CreatePlane( -10, 10, -10, 10 );
    }


}

//==== Compute Rotation Center ====//
void GearGeom::ComputeCenter()
{

}

//==== Scale ====//
void GearGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    // m_Length *= currentScale;
    m_LastScale = m_Scale();
}

void GearGeom::AddDefaultSources( double base_len )
{
}

Bogie * GearGeom::CreateAndAddBogie()
{
    Bogie * bogie = new Bogie();

    bogie->SetParentContainer( m_ID );

    m_Bogies.push_back( bogie );

    m_CurrBogieIndex = m_Bogies.size() - 1;
    return bogie;
}

string GearGeom::CreateAndAddBogie( int foo )
{
    Bogie * bogie = CreateAndAddBogie();

    return bogie->GetID();
}

Bogie * GearGeom::GetCurrentBogie()
{
    if ( m_CurrBogieIndex < 0 || m_CurrBogieIndex >= ( int )m_Bogies.size() )
    {
        return NULL;
    }

    return m_Bogies[ m_CurrBogieIndex ];
}

std::vector < Bogie * > GearGeom::GetBogieVec()
{
    return m_Bogies;
}

xmlNodePtr GearGeom::EncodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "Gear", NULL );

    XmlUtil::AddIntNode( child_node, "Num_of_Bogies", m_Bogies.size() );

    for ( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Bogie_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, NULL, BAD_CAST labelName, NULL );
        m_Bogies[i]->EncodeXml( label_node );
    }

    return child_node;
}

xmlNodePtr GearGeom::DecodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr label_root_node = XmlUtil::GetNode( node, "Gear", 0 );

    int numofLabels = XmlUtil::FindInt( label_root_node, "Num_of_Bogies", 0 );
    for ( int i = 0; i < numofLabels; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Bogie_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                CreateAndAddBogie();
                GetCurrentBogie()->DecodeXml( label_node );
            }
        }
    }

    return label_root_node;
}

void GearGeom::DelAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        delete m_Bogies[i];
    }
    m_Bogies.clear();

    m_CurrBogieIndex = -1;
}

void GearGeom::ShowAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        m_Bogies[i]->m_Visible = true;
    }
}

void GearGeom::HideAllBogies()
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        m_Bogies[i]->m_Visible = false;
    }
}

Bogie * GearGeom::GetBogie( const string &id )
{
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        if ( m_Bogies[i]->GetID() == id )
        {
            return m_Bogies[i];
        }
    }

    return NULL;
}

vector < string > GearGeom::GetAllBogies()
{
    vector < string > rulerList( m_Bogies.size() );

    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        rulerList[i] = m_Bogies[i]->GetID();
    }

    return rulerList;
}

void GearGeom::DelBogie( const int & i )
{
    if ( i < 0 || i >= ( int )m_Bogies.size() )
    {
        return;
    }

    Bogie* ruler = m_Bogies[i];

    m_Bogies.erase( m_Bogies.begin() +  i );

    delete ruler;
}

void GearGeom::DelBogie( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_Bogies.size(); i++ )
    {
        if ( m_Bogies[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelBogie( idel );
}

void GearGeom::UpdateBBox( )
{
    Geom::UpdateBBox( 1 );
}
