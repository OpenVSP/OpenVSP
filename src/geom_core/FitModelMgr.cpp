//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FitModelMgr.cpp: Fit Model Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "FitModelMgr.h"
#include "ParmMgr.h"
#include "PtCloudGeom.h"

#define CMINPACK_NO_DLL
#include <cminpack.h>

// The surface a target point is matched against, or null when there is not one.  A *.fit file
// carries Geom IDs, so one written against another model -- or against this one before a Geom was
// deleted or swapped for a Blank or a point cloud -- names Geoms that either are not here or have
// no surface to match against.  Geom::GetSurfPtr returns null for the latter, which was read
// through.
static const VspSurf * MatchSurf( Geom* matchgeom, int surfindx )
{
    if ( !matchgeom || surfindx < 0 || surfindx >= matchgeom->GetNumTotalSurfs() )
    {
        return nullptr;
    }

    return matchgeom->GetSurfPtr( surfindx );
}

vec3d TargetPt::GetMatchPt()
{
    Geom* matchgeom = VehicleMgr.GetVehicle()->FindGeom( m_MatchGeom );
    if ( matchgeom )
    {
        return GetMatchPt( matchgeom );
    }
    return vec3d();
}

vec3d TargetPt::GetMatchPt(Geom* matchgeom)
{
    if ( matchgeom )
    {
        assert( matchgeom->GetID() == m_MatchGeom );

        const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
        if ( !s )
        {
            return vec3d();
        }

        return s->CompPnt01( m_UW.x(), m_UW.y() );
    }
    return vec3d();
}

vec3d TargetPt::CalcDelta()
{
    Geom* matchgeom = VehicleMgr.GetVehicle()->FindGeom( m_MatchGeom );
    if ( matchgeom )
    {
        return CalcDelta( matchgeom );
    }
    return vec3d();
}

vec3d TargetPt::CalcDelta(Geom* matchgeom)
{
    if ( matchgeom )
    {
        assert( matchgeom->GetID() == m_MatchGeom );

        vec3d pt = GetPt();

        const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
        if ( !s )
        {
            return vec3d();
        }

        vec3d ps = s->CompPnt01( m_UW.x(), m_UW.y() );

        return (ps - pt);
    }
    return vec3d();
}

vec3d TargetPt::CalcDerivU( Geom* matchgeom )
{
    if ( matchgeom )
    {
        assert( matchgeom->GetID() == m_MatchGeom );

        const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
        if ( !s )
        {
            return vec3d();
        }

        return s->CompTanU01( m_UW.x(), m_UW.y() );
    }
    return vec3d();
}

vec3d TargetPt::CalcDerivW( Geom* matchgeom )
{
    if ( matchgeom )
    {
        assert( matchgeom->GetID() == m_MatchGeom );

        const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
        if ( !s )
        {
            return vec3d();
        }

        return s->CompTanW01( m_UW.x(), m_UW.y() );
    }
    return vec3d();
}

void TargetPt::SearchUW( Geom* matchgeom )
{
    if ( matchgeom )
    {
        assert( matchgeom->GetID() == m_MatchGeom );


        if( m_UType == vsp::FIT_MODEL_FREE && m_WType == vsp::FIT_MODEL_FREE )
        {
            double u, w, d, u0, w0, d0;

            vec3d pt = GetPt();

            u0 = m_UW.x();
            w0 = m_UW.y();

            d0 = CalcDelta( matchgeom ).mag();

            const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
            if ( !s )
            {
                return;
            }

            d = s->FindNearest01( u, w, pt );

            if ( d0 < d )
            {
                u = u0;
                w = w0;
            }

            m_UW.set_xy( u, w );

        }
        else if( m_UType == vsp::FIT_MODEL_FREE && m_WType == vsp::FIT_MODEL_FIXED )
        {
            double u, w, d, u0, w0, d0;

            vec3d pt = GetPt();

            u0 = m_UW.x();
            w0 = m_UW.y();

            d0 = CalcDelta( matchgeom ).mag();

            w = w0;

            const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
            if ( !s )
            {
                return;
            }

            VspCurve c;
            s->GetW01ConstCurve( c, w );

            d = c.FindNearest01(u, pt );

            if ( d0 < d )
            {
                u = u0;
            }

            m_UW.set_x(u);
        }
        else if( m_UType == vsp::FIT_MODEL_FIXED && m_WType == vsp::FIT_MODEL_FREE )
        {
            double u, w, d, u0, w0, d0;

            vec3d pt = GetPt();

            u0 = m_UW.x();
            w0 = m_UW.y();

            d0 = CalcDelta( matchgeom ).mag();

            u = u0;

            const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
            if ( !s )
            {
                return;
            }

            VspCurve c;
            s->GetU01ConstCurve( c, u );

            d = c.FindNearest01(w, pt );

            if ( d0 < d )
            {
                w = w0;
            }

            m_UW.set_y(w);
        }
        else // m_UType == vsp::FIT_MODEL_FIXED && m_WType == vsp::FIT_MODEL_FIXED
        {
            // Do nothing.
        }
    }
}

void TargetPt::RefineUW( Geom* matchgeom )
{
    if ( matchgeom )
    {

        assert( matchgeom->GetID() == m_MatchGeom );

        if( m_UType == vsp::FIT_MODEL_FREE && m_WType == vsp::FIT_MODEL_FREE )
        {
            double u, w, u0, w0;

            vec3d pt = GetPt();

            u0=m_UW.x();
            w0=m_UW.y();

            const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
            if ( !s )
            {
                return;
            }

            s->FindNearest01( u, w, pt, u0, w0 );

            m_UW.set_xy( u, w );

        }
        else if( m_UType == vsp::FIT_MODEL_FREE && m_WType == vsp::FIT_MODEL_FIXED )
        {
            double u, w, u0;

            vec3d pt = GetPt();

            u0=m_UW.x();
            w=m_UW.y();

            const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
            if ( !s )
            {
                return;
            }

            VspCurve c;
            s->GetW01ConstCurve( c, w );

            c.FindNearest01(u, pt, u0 );

            m_UW.set_x(u);
        }
        else if( m_UType == vsp::FIT_MODEL_FIXED && m_WType == vsp::FIT_MODEL_FREE )
        {
            double u, w, w0;

            vec3d pt = GetPt();

            u=m_UW.x();
            w0=m_UW.y();

            const VspSurf* s = MatchSurf( matchgeom, m_SurfIndx );
            if ( !s )
            {
                return;
            }

            VspCurve c;
            s->GetU01ConstCurve( c, u );

            c.FindNearest01(w, pt, w0 );

            m_UW.set_y(w);
        }
        else // m_UType == vsp::FIT_MODEL_FIXED && m_WType == vsp::FIT_MODEL_FIXED
        {
            // Do nothing.
        }
    }
}

bool TargetPt::IsValid()
{
    Geom* matchgeom = VehicleMgr.GetVehicle()->FindGeom( m_MatchGeom );

    // A Geom that is not here, or that has no surface at all, cannot carry a target point.
    // ValidateTargetPts drops the ones that answer no, which is what keeps a *.fit file written
    // against another model from being read in and then optimized against nothing.
    if ( !matchgeom || matchgeom->GetNumTotalSurfs() < 1 )
    {
        return false;
    }

    // The surface count moves with the model -- turning symmetry on or off, changing a blade
    // count -- so an index that was in range when it was set can fall out of it while the model
    // is open, not only across a file load.  The Geom is still here and the point still means
    // something, so bring the index back rather than throw the point away.
    if ( m_SurfIndx < 0 || m_SurfIndx >= matchgeom->GetNumTotalSurfs() )
    {
        m_SurfIndx = 0;
    }

    return true;
}

xmlNodePtr TargetPt::WrapXml( xmlNodePtr & node )
{
    xmlNodePtr targetpt_node = xmlNewChild( node, nullptr, BAD_CAST "TargetPt", nullptr );
    if ( targetpt_node )
    {
        XmlUtil::AddIntNode( targetpt_node, "UType", m_UType );
        XmlUtil::AddIntNode( targetpt_node, "WType", m_WType );

        XmlUtil::AddIntNode( targetpt_node, "UClosed", m_UClosed );
        XmlUtil::AddIntNode( targetpt_node, "WClosed", m_WClosed );

        XmlUtil::AddStringNode( targetpt_node, "MatchGeom", m_MatchGeom );
        XmlUtil::AddIntNode( targetpt_node, "SurfIndx", m_SurfIndx );

        XmlUtil::AddVec2dNode( targetpt_node, "UW", m_UW );
        XmlUtil::AddVec3dNode( targetpt_node, "Pt", m_Pt );
    }
    return targetpt_node;
}

xmlNodePtr TargetPt::UnwrapXml( xmlNodePtr & node )
{
    m_UType = XmlUtil::FindInt( node, "UType", m_UType );
    m_WType = XmlUtil::FindInt( node, "WType", m_WType );

    m_UClosed = (bool) XmlUtil::FindInt( node, "UClosed", m_UClosed );
    m_WClosed = (bool) XmlUtil::FindInt( node, "WClosed", m_WClosed );

    m_MatchGeom = XmlUtil::FindString( node, "MatchGeom", m_MatchGeom );

    // Written since file version 2.  An older file has no such node and reads as surface 0.
    m_SurfIndx = XmlUtil::FindInt( node, "SurfIndx", 0 );

    m_UW = XmlUtil::ExtractVec2dNode( node, "UW");
    m_Pt = XmlUtil::ExtractVec3dNode( node, "Pt");

    return node;
}

//==== Constructor ====//
FitModelMgrSingleton::FitModelMgrSingleton()
{
    m_DistMetric = 0;

    m_NumSelected = 0;
    m_LastSelGeom = "NONE";

    m_GUIShown = false;

    m_SaveFitFileName = string( "DefaultFitModel.fit" );

    Init();
}

FitModelMgrSingleton::~FitModelMgrSingleton()
{
    DelAllTargetPts();
}

void FitModelMgrSingleton::Init()
{
    m_WorkingParmID = "";
    m_CurrVarIndex = int();
    m_CurrTargetPtIndex = int();
    m_NumOptVars = int();

    m_XPrevious.clear();
    m_UndoSignature = string();
    m_UndoValid = false;
}

void FitModelMgrSingleton::Wype()
{
    m_CurrVarIndex = int();
    m_WorkingParmID = string();

    DelAllVars();

    DelAllTargetPts();
}

void FitModelMgrSingleton::Renew()
{
    Wype();
    Init();
}

//==== Get Current Design Variable ====//
string FitModelMgrSingleton::GetCurrVar()
{
    return GetVar( m_CurrVarIndex );
}

//==== Get Design Variable Given Index ====//
string FitModelMgrSingleton::GetVar( int index )
{
    if ( index >= 0 && index < ( int )m_VarVec.size() )
    {
        return m_VarVec[ index ];
    }
    return string();
}

//==== Add Curr Variable ====//
bool FitModelMgrSingleton::AddCurrVar()
{
    //==== Check if Modifying Already Add Link ====//
    if (  m_CurrVarIndex >= 0 && m_CurrVarIndex < ( int )m_VarVec.size() )
    {
        return false;
    }

    if ( CheckForDuplicateVar( m_WorkingParmID ) )
    {
        return false;
    }

    AddVar( m_WorkingParmID );

    return true;
}

//==== Check For Duplicate Variable  ====//
bool FitModelMgrSingleton::CheckForDuplicateVar( const string & p )
{
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        if ( m_VarVec[i] == p )
        {
            return true;
        }
    }
    return false;
}

bool FitModelMgrSingleton::SortVars()
{
    bool wassorted = std::is_sorted( m_VarVec.begin(), m_VarVec.end(), NameCompare );

    if ( !wassorted )
    {
        std::sort( m_VarVec.begin(), m_VarVec.end(), NameCompare );
    }

    return wassorted;
}

//==== Add New Variable ====//
bool FitModelMgrSingleton::AddVar( const string& parm_id )
{
    if ( CheckForDuplicateVar( parm_id ) )
    {
        return false;
    }

    //==== Check If ParmIDs Are Valid ====//
    Parm* p = ParmMgr.FindParm( parm_id );

    if ( p == nullptr )
    {
        return false;
    }

    m_VarVec.push_back( parm_id );
    SortVars();
    m_CurrVarIndex = -1;

    return true;
}

//==== Delete Specified Variable ====//
void FitModelMgrSingleton::DelVar( const string & parm_id )
{
    vector < string > newvec;
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        if ( m_VarVec[i] != parm_id )
        {
            newvec.push_back( m_VarVec[i] );
        }
    }
    m_VarVec = newvec;
}

//==== Check All Vars For Valid Parms ====//
void FitModelMgrSingleton::CheckVars()
{
    //==== Check If Any Parms Have Added/Removed From Last Check ====//
    static int check_links_stamp = 0;
    if ( ParmMgr.GetNumParmChanges() == check_links_stamp )
    {
        return;
    }

    check_links_stamp = ParmMgr.GetNumParmChanges();

    deque< int > del_indices;
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        Parm* pA = ParmMgr.FindParm( m_VarVec[i] );

        if ( !pA )
        {
            del_indices.push_front( i );
        }
    }

    if ( del_indices.size() )
    {
        m_CurrVarIndex = -1;
    }

    for ( int i = 0 ; i < ( int )del_indices.size() ; i++ )
    {
        m_VarVec.erase( m_VarVec.begin() + del_indices[i] );
    }

}

//==== Delete Curr Variable ====//
void FitModelMgrSingleton::DelCurrVar()
{
    if ( m_CurrVarIndex < 0 || m_CurrVarIndex >= ( int )m_VarVec.size() )
    {
        return;
    }

    m_VarVec.erase( m_VarVec.begin() +  m_CurrVarIndex );

    m_CurrVarIndex = -1;
}

//==== Delete All Variables ====//
void FitModelMgrSingleton::DelAllVars()
{
    m_VarVec.clear();
    m_CurrVarIndex = -1;
}

//==== Reset Working Variable ====//
void FitModelMgrSingleton::ResetWorkingVar()
{
    m_CurrVarIndex = -1;

    m_WorkingParmID = string();
}

void FitModelMgrSingleton::SetWorkingParmID( string parm_id )
{
    if ( !ParmMgr.FindParm( parm_id ) )
    {
        parm_id = string();
    }
    m_WorkingParmID = parm_id;
}

void FitModelMgrSingleton::AddTargetPt( TargetPt *tpt )
{
    m_TargetPts.push_back( tpt );
    m_CurrTargetPtIndex = -1;
}

TargetPt* FitModelMgrSingleton::GetCurrTargetPt()
{
    TargetPt* tpt = nullptr;

    if ( m_CurrTargetPtIndex < 0 || m_CurrTargetPtIndex >= ( int )m_TargetPts.size() )
    {
        return tpt;
    }

    tpt = m_TargetPts[m_CurrTargetPtIndex];

    return tpt;
}

TargetPt* FitModelMgrSingleton::GetTargetPt( int index )
{
    if ( index >= 0 && index < ( int )m_TargetPts.size() )
    {
        return m_TargetPts[ index ];
    }
    return nullptr;
}

void FitModelMgrSingleton::DelCurrTargetPt()
{
    if ( m_CurrTargetPtIndex < 0 || m_CurrTargetPtIndex >= ( int )m_TargetPts.size() )
    {
        return;
    }

    TargetPt* tpt = m_TargetPts[m_CurrTargetPtIndex];

    m_TargetPts.erase( m_TargetPts.begin() +  m_CurrTargetPtIndex );

    delete tpt;

    m_CurrTargetPtIndex = -1;
}

void FitModelMgrSingleton::DelAllTargetPts()
{
    for ( int i = 0 ; i < ( int )m_TargetPts.size() ; i++ )
    {
        delete m_TargetPts[i];
    }

    m_TargetPts.clear();
    m_CurrTargetPtIndex = -1;
}

void FitModelMgrSingleton::ValidateTargetPts()
{
    vector < TargetPt* > tokeep;
    vector < TargetPt* > todelete;

    for ( int i = 0 ; i < ( int )m_TargetPts.size() ; i++ )
    {
        if ( m_TargetPts[i] )
        {
            if ( m_TargetPts[i]->IsValid() )
            {
                tokeep.push_back( m_TargetPts[i] );
            }
            else
            {
                todelete.push_back( m_TargetPts[i] );
                m_CurrTargetPtIndex = -1;
            }
        }
    }

    if ( todelete.size() == 0 )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )todelete.size() ; i++ )
    {
        delete todelete[i];
    }

    m_TargetPts = tokeep;
}

void FitModelMgrSingleton::UpdateNumOptVars()
{
    int nvar = m_VarVec.size();

    m_NumOptVars = nvar;
    int npt = m_TargetPts.size();

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];

        if ( tpt->GetUType() == vsp::FIT_MODEL_FREE )
        {
            m_NumOptVars++;
        }
        if ( tpt->GetWType() == vsp::FIT_MODEL_FREE )
        {
            m_NumOptVars++;
        }
    }
}

void FitModelMgrSingleton::BuildPtrVec()
{
    unsigned int nvar = m_VarVec.size();
    unsigned int npt = m_TargetPts.size();

    UpdateNumOptVars();

    m_ParmPtrVec.clear();
    m_ParmPtrVec.resize( nvar );

    for ( int i = 0 ; i < nvar; i++ )
    {
        m_ParmPtrVec[i] = ParmMgr.FindParm( m_VarVec[i] );
    }

    // Whether a surface closes on itself in U or W decides how the optimizer wraps a free
    // coordinate, and it is a property of the surface rather than of the Geom -- two surfaces of
    // one Geom can differ.  So this is gathered per Geom and surface index, not per Geom.
    set< pair< string, int > > usedsurfs;
    set< pair< string, int > >::iterator it;

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];
        usedsurfs.insert( make_pair( tpt->GetMatchGeom(), tpt->GetSurfIndx() ) );
    }

    map< pair< string, int >, SurfData > surfdata;
    for ( it = usedsurfs.begin(); it != usedsurfs.end(); ++it )
    {
        SurfData s;
        s.m_GeomPtr = VehicleMgr.GetVehicle()->FindGeom( it->first );
        s.m_UClosed = false;
        s.m_WClosed = false;

        const VspSurf* surf = MatchSurf( s.m_GeomPtr, it->second );
        if ( surf )
        {
            s.m_UClosed = surf->IsClosedU();
            s.m_WClosed = surf->IsClosedW();
        }

        surfdata[ *it ] = s;
    }

    m_TargetGeomPtrVec.clear();
    m_TargetGeomPtrVec.resize( npt );

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];
        SurfData s = surfdata[ make_pair( tpt->GetMatchGeom(), tpt->GetSurfIndx() ) ];

        m_TargetGeomPtrVec[i] = s.m_GeomPtr;
        tpt->SetUClosed( s.m_UClosed );
        tpt->SetWClosed( s.m_WClosed );
    }
}

void FitModelMgrSingleton::RefineTargetUW()
{
    ValidateTargetPts();

    // Refine only moves the free surface coordinates, but the snapshot is the whole optimization
    // vector either way -- the Parm entries simply come back unchanged.
    BuildPtrVec();
    SaveUndoState();

    int npt = m_TargetPts.size();

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];
        Geom* g = VehicleMgr.GetVehicle()->FindGeom( tpt->GetMatchGeom() );

        tpt->RefineUW( g );
    }

    m_ParmPtrVec.clear();
    m_TargetGeomPtrVec.clear();
}

void FitModelMgrSingleton::SearchTargetUW()
{
    ValidateTargetPts();

    BuildPtrVec();
    SaveUndoState();

    int npt = m_TargetPts.size();

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];
        Geom* g = VehicleMgr.GetVehicle()->FindGeom( tpt->GetMatchGeom() );

        tpt->SearchUW( g );
    }

    m_ParmPtrVec.clear();
    m_TargetGeomPtrVec.clear();
}

void FitModelMgrSingleton::ParmToX( double *x )
{
    int nvar = m_VarVec.size();
    int npt = m_TargetPts.size();

    int xindx = 0;
    for ( int i = 0 ; i < nvar; i++ )
    {
        x[xindx] = m_ParmPtrVec[i]->Get();
        xindx++;
    }

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];

        vec2d uw = tpt->GetUW();

        if ( tpt->GetUType() == vsp::FIT_MODEL_FREE )
        {
            x[xindx] = uw.x();
            xindx++;
        }
        if ( tpt->GetWType() == vsp::FIT_MODEL_FREE )
        {
            x[xindx] = uw.y();
            xindx++;
        }
    }
}

void FitModelMgrSingleton::XtoParm( const double *x )
{
    int nvar = m_VarVec.size();
    int npt = m_TargetPts.size();

    int xindx = 0;
    for ( int i = 0 ; i < nvar; i++ )
    {
        m_ParmPtrVec[i]->Set( x[xindx] );
        xindx++;
    }

    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];

        vec2d uw = tpt->GetUW();

        if ( tpt->GetUType() == vsp::FIT_MODEL_FREE )
        {
            uw.set_x( Clamp01( x[xindx], tpt->IsUClosed() ) );
            xindx++;
        }
        if ( tpt->GetWType() == vsp::FIT_MODEL_FREE )
        {
            uw.set_y( Clamp01( x[xindx], tpt->IsWClosed() ) );
            xindx++;
        }

        tpt->SetUW( uw );
    }
}

string FitModelMgrSingleton::UndoSignature()
{
    // Everything the layout and the meaning of the optimization vector depends on: which Parms are
    // variables, in order, and for each target point the surface it is matched to and whether each
    // of its surface coordinates is free.
    string sig;

    for ( int i = 0 ; i < ( int )m_VarVec.size(); i++ )
    {
        sig += m_VarVec[i] + ";";
    }

    sig += "|";

    for ( int i = 0 ; i < ( int )m_TargetPts.size(); i++ )
    {
        TargetPt* tpt = m_TargetPts[i];

        sig += tpt->GetMatchGeom() + ",";
        sig += std::to_string( tpt->GetSurfIndx() ) + ",";
        sig += std::to_string( tpt->GetUType() ) + ",";
        sig += std::to_string( tpt->GetWType() ) + ";";
    }

    return sig;
}

void FitModelMgrSingleton::SaveUndoState()
{
    m_XPrevious.resize( m_NumOptVars );

    if ( m_NumOptVars > 0 )
    {
        ParmToX( m_XPrevious.data() );
    }

    m_UndoSignature = UndoSignature();
    m_UndoValid = true;
}

bool FitModelMgrSingleton::CanUndo()
{
    if ( !m_UndoValid )
    {
        return false;
    }

    // A snapshot taken against a different set of variables or target points cannot be put back.
    return UndoSignature() == m_UndoSignature;
}

bool FitModelMgrSingleton::Undo()
{
    ValidateTargetPts();

    if ( !CanUndo() )
    {
        // Whatever was saved no longer describes the current setup, so it is of no further use.
        m_UndoValid = false;
        return false;
    }

    BuildPtrVec();

    if ( ( int )m_XPrevious.size() != m_NumOptVars )
    {
        m_UndoValid = false;
        m_ParmPtrVec.clear();
        m_TargetGeomPtrVec.clear();
        return false;
    }

    if ( m_NumOptVars > 0 )
    {
        XtoParm( m_XPrevious.data() );
    }

    VehicleMgr.GetVehicle()->ForceUpdate( GeomBase::SURF );

    m_ParmPtrVec.clear();
    m_TargetGeomPtrVec.clear();

    // One level of undo; the snapshot is spent.
    m_UndoValid = false;

    UpdateDist();

    return true;
}

double FitModelMgrSingleton::Clamp01( double x, bool periodic )
{
    if ( periodic )
    {
        x = x - floor(x);
    }
    else
    {
        if ( x > 1.0 )
        {
            x = 1.0;
        }
        else if ( x < 0.0 )
        {
            x = 0.0;
        }
    }
    return x;
}

void FitModelMgrSingleton::UpdateDist()
{
    ValidateTargetPts();

    int npt = m_TargetPts.size();

    // With no target points there is no distance to report.  Averaging over none of them used to
    // divide by zero; -1 marks the absence, since zero is a real and wanted answer.
    if ( npt == 0 )
    {
        m_DistMetric = -1.0;
        return;
    }

    m_DistMetric = 0;

    // Calculate target point distances
    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];

        vec3d delta = tpt->CalcDelta();

        m_DistMetric += dot( delta, delta );
    }
    m_DistMetric = sqrt( m_DistMetric / npt );
}

int fcn( void *p, int m, int n, const double *x, double *fvec, double *fjac, int ldfjac, int iflag )
{
    if ( iflag == 1 )
    {
        FitModelMgr.CalcMetrics( x, fvec );
    }
    else if ( iflag == 2 )
    {
        FitModelMgr.CalcMetricDeriv( x, fvec, fjac );
    }

    return 0;
}

void FitModelMgrSingleton::CalcMetrics( const double *x, double *y )
{
    // Apply changes to geometry.
    XtoParm( x );
    VehicleMgr.GetVehicle()->Update( false );

    int npt = m_TargetPts.size();
    // Calculate target point distances
    for ( int i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];
        Geom* g = m_TargetGeomPtrVec[i];

        vec3d delta = tpt->CalcDelta( g );

        y[3 * i] = delta.x();
        y[3 * i + 1] = delta.y();
        y[3 * i + 2] = delta.z();
    }
}

void FitModelMgrSingleton::CalcMetricDeriv( const double *x, double *y, double *yprm )
{
    int n = m_NumOptVars;
    int nvar = m_VarVec.size();
    int npt = m_TargetPts.size();
    int m = 3 * npt;

    int i, j, xindx;
    double x0, dx;

    double *fprm;
    fprm = new double[m];
    double *xp;
    xp = new double[n];

    for (j = 0; j < n; ++j)
    {
        xp[j] = x[j];
    }

    double eps = sqrt( dpmpar( 1.0 ) ); // sqrt of machine precision

    xindx = 0;
    for (j = 0; j < nvar; ++j)
    {
        x0 = xp[xindx];
        dx = eps * std::abs(x0);
        if (dx == 0.)
        {
            dx = eps;
        }

        xp[xindx] = x0 + dx;
        FitModelMgr.CalcMetrics( xp, fprm );
        xp[xindx] = x0;

        for (i = 0; i < m; ++i)
        {
            yprm[i + xindx * m] = (fprm[i] - y[i]) / dx;
        }
        xindx++;
    }
    // Restore geometry to initial state.
    XtoParm( x );
    VehicleMgr.GetVehicle()->Update( false );

    // Pre-set remaining derivatives to zero.
    for ( j = xindx; j < n; j++ )
    {
        for (i = 0; i < m; ++i)
        {
            yprm[i + j * m] = 0.0;
        }
    }

    // Calculate exact derivatives of target point movement.
    for ( i = 0 ; i < npt; i++ )
    {
        TargetPt* tpt = m_TargetPts[i];
        Geom* g = m_TargetGeomPtrVec[i];

        if ( tpt->GetUType() == vsp::FIT_MODEL_FREE )
        {
            vec3d du = tpt->CalcDerivU( g );

            yprm[3 * i + xindx * m] = du.x();
            yprm[3 * i + 1 + xindx * m] = du.y();
            yprm[3 * i + 2 + xindx * m] = du.z();

            xindx++;
        }
        if ( tpt->GetWType() == vsp::FIT_MODEL_FREE )
        {
            vec3d dw = tpt->CalcDerivW( g );

            yprm[3 * i + xindx * m] = dw.x();
            yprm[3 * i + 1 + xindx * m] = dw.y();
            yprm[3 * i + 2 + xindx * m] = dw.z();

            xindx++;
        }
    }

    delete [] fprm;
    delete [] xp;
}

int FitModelMgrSingleton::Optimize()
{
    ValidateTargetPts();

    BuildPtrVec();

    SaveUndoState();

    int nvar = m_NumOptVars;
    int npt = m_TargetPts.size();
    int m = 3 * npt;

    double *x;
    x = new double[ m_NumOptVars ];

    ParmToX( x );

    double *y;
    y = new double[m];

    double *fjac;
    fjac = new double[m*nvar];

    int ldfjac = m;

    for ( int i = 0 ; i < m; i++ )
    {
        y[i] = 0.0;

        for ( int j = 0 ; j < nvar; j++ )
        {
            fjac[i + j * ldfjac ] = 0.0;
        }
    }

    double tol = sqrt( dpmpar( 1.0 ) ); // sqrt of machine precision

    int *ipvt;
    ipvt = new int[nvar];

    int lwa = 5 * nvar + m;

    double *wa;
    wa = new double[lwa];

    int info = lmder1( fcn, nullptr, m, nvar, x, y, fjac, ldfjac, tol, ipvt, wa, lwa );

    XtoParm( x );
    VehicleMgr.GetVehicle()->ForceUpdate( GeomBase::SURF ); // Update tesselation to ensure Geom is drawn properly

    m_ParmPtrVec.clear();
    m_TargetGeomPtrVec.clear();

    delete [] x;
    delete [] y;
    delete [] fjac;
    delete [] ipvt;
    delete [] wa;

    return info;
}

void FitModelMgrSingleton::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return;
    }

    // Draw target points.

    m_TargetPntDrawObj.m_PntVec.clear();
    m_TargetLineDrawObj.m_PntVec.clear();

    // Hard coded an id so the data in this drawobj does not get dumped each frame.
    m_TargetPntDrawObj.m_GeomID = "IDFORTARGETPNTDO";
    m_TargetPntDrawObj.m_Type = DrawObj::VSP_POINTS;
    m_TargetPntDrawObj.m_PointSize = 8.0;
    m_TargetPntDrawObj.m_PointColor = vec3d ( 0.0, 0.0, 0.0 );
    m_TargetPntDrawObj.m_GeomChanged = true;

    m_TargetLineDrawObj.m_GeomID = "IDFORTARGETLINEDO";
    m_TargetLineDrawObj.m_Type = DrawObj::VSP_LINES;
    m_TargetLineDrawObj.m_LineWidth = 1.0;
    m_TargetLineDrawObj.m_LineColor = vec3d( 0.0, 0.0, 0.0 );
    m_TargetLineDrawObj.m_GeomChanged = true;

    ValidateTargetPts();

    int numOfTargetPts = FitModelMgr.GetNumTargetPt();
    for ( int i = 0; i < numOfTargetPts; i++ )
    {
        TargetPt * tpnt = FitModelMgr.GetTargetPt( i );
        vec3d pt1 = tpnt->GetPt() ;
        vec3d pt2 = tpnt->GetMatchPt();
        m_TargetPntDrawObj.m_PntVec.push_back( pt1 );
        m_TargetPntDrawObj.m_PntVec.push_back( pt2 );

        m_TargetLineDrawObj.m_PntVec.push_back( pt1 );
        m_TargetLineDrawObj.m_PntVec.push_back( pt2 );
    }
    draw_obj_vec.push_back( &m_TargetPntDrawObj );
    draw_obj_vec.push_back( &m_TargetLineDrawObj );

}

std::string FitModelMgrSingleton::getFeedbackGroupName()
{
    return std::string("FitModelGUIGroup");
}

void FitModelMgrSingleton::SelectPoint( const string &gid, int index )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh->m_SelectOneFlag() && m_LastSelGeom != "NONE" && m_NumSelected == 1 )
    {
        Geom *g = veh->FindGeom( m_LastSelGeom );
        PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

        if ( pt_cloud )
        {
            pt_cloud->UnSelectLastSel();
        }
        m_NumSelected = 0;
    }

    Geom *g = veh->FindGeom( gid );
    PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

    if ( pt_cloud )
    {
        pt_cloud->SelectPoint( index );
    }

    m_NumSelected++;
    m_LastSelGeom = gid;

    VehicleMgr.GetVehicle()->UpdateGUI();
}

void FitModelMgrSingleton::SelectAllShown()
{
    m_NumSelected = 0;

    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->SelectAllShown();
                m_NumSelected += pt_cloud->GetNumSelected();
            }
        }
    }
}

void FitModelMgrSingleton::SelectNone()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->SelectNone();
            }
        }
    }
    m_NumSelected = 0;
}

void FitModelMgrSingleton::SelectInv()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    m_NumSelected = 0;

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->SelectInv();
                m_NumSelected += pt_cloud->GetNumSelected();
            }
        }
    }
}

void FitModelMgrSingleton::HideSelection()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->HideSelection();
            }
        }
    }
    m_NumSelected = 0;
}

void FitModelMgrSingleton::HideUnselected()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->HideUnselected();
            }
        }
    }
}

void FitModelMgrSingleton::HideAll()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->HideAll();
            }
        }
    }
    m_NumSelected = 0;
}

void FitModelMgrSingleton::HideInv()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->HideInv();
            }
        }
    }
    m_NumSelected = 0;
}

void FitModelMgrSingleton::ShowAll()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector < string > ptclouds = veh->GetPtCloudGeoms();

    for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
    {
        const string& gid = ptclouds[i];
        Geom *g = veh->FindGeom( gid );

        if ( g->GetSetFlag( vsp::SET_SHOWN ) )
        {
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->ShowAll();
            }
        }
    }
}

void FitModelMgrSingleton::SetSelectOne()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh->m_SelectOneFlag() )
    {
        SelectNone();

        if ( m_LastSelGeom != "NONE" )
        {
            Geom *g = veh->FindGeom( m_LastSelGeom );
            PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

            if ( pt_cloud )
            {
                pt_cloud->SelectLastSel();
            }
            m_NumSelected++;
        }

        veh->m_SelectBoxFlag.Set( false );
    }
}

void FitModelMgrSingleton::SetSelectBox()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh->m_SelectBoxFlag() )
    {
        veh->m_SelectOneFlag.Set( false );
    }
}

void FitModelMgrSingleton::AddSelectedPts( const string &tgtGeomID )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    vector< vec3d > targetCandidates;

    bool ufix = veh->m_UType.Get() == 0;
    bool wfix = veh->m_WType.Get() == 0;

    // If u and w is fixed, only one target can be add each time.
    if ( ufix && wfix )
    {
        if ( m_NumSelected == 1 )
        {
            if ( m_LastSelGeom != "NONE" )
            {
                Geom *g = veh->FindGeom( m_LastSelGeom );
                PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

                if ( pt_cloud )
                {
                    pt_cloud->GetSelectedPoints( targetCandidates );
                }
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        vector < string > ptclouds = veh->GetPtCloudGeoms();

        for ( int i = 0; i < ( int ) ptclouds.size(); i++ )
        {
            const string& gid = ptclouds[i];
            Geom *g = veh->FindGeom( gid );

            if ( g->GetSetFlag( vsp::SET_SHOWN ) )
            {
                PtCloudGeom* pt_cloud = dynamic_cast< PtCloudGeom* >( g );

                if ( pt_cloud )
                {
                    pt_cloud->GetSelectedPoints( targetCandidates );
                }
            }
        }
    }

    for ( int i = 0; i < ( int )targetCandidates.size(); i++ )
    {
        const vec3d& pt = targetCandidates[i];
        vec2d uw( veh->m_UTargetPt.Get(), veh->m_WTargetPt.Get() );

        TargetPt *tpt = new TargetPt();
        tpt->SetPt( pt );
        tpt->SetMatchGeom( tgtGeomID );
        tpt->SetUW( uw );
        tpt->SetSurfIndx( veh->m_SurfIndx.Get() );
        tpt->SetUType( veh->m_UType.Get() );
        tpt->SetWType( veh->m_WType.Get() );

        Geom* g = veh->FindGeom( tpt->GetMatchGeom() );

        tpt->SearchUW( g );

        AddTargetPt( tpt );
    }

    SelectNone();
}

bool FitModelMgrSingleton::Save()
{
    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( nullptr, ( const xmlChar * )"Vsp_FitModel" );
    xmlDocSetRootElement( doc, root );
    XmlUtil::AddIntNode( root, "Version", CURRENT_FIT_FILE_VER);

    //Wrap Target Points into a xml file
    for (int index = 0; index < m_TargetPts.size(); ++index)
    {
        m_TargetPts[index]->WrapXml( root );
    }

    //Wrap Variables into a xml file
    for (int index = 0; index < m_VarVec.size(); ++index)
    {
        xmlNodePtr variable_node = xmlNewChild( root, nullptr, BAD_CAST "Variable", nullptr );
        if ( variable_node )
        {
            XmlUtil::AddStringNode(variable_node, "ParmID", m_VarVec[index]);
        }
    }

    //===== Save XML Tree and Free Doc =====//
    int err = xmlSaveFormatFile( m_SaveFitFileName.c_str(), doc, 1 );
    xmlFreeDoc( doc );

    if( err == -1 )  // Failure occurred
    {
        return false;
    }

    return true;
}

int FitModelMgrSingleton::Load()
{
    //==== Read Xml File ====//
    xmlDocPtr doc;

    LIBXML_TEST_VERSION
            xmlKeepBlanksDefault( 0 );

    //==== Build an XML tree from a the file ====//
    doc = xmlReadFile( m_LoadFitFileName.c_str(), nullptr, 0 );
    if ( doc == nullptr )
    {
        fprintf( stderr, "could not parse XML document\n" );
        return 1;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == nullptr )
    {
        fprintf( stderr, "empty document\n" );
        xmlFreeDoc( doc );
        return 2;
    }

    if ( xmlStrcmp( root->name, ( const xmlChar * )"Vsp_FitModel" ) )
    {
        fprintf( stderr, "document of the wrong type, Vsp Fit Model not found\n" );
        xmlFreeDoc( doc );
        return 3;
    }

    //==== Find Version Number ====//
    int fileOpenVersion = XmlUtil::FindInt( root, "Version", 0 );

    if ( fileOpenVersion < MIN_FIT_FILE_VER )
    {
        fprintf( stderr, "document version not supported \n");
        xmlFreeDoc( doc );
        return 4;
    }

    int num = XmlUtil::GetNumNames( root, "TargetPt" );

    xmlNodePtr iter_node = root->xmlChildrenNode;

    while( iter_node != nullptr )
    {
        if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )"TargetPt" ) )
        {
            TargetPt* point = new TargetPt();

            point->UnwrapXml(iter_node);
            AddTargetPt( point );
        }
        iter_node = iter_node->next;
    }

    num = XmlUtil::GetNumNames( root, "Variable" );

    iter_node = root->xmlChildrenNode;

    while( iter_node != nullptr )
    {
        if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )"Variable" ) )
        {
            string str = XmlUtil::FindString( iter_node, "ParmID", string() );
            AddVar( str );
        }
        iter_node = iter_node->next;
    }

    //===== Free Doc =====//
    xmlFreeDoc( doc );

    // The file names Geoms by ID.  Drop any target point naming a Geom this model does not have,
    // or one without a surface, rather than carrying it into the optimizer.
    ValidateTargetPts();

    return 0;
}
