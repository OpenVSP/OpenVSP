//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ScreenDump.h: PNG and EPS images of every screen, for testing and documentation
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCREENDUMP__INCLUDED_)
#define SCREENDUMP__INCLUDED_

#include <string>
#include <vector>

#include "ScreenBase.h"

class ScreenMgr;
class Vehicle;

// Writes a PNG and an EPS of each screen and of every page of its tab sets.  The model gains a Geom
// of every type it lacks, so each Geom screen shows a Geom of its own type; the model is left so, for
// a program that exits after the dump.
class ScreenDumper
{
public:

    ScreenDumper( ScreenMgr* mgr );

    // Dumps every screen into dir, the images at scale times the screens' size, and lists the files
    // in dir/snap_manifest.txt.  A screen of vsp::GUI_VSP_SCREEN or a Geom screen of
    // vsp::GUI_GEOM_SCREEN, if not -1, is the only one dumped.  Returns the number of files that could
    // not be written, or -1 when nothing was written.
    int Dump( int screen, int geom_screen, const std::string &dir, double scale );

private:

    bool SetScale( double scale );
    void AddDumpGeoms();
    std::string MakeMesh( const std::string &geom_id );
    std::string MakeWireFrame( const std::string &geom_id );
    std::vector< bool > PutAloneInSet( int set, const std::string &geom_id );
    void RestoreSet( int set, const std::vector< bool > &flags );
    bool DumpEditCurve();
    void DumpGeomScreens( int geom_screen );
    void WriteManifest();

    ScreenMgr* m_ScreenMgr;
    Vehicle* m_Vehicle;

    // One Geom of each type, which the Geom screens are dumped with
    std::vector< std::string > m_DumpGeomVec;

    ScreenDumpLog m_Log;
};

#endif // !defined(SCREENDUMP__INCLUDED_)
