//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmIndexTestSuite.cpp: Tests of how containers and Parms are found under the containers that
// hold them.
//
//////////////////////////////////////////////////////////////////////

#include "ParmIndexTestSuite.h"
#include "VSP_Geom_API.h"
#include "APIErrorMgr.h"
#include "APIDefines.h"
#include "ParmMgr.h"
#include "ParmContainer.h"
#include "Parm.h"
#include "SubSurface.h"
#include "FeaStructure.h"
#include "StructureMgr.h"
#include "Geom.h"
#include "GridDensity.h"
#include "Color.h"

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
