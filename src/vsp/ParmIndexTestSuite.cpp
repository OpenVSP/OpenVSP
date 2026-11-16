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
#include "APIDefines.h"
#include "ParmMgr.h"
#include "ParmContainer.h"

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
