//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "PodGeom.h"
#include "FuselageGeom.h"
#include "WingGeom.h"
#include "BlankGeom.h"
#include "MeshGeom.h"
#include "StackGeom.h"
#include "CustomGeom.h"
#include "ScriptMgr.h"
#include "MessageMgr.h"
#include "StlHelper.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "Quat.h"
#include "StringUtil.h"
#include "DesignVarMgr.h"
#include "XmlUtil.h"
#include "APIDefines.h"
#include "ResultsMgr.h"
using namespace vsp;

#include <utility>

#define NUMOFLIGHTS 8

//==== Constructor ====//
Vehicle::Vehicle()
{
    Init();

    // Initialize Lights.
    m_Lights.resize( NUMOFLIGHTS );
    for( int i = 0; i < NUMOFLIGHTS; i++ )
    {
        char name[256];

        LightInfo light;
        m_Lights[i] = light;

        sprintf( name, "Light%d_Active", i );
        m_Lights[i].Active.Init( name, "Lighting", this, 0, 0, 0, false );
        m_Lights[i].Active.Set( false );

        sprintf( name, "Light%d_X", i );
        m_Lights[i].XPos.Init( name, "Lighting", this, 0, -100.0, 100.0, false );

        sprintf( name, "Light%d_Y", i );
        m_Lights[i].YPos.Init( name, "Lighting", this, 0, -100.0, 100.0, false );

        sprintf( name, "Light%d_Z", i );
        m_Lights[i].ZPos.Init( name, "Lighting", this, 0, -100.0, 100.0, false );

        sprintf( name, "Light%d_Amb", i );
        m_Lights[i].Amb.Init( name, "Lighting", this, 0, 0, 1, false );

        sprintf( name, "Light%d_Diff", i );
        m_Lights[i].Diff.Init( name, "Lighting", this, 0, 0, 1, false );

        sprintf( name, "Light%d_Spec", i );
        m_Lights[i].Spec.Init( name, "Lighting", this, 0, 0, 1, false );
    }

    m_Lights[0].Active = true;
    m_Lights[0].XPos = 10.0;
    m_Lights[0].YPos = -50.0;
    m_Lights[0].ZPos = 20.0;
    m_Lights[0].Amb = 0.5;
    m_Lights[0].Diff = 0.35;
    m_Lights[0].Spec = 1.0;

    m_Lights[1].Active = true;
    m_Lights[1].XPos = 10.0;
    m_Lights[1].YPos = 15.0;
    m_Lights[1].ZPos = 30.0;
    m_Lights[1].Amb = 0.5;
    m_Lights[1].Diff = 0.5;
    m_Lights[1].Spec = 1.0;

    m_Lights[2].Active = true;
    m_Lights[2].XPos = -50.0;
    m_Lights[2].YPos = 30.0;
    m_Lights[2].ZPos = 10.0;
    m_Lights[2].Amb = 0.0;
    m_Lights[2].Diff = 0.5;
    m_Lights[2].Spec = 0.5;

    m_BbXLen.Init( "X_Len", "BBox", this, 0, 0, 1e12 );
    m_BbXLen.SetDescript( "X length of vehicle bounding box" );
    m_BbYLen.Init( "Y_Len", "BBox", this, 0, 0, 1e12 );
    m_BbYLen.SetDescript( "Y length of vehicle bounding box" );
    m_BbZLen.Init( "Z_Len", "BBox", this, 0, 0, 1e12 );
    m_BbZLen.SetDescript( "Z length of vehicle bounding box" );
    m_BbXMin.Init( "X_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbXMin.SetDescript( "Minimum X coordinate of vehicle bounding box" );
    m_BbYMin.Init( "Y_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbYMin.SetDescript( "Minimum Y coordinate of vehicle bounding box" );
    m_BbZMin.Init( "Z_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbZMin.SetDescript( "Minimum Z coordinate of vehicle bounding box" );
}

//==== Destructor ====//
Vehicle::~Vehicle()
{
    LinkMgr.UnRegisterContainer( this->GetID() );

    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        delete m_GeomStoreVec[i];
    }

}

//=== Init ====//
void Vehicle::Init()
{
    //==== Init Custom Geom and Script Mgr ====//
    CustomGeomMgr.Init();
    ScriptMgr.Init();
    CustomGeomMgr.ReadCustomScripts();

    m_Name = "Vehicle";

    m_VSP3FileName = "Unnamed.vsp3";
    m_FileOpenVersion = -1;

    //==== Load Default Set Names =====//
    m_SetNameVec.push_back( "All" );        // SET_ALL
    m_SetNameVec.push_back( "Shown" );      // SET_SHOWN
    m_SetNameVec.push_back( "Not_Shown" );  // SET_NOT_SHOWN
    for ( int i = 0 ; i < 10 ; i++ )
    {
        char str[256];
        sprintf( str, "Set_%d", i );
        m_SetNameVec.push_back( str );
    }

    //==== Load Geom Types =====//
    m_GeomTypeVec.push_back( GeomType( POD_GEOM_TYPE, "POD", true ) );
    m_GeomTypeVec.push_back( GeomType( FUSELAGE_GEOM_TYPE, "FUSELAGE", true ) );
    m_GeomTypeVec.push_back( GeomType( MS_WING_GEOM_TYPE, "WING", true ) );
    m_GeomTypeVec.push_back( GeomType( STACK_GEOM_TYPE, "STACK", true ) );
    m_GeomTypeVec.push_back( GeomType( BLANK_GEOM_TYPE, "BLANK", true ) );
    m_GeomTypeVec.push_back( GeomType( CUSTOM_GEOM_TYPE, "CUSTOM", true ) );

    //==== Get Custom Geom Types =====//
    vector< GeomType > custom_types = CustomGeomMgr.GetCustomTypes();
    for ( int i = 0 ; i < ( int ) custom_types.size() ; i++ )
    {
        m_GeomTypeVec.push_back( custom_types[i] );
    }

    LinkMgr.RegisterContainer( this->GetID() );

    //==== Export Files ====//
    m_exportCompGeomCsvFile = false;
    m_exportDragBuildTsvFile = false;
    m_compGeomTxtFileName = "VspAircraft_CompGeom.txt";
    m_compGeomCsvFileName = "VspAircraft_CompGeom.csv";
    m_compGeomTsvFileName = "VspAircraft_DragBuild.tsv";
    m_MassPropFileName = "VspAircraft_MassProps.txt";
    m_AwaveFileName = "VspAircraft_AwaveSlice.txt";

    m_IxxIyyIzz = vec3d( 0, 0, 0 );
    m_IxyIxzIyz = vec3d( 0, 0, 0 );
    m_CG = vec3d( 0, 0, 0 );
    m_NumMassSlices = 20;
    m_TotalMass = 0;
}

void Vehicle::RunTestScripts()
{
    ScriptMgr.RunTestScripts();
}

//=== Wype ===//
void Vehicle::Wype()
{
    // Re-initialize everything to default.  Care must be taken with dynamic memory
    // or cases where outside pointers access items being freed.  Otherwise, just
    // clobber it with a default assignment, let the compiler take care of cleaning
    // up the variables on the stack.

    // Remove references to this set up in Init()
    LinkMgr.UnRegisterContainer( this->GetID() );

    // Public member variables
    m_IxxIyyIzz = vec3d();
    m_IxyIxzIyz = vec3d();
    m_CG = vec3d();
    m_NumMassSlices = int();
    m_TotalMass = double();


    // Private member variables
    m_Name = string();

    m_VSP3FileName = string();

//  ParmMgr.RemoveParm( &m_TestParm );
//  m_TestParm = Parm();

    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        delete m_GeomStoreVec[i];
    }

    m_GeomStoreVec = vector< Geom* >();

    m_ActiveGeom = vector< string >();
    m_TopGeom = vector< string >();
    m_ClipBoard = vector< string >();

    m_ParmUndoStack = stack< ParmUndo >();

    m_SetNameVec = vector< string >();

    m_GeomTypeVec = vector< GeomType >();

    m_BBox = BndBox();

    m_compGeomTxtFileName = string();
    m_compGeomCsvFileName = string();
    m_compGeomTsvFileName = string();
    m_MassPropFileName = string();
    m_AwaveFileName = string();
    m_exportCompGeomCsvFile = bool();
    m_exportDragBuildTsvFile = bool();

    // Clear out various managers...
    LinkMgr.Renew();
    DesignVarMgr.Renew();
}

//=== NewFile ===//
// Reset VSP state to nearly that of startup.  Leave clipboard contents intact.
void Vehicle::Renew()
{
    Wype();
    Init();
}

//==== Parm Changed ====//
void Vehicle::ParmChanged( Parm* parm_ptr, int type )
{
//    UpdateBBox();
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

//==== Undo Last Parameter Change ====//
void Vehicle::UnDo()
{
    ParmMgr.UnDo();
}

////=== Get Parm Based on GeomID and Parm Name and Group_Name ====//
//Parm* Vehicle::FindParm( string geom_id, const string& parm_name, const string& group_name )
//{
//  Geom* geom_ptr = FindGeom( geom_id );
//  if ( geom_ptr )
//  {
//      Parm* p = ParmMgr.FindParm( parm_name, group_name, geom_id );
//      return p;
//  }
//  return NULL;
//}

void Vehicle::Update()
{
    for ( int i = 0 ; i < ( int )m_TopGeom.size() ; i++ )
    {
        Geom* g_ptr = FindGeom( m_TopGeom[i] );
        if ( g_ptr )
        {
            g_ptr->Update();
        }
    }
}

//==== Find Geom Based on GeomID ====//
Geom* Vehicle::FindGeom( string geom_id )
{
    if ( geom_id == string( "NONE" ) )
    {
        return NULL;
    }
    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        if ( m_GeomStoreVec[i]->IsMatch( geom_id ) )
        {
            return m_GeomStoreVec[i];
        }
    }
    return NULL;
}

//==== Find Vector of Geom Ptrs Based on GeomID ====//
vector< Geom* > Vehicle::FindGeomVec( const vector< string > & geom_id_vec )
{
    vector< Geom* > geom_vec;
    for ( int i = 0 ; i < ( int )geom_id_vec.size() ; i++ )
    {
        Geom* gptr = FindGeom( geom_id_vec[i] );
        if ( gptr != NULL )
        {
            geom_vec.push_back( gptr );
        }
    }
    return geom_vec;
}


//=== Create Geom of Type, Add To Storage and Return ID ====//
string Vehicle::CreateGeom( const GeomType & type )
{
    Geom* new_geom = NULL;
    if ( type.m_Type == POD_GEOM_TYPE )
    {
        new_geom = new PodGeom( this );
    }
    else if ( type.m_Type == FUSELAGE_GEOM_TYPE )
    {
        new_geom = new FuselageGeom( this );
    }
    else if ( type.m_Type == MS_WING_GEOM_TYPE )
    {
        new_geom = new WingGeom( this );
    }
    else if ( type.m_Type == BLANK_GEOM_TYPE )
    {
        new_geom = new BlankGeom( this );
    }
    else if ( type.m_Type == MESH_GEOM_TYPE )
    {
        new_geom = new MeshGeom( this );
    }
    else if ( type.m_Type == STACK_GEOM_TYPE )
    {
        new_geom = new StackGeom( this );
    }
    else if ( type.m_Type == CUSTOM_GEOM_TYPE )
    {
        new_geom = new CustomGeom( this );
    }

    if ( !new_geom )
    {
        return "NONE";
    }

    new_geom->Update();
    m_GeomStoreVec.push_back( new_geom );

    Geom* type_geom_ptr = FindGeom( type.m_GeomID );
    if ( type_geom_ptr )
    {
        string id = new_geom->GetID();
        new_geom->CopyFrom( type_geom_ptr );
        new_geom->SetName( type.m_Name );
        new_geom->Update();
    }

    return new_geom->GetID();
}

//=== Create Geom and Set Up Parent/Child ====//
string Vehicle::AddGeom( GeomType & type )
{
    string add_id = CreateGeom( type );
    Geom* add_geom = FindGeom( add_id );

    string geom_id =  AddGeom( add_geom );

    if ( type.m_Type == CUSTOM_GEOM_TYPE )
    {
        add_geom->SetType( type );
        CustomGeomMgr.InitGeom( geom_id );
        add_geom->Update();
    }

    return geom_id;
}


//=== Create Geom and Set Up Parent/Child ====//
string Vehicle::AddGeom( Geom* add_geom )
{
    string add_id = add_geom->GetID();

    if ( !add_geom )
    {
        return string( "NONE" );
    }

    //==== Set Parent/Child ====//
    vector< string > active_vec = GetActiveGeomVec();
    if ( active_vec.size()  )
    {
        string parent_id = active_vec.back();           // Parent
        Geom* parent_geom = FindGeom( parent_id );
        if ( parent_geom )
        {
            add_geom->SetParentID( parent_geom->GetID() );      // Set Parent ID
            parent_geom->AddChildID( add_id );                  // Add Child ID
        }
    }
    else
    {
        m_TopGeom.push_back( add_id );
    }

    UpdateBBox();
    return add_id;
}

string Vehicle::AddMeshGeom( int set )
{
    ClearActiveGeom();

    vector<string> geom_vec = GetGeomVec(); // Get geom vec before mesh is added

    GeomType type = GeomType( MESH_GEOM_TYPE, "MESH", true );
    string id = AddGeom( type );
    Geom* geom_ptr = FindGeom( id );
    if ( !geom_ptr )
    {
        return string( "NONE" );
    }

    MeshGeom* mesh_geom = ( MeshGeom* )( geom_ptr );

    if ( set > 2 )
    {
        mesh_geom->SetSetFlag( set, true );
    }
    else
    {
        mesh_geom->SetSetFlag( 1, true );    // Ensure Shown Flag is Set
    }

    // Create TMeshVec
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* geom_ptr = FindGeom( geom_vec[i] );
        if ( geom_ptr )
        {
            if ( geom_ptr->GetSetFlag( set ) )
            {
                vector< TMesh* > tMeshVec = geom_ptr->CreateTMeshVec();
                for ( int j = 0 ; j < ( int )tMeshVec.size() ; j++ )
                {
                    mesh_geom->m_TMeshVec.push_back( tMeshVec[j] );
                }
            }

            // No Show All Other Components
            geom_ptr->SetSetFlag( 1, false ); //remove from shown
            geom_ptr->SetSetFlag( 2, true ); //add to no show
        }
    }

    SetActiveGeom( id );
    return id;
}

//==== Traverse Top Geoms And Get All Geoms - Check Display Flag if True ====//
vector< string > Vehicle::GetGeomVec( bool check_display_flag )
{
    vector< string > geom_vec;
    for ( int i = 0 ; i < ( int )m_TopGeom.size() ; i++ )
    {
        Geom* g_ptr = FindGeom( m_TopGeom[i] );
        if ( g_ptr )
        {
            g_ptr->LoadIDAndChildren( geom_vec, check_display_flag );
        }
    }
    return geom_vec;
}

//==== Add ID to Active Geom Vec ====//
void Vehicle::AddActiveGeom( string id )
{
    if ( FindGeom( id ) )
    {
        m_ActiveGeom.push_back( id );
    }
}

//==== Add ID to Cleard Active Geom Vec ====//
void Vehicle::SetActiveGeom( string id )
{
    if ( FindGeom( id ) )
    {
        m_ActiveGeom.clear();
        m_ActiveGeom.push_back( id );
    }
}

//==== Is This Geom Active? ====//
bool Vehicle::IsGeomActive( string geom_id )
{
    return vector_contains_val( m_ActiveGeom, geom_id );
}

void Vehicle::CutGeomVec( vector< string > cut_vec )
{
    //=== Build Ancestor ID vector before changing hierarchy ===//
    vector< string > ancest_vec;
    ancest_vec.resize( cut_vec.size() );

    //=== All Geoms To Be Cut ====//
    for ( int c = 0 ; c < ( int )cut_vec.size() ; c++ )
    {
        string id = cut_vec[c];
        Geom* gPtr = FindGeom( id );
        if ( gPtr )
        {
            //==== Check For Parent ====//
            Geom* ancestPtr = FindGeom( gPtr->GetParentID() );
            string ancestID;

            if ( ancestPtr )
            {
                ancestID = ancestPtr->GetID();
            }
            while ( ancestPtr && vector_contains_val( cut_vec, ancestID ) ) // Loop until ancestor out of cut list.
            {
                ancestID = ancestPtr->GetParentID();
                ancestPtr = FindGeom( ancestID );
            }
            ancest_vec[c] = ancestID;
        }
    }

    //=== All Geoms To Be Cut ====//
    for ( int c = 0 ; c < ( int )cut_vec.size() ; c++ )
    {
        string id = cut_vec[c];
        Geom* gPtr = FindGeom( id );
        if ( gPtr )
        {
            //==== Check For Parent ====//
            Geom* parentPtr = FindGeom( gPtr->GetParentID() );
            if ( parentPtr )
            {
                //=== Check If Parent is In List Of IDs to Cut ====//
                if ( !vector_contains_val( cut_vec, parentPtr->GetID() ) )  // Parent is outside cut list.
                {
                    gPtr->SetParentID( "NONE" );            // Reassign Parent
                    parentPtr->RemoveChildID( id );         // Remove From Parents Child Vec
                }
            }

            //==== Children ====//
            vector< string > child_vec = gPtr->GetChildIDVec();
            for ( int i = 0 ; i < ( int )child_vec.size() ; i++ )
            {
                string child_id = child_vec[i];
                if ( !vector_contains_val( cut_vec, child_id ) )        // Child is outside cut list.
                {
                    Geom* childPtr = FindGeom( child_id );
                    Geom* ancestPtr = FindGeom( ancest_vec[c] );
                    if ( childPtr && ancestPtr )                        // Ancestor and child exist
                    {
                        ancestPtr->AddChildID( child_id );
                        childPtr->SetParentID( ancestPtr->GetID() );
                    }
                    else                                                // No ancestor
                    {
                        childPtr->SetParentID( "NONE" );
                        m_TopGeom.push_back( child_id );
                    }
                    gPtr->RemoveChildID( child_id );
                }
            }
            m_ClipBoard.push_back( id );
        }

        //==== Remove From Top Geom Deque ====//
        vector_remove_val( m_TopGeom, id );
    }

    //==== Make Sure Destructor is Called On Cut Geoms ===//
    vector< string > stored = CopyGeomVec( m_ClipBoard );
    DeleteClipBoard();
    m_ClipBoard = stored;
}

//==== Reorder Active Geom  ====//
void Vehicle::ReorderActiveGeom( int action )
{
    //==== Find Active Geom ====//
    vector< string > active_geom_vec = GetActiveGeomVec();
    if ( active_geom_vec.size() != 1 )
    {
        return;
    }

    string active_geom_id = active_geom_vec[0];
    Geom* active_geom = FindGeom( active_geom_id );
    if ( !active_geom )
    {
        return;
    }

    vector< string > id_vec;
    string parent_id = active_geom->GetParentID();
    Geom* parent_geom = FindGeom( parent_id );
    if ( !parent_geom )
    {
        id_vec = m_TopGeom;
    }
    else
    {
        id_vec = parent_geom->GetChildIDVec();
    }

    vector< string > new_id_vec;
    if ( action == REORDER_MOVE_TOP || action == REORDER_MOVE_BOTTOM )
    {
        if ( action == REORDER_MOVE_TOP )
        {
            new_id_vec.push_back( active_geom_id );
        }

        for ( int i = 0 ; i < ( int )id_vec.size() ; i++ )
            if ( id_vec[i] != active_geom_id )
            {
                new_id_vec.push_back( id_vec[i] );
            }

        if ( action == REORDER_MOVE_BOTTOM )
        {
            new_id_vec.push_back( active_geom_id );
        }
    }
    else if ( action == REORDER_MOVE_UP || action == REORDER_MOVE_DOWN )
    {
        for ( int i = 0 ; i < ( int )id_vec.size() ; i++ )
        {
            if ( i < ( int )( id_vec.size() - 1 ) &&
                    ( ( action == REORDER_MOVE_DOWN && id_vec[i] == active_geom_id ) ||
                      ( action == REORDER_MOVE_UP   && id_vec[i + 1] == active_geom_id ) ) )
            {
                new_id_vec.push_back( id_vec[i + 1] );
                new_id_vec.push_back( id_vec[i] );
                i++;
            }
            else
            {
                new_id_vec.push_back( id_vec[i] );
            }
        }
    }

    if ( !parent_geom )
    {
        m_TopGeom = new_id_vec;
    }
    else
    {
        parent_geom->SetChildIDVec( new_id_vec );
    }

}


//==== Cut Active Geom and Place in Clipboard ====//
void Vehicle::CutActiveGeomVec()
{
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() == 0 )
    {
        return;
    }

    DeleteClipBoard();
    CutGeomVec( sel_vec );

    ClearActiveGeom();
}
//==== Copy Active Geom and Place in Clipboard ====//
void Vehicle::CopyActiveGeomVec()
{
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() == 0 )
    {
        return;
    }

    DeleteClipBoard();

    m_ClipBoard = CopyGeomVec( sel_vec );
}

//==== Delete All Geoms in Clipboard ====//
void Vehicle::DeleteClipBoard()
{
    for ( int i = 0 ; i < ( int )m_ClipBoard.size() ; i++ )
    {
        Geom* gPtr = FindGeom( m_ClipBoard[i] );
        if ( gPtr )
        {
            vector_remove_val( m_GeomStoreVec, gPtr );
            delete gPtr;
        }
    }
    m_ClipBoard.clear();
}

void Vehicle::DeleteGeom( string geom_id )
{
    Geom* gPtr = FindGeom( geom_id );
    if ( gPtr )
    {
        vector_remove_val( m_GeomStoreVec, gPtr );
        vector_remove_val( m_ActiveGeom, geom_id );
        delete gPtr;
    }

}

//==== Paste All Geoms in Clipboard ====//
void Vehicle::PasteClipboard()
{
    //==== Find Current Parent ====//
    string parent_id = "NONE";
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() )
    {
        parent_id = sel_vec.back();
    }

    //==== Copy Clipboard ====//
    vector< string > copy_geom = CopyGeomVec( m_ClipBoard );

    //==== Find First Unattached Geom In ClipBoard ====//
    Geom* parentGeom = FindGeom( parent_id );
    for ( int i = 0 ; i < ( int )m_ClipBoard.size() ; i++ )
    {
        Geom* gPtr = FindGeom( m_ClipBoard[i] );
        if ( gPtr && gPtr->GetParentID() == "NONE" )
        {
            if ( parentGeom )
            {
                gPtr->SetParentID( parent_id );
                parentGeom->AddChildID( gPtr->GetID() );
            }

            //==== Update gPtr and all children  ====//
            gPtr->SetIgnoreAbsFlag( true );
            gPtr->Update();
            gPtr->SetIgnoreAbsFlag( false );
        }
    }

    //==== Add To Active Geoms ====//
    for ( int i = 0 ; i < ( int )m_ClipBoard.size() ; i++ )
    {
        Geom* gPtr = FindGeom( m_ClipBoard[i] );
        if ( gPtr && gPtr->GetParentID() == "NONE" )
        {
            m_TopGeom.push_back(  m_ClipBoard[i] );
        }
    }

    //==== Move Copied Geoms into ClipBoard ====//
    m_ClipBoard.clear();
    for  ( int i = 0 ; i < ( int )copy_geom.size() ; i++ )
    {
        m_ClipBoard.push_back( copy_geom[i] );
    }


}

//==== Copy Geoms In Vec - Create New IDs But Keep Parent/Child ====//
vector< string > Vehicle::CopyGeomVec( vector< string > geom_vec )
{
    ParmMgr.ResetRemapID();

    //==== Create New Geoms ====//
    vector< string > created_id_vec;
    vector< Geom* > created_geom_vec;

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* fromPtr = FindGeom( geom_vec[i] );
        if ( fromPtr )
        {
            GeomType t = fromPtr->GetType();
            string id = CreateGeom( t );
            Geom* toPtr = FindGeom( id );
            if ( toPtr )
            {
                toPtr->CopyFrom( fromPtr );
                id = toPtr->GetID();
                created_id_vec.push_back( id );
            }
        }
    }

    ParmMgr.ResetRemapID();

    // Scan through and look for parents & children outside copied vector
    // These have nonexistant ID's because the remap created a new unique
    // ID, but no geom was ever created and changed to that ID.
    for ( int i = 0 ; i < ( int )created_id_vec.size() ; i++ )
    {
        Geom* geom = FindGeom( created_id_vec[i] );
        if ( geom )
        {
            Geom* par = FindGeom( geom->GetParentID() );

            if ( par == NULL )
            {
                geom->SetParentID( "NONE" );
            }

            vector< string > childvec = geom->GetChildIDVec();

            for ( int j = 0; j < static_cast<int>( childvec.size() ); j++ )
            {
                Geom* child = FindGeom( childvec[j] );

                if ( child == NULL )
                {
                    geom->RemoveChildID( childvec[j] );
                }
            }
        }
    }

    return created_id_vec;
}

//==== Get Draw Objects ====//
vector< DrawObj* > Vehicle::GetDrawObjs()
{
    vector< DrawObj* > draw_obj_vec;

    // Create DrawObj that provides Global Lighting Setting.
    m_LightingObj.m_Type = DrawObj::VSP_SETTING;
    m_LightingObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;

    // Clear Light info, redo list on every GetDrawObjs().
    m_LightingObj.m_LightingInfos.clear();

    // Load Lighting Info.
    for ( int i = 0; i < ( int )m_Lights.size() ; i++ )
    {
        DrawObj::LightSourceInfo info;

        info.Active = m_Lights[i].Active();
        info.X = ( float )m_Lights[i].XPos();
        info.Y = ( float )m_Lights[i].YPos();
        info.Z = ( float )m_Lights[i].ZPos();
        info.Amb = ( float )m_Lights[i].Amb();
        info.Diff = ( float )m_Lights[i].Diff();
        info.Spec = ( float )m_Lights[i].Spec();

        m_LightingObj.m_LightingInfos.push_back( info );
    }
    draw_obj_vec.push_back( &m_LightingObj );

    //==== Traverse All Active Displayed Geom and Load DrawObjs ====//
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->LoadDrawObjs( draw_obj_vec );
    }

    return draw_obj_vec;
}

void Vehicle::ResetDrawObjsGeomChangedFlags()
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->ResetGeomChangedFlag();
    }
}

void Vehicle::SetSetName( int index, const string& name )
{
    char str[256];

    if ( index < 0 || index > 511 )
    {
        return;
    }

    while ( ( int )m_SetNameVec.size() <= index )
    {
        sprintf( str, "Set_%d", ( int )m_SetNameVec.size() );
        m_SetNameVec.push_back( string( str ) );
    }
    m_SetNameVec[index] = name;
}

//=== Set 'Show' set to specified index set ===//
void Vehicle::SetShowSet( int index )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        bool f = geom_vec[i]->GetSetFlag( index );

        geom_vec[i]->SetSetFlag( SET_SHOWN, f );
        geom_vec[i]->SetSetFlag( SET_NOT_SHOWN, !f );
    }
}

//=== Get Vector of Geom IDs That Are In Set Index ===//
vector< string > Vehicle::GetGeomSet( int index )
{
    vector< string > geom_id_vec;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( index ) )
        {
            geom_id_vec.push_back( geom_vec[i]->GetID() );
        }
    }
    return geom_id_vec;
}


//==== Get Number of Fixed Geometry Types ====//
int Vehicle::GetNumFixedGeomTypes()
{
    int num = 0;
    for ( int i = 0 ; i < ( int )m_GeomTypeVec.size() ; i++ )
    {
        if ( m_GeomTypeVec[i].m_FixedFlag )
        {
            num++;
        }
    }
    return num;
}

//==== Get Pointer To Geom Type ====//
GeomType* Vehicle::GetGeomType( int index )
{
    if ( index < 0 || index >= ( int )m_GeomTypeVec.size() )
    {
        return NULL;
    }

    return &m_GeomTypeVec[index];
}

//==== Add Type From Geometry ====//
void Vehicle::AddType( string geom_id )
{
    Geom* gptr = FindGeom( geom_id );
    if ( gptr )
    {
        GeomType type( gptr->GetType().m_Type, gptr->GetName(), false );

        //===== Create Geom ====//
        GeomType t = gptr->GetType();
        string id = CreateGeom( t );
        Geom* toPtr = FindGeom( id );
        if ( toPtr )
        {
            toPtr->CopyFrom( gptr );
            toPtr->Update();
        }

        type.m_GeomID = id;
        m_GeomTypeVec.push_back( type );
    }

}

//==== Delete Type ====//
void Vehicle::DeleteType( int index )
{
    GeomType* typeptr = GetGeomType( index );

    if ( !typeptr )
    {
        return;
    }
    if ( typeptr->m_FixedFlag )
    {
        return;
    }

    Geom* gPtr = FindGeom( typeptr->m_GeomID );
    if ( gPtr )
    {
        vector_remove_val( m_GeomStoreVec, gPtr );
        delete gPtr;
    }

    m_GeomTypeVec.erase( m_GeomTypeVec.begin() + index );

}

xmlNodePtr Vehicle::EncodeXml( xmlNodePtr & node, int set )
{
    xmlNodePtr vehicle_node = xmlNewChild( node, NULL, BAD_CAST"Vehicle", NULL );

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( set ) )
        {
            geom_vec[i]->EncodeGeom( vehicle_node );
        }
    }

    LinkMgr.EncodeXml( node );

//  CdfMeshMgr.EncodeXml( node );

    return vehicle_node;
}

xmlNodePtr Vehicle::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr vehicle_node = XmlUtil::GetNode( node, "Vehicle", 0 );
    if ( vehicle_node )
    {
        int num = XmlUtil::GetNumNames( vehicle_node, "Geom" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr geom_node = XmlUtil::GetNode( vehicle_node, "Geom", i );
            if ( geom_node )
            {
                xmlNodePtr base_node = XmlUtil::GetNode( geom_node, "GeomBase", 0 );

                GeomType type;
                type.m_Name   = XmlUtil::FindString( base_node, "TypeName", type.m_Name );
                type.m_Type   = XmlUtil::FindInt( base_node, "TypeID", type.m_Type );
                type.m_FixedFlag = !!XmlUtil::FindInt( base_node, "TypeFixed", type.m_FixedFlag );

                string id = CreateGeom( type );
                Geom* geom = FindGeom( id );

                if ( geom )
                {
                    geom->DecodeXml( geom_node );

                    if ( geom->GetParentID().compare( "NONE" ) == 0 )
                    {
                        AddGeom( geom );
                    }
                }
            }
        }
        Update();
    }

    LinkMgr.DecodeXml( node );

//  CdfMeshMgr.DecodeXml( node );

    return vehicle_node;
}


//==== Write File ====//
bool Vehicle::WriteXMLFile( const string & file_name, int set )
{
    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );
    xmlDocSetRootElement( doc, root );
    xmlNodePtr versionNode = XmlUtil::AddIntNode( root, "Version", CURRENT_FILE_VER );

    xmlNodePtr vehicle_node = EncodeXml( root, set );

    //===== Save XML Tree and Free Doc =====//
    int err = xmlSaveFormatFile( file_name.c_str(), doc, 1 );
    xmlFreeDoc( doc );

    if( err == -1 )  // Failure occurred
    {
        return false;
    }

    return true;
}

//==== Read File ====//
int Vehicle::ReadXMLFile( const string & file_name )
{
    ParmMgr.ResetRemapID();

    //==== Read Xml File ====//
    xmlDocPtr doc;

    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    //==== Build an XML tree from a the file ====//
    doc = xmlParseFile( file_name.c_str() );
    if ( doc == NULL )
    {
        fprintf( stderr, "could not parse XML document\n" );
        return 1;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == NULL )
    {
        fprintf( stderr, "empty document\n" );
        xmlFreeDoc( doc );
        return 2;
    }

    if ( xmlStrcmp( root->name, ( const xmlChar * )"Vsp_Geometry" ) )
    {
        fprintf( stderr, "document of the wrong type, Vsp Geometry not found\n" );
        xmlFreeDoc( doc );
        return 3;
    }

    //==== Find Version Number ====//
    m_FileOpenVersion = XmlUtil::FindInt( root, "Version", 0 );

    //==== Decode Vehicle from document ====//
    DecodeXml( root );

    Update();

    //===== Free Doc =====//
    xmlFreeDoc( doc );

    ParmMgr.ResetRemapID();

    return 0;
}

//==== Write Cross Section File ====//
void Vehicle::WriteXSecFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );

    int geom_cnt = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_cnt += geom_vec[i]->GetNumTotalSurfs();
        }
    }

    //==== Open file ====//
    FILE* dump_file = fopen( file_name.c_str(), "w" );

    fprintf( dump_file, " HERMITE INPUT FILE\n\n" );
    fprintf( dump_file, " NUMBER OF COMPONENTS = %d\n", geom_cnt );

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WriteXSecFile( i, dump_file );
        }
    }

    fclose( dump_file );
}

//==== Check for an existing mesh in set ====//
bool Vehicle::ExistMesh( int set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return false;
    }

    // Check to see if a MeshGeom Exists
    bool existMesh = false;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            existMesh = true;
        }
    }

    return existMesh;
}

//==== Write STL File ====//
void Vehicle::WriteSTLFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return;
    }

    if ( !ExistMesh( write_set ) )
    {
        string mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            geom_vec.push_back( FindGeom( mesh_id ) );
        }
    }

    // Open File
    FILE* fid = fopen( file_name.c_str(), "w" );
    fprintf( fid, "solid\n" );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            geom_vec[i]->WriteStl( fid );
        }
    }

    fprintf( fid, "endsolid\n" );
    fclose( fid );
}

//==== Write Tri File ====//
void Vehicle::WriteTRIFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        string mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            geom_vec.push_back( FindGeom( mesh_id ) );
        }
    }

    //==== Open file ====//
    FILE* file_id = fopen( file_name.c_str(), "w" );

    if ( !file_id )
    {
        return;
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int i;

    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildNascartMesh( num_parts );
            num_parts += mg->GetNumNascartParts();
            num_pnts += mg->GetNumNascartPnts();
            num_tris += mg->GetNumNascartTris();
        }
    }

    fprintf( file_id, "%d %d\n", num_pnts, num_tris );

    //==== Dump Points ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE  )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->WriteCart3DPnts( file_id );
        }
    }

    int offset = 0;
    //==== Dump Tris ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE  )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            offset = mg->WriteCart3DTris( file_id, offset );
        }
    }

    int part_count = 1;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            part_count = mg->WriteCart3DParts( file_id, part_count );
        }
    }

    fclose( file_id );

}

//==== Write Nascart Files ====//
void Vehicle::WriteNascartFiles( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        string mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            geom_vec.push_back( FindGeom( mesh_id ) );
        }
    }

    FILE* file_id = fopen( file_name.c_str(), "w" );

    if ( !file_id )
    {
        return;
    }

    //==== Count Number of Points & Tris ====//
    int i;
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildNascartMesh( num_parts );
            num_parts += mg->GetNumNascartParts();
            num_pnts += mg->GetNumNascartPnts();
            num_tris += mg->GetNumNascartTris();
        }
    }
    fprintf( file_id, "%d %d\n", num_pnts, num_tris );

    //==== Dump Points ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->WriteNascartPnts( file_id );
        }
    }

    int offset = 0;
    //==== Dump Tris ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            offset = mg->WriteNascartTris( file_id, offset );
        }
    }

    fclose( file_id );

    string key_name = file_name;
    unsigned int loc = key_name.find_last_of( "." );
    if ( loc == key_name.npos )
    {
        key_name = string( "bodyin.key" );
    }
    else
    {
        key_name = key_name.substr( 0, loc ).append( ".key" );
    }

    //==== Open file ====//
    file_id = fopen( key_name.c_str(), "w" );
    if ( !file_id )
    {
        return;
    }

    fprintf( file_id, "Color	Name			BCType\n" );

    int part_count = 1;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            part_count = mg->WriteNascartParts( file_id, part_count );
        }
    }

    fclose( file_id );

}

void Vehicle::WriteGmshFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        string mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            geom_vec.push_back( FindGeom( mesh_id ) );
        }
    }

    FILE* file_id = fopen( file_name.c_str(), "w" );

    if( !file_id )
    {
        return;
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int i;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildNascartMesh( num_parts );
            num_parts += mg->GetNumNascartParts();
            num_pnts += mg->GetNumNascartPnts();
            num_tris += mg->GetNumNascartTris();
        }
    }

    fprintf( file_id, "$MeshFormat\n" );
    fprintf( file_id, "2.2 0 %d\n", ( int )sizeof( double ) );
    fprintf( file_id, "$EndMeshFormat\n" );

    //==== Dump Nodes ====//
    fprintf( file_id, "$Nodes\n" );
    fprintf( file_id, "%d\n", num_pnts );
    int node_offset = 0;
    vector< int > node_offset_vec;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        node_offset_vec.push_back( node_offset );
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            node_offset = mg->WriteGMshNodes( file_id, node_offset );
        }
    }
    fprintf( file_id, "$EndNodes\n" );

    //==== Dump Tris ====//
    fprintf( file_id, "$Elements\n" );
    fprintf( file_id, "%d\n", num_tris );
    int tri_offset = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            tri_offset = mg->WriteGMshTris( file_id, node_offset_vec[i], tri_offset );
        }
    }
    fprintf( file_id, "$EndElements\n" );

    fclose( file_id );

}

void Vehicle::WriteX3DFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return;
    }

    xmlDocPtr doc = xmlNewDoc( BAD_CAST "1.0" );

    xmlNodePtr root = xmlNewNode( NULL, BAD_CAST "X3D" );
    xmlDocSetRootElement( doc, root );

    xmlNodePtr scene_node = xmlNewChild( root, NULL, BAD_CAST "Scene", NULL );

    WriteX3DViewpoints( scene_node );

    //==== All Geometry ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type != BLANK_GEOM_TYPE )
        {
            xmlNodePtr shape_node = xmlNewChild( scene_node, NULL, BAD_CAST "Shape", NULL );

            xmlNodePtr app_node = xmlNewChild( shape_node, NULL, BAD_CAST "Appearance", NULL );

            //int matid = geomVec[i]->getMaterialID();
            WriteX3DMaterial( app_node, 0 ); // Default until materials are implemented in Geom

            geom_vec[i]->WriteX3D( shape_node );
        }
    }

    //===== Save XML Tree and Free Doc =====//
    xmlSaveFormatFile( file_name.c_str(), doc, 1 );
    xmlFreeDoc( doc );
}

void Vehicle::WriteX3DViewpoints( xmlNodePtr node )
{
    char numstr[255];
    string format3;
    string format4;
    format3 = "%lf %lf %lf";
    format4 = format3 + " %lf";

    //==== Update box and get key values ====//
    UpdateBBox();
    vec3d center = m_BBox.GetCenter();
    double len = m_BBox.DiagDist();
    double fov = .4;
    double dist = len / ( 2 * tan( fov / 2 ) );

    // Set the names and vectors to the different viewpoints //
    string x3d_views[] = {"iso", "front", "top", "right"};
    double view_degree[4][4] = { { -1, -1, 1, -PI / 4 } , { -1, 0, 0, -PI / 2}, {0, 0, 1, 0}, {0, -1, 0, 0} };
    vec3d k = vec3d( 0, 0, 1 );

    for( int i = 0; i < sizeof( view_degree ) / sizeof( double[4] ) ; i++ )
    {
        vec3d view_axis = vec3d( view_degree[i][0], view_degree[i][1], view_degree[i][2] );
        view_axis.normalize();

        vec3d rot_axis = cross( k, view_axis );
        double angle = asin( rot_axis.mag() );
        rot_axis.normalize();

        // if rotating view again combine rotations using quaternions //
        if ( view_degree[i][3] != 0 )
        {
            quat rot1 = quat( rot_axis, angle );
            quat rot2 = quat( view_axis, view_degree[i][3] );
            quat combined_rot = hamilton( rot2, rot1 );
            combined_rot.quat2axisangle( rot_axis, angle );
        }

        vec3d position = center + ( view_axis * dist );

        // Convert vectors to strings //
        string orients, cents, posits, name, sfov;

        sprintf( numstr, format4.c_str(),  rot_axis.x(), rot_axis.y(), rot_axis.z(), angle );
        orients = numstr;

        sprintf( numstr, format3.c_str(), center.x(), center.y(), center.z() );
        cents = numstr;

        sprintf( numstr, format3.c_str(), position.x(), position.y(), position.z() );
        posits = numstr;

        sprintf( numstr, "%lf", fov );
        sfov = numstr;

        // write first viewpoint twice so viewpoint buttons will work correctly //
        if ( i == 0 )
        {
            xmlNodePtr first_view_node = xmlNewChild( node, NULL, BAD_CAST "Viewpoint", BAD_CAST " " );
            WriteX3DViewpointProps( first_view_node, orients, cents, posits, sfov, string( "first" ) );
        }

        // write each viewpoint node's properties //
        xmlNodePtr viewpoint_node = xmlNewChild( node, NULL, BAD_CAST "Viewpoint", BAD_CAST " " );
        WriteX3DViewpointProps( viewpoint_node, orients, cents, posits, sfov, x3d_views[i].c_str() );
    }
}

void Vehicle::WriteX3DViewpointProps( xmlNodePtr node, string orients, string cents, string posits, string sfov, string name )
{
    xmlSetProp( node, BAD_CAST "id", BAD_CAST name.c_str() );
    xmlSetProp( node, BAD_CAST "description", BAD_CAST name.c_str() );
    xmlSetProp( node, BAD_CAST "orientation", BAD_CAST orients.c_str() );
    xmlSetProp( node, BAD_CAST "centerOfRotation", BAD_CAST cents.c_str() );
    xmlSetProp( node, BAD_CAST "position", BAD_CAST posits.c_str() );
    xmlSetProp( node, BAD_CAST "fieldOfView", BAD_CAST sfov.c_str() );
}

void Vehicle::WriteX3DMaterial( xmlNodePtr node, int matid )
{
    // Diffs, emisss, specs are all set to default values and need to be redone when materials are implemented.

    string diffs, emisss, specs;
    char numstr[255];
    xmlNodePtr mat_node = xmlNewChild( node, NULL, BAD_CAST "Material", BAD_CAST " " );

    diffs = "0.5 0.5 0.5";
    xmlSetProp( mat_node, BAD_CAST "diffusecolor", BAD_CAST diffs.c_str() );

    emisss = "0 0 0";
    xmlSetProp( mat_node, BAD_CAST "specularcolor", BAD_CAST emisss.c_str() );

    specs = "0.5 0.5 0.5";
    xmlSetProp( mat_node, BAD_CAST "specularcolor", BAD_CAST specs.c_str() );

    sprintf( numstr, "%lf", 0.0f );
    xmlSetProp( mat_node, BAD_CAST "transparency", BAD_CAST numstr );

    sprintf( numstr, "%lf", 10.0f );
    xmlSetProp( mat_node, BAD_CAST "shininess", BAD_CAST numstr );

    //float ambf = 0.0f;
    //for( int i = 0; i < 3; i++ )
    //{
    //  ambf += mat->amb[i] / mat->diff[i];
    //}
    //ambf = ambf / 3.0f;

    sprintf( numstr, "%lf", 0.5f );
    xmlSetProp( mat_node, BAD_CAST "ambientIntensity", BAD_CAST numstr );
}

void Vehicle::WritePovRayFile( const string & file_name, int write_set )
{
    UpdateBBox();
    int i;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( !geom_vec[0] )
    {
        return;
    }

    //==== Figure Out Basename ====//
    string base_name = file_name;
    unsigned int loc = base_name.find_last_of( "." );
    if ( loc != base_name.npos )
    {
        base_name = base_name.substr( 0, loc );
    }

    string inc_file_name = base_name;
    inc_file_name.append( ".inc" );

    FILE* inc_out = fopen( inc_file_name.c_str(), "w" );
    int comp_num = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePovRay( inc_out, comp_num );
            comp_num++;
        }
    }

    //==== Add Some Decent Textures ====//
    fprintf( inc_out, "#declare lightgreymetal = texture {\n  pigment { color rgb < 0.9, 0.9, 0.9 > } \n" );
    fprintf( inc_out, "    normal { granite, 0.01 }\n" );
    fprintf( inc_out, "    finish { ambient 0.2  diffuse 1.0   specular 0.5  roughness 0.1  metallic reflection 0.05 }\n}\n" );

    fprintf( inc_out, "#declare darkgreymetal = texture {\n  pigment { color rgb < 0.7, 0.7, 0.7 > } \n" );
    fprintf( inc_out, "    normal { granite, 0.005 }\n" );
    fprintf( inc_out, "    finish { ambient 0.2  diffuse 1.0   specular 0.8  roughness 0.15   }\n}\n" );

    fprintf( inc_out, "#declare bluegreymetal = texture {\n  pigment { color rgb < 0.4, 0.4, 0.45 > } \n" );
    fprintf( inc_out, "    normal { granite, 0.02 }\n" );
    fprintf( inc_out, "    finish { ambient 0.2  diffuse 1.0   specular 0.8  roughness 0.15 metallic  }\n}\n" );

    fprintf( inc_out, "#declare canopyglass = texture {\n  pigment { color rgb < 0.3, 0.3, 0.3, 0.5 > } \n" );
    fprintf( inc_out, "    finish { ambient 0.1  diffuse 1.0   specular 1.0  roughness 0.01 reflection 0.4  }\n}\n" );

    fclose( inc_out );

    //==== Open POV File  ====//
    string pov_file_name = base_name;
    pov_file_name += ".pov";

    FILE* pov_file = fopen( pov_file_name.c_str(), "w" );

    fprintf( pov_file, "#version 3.6;\n\n" );
    fprintf( pov_file, "#include \"%s\"\n", inc_file_name.c_str() );

    fprintf( pov_file, "#include \"colors.inc\"\n" );
    fprintf( pov_file, "#include \"shapes.inc\"\n" );
    fprintf( pov_file, "#include \"textures.inc\"\n\n" );

    fprintf( pov_file, "global_settings { assumed_gamma 1 }\n\n" );

    BndBox bb = GetBndBox();
    vec3d center = bb.GetCenter();
    double diag  = bb.DiagDist();
    double mult  = 1.3;

    fprintf( pov_file, "camera { location < %6.1f, %6.1f, %6.1f > sky < 0.0, 0.0, -1.0 >  look_at < %6.1f, %6.1f, %6.1f > }\n",
             mult * bb.GetMin( 0 ), mult * bb.GetMin( 1 ), mult * bb.GetMin( 2 ), center.x(), center.z(), center.y() );

    fprintf( pov_file, "light_source { < %6.1f, %6.1f, %6.1f >  color White }\n", center.x(), center.z() + diag, center.y() );

    comp_num = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if (  geom_vec[i]->GetSetFlag( write_set ) )
        {
            string name = geom_vec[i]->GetName();
            StringUtil::chance_space_to_underscore( name );
            fprintf( pov_file, "mesh { %s_%d texture {darkgreymetal} } \n", name.c_str(), comp_num );
            comp_num++;
        }
    }

    fclose( pov_file );
}

void Vehicle::WriteBezFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec( false ) );

    int num_comps = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            num_comps += geom_vec[i]->GetNumTotalSurfs();
        }
    }

    FILE* id = fopen( file_name.c_str(), "w" );

    fprintf( id, "%d  Num_Components\n", num_comps );

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            vector<VspSurf> surf_vec;
            geom_vec[i]->GetSurfVec( surf_vec );

            for ( int j = 0; j < (int)surf_vec.size(); j++ )
            {
                surf_vec[j].WriteBezFile( id, geom_vec[i]->GetID() );
            }
        }
    }

    fclose( id );
}

void Vehicle::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    ParmContainer::AddLinkableContainers( linkable_container_vec );

    //==== Add Geom Containers ====//
    vector< string > geom_id_vec = GetGeomVec();
    vector< Geom* > geom_vec = FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->AddLinkableContainers( linkable_container_vec );
    }
}

Vehicle::LightInfo * Vehicle::FindLight( unsigned int index )
{
    assert( index >= 0 && index < m_Lights.size() );
    if( index < 0 || index >= m_Lights.size() )
    {
        return NULL;
    }
    return &m_Lights[index];
}

void Vehicle::UpdateBBox()
{
    BndBox new_box;
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        new_box.Update( geom_vec[i]->GetBndBox() );
    }

    m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
    m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
    m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

    m_BbXMin = new_box.GetMin( 0 );
    m_BbYMin = new_box.GetMin( 1 );
    m_BbZMin = new_box.GetMin( 2 );

//  CfdMeshMgr.m_FarXScale.SetRefVal( m_BbXLen() );
//  CfdMeshMgr.m_FarYScale.SetRefVal( m_BbYLen() );
//  CfdMeshMgr.m_FarZScale.SetRefVal( m_BbZLen() );

    m_BBox = new_box;
}

string Vehicle::getExportFileName( int type )
{
    if ( type == COMP_GEOM_TXT_TYPE )
    {
        return m_compGeomTxtFileName;
    }
    else if ( type == COMP_GEOM_CSV_TYPE )
    {
        return m_compGeomCsvFileName;
    }
    else if ( type == DRAG_BUILD_TSV_TYPE )
    {
        return m_compGeomTsvFileName;
    }
    else if ( type == MASS_PROP_TXT_TYPE )
    {
        return m_MassPropFileName;
    }
    else if ( type == SLICE_TXT_TYPE )
    {
        return m_AwaveFileName;
    }
    else
    {
        return string( "default_export.txt" );
    }
}

void Vehicle::setExportFileName( int type, string f_name )
{
    if ( f_name.compare( "" ) == 0 || f_name.compare( "/" ) == 0 )
    {
        return;
    }

    if ( type == COMP_GEOM_TXT_TYPE )
    {
        m_compGeomTxtFileName = f_name;
    }
    else if ( type == COMP_GEOM_CSV_TYPE )
    {
        m_compGeomCsvFileName = f_name;
    }
    else if ( type == DRAG_BUILD_TSV_TYPE )
    {
        m_compGeomTsvFileName = f_name;
    }
    else if ( type == MASS_PROP_TXT_TYPE )
    {
        m_MassPropFileName = f_name;
    }
    else if ( type == SLICE_TXT_TYPE )
    {
        m_AwaveFileName = f_name;
    }
}

string Vehicle::CompGeom( int set, int sliceFlag, int meshFlag, int halfFlag )
{

    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    MeshGeom* mesh_ptr = dynamic_cast<MeshGeom*> ( FindGeom( id ) );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    if ( halfFlag )
    {
        mesh_ptr->AddHalfBox();
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        if ( sliceFlag )
        {
            mesh_ptr->SliceX( sliceFlag );
        }
        else
        {
            mesh_ptr->IntersectTrim( meshFlag, halfFlag );
        }
    }
    else
    {
        // Not sure if this is the best way to delete a mesh geom with no tMeshVec
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::CompGeomAndFlatten( int set, int sliceFlag, int meshFlag, int halfFlag )
{
    string id = CompGeom( set, sliceFlag, meshFlag, halfFlag );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->Update();
    return id;
}

string Vehicle::MassProps( int set, int numSlices )
{
    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    //==== Load Point Mass Properties From Blank Geom ====//
    vector<string> geom_vec = GetGeomVec();
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i].compare( id ) != 0 )
        {
            Geom* geom_ptr = FindGeom( geom_vec[i] );

            if ( geom_ptr->GetSetFlag( set ) && geom_ptr->GetType().m_Type == BLANK_GEOM_TYPE )
            {
                BlankGeom* BGeom = ( BlankGeom* ) geom_ptr;

                if ( BGeom->m_PointMassFlag() )
                {
                    TetraMassProp* pm = new TetraMassProp(); // Deleted by mesh_ptr
                    vec3d loc = vec3d( BGeom->m_XLoc(), BGeom->m_YLoc(), BGeom->m_ZLoc() );
                    pm->SetPointMass( BGeom->m_PointMass(), loc );
                    pm->m_CompId = BGeom->GetID();
                    mesh_ptr->AddPointMass( pm );
                }
            }
        }
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        mesh_ptr->MassSliceX( numSlices );
        m_TotalMass = mesh_ptr->m_TotalMass;
        m_IxxIyyIzz = vec3d( mesh_ptr->m_TotalIxx, mesh_ptr->m_TotalIyy, mesh_ptr->m_TotalIzz );
        m_IxyIxzIyz = vec3d( mesh_ptr->m_TotalIxy, mesh_ptr->m_TotalIxz, mesh_ptr->m_TotalIyz );
        m_CG = mesh_ptr->m_CenterOfGrav;

    }
    else
    {
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::MassPropsAndFlatten( int set, int numSlices )
{
    string id = MassProps( set, numSlices );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->FlattenSliceVec();
    mesh_ptr->Update();
    return id;
}

string Vehicle::AwaveSlice( int set, int numSlices, int numRots, double AngleControlVal, bool computeAngle,
                            vec3d norm, bool autoBoundsFlag, double start , double end )
{
    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        // Compute Angle if necessary for awave slice
        if ( computeAngle )
        {
            AngleControlVal = asin( 1 / AngleControlVal ) * RAD_2_DEG;
        }
        mesh_ptr->AreaSlice( vsp::SLICE_AWAVE, numSlices, AngleControlVal, numRots, norm, autoBoundsFlag, start, end );
    }
    else
    {
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::AwaveSliceAndFlatten( int set, int numSlices, int numRots, double AngleControlVal, bool computeAngle,
                                      vec3d norm, bool autoBoundsFlag, double start , double end )
{
    string id = AwaveSlice( set, numSlices, numRots, AngleControlVal, computeAngle, norm, autoBoundsFlag, start, end );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->FlattenSliceVec();
    mesh_ptr->Update();
    return id;
}

string Vehicle::PSlice( int set, int numSlices, vec3d axis, bool autoBoundsFlag, double start, double end )
{

    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        mesh_ptr->AreaSlice( vsp::SLICE_PLANAR, numSlices, 90, 0, axis, autoBoundsFlag, start, end );
    }
    else
    {
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::PSliceAndFlatten( int set, int numSlices, vec3d axis, bool autoBoundsFlag, double start, double end )
{
    string id = PSlice( set, numSlices, axis, autoBoundsFlag, start, end );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->FlattenSliceVec();
    mesh_ptr->Update();
    return id;
}

//==== Import File Methods ====//
string Vehicle::ImportFile( const string & file_name, int file_type )
{
    GeomType type = GeomType( MESH_GEOM_TYPE, "MESH", true );
    string id = AddGeom( type );
    if ( !id.compare( "NONE" ) )
    {
        return id;
    }

    MeshGeom* new_geom = ( MeshGeom* )FindGeom( id );

    int validFlag;
    if ( file_type == IMPORT_STL )
    {
        validFlag = new_geom->ReadSTL( file_name.c_str() );
    }
    else if ( file_type == IMPORT_NASCART )
    {
        validFlag = new_geom->ReadNascart( file_name.c_str() );
    }
    else if ( file_type == IMPORT_CART3D_TRI )
    {
        validFlag = new_geom->ReadTriFile( file_name.c_str() );
    }
    else if ( file_type == IMPORT_XSEC_MESH )
    {
        validFlag = new_geom->ReadXSec( file_name.c_str() );
    }
    else
    {
        validFlag = 0;
    }

    if ( !validFlag )
    {
        DeleteGeom( id );
        id = "NONE";
    }
    else
    {
        SetActiveGeom( id );
        new_geom->Update();
    }

    return id;
}


//==== Import File Methods ====//
void Vehicle::ExportFile( const string & file_name, int write_set, int file_type )
{
    if ( file_type == EXPORT_XSEC )
    {
        WriteXSecFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_STL )
    {
        WriteSTLFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_CART3D )
    {
        WriteTRIFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_NASCART )
    {
        WriteNascartFiles( file_name, write_set );
    }
    else if ( file_type == EXPORT_GMSH )
    {
        WriteGmshFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_POVRAY )
    {
        WritePovRayFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_X3D )
    {
        WriteX3DFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_BEZ )
    {
        WriteBezFile( file_name, write_set );
    }
}
