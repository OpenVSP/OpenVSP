//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmIndexTestSuite.cpp: Tests of how containers and Parms are found under the containers that
// hold them.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "ParmIndexTestSuite.h"
#include "VSP_Geom_API.h"
#include "APIErrorMgr.h"
#include "APIDefines.h"
#include "ParmMgr.h"
#include "ParmContainer.h"
#include "StringUtil.h"
#include "Parm.h"
#include "SubSurface.h"
#include "FeaStructure.h"
#include "StructureMgr.h"
#include "Geom.h"
#include "GridDensity.h"
#include "Color.h"
#include "GearGeom.h"
#include "XSecSurf.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "PCurve.h"
#include "RoutingGeom.h"
#include "Texture.h"
#include "MeshCommonSettings.h"
#include "VSPAEROMgr.h"
#include "CustomGeom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "LinkMgr.h"
#include "Link.h"
#include "ModeMgr.h"

#include <typeinfo>
#include <set>
#include <algorithm>
#include <map>

#include <string>
#include <vector>

using std::string;
using std::vector;

//==== A model holding at least one of each kind of container a user can make ====//
static void BuildEveryKindOfContainer()
{
    vsp::VSPRenew();

    string fuse = vsp::AddGeom( "FUSELAGE" );
    string wing = vsp::AddGeom( "WING" );
    string pod = vsp::AddGeom( "POD" );
    vsp::AddGeom( "STACK" );
    string prop = vsp::AddGeom( "PROP" );
    vsp::SetParmVal( prop, "PropMode", "Design", vsp::PROP_DISK );
    vsp::AddGeom( "BODYOFREVOLUTION" );
    vsp::AddGeom( "ELLIPSOID" );
    vsp::AddGeom( "CONFORMAL", wing );
    vsp::AddGeom( "AUXILIARY", fuse );
    string gear = vsp::AddGeom( "GEAR" );
    string route = vsp::AddGeom( "ROUTING" );
    vsp::CloneGeomVec( vector< string >( 1, pod ) );
    vsp::Update();

    //==== Subsurfaces of each kind ====//
    vsp::AddSubSurf( fuse, vsp::SS_LINE );
    vsp::AddSubSurf( fuse, vsp::SS_RECTANGLE );
    vsp::AddSubSurf( fuse, vsp::SS_ELLIPSE );
    vsp::AddSubSurf( fuse, vsp::SS_XSEC_CURVE );
    vsp::AddSubSurf( fuse, vsp::SS_FINITE_LINE );
    string ssint = vsp::AddSubSurf( fuse, vsp::SS_INTERSECT );
    vsp::SetIntersectSubSurfGeomID( ssint, pod );
    vsp::AddSubSurf( wing, vsp::SS_CONTROL );

    //==== Gear and routing ====//
    vsp::CreateAndAddBogie( gear );
    vsp::AddRoutingPt( route, fuse, 0 );
    vsp::AddRoutingPt( route, wing, 0 );

    //==== A structure with each kind of part, a subsurface and a boundary condition ====//
    int istruct = vsp::AddFeaStruct( wing );
    string fea = vsp::GetFeaStructID( wing, istruct );
    vsp::AddFeaPart( wing, istruct, vsp::FEA_RIB );
    vsp::AddFeaPart( wing, istruct, vsp::FEA_SPAR );
    vsp::AddFeaPart( wing, istruct, vsp::FEA_FIX_POINT );
    string pspar = vsp::AddFeaPart( wing, istruct, vsp::FEA_POLY_SPAR );
    vsp::AddFeaPolySparPt( pspar );
    vsp::AddFeaSubSurf( wing, istruct, vsp::SS_LINE );
    vsp::AddFeaBC( fea, vsp::FEA_BC_STRUCTURE );
    string mat = vsp::AddFeaMaterial();
    vsp::SetParmVal( mat, "FeaMaterialType", "FeaMaterial", vsp::FEA_LAMINATE );
    vsp::AddFeaLayer( mat );
    vsp::AddFeaProperty();
    vsp::AddFeaAssembly();

    //==== Analysis, mesh and measurement containers ====//
    vsp::GetNumActuatorDisks();
    vsp::CreateVSPAEROControlSurfaceGroup();
    vsp::AutoGroupVSPAEROControlSurfaces();
    vsp::AddCpSlice( vsp::X_DIR, 0.5 );
    vsp::AddCFDSource( vsp::POINT_SOURCE, fuse, 0, 0.1, 0.2, 0.5, 0.5 );
    vsp::AddRuler( fuse, 0, 0.2, 0.2, wing, 0, 0.5, 0.5, "Ruler" );
    vsp::AddProbe( fuse, 0, 0.5, 0.5, "Probe" );
    vsp::AddRSTProbe( wing, 0, 0.5, 0.5, 0.5, "RSTProbe" );
    vsp::AddProtractor( fuse, 0, 0.2, 0.2, wing, 0, 0.5, 0.5, pod, 0, 0.5, 0.5, "Protractor" );
    vsp::AddBackground3D();
    vsp::CreateAndAddMode( "Mode", vsp::SET_ALL, vsp::SET_NONE );

    //==== User Parms and a link ====//
    string up = vsp::AddUserParm( vsp::PARM_DOUBLE_TYPE, "UserA", "UserGroup" );
    vsp::AddParmLink( up, vsp::GetParm( pod, "Length", "Design" ) );

    vsp::Update();
}

void ParmIndexTestSuite::ChildrenAreKeptForEveryKindOfContainer()
{
    BuildEveryKindOfContainer();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

void ParmIndexTestSuite::ChildrenAreKeptThroughEdits()
{
    BuildEveryKindOfContainer();

    vector< string > fuses = vsp::FindGeomsWithName( "FuselageGeom" );
    TEST_ASSERT( fuses.size() == 1 );
    string fuse = fuses[0];
    string xss = vsp::GetXSecSurf( fuse, 0 );

    //==== A shape change replaces a curve; a new section and a cut one ====//
    vsp::ChangeXSecShape( xss, 1, vsp::XS_SUPER_ELLIPSE );
    vsp::ChangeXSecShape( xss, 2, vsp::XS_EDIT_CURVE );
    vsp::InsertXSec( fuse, 1, vsp::XS_ROUNDED_RECTANGLE );
    vsp::CutXSec( fuse, 2 );
    vsp::Update();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    //==== Copied, pasted and deleted Geoms and subsurfaces ====//
    vsp::CopyGeomToClipboard( fuse );
    vector< string > pasted = vsp::PasteGeomClipboard();
    TEST_ASSERT( pasted.size() > 0 );
    vsp::Update();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    vector< string > subs = vsp::GetSubSurfIDVec( fuse );
    TEST_ASSERT( subs.size() > 1 );
    vsp::DeleteSubSurf( subs[0] );
    vsp::DeleteGeom( pasted[0] );
    vsp::Update();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    vsp::VSPRenew();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

void ParmIndexTestSuite::ChildrenAreKeptThroughAFileRead()
{
    BuildEveryKindOfContainer();
    vsp::WriteVSPFile( "ParmIndexEveryKind.vsp3" );

    vsp::VSPRenew();
    vsp::ReadVSPFile( "ParmIndexEveryKind.vsp3" );
    vsp::Update();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    //==== Read in again on top, so every ID is remapped ====//
    vsp::InsertVSPFile( "ParmIndexEveryKind.vsp3", string() );
    vsp::Update();
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

//==== A container that can take another's ID, as one that takes another's place does ====//
class RenamableContainer : public ParmContainer
{
public:
    void TakeID( const string & id )
    {
        ChangeID( id );
    }
};

void ParmIndexTestSuite::ChildrenFollowAnExchangedIdentity()
{
    ParmContainer* a = new ParmContainer();
    ParmContainer* b = new ParmContainer();
    ParmContainer* a_child = new ParmContainer();
    ParmContainer* b_child = new ParmContainer();
    a_child->SetParentContainer( a->GetID() );
    b_child->SetParentContainer( b->GetID() );

    string a_id = a->GetID();
    string b_id = b->GetID();

    //==== Each child stays with its parent object, under the parent's new ID ====//
    a->SwapIdentity( b );
    TEST_ASSERT( a->GetID() == b_id );
    TEST_ASSERT( a_child->GetParentContainer() == b_id );
    TEST_ASSERT( b_child->GetParentContainer() == a_id );
    TEST_ASSERT( ParmMgr.GetChildContainers( b_id ).size() == 1 );
    TEST_ASSERT( ParmMgr.GetChildContainers( b_id )[0] == a_child );
    TEST_ASSERT( ParmMgr.GetChildContainers( a_id ).size() == 1 );
    TEST_ASSERT( ParmMgr.GetChildContainers( a_id )[0] == b_child );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    //==== A plain change of ID too ====//
    RenamableContainer* r = new RenamableContainer();
    ParmContainer* r_child = new ParmContainer();
    r_child->SetParentContainer( r->GetID() );
    string old_id = r->GetID();
    r->TakeID( "PARMINDEXTESTID" );
    TEST_ASSERT( r_child->GetParentContainer() == "PARMINDEXTESTID" );
    TEST_ASSERT( ParmMgr.GetChildContainers( old_id ).empty() );
    TEST_ASSERT( ParmMgr.GetChildContainers( "PARMINDEXTESTID" ).size() == 1 );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    delete r_child;
    delete r;
    delete a_child;
    delete b_child;
    delete a;
    delete b;
    TEST_ASSERT( ParmMgr.GetChildContainers( a_id ).empty() );
    TEST_ASSERT( ParmMgr.GetChildContainers( b_id ).empty() );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

void ParmIndexTestSuite::ADestroyedChildLeavesItsParent()
{
    ParmContainer* parent = new ParmContainer();
    ParmContainer* first = new ParmContainer();
    ParmContainer* second = new ParmContainer();
    first->SetParentContainer( parent->GetID() );
    second->SetParentContainer( parent->GetID() );
    TEST_ASSERT( ParmMgr.GetChildContainers( parent->GetID() ).size() == 2 );

    delete first;
    TEST_ASSERT( ParmMgr.GetChildContainers( parent->GetID() ).size() == 1 );
    TEST_ASSERT( ParmMgr.GetChildContainers( parent->GetID() )[0] == second );

    //==== Moved to another parent, and then to none ====//
    ParmContainer* other = new ParmContainer();
    second->SetParentContainer( other->GetID() );
    TEST_ASSERT( ParmMgr.GetChildContainers( parent->GetID() ).empty() );
    TEST_ASSERT( ParmMgr.GetChildContainers( other->GetID() ).size() == 1 );
    second->SetParentContainer( string() );
    TEST_ASSERT( ParmMgr.GetChildContainers( other->GetID() ).empty() );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    delete second;
    delete other;
    delete parent;
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

void ParmIndexTestSuite::AContainersChildrenHaveNoParentOnceItIsGone()
{
    ParmContainer* parent = new ParmContainer();
    ParmContainer child;
    ParmContainer grandchild;
    string parent_id = parent->GetID();
    child.SetParentContainer( parent_id );
    grandchild.SetParentContainer( child.GetID() );

    delete parent;
    TEST_ASSERT( child.GetParentContainer().empty() );
    TEST_ASSERT( ParmMgr.GetChildContainers( parent_id ).empty() );
    TEST_ASSERT( grandchild.GetParentContainer() == child.GetID() );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    grandchild.SetParentContainer( string() );
}

void ParmIndexTestSuite::AParentThatWouldMakeALoopIsRefused()
{
    ParmContainer* a = new ParmContainer();
    ParmContainer* b = new ParmContainer();
    ParmContainer* c = new ParmContainer();
    b->SetParentContainer( a->GetID() );
    c->SetParentContainer( b->GetID() );

    //==== Itself, its child, its grandchild: each refused, and nothing changes ====//
    a->SetParentContainer( a->GetID() );
    a->SetParentContainer( b->GetID() );
    a->SetParentContainer( c->GetID() );
    TEST_ASSERT( a->GetParentContainer().empty() );
    TEST_ASSERT( b->GetParentContainer() == a->GetID() );
    TEST_ASSERT( ParmMgr.GetChildContainers( b->GetID() ).size() == 1 );
    TEST_ASSERT( ParmMgr.GetChildContainers( c->GetID() ).empty() );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );

    //==== A parent beside the chain is taken ====//
    ParmContainer* d = new ParmContainer();
    a->SetParentContainer( d->GetID() );
    TEST_ASSERT( a->GetParentContainer() == d->GetID() );

    c->SetParentContainer( string() );
    b->SetParentContainer( string() );
    a->SetParentContainer( string() );
    delete c;
    delete b;
    delete a;
    delete d;
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

void ParmIndexTestSuite::ACustomGeomsCrossSectionsAreNotListed()
{
    vsp::VSPRenew();
    string cone = vsp::AddGeom( "Cone" );
    string fuse = vsp::AddGeom( "FUSELAGE" );
    vsp::Update();
    TEST_ASSERT( !ParmMgr.FindParmContainer( vsp::GetXSecSurf( cone, 0 ) )->GetListedFlag() );
    TEST_ASSERT( ParmMgr.FindParmContainer( vsp::GetXSecSurf( fuse, 0 ) )->GetListedFlag() );

    //==== Still not listed after the script makes them again ====//
    vsp::SetParmVal( cone, "Height", "Design", 3.0 );
    vsp::Update();
    TEST_ASSERT( !ParmMgr.FindParmContainer( vsp::GetXSecSurf( cone, 0 ) )->GetListedFlag() );
}

void ParmIndexTestSuite::TheTopOfANestedCurveIsItsGeom()
{
    vsp::VSPRenew();
    string fuse = vsp::AddGeom( "FUSELAGE" );
    vsp::Update();

    //==== A cross section's curve, two levels below the Geom's cross section surface ====//
    string xs = vsp::GetXSec( vsp::GetXSecSurf( fuse, 0 ), 1 );
    Parm* width = ParmMgr.FindParm( vsp::GetXSecParm( xs, "Ellipse_Width" ) );
    TEST_ASSERT( width != nullptr );
    ParmContainer* curve = width->GetContainer();
    TEST_ASSERT( curve != nullptr );
    TEST_ASSERT( curve->GetID() != fuse );
    TEST_ASSERT( curve->GetTopContainer() == ParmMgr.FindParmContainer( fuse ) );

    //==== A Custom Geom's: the cross section surface, which is not listed ====//
    string cone = vsp::AddGeom( "Cone" );
    vsp::Update();
    string cxss = vsp::GetXSecSurf( cone, 0 );
    XSec* cxs = dynamic_cast< XSec* >( ParmMgr.FindParmContainer( vsp::GetXSec( cxss, 1 ) ) );
    Parm* cparm = ParmMgr.FindParm( cxs->GetXSecCurve()->GetWidthParmID() );
    TEST_ASSERT( cparm->GetContainer()->GetTopContainer() == ParmMgr.FindParmContainer( cxss ) );

    //==== And a subsurface ====//
    string ss = vsp::AddSubSurf( fuse, vsp::SS_ELLIPSE );
    TEST_ASSERT( ParmMgr.FindParmContainer( ss )->GetTopContainer() == ParmMgr.FindParmContainer( fuse ) );
}

void ParmIndexTestSuite::AContainerWithNoParentIsItsOwnTop()
{
    vsp::VSPRenew();
    string fuse = vsp::AddGeom( "FUSELAGE" );
    ParmContainer* geom = ParmMgr.FindParmContainer( fuse );
    TEST_ASSERT( geom->GetTopContainer() == geom );

    //==== A parent that is gone is no parent ====//
    ParmContainer* parent = new ParmContainer();
    ParmContainer* child = new ParmContainer();
    child->SetParentContainer( parent->GetID() );
    TEST_ASSERT( child->GetTopContainer() == parent );
    delete parent;
    TEST_ASSERT( child->GetTopContainer() == child );
    delete child;
}

void ParmIndexTestSuite::EveryLiveContainerIsListed()
{
    BuildEveryKindOfContainer();

    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );
    TEST_ASSERT( !all.empty() );

    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        TEST_ASSERT( ParmMgr.FindParmContainer( all[i]->GetID() ) == all[i] );
    }

    //==== One more, and then one fewer ====//
    ParmContainer* extra = new ParmContainer();
    vector< ParmContainer* > more;
    ParmMgr.GetAllParmContainers( more );
    TEST_ASSERT( more.size() == all.size() + 1 );
    delete extra;
    vector< ParmContainer* > fewer;
    ParmMgr.GetAllParmContainers( fewer );
    TEST_ASSERT( fewer.size() == all.size() );
}

void ParmIndexTestSuite::ASubsurfacesParentIsItsOwner()
{
    vsp::VSPRenew();
    string wing = vsp::AddGeom( "WING" );
    vsp::Update();

    //==== A Geom's own subsurface: the Geom both ways ====//
    string own = vsp::AddSubSurf( wing, vsp::SS_RECTANGLE );
    SubSurface* gsub = dynamic_cast< SubSurface* >( ParmMgr.FindParmContainer( own ) );
    TEST_ASSERT( gsub && gsub->GetParentContainer() == wing && gsub->GetCompID() == wing );

    //==== A structure's: the structure is the parent, the surface it lies on is still the Geom's ====//
    int istruct = vsp::AddFeaStruct( wing );
    string fea = vsp::GetFeaStructID( wing, istruct );
    vsp::AddFeaSubSurf( wing, istruct, vsp::SS_RECTANGLE );

    //==== As are the lines a line array is made into ====//
    string array = vsp::AddFeaSubSurf( wing, istruct, vsp::SS_LINE_ARRAY );
    vsp::IndividualizeFeaPart( wing, istruct, array );
    vsp::Update();

    vector< string > subs = vsp::GetFeaSubSurfIDVec( fea );
    TEST_ASSERT( subs.size() > 2 );
    for ( int i = 0; i < ( int )subs.size(); i++ )
    {
        SubSurface* sub = dynamic_cast< SubSurface* >( ParmMgr.FindParmContainer( subs[i] ) );
        TEST_ASSERT( sub && sub->GetParentContainer() == fea && sub->GetCompID() == wing );
    }
}

void ParmIndexTestSuite::AStructureIsTheParentOfWhatItHolds()
{
    vsp::VSPRenew();
    string wing = vsp::AddGeom( "WING" );
    vsp::Update();
    int istruct = vsp::AddFeaStruct( wing );
    string fea = vsp::GetFeaStructID( wing, istruct );
    string rib = vsp::AddFeaPart( wing, istruct, vsp::FEA_RIB );
    string bc = vsp::AddFeaBC( fea, vsp::FEA_BC_STRUCTURE );

    TEST_ASSERT( ParmMgr.FindParmContainer( rib )->GetParentContainer() == fea );
    TEST_ASSERT( ParmMgr.FindParmContainer( bc )->GetParentContainer() == fea );

    //==== Its settings, and its skin ====//
    FeaStructure* fs = StructureMgr.GetFeaStruct( fea );
    TEST_ASSERT( fs != nullptr );
    TEST_ASSERT( fs->GetStructSettingsPtr()->GetParentContainer() == fea );
    TEST_ASSERT( fs->GetFeaGridDensityPtr()->GetParentContainer() == fea );
    TEST_ASSERT( fs->GetFeaSkin() && fs->GetFeaSkin()->GetParentContainer() == fea );

    //==== Through a file read ====//
    vsp::WriteVSPFile( "ParmIndexStructure.vsp3" );
    vsp::VSPRenew();
    vsp::ReadVSPFile( "ParmIndexStructure.vsp3" );
    fs = StructureMgr.GetFeaStruct( fea );
    TEST_ASSERT( fs != nullptr );
    vector< FeaPart* > parts = fs->GetFeaPartVec();
    TEST_ASSERT( parts.size() == 2 );
    for ( int i = 0; i < ( int )parts.size(); i++ )
    {
        TEST_ASSERT( parts[i]->GetParentContainer() == fea );
    }
    TEST_ASSERT( fs->GetStructSettingsPtr()->GetParentContainer() == fea );
}

void ParmIndexTestSuite::AMaterialAndAnAssemblyAreTheParentsOfWhatTheyHold()
{
    vsp::VSPRenew();

    //==== A laminate's layers ====//
    string mat = vsp::AddFeaMaterial();
    vsp::SetParmVal( mat, "FeaMaterialType", "FeaMaterial", vsp::FEA_LAMINATE );
    string layer = vsp::AddFeaLayer( mat );
    TEST_ASSERT( ParmMgr.FindParmContainer( layer )->GetParentContainer() == mat );

    //==== An assembly's connections ====//
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    int s1 = vsp::AddFeaStruct( pod );
    int s2 = vsp::AddFeaStruct( pod );
    string id1 = vsp::GetFeaStructID( pod, s1 );
    string id2 = vsp::GetFeaStructID( pod, s2 );
    string fix1 = vsp::AddFeaPart( pod, s1, vsp::FEA_FIX_POINT );
    string fix2 = vsp::AddFeaPart( pod, s2, vsp::FEA_FIX_POINT );
    string assy = vsp::AddFeaAssembly();
    vsp::AddFeaAssemblyStructure( assy, id1 );
    vsp::AddFeaAssemblyStructure( assy, id2 );
    string conn = vsp::AddFeaAssemblyConnection( assy, fix1, id1, 0, fix2, id2, 0 );
    TEST_ASSERT( ParmMgr.FindParmContainer( conn )->GetParentContainer() == assy );
    TEST_ASSERT( StructureMgr.GetFeaAssembly( assy )->m_AssemblySettings.GetParentContainer() == assy );

    //==== Both through a file read ====//
    vsp::WriteVSPFile( "ParmIndexAssembly.vsp3" );
    vsp::VSPRenew();
    vsp::ReadVSPFile( "ParmIndexAssembly.vsp3" );
    TEST_ASSERT( ParmMgr.FindParmContainer( layer )->GetParentContainer() == mat );
    vector< string > conns = vsp::GetFeaAssemblyConnectionIDVec( assy );
    TEST_ASSERT( conns.size() == 1 && ParmMgr.FindParmContainer( conns[0] )->GetParentContainer() == assy );
}

void ParmIndexTestSuite::AGeomIsTheParentOfItsSourcesTexturesAndColor()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    string src = vsp::AddCFDSource( vsp::POINT_SOURCE, pod, 0, 0.1, 0.2, 0.5, 0.5 );
    string tex = vsp::AttachGeomTexture( pod, "ParmIndexTexture.png" );
    Geom* geom = dynamic_cast< Geom* >( ParmMgr.FindParmContainer( pod ) );
    TEST_ASSERT( geom != nullptr );

    TEST_ASSERT( ParmMgr.FindParmContainer( src )->GetParentContainer() == pod );
    TEST_ASSERT( ParmMgr.FindParmContainer( tex )->GetParentContainer() == pod );
    const Color* wire = geom->m_GuiDraw.getColorMgr()->GetWireColor();
    TEST_ASSERT( wire->GetParentContainer() == pod );

    //==== They follow the Geom to a new ID, as a pasted copy gets ====//
    vsp::CopyGeomToClipboard( pod );
    vector< string > pasted = vsp::PasteGeomClipboard();
    TEST_ASSERT( pasted.size() == 1 && pasted[0] != pod );
    Geom* copy = dynamic_cast< Geom* >( ParmMgr.FindParmContainer( pasted[0] ) );
    TEST_ASSERT( copy->m_GuiDraw.getColorMgr()->GetWireColor()->GetParentContainer() == pasted[0] );
    vector< BaseSource* > sources = copy->GetCfdMeshMainSourceVec();
    TEST_ASSERT( sources.size() == 1 && sources[0]->GetParentContainer() == pasted[0] );
    vector< string > textures = copy->m_GuiDraw.getTextureMgr()->GetTextureVec();
    TEST_ASSERT( textures.size() == 1 && ParmMgr.FindParmContainer( textures[0] )->GetParentContainer() == pasted[0] );

    while ( vsp::ErrorMgr.GetNumTotalErrors() > 0 )
    {
        vsp::ErrorMgr.PopLastError();
    }
}

//==== What a container of each nested kind has to sit under ====//
// The kind of the container at the top of its parents, or empty for a kind that is not nested.
static string OwnerKind( ParmContainer* pc )
{
    if ( dynamic_cast< FeaPart* >( pc ) || dynamic_cast< FeaBC* >( pc ) || dynamic_cast< PolySparPoint* >( pc ) ||
         dynamic_cast< StructSettings* >( pc ) || dynamic_cast< FeaGridDensity* >( pc ) )
    {
        return "FeaStructure";
    }
    if ( dynamic_cast< FeaLayer* >( pc ) )
    {
        return "FeaMaterial";
    }
    if ( dynamic_cast< FeaConnection* >( pc ) || dynamic_cast< AssemblySettings* >( pc ) )
    {
        return "FeaAssembly";
    }
    if ( dynamic_cast< RotorDisk* >( pc ) || dynamic_cast< ControlSurfaceGroup* >( pc ) ||
         dynamic_cast< CpSlice* >( pc ) || dynamic_cast< UnsteadyGroup* >( pc ) )
    {
        return "VSPAERO";
    }
    if ( dynamic_cast< SubSurface* >( pc ) )
    {
        return "Geom or FeaStructure";
    }
    if ( dynamic_cast< XSecSurf* >( pc ) || dynamic_cast< XSec* >( pc ) || dynamic_cast< XSecCurve* >( pc ) ||
         dynamic_cast< SkinSpine* >( pc ) || dynamic_cast< PCurve* >( pc ) || dynamic_cast< Bogie* >( pc ) ||
         dynamic_cast< RoutingPoint* >( pc ) || dynamic_cast< BaseSource* >( pc ) || dynamic_cast< Texture* >( pc ) ||
         dynamic_cast< Color* >( pc ) )
    {
        return "Geom";
    }
    return string();
}

static bool IsKind( ParmContainer* pc, const string & kind )
{
    if ( kind == "FeaStructure" )
    {
        return dynamic_cast< FeaStructure* >( pc ) != nullptr;
    }
    if ( kind == "FeaMaterial" )
    {
        return dynamic_cast< FeaMaterial* >( pc ) != nullptr;
    }
    if ( kind == "FeaAssembly" )
    {
        return dynamic_cast< FeaAssembly* >( pc ) != nullptr;
    }
    if ( kind == "VSPAERO" )
    {
        return pc == &VSPAEROMgr;
    }
    if ( kind == "Geom or FeaStructure" )
    {
        return dynamic_cast< Geom* >( pc ) != nullptr || dynamic_cast< FeaStructure* >( pc ) != nullptr;
    }
    if ( kind == "Geom" )
    {
        return dynamic_cast< Geom* >( pc ) != nullptr;
    }
    return false;
}

// Every live container of a nested kind, other than one copied to a clipboard (at the top of a chain whose
// parent is "NONE"), climbs to a container of the kind that owns it.  Returns how many failed, printing each.
static int CountMisplacedContainers()
{
    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );

    int bad = 0;
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        ParmContainer* pc = all[i];
        string kind = OwnerKind( pc );
        ParmContainer* top = pc->GetTopContainer();
        if ( kind.empty() || top->GetParentContainer() == "NONE" )
        {
            continue;
        }

        if ( top == pc || !IsKind( top, kind ) )
        {
            printf( "\n  %s %s \"%s\" (parent \"%s\") sits under %s, not a %s", typeid( *pc ).name(),
                    pc->GetID().c_str(), pc->GetName().c_str(), pc->GetParentContainer().c_str(),
                    typeid( *top ).name(), kind.c_str() );
            bad++;
        }
    }
    return bad;
}

void ParmIndexTestSuite::EveryNestedContainerSitsUnderItsOwner()
{
    BuildEveryKindOfContainer();
    TEST_ASSERT( vsp::GetNumActuatorDisks() > 0 );
    TEST_ASSERT( CountMisplacedContainers() == 0 );
}

void ParmIndexTestSuite::NestedContainersSitUnderTheirOwnerAfterAFileRead()
{
    BuildEveryKindOfContainer();
    vsp::WriteVSPFile( "ParmIndexOwners.vsp3" );
    vsp::VSPRenew();
    vsp::ReadVSPFile( "ParmIndexOwners.vsp3" );
    vsp::Update();
    TEST_ASSERT( CountMisplacedContainers() == 0 );

    vsp::InsertVSPFile( "ParmIndexOwners.vsp3", string() );
    vsp::Update();
    TEST_ASSERT( CountMisplacedContainers() == 0 );
    TEST_ASSERT( ParmMgr.ValidateChildContainerMap() );
}

void ParmIndexTestSuite::AParmIsUnderItsOwnerAtOnce()
{
    // Straight after it is made, with nothing asked of the model in between.
    vsp::VSPRenew();
    string fuse = vsp::AddGeom( "FUSELAGE" );
    string ss = vsp::AddSubSurf( fuse, vsp::SS_XSEC_CURVE );
    Parm* center = ParmMgr.FindParm( vsp::GetParm( ss, "Center_U", "SS_XsecCurve" ) );
    TEST_ASSERT( center && center->GetTopContainerID() == fuse );

    //==== The subsurface's own curve, below the subsurface ====//
    SSXSecCurve* sxc = dynamic_cast< SSXSecCurve* >( ParmMgr.FindParmContainer( ss ) );
    TEST_ASSERT( sxc != nullptr );
    vector< string > curve_parms = vsp::FindContainerParmIDs( sxc->GetXSecCurve()->GetID() );
    TEST_ASSERT( !curve_parms.empty() );
    TEST_ASSERT( ParmMgr.FindParm( curve_parms[0] )->GetTopContainerID() == fuse );

    //==== A shape change makes a new curve; its Parms are under the Geom too ====//
    string xss = vsp::GetXSecSurf( fuse, 0 );
    vsp::ChangeXSecShape( xss, 1, vsp::XS_SUPER_ELLIPSE );
    Parm* m = ParmMgr.FindParm( vsp::GetXSecParm( vsp::GetXSec( xss, 1 ), "Super_M" ) );
    TEST_ASSERT( m && m->GetTopContainerID() == fuse );

    //==== A structure's part, subsurface and condition are under the structure ====//
    string wing = vsp::AddGeom( "WING" );
    int istruct = vsp::AddFeaStruct( wing );
    string fea = vsp::GetFeaStructID( wing, istruct );
    string rib = vsp::AddFeaPart( wing, istruct, vsp::FEA_RIB );
    string fss = vsp::AddFeaSubSurf( wing, istruct, vsp::SS_LINE );
    string bc = vsp::AddFeaBC( fea, vsp::FEA_BC_STRUCTURE );
    vector< string > ids = vsp::FindContainerParmIDs( rib );
    vector< string > fss_ids = vsp::FindContainerParmIDs( fss );
    vector< string > bc_ids = vsp::FindContainerParmIDs( bc );
    ids.insert( ids.end(), fss_ids.begin(), fss_ids.end() );
    ids.insert( ids.end(), bc_ids.begin(), bc_ids.end() );
    TEST_ASSERT( ids.size() > 3 );
    for ( int i = 0; i < ( int )ids.size(); i++ )
    {
        TEST_ASSERT( ParmMgr.FindParm( ids[i] )->GetTopContainerID() == fea );
    }

    //==== A layer is under its material ====//
    string mat = vsp::AddFeaMaterial();
    vsp::SetParmVal( mat, "FeaMaterialType", "FeaMaterial", vsp::FEA_LAMINATE );
    string layer = vsp::AddFeaLayer( mat );
    Parm* thick = ParmMgr.FindParm( vsp::GetParm( layer, "Thickness", "FeaLayer" ) );
    TEST_ASSERT( thick && thick->GetTopContainerID() == mat );
}

// A part's first Parm and the group it is shown in.
static Parm* FirstParmOf( const string & container_id )
{
    vector< string > ids = vsp::FindContainerParmIDs( container_id );
    return ParmMgr.FindParm( ids[0] );
}

void ParmIndexTestSuite::AStructuresPartsAreNumberedAsTheyChange()
{
    vsp::VSPRenew();
    string wing = vsp::AddGeom( "WING" );
    vsp::Update();
    int istruct = vsp::AddFeaStruct( wing );
    string fea = vsp::GetFeaStructID( wing, istruct );
    string r1 = vsp::AddFeaPart( wing, istruct, vsp::FEA_RIB );
    string r2 = vsp::AddFeaPart( wing, istruct, vsp::FEA_RIB );
    string s1 = vsp::AddFeaSubSurf( wing, istruct, vsp::SS_LINE );
    string s2 = vsp::AddFeaSubSurf( wing, istruct, vsp::SS_LINE );

    //==== Two parts of a kind are told apart by their numbered groups, and found by them ====//
    Parm* p1 = FirstParmOf( r1 );
    Parm* p2 = FirstParmOf( r2 );
    TEST_ASSERT( p1->GetName() == p2->GetName() );
    TEST_ASSERT( p1->GetDisplayGroupName() != p2->GetDisplayGroupName() );
    TEST_ASSERT( vsp::FindParm( fea, p1->GetName(), p1->GetDisplayGroupName() ) == p1->GetID() );
    TEST_ASSERT( vsp::FindParm( fea, p2->GetName(), p2->GetDisplayGroupName() ) == p2->GetID() );

    Parm* q1 = FirstParmOf( s1 );
    Parm* q2 = FirstParmOf( s2 );
    TEST_ASSERT( q1->GetDisplayGroupName() != q2->GetDisplayGroupName() );
    TEST_ASSERT( vsp::FindParm( fea, q2->GetName(), q2->GetDisplayGroupName() ) == q2->GetID() );

    //==== Deleting the first of each renumbers the rest, which are found by their new numbers ====//
    string g2 = p2->GetDisplayGroupName();
    vsp::DeleteFeaPart( wing, istruct, r1 );
    vsp::DeleteFeaSubSurf( wing, istruct, s1 );
    TEST_ASSERT( p2->GetDisplayGroupName() != g2 );
    TEST_ASSERT( vsp::FindParm( fea, p2->GetName(), p2->GetDisplayGroupName() ) == p2->GetID() );
    TEST_ASSERT( vsp::FindParm( fea, q2->GetName(), q2->GetDisplayGroupName() ) == q2->GetID() );

    //==== Moving a part renumbers it and those it passes ====//
    string r3 = vsp::AddFeaPart( wing, istruct, vsp::FEA_RIB );
    Parm* p3 = FirstParmOf( r3 );
    string g3 = p3->GetDisplayGroupName();
    FeaStructure* fs = StructureMgr.GetFeaStruct( fea );
    fs->ReorderFeaPart( fs->GetFeaPartIndex( r3 ), vsp::REORDER_MOVE_TOP );
    TEST_ASSERT( p3->GetDisplayGroupName() != g3 );
    TEST_ASSERT( vsp::FindParm( fea, p3->GetName(), p3->GetDisplayGroupName() ) == p3->GetID() );
}

// The display group of a container's own Parms.
static string GroupOf( const string & container_id )
{
    return FirstParmOf( container_id )->GetDisplayGroupName();
}

// The display group names of a container's own Parms.
static std::set< string > DisplayGroupsOf( const string & container_id )
{
    std::set< string > groups;
    vector< string > ids = vsp::FindContainerParmIDs( container_id );
    for ( int i = 0; i < ( int )ids.size(); i++ )
    {
        Parm* p = ParmMgr.FindParm( ids[i] );
        if ( p && p->GetContainer()->GetID() == container_id )
        {
            groups.insert( p->GetDisplayGroupName() );
        }
    }
    return groups;
}

// Whether no two of these containers have a display group name in common among their own Parms.
static bool NoGroupIsShared( const vector< string > & container_ids )
{
    std::set< string > all;
    size_t total = 0;
    for ( int i = 0; i < ( int )container_ids.size(); i++ )
    {
        std::set< string > groups = DisplayGroupsOf( container_ids[i] );
        all.insert( groups.begin(), groups.end() );
        total += groups.size();
    }
    return all.size() == total;
}

void ParmIndexTestSuite::AGeomsSubsurfacesOfEveryKindAreNumbered()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    vector< string > ss;
    ss.push_back( vsp::AddSubSurf( pod, vsp::SS_LINE ) );
    ss.push_back( vsp::AddSubSurf( pod, vsp::SS_ELLIPSE ) );
    ss.push_back( vsp::AddSubSurf( pod, vsp::SS_LINE ) );
    ss.push_back( vsp::AddSubSurf( pod, vsp::SS_RECTANGLE ) );
    ss.push_back( vsp::AddSubSurf( pod, vsp::SS_XSEC_CURVE ) );
    ss.push_back( vsp::AddSubSurf( pod, vsp::SS_INTERSECT ) );

    //==== Kinds share the SubSurface group, an intersection a cross section curve's: still told apart ====//
    TEST_ASSERT( NoGroupIsShared( ss ) );

    //==== A kind's own group is still numbered among that kind ====//
    TEST_ASSERT( vsp::FindParm( pod, "Center_W", "SS_Rectangle_1" ) == vsp::GetParm( ss[3], "Center_W", "SS_Rectangle" ) );
    TEST_ASSERT( vsp::FindParm( pod, "Center_U", "SS_Ellipse_1" ) == vsp::GetParm( ss[1], "Center_U", "SS_Ellipse" ) );

    //==== Moved to the top, the last takes the first's number ====//
    string g0 = GroupOf( ss[0] );
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( pod );
    geom->ReorderSubSurf( ss[3], vsp::REORDER_MOVE_TOP );
    TEST_ASSERT( GroupOf( ss[3] ) == g0 );
    TEST_ASSERT( NoGroupIsShared( ss ) );

    //==== And after one is deleted ====//
    vsp::DeleteSubSurf( pod, ss[1] );
    ss.erase( ss.begin() + 1 );
    TEST_ASSERT( NoGroupIsShared( ss ) );
}

void ParmIndexTestSuite::AGeomsSourcesAreNumbered()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    string s0 = vsp::AddCFDSource( vsp::POINT_SOURCE, pod, 0, 0.1, 0.2, 0.3, 0.5 );
    string s1 = vsp::AddCFDSource( vsp::POINT_SOURCE, pod, 0, 0.2, 0.2, 0.3, 0.5 );

    //==== Two on one Geom are in groups of their own ====//
    TEST_ASSERT( GroupOf( s0 ) != GroupOf( s1 ) );

    //==== Removing the first renumbers the second, taking the first's number ====//
    string g0 = GroupOf( s0 );
    vsp::DeleteCFDSource( pod, 0 );
    TEST_ASSERT( GroupOf( s1 ) == g0 );
}

void ParmIndexTestSuite::AGeomsTexturesAreNumbered()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    string t0 = vsp::AttachGeomTexture( pod, "ParmIndexTexture.png" );
    string t1 = vsp::AttachGeomTexture( pod, "ParmIndexTexture.png" );

    //==== Two on one Geom are in groups of their own ====//
    TEST_ASSERT( GroupOf( t0 ) != GroupOf( t1 ) );

    //==== Removing the first renumbers the second, taking the first's number ====//
    string h0 = GroupOf( t0 );
    vsp::RemoveGeomTexture( pod, t0 );
    TEST_ASSERT( GroupOf( t1 ) == h0 );

    while ( vsp::ErrorMgr.GetNumTotalErrors() > 0 )
    {
        vsp::ErrorMgr.PopLastError();
    }
}

void ParmIndexTestSuite::AGearsBogiesAreNumbered()
{
    vsp::VSPRenew();
    string gear = vsp::AddGeom( "GEAR" );
    vector< string > bogies;
    for ( int i = 0; i < 3; i++ )
    {
        bogies.push_back( vsp::CreateAndAddBogie( gear ) );
    }
    TEST_ASSERT( NoGroupIsShared( bogies ) );

    //==== Removing the first renumbers the rest, the second taking the first's number ====//
    string g0 = GroupOf( bogies[0] );
    vsp::DelBogie( gear, bogies[0] );
    bogies.erase( bogies.begin() );
    TEST_ASSERT( GroupOf( bogies[0] ) == g0 );
    TEST_ASSERT( NoGroupIsShared( bogies ) );

    //==== Still found by its own group name through its own ID ====//
    TEST_ASSERT( !vsp::FindParm( bogies[0], "NumAcross", "Bogie" ).empty() );
}

void ParmIndexTestSuite::ARoutesPointsAreNumbered()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    string route = vsp::AddGeom( "ROUTING" );
    vsp::Update();
    vector< string > pts;
    for ( int i = 0; i < 3; i++ )
    {
        pts.push_back( vsp::AddRoutingPt( route, pod, 0 ) );
    }
    TEST_ASSERT( NoGroupIsShared( pts ) );

    //==== Moving the last to the top gives it the first's number ====//
    string g0 = GroupOf( pts[0] );
    vsp::MoveRoutingPt( route, 2, vsp::REORDER_MOVE_TOP );
    TEST_ASSERT( GroupOf( pts[2] ) == g0 );
    TEST_ASSERT( NoGroupIsShared( pts ) );

    //==== Inserted at the top, and one removed ====//
    pts.push_back( vsp::InsertRoutingPt( route, 0, pod, 0 ) );
    TEST_ASSERT( GroupOf( pts.back() ) == g0 );
    vsp::DelRoutingPt( route, 1 );
    pts.erase( pts.begin() + 2 );
    TEST_ASSERT( NoGroupIsShared( pts ) );
}

void ParmIndexTestSuite::AStructuresConditionsAreNumbered()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    int is = vsp::AddFeaStruct( pod );
    string fea = vsp::GetFeaStructID( pod, is );
    vector< string > bcs;
    for ( int i = 0; i < 3; i++ )
    {
        bcs.push_back( vsp::AddFeaBC( fea, vsp::FEA_BC_STRUCTURE ) );
    }
    TEST_ASSERT( NoGroupIsShared( bcs ) );

    //==== Removing the first renumbers the rest, the second taking the first's number ====//
    string g0 = GroupOf( bcs[0] );
    vsp::DelFeaBC( fea, bcs[0] );
    bcs.erase( bcs.begin() );
    TEST_ASSERT( GroupOf( bcs[0] ) == g0 );
    TEST_ASSERT( NoGroupIsShared( bcs ) );
}

void ParmIndexTestSuite::CpSlicesAreNumbered()
{
    vsp::VSPRenew();
    VSPAEROMgr.ClearCpSliceVec();
    vector< string > slices;
    for ( int i = 0; i < 3; i++ )
    {
        slices.push_back( vsp::AddCpSlice( vsp::Y_DIR, 0.1 * i ) );
    }
    TEST_ASSERT( NoGroupIsShared( slices ) );

    //==== Removing the first renumbers the rest, the second taking the first's number ====//
    string g0 = GroupOf( slices[0] );
    vsp::DelCpSlice( 0 );
    slices.erase( slices.begin() );
    TEST_ASSERT( GroupOf( slices[0] ) == g0 );
    TEST_ASSERT( NoGroupIsShared( slices ) );
    VSPAEROMgr.ClearCpSliceVec();
}

void ParmIndexTestSuite::UnsteadyGroupsAreNumbered()
{
    vsp::VSPRenew();
    VSPAEROMgr.ClearUnsteadyGroupVec();
    UnsteadyGroup* a = VSPAEROMgr.AddUnsteadyGroup();
    UnsteadyGroup* b = VSPAEROMgr.AddUnsteadyGroup();
    string ga = GroupOf( a->GetID() );
    TEST_ASSERT( ga != GroupOf( b->GetID() ) );

    //==== Removing the first renumbers the second, taking the first's number ====//
    VSPAEROMgr.DeleteUnsteadyGroup( vector< int >( 1, 0 ) );
    TEST_ASSERT( GroupOf( b->GetID() ) == ga );
    VSPAEROMgr.ClearUnsteadyGroupVec();
}

void ParmIndexTestSuite::AMaterialsLayersAreNumbered()
{
    vsp::VSPRenew();

    //==== A laminate's layers ====//
    string mat = vsp::AddFeaMaterial();
    vsp::SetParmVal( mat, "FeaMaterialType", "FeaMaterial", vsp::FEA_LAMINATE );
    string l0 = vsp::AddFeaLayer( mat );
    string l1 = vsp::AddFeaLayer( mat );
    string gl0 = GroupOf( l0 );
    TEST_ASSERT( gl0 != GroupOf( l1 ) );
    vsp::DeleteFeaLayer( mat, l0 );
    TEST_ASSERT( GroupOf( l1 ) == gl0 );
}

void ParmIndexTestSuite::AnAssemblysConnectionsAreNumbered()
{
    vsp::VSPRenew();

    //==== An assembly's connections ====//
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    int s1 = vsp::AddFeaStruct( pod );
    int s2 = vsp::AddFeaStruct( pod );
    string id1 = vsp::GetFeaStructID( pod, s1 );
    string id2 = vsp::GetFeaStructID( pod, s2 );
    string a1 = vsp::AddFeaPart( pod, s1, vsp::FEA_FIX_POINT );
    string a2 = vsp::AddFeaPart( pod, s1, vsp::FEA_FIX_POINT );
    string b1 = vsp::AddFeaPart( pod, s2, vsp::FEA_FIX_POINT );
    string assy = vsp::AddFeaAssembly();
    vsp::AddFeaAssemblyStructure( assy, id1 );
    vsp::AddFeaAssemblyStructure( assy, id2 );
    string c0 = vsp::AddFeaAssemblyConnection( assy, a1, id1, 0, b1, id2, 0 );
    string c1 = vsp::AddFeaAssemblyConnection( assy, a2, id1, 0, b1, id2, 0 );
    string gc0 = GroupOf( c0 );
    TEST_ASSERT( gc0 != GroupOf( c1 ) );
    vsp::DeleteFeaAssemblyConnection( assy, 0 );
    TEST_ASSERT( GroupOf( c1 ) == gc0 );
}

void ParmIndexTestSuite::PolySparPointsAreNumberedAcrossTheirStructure()
{
    vsp::VSPRenew();

    //==== Two poly spars' points: numbered across the structure they are both listed under ====//
    string wing = vsp::AddGeom( "WING" );
    vsp::Update();
    int ws = vsp::AddFeaStruct( wing );
    string spar1 = vsp::AddFeaPart( wing, ws, vsp::FEA_POLY_SPAR );
    string spar2 = vsp::AddFeaPart( wing, ws, vsp::FEA_POLY_SPAR );
    vector< string > pts = vsp::GetAllFeaPolySparPtIDVec( spar1 );
    vector< string > pts2 = vsp::GetAllFeaPolySparPtIDVec( spar2 );
    pts.insert( pts.end(), pts2.begin(), pts2.end() );
    TEST_ASSERT( pts.size() >= 4 );
    std::set< string > groups;
    for ( int i = 0; i < ( int )pts.size(); i++ )
    {
        TEST_ASSERT( groups.insert( GroupOf( pts[i] ) ).second );
    }

    //==== And after a point is taken from the first ====//
    vsp::DelFeaPolySparPt( spar1, 0 );
    pts = vsp::GetAllFeaPolySparPtIDVec( spar1 );
    pts2 = vsp::GetAllFeaPolySparPtIDVec( spar2 );
    pts.insert( pts.end(), pts2.begin(), pts2.end() );
    groups.clear();
    for ( int i = 0; i < ( int )pts.size(); i++ )
    {
        TEST_ASSERT( groups.insert( GroupOf( pts[i] ) ).second );
    }
}

void ParmIndexTestSuite::TheParmsUnderAGeomAreItsOwnAndItsPartsInOrder()
{
    vsp::VSPRenew();
    string fuse = vsp::AddGeom( "FUSELAGE" );
    vsp::Update();
    string ss = vsp::AddSubSurf( fuse, vsp::SS_ELLIPSE );
    ParmContainer* geom = ParmMgr.FindParmContainer( fuse );

    vector< string > under;
    geom->GetParmIDsUnder( under );

    //==== Each once; the Geom's own first ====//
    std::set< string > once( under.begin(), under.end() );
    TEST_ASSERT( once.size() == under.size() );
    Parm* first = ParmMgr.FindParm( under[0] );
    TEST_ASSERT( first && first->GetContainer() == geom );

    //==== Every Parm whose container is below the Geom, and no other ====//
    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );
    int expected = 0;
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        if ( all[i]->GetTopContainer() == geom )
        {
            vector< string > own = vsp::FindContainerParmIDs( all[i]->GetID() );
            for ( int j = 0; j < ( int )own.size(); j++ )
            {
                if ( ParmMgr.FindParm( own[j] )->GetContainer() == all[i] )
                {
                    TEST_ASSERT( once.count( own[j] ) == 1 );
                    expected++;
                }
            }
        }
    }
    TEST_ASSERT( expected == ( int )under.size() );

    //==== A cross section's curve and a subsurface among them ====//
    string width = vsp::GetXSecParm( vsp::GetXSec( vsp::GetXSecSurf( fuse, 0 ), 1 ), "Ellipse_Width" );
    TEST_ASSERT( once.count( width ) == 1 );
    TEST_ASSERT( once.count( vsp::GetParm( ss, "Center_U", "SS_Ellipse" ) ) == 1 );

    //==== A container with nothing below it: its own ====//
    vector< string > leaf;
    ParmMgr.FindParmContainer( ss )->GetParmIDsUnder( leaf );
    vector< string > ss_own = vsp::FindContainerParmIDs( ss );
    TEST_ASSERT( std::set< string >( leaf.begin(), leaf.end() ) == std::set< string >( ss_own.begin(), ss_own.end() ) );
}

void ParmIndexTestSuite::AContainerNotListedIsLeftOutOfTheWalkAboveIt()
{
    ParmContainer top;
    ParmContainer hidden;
    ParmContainer below;
    Parm p;
    hidden.SetParentContainer( top.GetID() );
    below.SetParentContainer( hidden.GetID() );
    p.Init( "HiddenParm", "HiddenGroup", &below, 1.0, 0.0, 2.0 );
    hidden.SetListedFlag( false );

    vector< string > ids;
    top.GetParmIDsUnder( ids );
    TEST_ASSERT( ids.empty() );

    //==== It still walks what is below it ====//
    hidden.GetParmIDsUnder( ids );
    TEST_ASSERT( ids.size() == 1 && ids[0] == p.GetID() );

    //==== Listed again, it is walked again ====//
    hidden.SetListedFlag( true );
    ids.clear();
    top.GetParmIDsUnder( ids );
    TEST_ASSERT( ids.size() == 1 && ids[0] == p.GetID() );

    below.SetParentContainer( string() );
    hidden.SetParentContainer( string() );
}

// Every live Parm of every live container.
static void GetAllParms( vector< Parm* > & parms )
{
    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        vector< string > ids;
        all[i]->GetParmIDsUnder( ids );
        for ( int j = 0; j < ( int )ids.size(); j++ )
        {
            Parm* p = ParmMgr.FindParm( ids[j] );
            if ( p && p->GetContainer() == all[i] )
            {
                parms.push_back( p );
            }
        }
    }
}

// Whether a Parm's container, or one between it and its top, is not listed, so that the Parm is not listed
// under its top.
static bool IsBelowAContainerNotListed( Parm* p )
{
    ParmContainer* top = p->GetTopContainer();
    for ( ParmContainer* c = p->GetContainer(); c && c != top; c = c->GetParentContainerPtr() )
    {
        if ( !c->GetListedFlag() )
        {
            return true;
        }
    }
    return false;
}

// The Parms of every live container, by the ID of the container at the top of its parents, but for those
// below a container that is not listed.
static std::map< string, std::set< string > > ParmsByTop()
{
    vector< Parm* > parms;
    GetAllParms( parms );

    std::map< string, std::set< string > > by_top;
    for ( int i = 0; i < ( int )parms.size(); i++ )
    {
        if ( !IsBelowAContainerNotListed( parms[i] ) )
        {
            by_top[ parms[i]->GetTopContainerID() ].insert( parms[i]->GetID() );
        }
    }
    return by_top;
}

void ParmIndexTestSuite::EveryContainerListsExactlyTheParmsUnderIt()
{
    BuildEveryKindOfContainer();
    std::map< string, std::set< string > > by_top = ParmsByTop();

    vector< string > containers = vsp::FindContainers();
    TEST_ASSERT( !containers.empty() );
    for ( int i = 0; i < ( int )containers.size(); i++ )
    {
        vector< string > listed = vsp::FindContainerParmIDs( containers[i] );
        std::set< string > listed_set( listed.begin(), listed.end() );
        TEST_ASSERT( listed_set.size() == listed.size() );
        TEST_ASSERT( listed_set == by_top[ containers[i] ] );
    }
}

// Link all of one container to another of the same kind; whether any Parm is driven twice.
static bool LinkingAllDrivesAParmTwice( const string & a, const string & b )
{
    vsp::LinkAllComp( vsp::FindContainerParmIDs( a )[0], vsp::FindContainerParmIDs( b )[0] );
    std::set< string > driven;
    for ( int i = 0; i < LinkMgr.GetNumLinks(); i++ )
    {
        if ( !driven.insert( LinkMgr.GetLink( i )->GetParmB() ).second )
        {
            return true;
        }
    }
    return driven.empty();
}

void ParmIndexTestSuite::LinkingAllOfAContainerDrivesEachParmOnce()
{
    //==== Two Geoms, each with two sources and two textures ====//
    vsp::VSPRenew();
    string pods[2];
    for ( int g = 0; g < 2; g++ )
    {
        pods[g] = vsp::AddGeom( "POD" );
        vsp::Update();
        for ( int k = 0; k < 2; k++ )
        {
            vsp::AddCFDSource( vsp::POINT_SOURCE, pods[g], 0, 0.1 + k, 0.2, 0.3, 0.5 );
            vsp::AttachGeomTexture( pods[g], "ParmIndexTexture.png" );
        }
    }
    TEST_ASSERT( !LinkingAllDrivesAParmTwice( pods[0], pods[1] ) );

    //==== Two laminates, each with two layers ====//
    vsp::VSPRenew();
    string mats[2];
    for ( int m = 0; m < 2; m++ )
    {
        mats[m] = vsp::AddFeaMaterial();
        vsp::SetParmVal( mats[m], "FeaMaterialType", "FeaMaterial", vsp::FEA_LAMINATE );
        vsp::AddFeaLayer( mats[m] );
        vsp::AddFeaLayer( mats[m] );
    }
    TEST_ASSERT( !LinkingAllDrivesAParmTwice( mats[0], mats[1] ) );

    //==== Two structures, each with two poly spars ====//
    vsp::VSPRenew();
    string wing = vsp::AddGeom( "WING" );
    vsp::Update();
    string structs[2];
    for ( int st = 0; st < 2; st++ )
    {
        int is = vsp::AddFeaStruct( wing );
        structs[st] = vsp::GetFeaStructID( wing, is );
        vsp::AddFeaPart( wing, is, vsp::FEA_POLY_SPAR );
        vsp::AddFeaPart( wing, is, vsp::FEA_POLY_SPAR );
    }
    TEST_ASSERT( !LinkingAllDrivesAParmTwice( structs[0], structs[1] ) );

    while ( vsp::ErrorMgr.GetNumTotalErrors() > 0 )
    {
        vsp::ErrorMgr.PopLastError();
    }
}

void ParmIndexTestSuite::TheChangeCountMovesWhenAContainerTakesAParent()
{
    ParmContainer top;
    ParmContainer below;
    int count = ParmMgr.GetNumParmChanges();
    below.SetParentContainer( top.GetID() );
    TEST_ASSERT( ParmMgr.GetNumParmChanges() != count );

    //==== But not when the parent is the one it had ====//
    count = ParmMgr.GetNumParmChanges();
    below.SetParentContainer( top.GetID() );
    TEST_ASSERT( ParmMgr.GetNumParmChanges() == count );

    below.SetParentContainer( string() );
}

// Whether a container's listing holds exactly the Parms under it, its groups sorted and each group's Parms in
// that group and sorted by name.
static bool ListingIsSound( ParmContainer* pc )
{
    const ParmListing & listing = ParmMgr.GetListing( pc );
    for ( int i = 1; i < ( int )listing.m_GroupNames.size(); i++ )
    {
        if ( StringUtil::natural_compare( listing.m_GroupNames[ i - 1 ], listing.m_GroupNames[i] ) > 0 )
        {
            return false;
        }
    }
    if ( listing.m_GroupNames.size() != listing.m_GroupParmIDs.size() )
    {
        return false;
    }

    std::multiset< string > listed;
    for ( int i = 0; i < ( int )listing.m_GroupNames.size(); i++ )
    {
        const vector< string > & ids = listing.m_GroupParmIDs.at( listing.m_GroupNames[i] );
        for ( int j = 0; j < ( int )ids.size(); j++ )
        {
            Parm* p = ParmMgr.FindParm( ids[j] );
            if ( !p || p->GetDisplayGroupName() != listing.m_GroupNames[i] )
            {
                return false;
            }
            if ( j > 0 && StringUtil::natural_compare( ParmMgr.FindParm( ids[ j - 1 ] )->GetName(), p->GetName() ) > 0 )
            {
                return false;
            }
            listed.insert( ids[j] );
        }
    }

    vector< string > under;
    pc->GetParmIDsUnder( under );
    return listed == std::multiset< string >( under.begin(), under.end() );
}

void ParmIndexTestSuite::EveryListingIsSorted()
{
    BuildEveryKindOfContainer();
    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        TEST_ASSERT( ListingIsSound( all[i] ) );
    }
}

void ParmIndexTestSuite::TheListingFollowsEachChange()
{
    ParmContainer top;
    ParmContainer below;
    ParmContainer other;
    Parm a;
    Parm b;
    a.Init( "A", "GroupA", &top, 1.0, 0.0, 2.0 );
    b.Init( "B", "GroupB", &below, 1.0, 0.0, 2.0 );

    //==== Not below it yet ====//
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupNames == vector< string >( 1, "GroupA" ) );

    //==== Taken as a child ====//
    below.SetParentContainer( top.GetID() );
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupNames.size() == 2 );

    //==== Renumbered ====//
    b.SetGroupDisplaySuffix( 3 );
    TEST_ASSERT( ListingIsSound( &top ) );
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupParmIDs.count( b.GetDisplayGroupName() ) == 1 );

    //==== Renamed into the other's group ====//
    b.SetGroupDisplaySuffix( -1 );
    b.SetGroupName( "GroupA" );
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupNames == vector< string >( 1, "GroupA" ) );
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupParmIDs.at( "GroupA" ).size() == 2 );

    //==== Not listed ====//
    below.SetListedFlag( false );
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupParmIDs.at( "GroupA" ).size() == 1 );
    below.SetListedFlag( true );

    //==== Moved to another parent ====//
    below.SetParentContainer( other.GetID() );
    TEST_ASSERT( ParmMgr.GetListing( &top ).m_GroupParmIDs.at( "GroupA" ).size() == 1 );
    TEST_ASSERT( ParmMgr.GetListing( &other ).m_GroupParmIDs.at( "GroupA" ).size() == 1 );

    below.SetParentContainer( string() );
}

void ParmIndexTestSuite::ThePickerPicksBackTheParmItShows()
{
    BuildEveryKindOfContainer();
    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );

    int checked = 0;
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        vector< string > ids;
        all[i]->GetParmIDsUnder( ids );
        if ( ids.empty() || all[i]->GetTopContainer() != all[i] )
        {
            continue;
        }

        vector< string > containers, groups, parms;
        int ci = LinkMgr.GetCurrContainerVec( ids.back(), containers );
        int gi = LinkMgr.GetCurrGroupNameVec( ids.back(), groups );
        int pi = LinkMgr.GetCurrParmIDVec( ids.back(), parms );
        if ( LinkMgr.FindParmContainer( ci ) != all[i] )
        {
            continue;    // A container the pickers do not offer.
        }
        TEST_ASSERT( groups[ gi ] == ParmMgr.FindParm( ids.back() )->GetDisplayGroupName() );
        TEST_ASSERT( parms[ pi ] == ids.back() );
        TEST_ASSERT( LinkMgr.PickParm( ci, gi, pi ) == ids.back() );
        checked++;
    }
    TEST_ASSERT( checked > 10 );
}

void ParmIndexTestSuite::EveryListedParmIsFoundByName()
{
    BuildEveryKindOfContainer();

    int found = 0;
    vector< string > containers = vsp::FindContainers();
    for ( int i = 0; i < ( int )containers.size(); i++ )
    {
        //==== The first in the list with each name and group is the one found ====//
        std::set< string > seen;
        vector< string > ids = vsp::FindContainerParmIDs( containers[i] );
        for ( int j = 0; j < ( int )ids.size(); j++ )
        {
            Parm* p = ParmMgr.FindParm( ids[j] );
            if ( seen.insert( p->GetName() + "/" + p->GetDisplayGroupName() ).second )
            {
                TEST_ASSERT( vsp::FindParm( containers[i], p->GetName(), p->GetDisplayGroupName() ) == ids[j] );
                found++;
            }
        }
    }
    TEST_ASSERT( found > 1000 );
}

void ParmIndexTestSuite::AContainersOwnParmIsFoundFirst()
{
    ParmContainer top;
    ParmContainer below;
    Parm child_parm;
    Parm own_parm;

    //==== The child's Parm is made first ====//
    below.SetParentContainer( top.GetID() );
    child_parm.Init( "Shared", "SharedGroup", &below, 1.0, 0.0, 2.0 );
    own_parm.Init( "Shared", "SharedGroup", &top, 1.0, 0.0, 2.0 );

    TEST_ASSERT( ParmMgr.FindParmUnder( &top, "Shared", "SharedGroup" ) == own_parm.GetID() );
    TEST_ASSERT( ParmMgr.FindParmUnder( &below, "Shared", "SharedGroup" ) == child_parm.GetID() );

    below.SetParentContainer( string() );
}

void ParmIndexTestSuite::AContainersOwnParmIsFoundByItsGroupName()
{
    vsp::VSPRenew();
    string pod = vsp::AddGeom( "POD" );
    vsp::Update();
    string ss = vsp::AddSubSurf( pod, vsp::SS_ELLIPSE );
    vector< string > ids = vsp::FindContainerParmIDs( ss );
    Parm* center = nullptr;
    for ( int i = 0; i < ( int )ids.size(); i++ )
    {
        if ( ParmMgr.FindParm( ids[i] )->GetName() == "Center_U" )
        {
            center = ParmMgr.FindParm( ids[i] );
        }
    }
    TEST_ASSERT( center && center->GetDisplayGroupName() != center->GetGroupName() );
    if ( !center )
    {
        return;
    }

    //==== By its display group name, and by its group name without the number the display name adds ====//
    TEST_ASSERT( vsp::FindParm( ss, "Center_U", center->GetDisplayGroupName() ) == center->GetID() );
    TEST_ASSERT( vsp::FindParm( ss, "Center_U", center->GetGroupName() ) == center->GetID() );

    //==== Under the Geom, by its display group name ====//
    TEST_ASSERT( vsp::FindParm( pod, "Center_U", center->GetDisplayGroupName() ) == center->GetID() );
}

void ParmIndexTestSuite::AParmIsFoundAsSoonAsItIsMade()
{
    vsp::VSPRenew();
    for ( int i = 0; i < 20; i++ )
    {
        string pod = vsp::AddGeom( "POD" );
        TEST_ASSERT( !vsp::FindParm( pod, "X_Rel_Location", "XForm" ).empty() );
        string ss = vsp::AddSubSurf( pod, vsp::SS_LINE );
        TEST_ASSERT( vsp::FindParm( ss, "Const_Line_Value", "SubSurface" ) == vsp::GetParm( ss, "Const_Line_Value", "SubSurface" ) );
        TEST_ASSERT( !vsp::FindParm( ss, "Const_Line_Value", "SubSurface" ).empty() );
    }
}

static bool Contains( const vector< string > & vec, const string & val )
{
    return std::find( vec.begin(), vec.end(), val ) != vec.end();
}

void ParmIndexTestSuite::TheContainerListFollowsTheModel()
{
    vsp::VSPRenew();
    string a = vsp::AddGeom( "POD" );
    TEST_ASSERT( Contains( vsp::FindContainers(), a ) );

    string b = vsp::AddGeom( "POD" );
    vector< string > containers = vsp::FindContainers();
    TEST_ASSERT( Contains( containers, b ) );
    TEST_ASSERT( std::set< string >( containers.begin(), containers.end() ).size() == containers.size() );

    vsp::DeleteGeom( a );
    TEST_ASSERT( !Contains( vsp::FindContainers(), a ) );

    //==== A Geom held on the clipboard is not in the model ====//
    vsp::CopyGeomToClipboard( b );
    containers = vsp::FindContainers();
    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );
    int geoms = 0;
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        if ( dynamic_cast< Geom* >( all[i] ) && all[i]->GetID() != b )
        {
            TEST_ASSERT( !Contains( containers, all[i]->GetID() ) );
            geoms++;
        }
    }
    TEST_ASSERT( geoms >= 1 );

    //==== Pasted, it is ====//
    vector< string > pasted = vsp::PasteGeomClipboard();
    TEST_ASSERT( pasted.size() == 1 && Contains( vsp::FindContainers(), pasted[0] ) );
}

// Every container at the top of its parents that holds Parms, itself or below it, is offered by FindContainers --
// but for a link or a mode, whose Parms are its own settings, a container that is not listed, and a container
// held on a clipboard, whose top's parent is "NONE".  Returns how many are not, printing each.
static int CountTopsNotListed()
{
    vector< string > listed = vsp::FindContainers();
    std::set< string > listed_set( listed.begin(), listed.end() );

    vector< ParmContainer* > all;
    ParmMgr.GetAllParmContainers( all );
    int missing = 0;
    for ( int i = 0; i < ( int )all.size(); i++ )
    {
        ParmContainer* pc = all[i];
        vector< string > ids;
        pc->GetParmIDsUnder( ids );
        if ( pc->GetTopContainer() != pc || ids.empty() || pc->GetParentContainer() == "NONE" || !pc->GetListedFlag() ||
             dynamic_cast< Link* >( pc ) || dynamic_cast< Mode* >( pc ) || listed_set.count( pc->GetID() ) )
        {
            continue;
        }
        printf( "Not listed: %s %s, %d Parms\n", typeid( *pc ).name(), pc->GetName().c_str(), ( int )ids.size() );
        missing++;
    }
    return missing;
}

void ParmIndexTestSuite::EveryTopWithParmsIsListed()
{
    BuildEveryKindOfContainer();
    vsp::AddGeom( "Cone" );
    vsp::Update();
    TEST_ASSERT( CountTopsNotListed() == 0 );

    vsp::VSPRenew();
    TEST_ASSERT( CountTopsNotListed() == 0 );
}

void ParmIndexTestSuite::NoCustomScriptIsRunningBetweenCalls()
{
    vsp::VSPRenew();
    vsp::AddGeom( "Cone" );
    vsp::Update();
    TEST_ASSERT( CustomGeomMgr.GetRunningCustomGeom().empty() );
}
