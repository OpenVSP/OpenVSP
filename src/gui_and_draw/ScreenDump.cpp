//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ScreenDump.cpp: PNG and EPS images of every screen, for testing and documentation
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cstdio>

#include "FL/Fl_Image_Surface.H"

#include "ScreenDump.h"
#include "CurveEditScreen.h"
#include "ManageGeomScreen.h"
#include "MeshGeom.h"
#include "ScreenMgr.h"
#include "VehicleMgr.h"

using namespace vsp;

// The first Geom of a type among some Geoms, or empty
static string FirstGeomOfType( Vehicle* veh, const vector< string > &ids, int type )
{
    for ( int i = 0; i < ( int )ids.size(); i++ )
    {
        Geom* g = veh->FindGeom( ids[i] );
        if ( g && g->GetType().m_Type == type )
        {
            return ids[i];
        }
    }
    return string();
}

// The Geoms in after that are not in before
static vector< string > NewGeoms( const vector< string > &before, const vector< string > &after )
{
    vector< string > added;
    for ( int i = 0; i < ( int )after.size(); i++ )
    {
        if ( std::find( before.begin(), before.end(), after[i] ) == before.end() )
        {
            added.push_back( after[i] );
        }
    }
    return added;
}

ScreenDumper::ScreenDumper( ScreenMgr* mgr )
{
    m_ScreenMgr = mgr;
    m_Vehicle = VehicleMgr.GetVehicle();
}

int ScreenDumper::Dump( int screen, int geom_screen, const string &dir, double scale )
{
    m_Log = ScreenDumpLog();
    m_Log.m_Dir = dir;
    m_Log.m_Scale = scale;

    if ( !SetScale( scale ) )
    {
        return -1;
    }

    AddDumpGeoms();

    bool all = ( screen < 0 && geom_screen < 0 );

    // The Edit Curve screen is dumped on a curve from the model when there is one, and empty otherwise
    bool curve_dumped = false;
    if ( all || screen == VSP_CURVE_EDIT_SCREEN )
    {
        curve_dumped = DumpEditCurve();
    }

    for ( int sid = 0; sid < VSP_NUM_SCREENS; sid++ )
    {
        if ( ( all || sid == screen ) && !( sid == VSP_CURVE_EDIT_SCREEN && curve_dumped ) )
        {
            VspScreen* s = m_ScreenMgr->GetScreen( sid );
            if ( s )
            {
                s->ScreenDump( m_Log );
            }
        }
    }

    if ( all || geom_screen >= 0 )
    {
        DumpGeomScreens( geom_screen );
    }

    if ( m_Log.m_Lines.empty() )
    {
        printf( "No screen was dumped\n" );
        return -1;
    }

    WriteManifest();

    printf( "Wrote %d files to %s\n", ( int )m_Log.m_Lines.size(), dir.c_str() );
    return m_Log.m_NumFailed;
}

// The images are drawn at the pixel density of the display: FLTK's scale for the screen, doubled on
// a high-resolution display.  Every screen's scale is set so that the density is the scale asked for.
bool ScreenDumper::SetScale( double scale )
{
    // Measured with the main window on the first display, where every dumped window is put
    VspScreen* main_screen = m_ScreenMgr->GetScreen( VSP_MAIN_SCREEN );
    if ( main_screen && main_screen->GetFlWindow() )
    {
        int X, Y, W, H;
        Fl::screen_work_area( X, Y, W, H, 0 );
        main_screen->GetFlWindow()->position( X, Y );
        Fl::wait( 0.1 );
    }

    for ( int pass = 0; pass < 2; pass++ )
    {
        Fl_Image_Surface probe( 100, 100, 1 );
        Fl_RGB_Image* img = probe.image();
        if ( !img )
        {
            break;
        }
        double density = img->data_w() / 100.0;
        delete img;

        if ( std::abs( density - scale ) < 1e-6 )
        {
            return true;
        }

        // Read before any is set, since setting one display's scale sets every display's where the
        // system has one scale for all
        vector< float > screen_scales;
        for ( int i = 0; i < Fl::screen_count(); i++ )
        {
            screen_scales.push_back( Fl::screen_scale( i ) );
        }

        for ( int i = 0; i < Fl::screen_count(); i++ )
        {
            Fl::screen_scale( i, ( float )( screen_scales[i] * scale / density ) );
        }
        Fl::wait( 0.1 );
    }

    printf( "Could not draw the screens at scale %g\n", scale );
    return false;
}

void ScreenDumper::AddDumpGeoms()
{
    m_DumpGeomVec.clear();

    // One Geom of each type: the first already in the model, or one added.  Custom types all share
    // one screen, so one of them is enough.  The types that only analyses and files make follow.
    vector< int > done_types;
    vector< string > geom_vec = m_Vehicle->GetGeomVec();
    vector< string > added;
    int conformal_index = -1;

    for ( int i = 0; i < m_Vehicle->GetNumGeomTypes(); i++ )
    {
        GeomType type = m_Vehicle->GetGeomType( i );

        if ( type.m_Type >= NUM_GEOM_TYPE ||
             std::find( done_types.begin(), done_types.end(), type.m_Type ) != done_types.end() )
        {
            continue;
        }
        done_types.push_back( type.m_Type );

        string id = FirstGeomOfType( m_Vehicle, geom_vec, type.m_Type );

        // A conformal Geom needs a parent, so it is added once the others are there
        if ( id.empty() && type.m_Type == CONFORMAL_GEOM_TYPE )
        {
            conformal_index = i;
            continue;
        }

        if ( id.empty() )
        {
            // Added with nothing active, so it has no parent
            m_Vehicle->SetActiveGeomVec( vector< string >() );
            id = m_Vehicle->AddGeom( type );
            if ( id.empty() || id == "NONE" )
            {
                continue;
            }
            added.push_back( id );
        }

        m_DumpGeomVec.push_back( id );
    }

    // On a Geom with a surface, a wing if there is one; one the dump added if it can, so the model's
    // own Geoms keep the children they had
    if ( conformal_index >= 0 )
    {
        string parent;
        for ( int pass = 0; pass < 2 && parent.empty(); pass++ )
        {
            vector< string > candidates = added;
            if ( pass == 1 )
            {
                candidates = m_DumpGeomVec;
            }

            for ( int i = 0; i < ( int )candidates.size(); i++ )
            {
                Geom* g = m_Vehicle->FindGeom( candidates[i] );
                if ( g && g->isSurfaceType() && g->GetType().m_Type != CONFORMAL_GEOM_TYPE )
                {
                    if ( parent.empty() || g->GetType().m_Type == MS_WING_GEOM_TYPE )
                    {
                        parent = candidates[i];
                    }
                }
            }
        }

        if ( !parent.empty() )
        {
            m_Vehicle->SetActiveGeom( parent );
            string id = m_Vehicle->AddGeom( m_Vehicle->GetGeomType( conformal_index ) );
            if ( !id.empty() && id != "NONE" )
            {
                m_DumpGeomVec.push_back( id );
            }
            m_Vehicle->SetActiveGeomVec( vector< string >() );
        }
    }

    // A mesh, a point cloud and an NGon mesh where the model has none, made from a mesh of the pod
    // alone; the model's own meshes are left alone
    vector< string > all_vec = m_Vehicle->GetGeomVec();
    string pod = FirstGeomOfType( m_Vehicle, m_DumpGeomVec, POD_GEOM_TYPE );

    string mesh = FirstGeomOfType( m_Vehicle, all_vec, MESH_GEOM_TYPE );
    string cloud = FirstGeomOfType( m_Vehicle, all_vec, PT_CLOUD_GEOM_TYPE );
    string ngon = FirstGeomOfType( m_Vehicle, all_vec, NGON_GEOM_TYPE );

    if ( ( mesh.empty() || cloud.empty() || ngon.empty() ) && !pod.empty() )
    {
        string pod_mesh = MakeMesh( pod );
        MeshGeom* mesh_ptr = dynamic_cast< MeshGeom* >( m_Vehicle->FindGeom( pod_mesh ) );

        if ( mesh.empty() )
        {
            mesh = pod_mesh;
        }

        if ( cloud.empty() && mesh_ptr )
        {
            vector< string > before = m_Vehicle->GetGeomVec();
            mesh_ptr->CreatePtCloudGeom();
            cloud = FirstGeomOfType( m_Vehicle, NewGeoms( before, m_Vehicle->GetGeomVec() ), PT_CLOUD_GEOM_TYPE );
        }

        if ( ngon.empty() && mesh_ptr )
        {
            ngon = mesh_ptr->CreateNGonMeshGeom();
            if ( ngon == "NONE" )
            {
                ngon.clear();
            }
        }
    }

    // A wireframe comes from a Plot3D file read as one
    string wire = FirstGeomOfType( m_Vehicle, all_vec, WIRE_FRAME_GEOM_TYPE );
    if ( wire.empty() && !pod.empty() )
    {
        wire = MakeWireFrame( pod );
    }

    vector< string > made = { mesh, cloud, ngon, wire };
    for ( int i = 0; i < ( int )made.size(); i++ )
    {
        if ( !made[i].empty() )
        {
            m_DumpGeomVec.push_back( made[i] );
        }
    }

    m_Vehicle->Update();
}

// Puts the Geom alone in a set and returns the set's flags as they were, in the order of GetGeomVec
vector< bool > ScreenDumper::PutAloneInSet( int set, const string &geom_id )
{
    vector< string > ids = m_Vehicle->GetGeomVec();
    vector< bool > flags( ids.size(), false );
    for ( int i = 0; i < ( int )ids.size(); i++ )
    {
        Geom* g = m_Vehicle->FindGeom( ids[i] );
        if ( g )
        {
            flags[i] = g->GetSetFlag( set );
            g->SetSetFlag( set, ids[i] == geom_id );
        }
    }
    return flags;
}

// Puts back the flags PutAloneInSet returned, to the Geoms there were then
void ScreenDumper::RestoreSet( int set, const vector< bool > &flags )
{
    vector< string > ids = m_Vehicle->GetGeomVec();
    for ( int i = 0; i < ( int )ids.size() && i < ( int )flags.size(); i++ )
    {
        Geom* g = m_Vehicle->FindGeom( ids[i] );
        if ( g )
        {
            g->SetSetFlag( set, flags[i] );
        }
    }
}

// A mesh of the Geom alone, in the last set, with CompGeom's text and CSV files turned off
string ScreenDumper::MakeMesh( const string &geom_id )
{
    int set = ( int )m_Vehicle->GetSetNameVec().size() - 1;
    vector< bool > flags = PutAloneInSet( set, geom_id );

    bool txt = m_Vehicle->getExportCompGeomTxtFile();
    bool csv = m_Vehicle->getExportCompGeomCsvFile();
    m_Vehicle->setExportCompGeomTxtFile( false );
    m_Vehicle->setExportCompGeomCsvFile( false );

    m_Vehicle->SetActiveGeomVec( vector< string >() );
    string mesh = m_Vehicle->CompGeom( set, SET_NONE, 0, 0, false, false, false, string(), 0, vector< string >(),
                                       string() );

    m_Vehicle->setExportCompGeomTxtFile( txt );
    m_Vehicle->setExportCompGeomCsvFile( csv );
    RestoreSet( set, flags );

    if ( mesh == "NONE" )
    {
        mesh.clear();
    }
    return mesh;
}

// A wireframe of the Geom, written alone to a Plot3D file in the dump folder and read back
string ScreenDumper::MakeWireFrame( const string &geom_id )
{
    int set = ( int )m_Vehicle->GetSetNameVec().size() - 1;
    vector< bool > flags = PutAloneInSet( set, geom_id );

    string file = m_Log.m_Dir + "/vsp_dump_wire.p3d";
    m_Vehicle->WritePLOT3DFile( file, set, false, string() );
    RestoreSet( set, flags );

    vector< string > before = m_Vehicle->GetGeomVec();
    m_Vehicle->ImportFile( file, IMPORT_P3D_WIRE );
    remove( file.c_str() );

    return FirstGeomOfType( m_Vehicle, NewGeoms( before, m_Vehicle->GetGeomVec() ), WIRE_FRAME_GEOM_TYPE );
}

// Dumps the Edit Curve screen showing the active cross section of the first Geom whose active cross
// section is an edit curve.  Returns true if it did.
bool ScreenDumper::DumpEditCurve()
{
    ManageGeomScreen* mgs = dynamic_cast< ManageGeomScreen* >( m_ScreenMgr->GetScreen( VSP_MANAGE_GEOM_SCREEN ) );
    CurveEditScreen* cedit = dynamic_cast< CurveEditScreen* >( m_ScreenMgr->GetScreen( VSP_CURVE_EDIT_SCREEN ) );
    if ( !mgs || !cedit )
    {
        return false;
    }

    for ( int i = 0; i < ( int )m_DumpGeomVec.size(); i++ )
    {
        GeomXSec* geom = dynamic_cast< GeomXSec* >( m_Vehicle->FindGeom( m_DumpGeomVec[i] ) );
        if ( !geom )
        {
            continue;
        }

        XSec* xs = geom->GetXSec( geom->m_ActiveXSec() );
        if ( xs && xs->GetXSecCurve() && xs->GetXSecCurve()->GetType() == XS_EDIT_CURVE )
        {
            mgs->ClearVehSelectedFlag();
            m_Vehicle->SetActiveGeom( m_DumpGeomVec[i] );
            cedit->SetXSecCurve( xs->GetXSecCurve()->GetID() );
            cedit->ScreenDump( m_Log );
            return true;
        }
    }
    return false;
}

// Dumps each Geom screen showing a Geom of its type, the screen for several Geoms with two selected,
// and a screen that no Geom in the model claims empty.  Only geom_screen, if not -1.
void ScreenDumper::DumpGeomScreens( int geom_screen )
{
    ManageGeomScreen* mgs = dynamic_cast< ManageGeomScreen* >( m_ScreenMgr->GetScreen( VSP_MANAGE_GEOM_SCREEN ) );
    if ( !mgs )
    {
        return;
    }

    vector< VspScreen* > screens = mgs->GetGeomScreenVec();

    // A screen not asked for counts as dumped already.  Only a screen showing for the active Geom is
    // dumped, since one shown otherwise would show a Geom of another type.
    vector< bool > dumped( screens.size(), false );
    if ( geom_screen >= 0 )
    {
        for ( int j = 0; j < ( int )screens.size(); j++ )
        {
            dumped[j] = ( j != geom_screen );
        }
    }

    vector< vector< string > > selections;
    for ( int i = 0; i < ( int )m_DumpGeomVec.size(); i++ )
    {
        selections.push_back( vector< string >( 1, m_DumpGeomVec[i] ) );
    }

    if ( m_DumpGeomVec.size() >= 2 )
    {
        selections.push_back( vector< string >( m_DumpGeomVec.begin(), m_DumpGeomVec.begin() + 2 ) );
    }

    selections.push_back( vector< string >() );

    for ( int i = 0; i < ( int )selections.size(); i++ )
    {
        mgs->ClearVehSelectedFlag();
        m_Vehicle->SetActiveGeomVec( selections[i] );
        mgs->ShowHideGeomScreens();

        bool empty = selections[i].empty();
        for ( int j = 0; j < ( int )screens.size(); j++ )
        {
            if ( !dumped[j] && ( empty || screens[j]->IsShown() ) )
            {
                screens[j]->ScreenDump( m_Log );
                dumped[j] = true;
            }
        }
    }
}

// One line per file: its name, then the image's width and height (pixels for a PNG, points for an EPS)
void ScreenDumper::WriteManifest()
{
    string fname = m_Log.m_Dir + "/snap_manifest.txt";
    FILE* fp = fopen( fname.c_str(), "w" );
    if ( !fp )
    {
        printf( "Could not write %s\n", fname.c_str() );
        m_Log.m_NumFailed++;
        return;
    }

    for ( int i = 0; i < ( int )m_Log.m_Lines.size(); i++ )
    {
        fprintf( fp, "%s\n", m_Log.m_Lines[i].c_str() );
    }
    fclose( fp );
}
