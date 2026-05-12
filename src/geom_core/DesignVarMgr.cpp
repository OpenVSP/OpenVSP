//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DesignVarMgr.h: Design Variable Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "DesignVarMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"

//==== Constructor ====//
DesignVar:: DesignVar()
{
    m_ParmID = "";
    m_XDDM_Type = vsp::XDDM_VAR;
    m_LowerLimit = 0.0;
    m_UpperLimit = 0.0;
}

bool DesignVarNameCompare( const DesignVar *dvA, const DesignVar *dvB )
{
    return NameCompare( dvA->m_ParmID, dvB->m_ParmID );
}

//==== Constructor ====//
DesignVarMgrSingleton::DesignVarMgrSingleton()
{
    Init();
}

DesignVarMgrSingleton::~DesignVarMgrSingleton()
{
    DelAllVars();
}

void DesignVarMgrSingleton::Init()
{
    m_CurrVarIndex = 0;
    m_WorkingParmID = "";
}

void DesignVarMgrSingleton::Wype()
{
    m_CurrVarIndex = int();
    m_WorkingParmID = string();

    DelAllVars();
    m_VarVec = vector< DesignVar* >();
}

void DesignVarMgrSingleton::Renew()
{
    Wype();
    Init();
}

//==== Get Current Design Variable ====//
DesignVar* DesignVarMgrSingleton::GetCurrVar()
{
    return GetVar( m_CurrVarIndex );
}

//==== Get Design Variable Given Index ====//
DesignVar* DesignVarMgrSingleton::GetVar( int index )
{
    if ( index >= 0 && index < ( int )m_VarVec.size() )
    {
        return m_VarVec[ index ];
    }
    return nullptr;
}

//==== Add Curr Variable ====//
bool DesignVarMgrSingleton::AddCurrVar()
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

    Vehicle* veh = VehicleMgr.GetVehicle();

    // The limit sliders sit directly above the Add Variable button and are seeded from the
    // parm when it is picked, so what they hold is what the user means.  Taking the parm's
    // own limits here instead would throw away any narrowing that had been typed, with
    // nothing to say it had happened.
    double lowerlimit = veh->m_WorkingDVMin();
    double upperlimit = veh->m_WorkingDVMax();

    Parm *p = ParmMgr.FindParm( m_WorkingParmID );

    // A zero width range means the sliders were never seeded -- fall back to the parm.
    if ( p && lowerlimit == upperlimit )
    {
        lowerlimit = p->GetLowerLimit();
        upperlimit = p->GetUpperLimit();
    }

    AddVar( m_WorkingParmID, veh->m_WorkingXDDMType.Get(), lowerlimit, upperlimit );

    return true;
}

//==== Check For Duplicate Variable  ====//
bool DesignVarMgrSingleton::CheckForDuplicateVar( const string & p )
{
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        if ( m_VarVec[i]->m_ParmID == p )
        {
            return true;
        }
    }
    return false;
}

bool DesignVarMgrSingleton::SortVars()
{
    bool wassorted = std::is_sorted( m_VarVec.begin(), m_VarVec.end(), DesignVarNameCompare );

    if ( !wassorted )
    {
        std::sort( m_VarVec.begin(), m_VarVec.end(), DesignVarNameCompare );
    }

    return wassorted;
}

//==== Add New Variable ====//
bool DesignVarMgrSingleton::AddVar( const string& parm_id, int xddmtype )
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

    DesignVar* dv = new DesignVar();

    dv->m_ParmID = parm_id;
    dv->m_XDDM_Type = xddmtype;
    dv->m_LowerLimit = p->GetLowerLimit();
    dv->m_UpperLimit = p->GetUpperLimit();

    m_VarVec.push_back( dv );
    SortVars();
    m_CurrVarIndex = -1;

    return true;
}

//==== Add New Variable ====//
bool DesignVarMgrSingleton::AddVar( const string& parm_id, int xddmtype, double lowerlimit, double upperlimit )
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

    DesignVar* dv = new DesignVar();

    dv->m_ParmID = parm_id;
    dv->m_XDDM_Type = xddmtype;
    dv->m_LowerLimit = lowerlimit;
    dv->m_UpperLimit = upperlimit;

    m_VarVec.push_back( dv );
    SortVars();
    m_CurrVarIndex = -1;

    return true;
}

//==== Check All Vars For Valid Parms ====//
void DesignVarMgrSingleton::CheckVars()
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
        Parm* pA = ParmMgr.FindParm( m_VarVec[i]->m_ParmID );

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
void DesignVarMgrSingleton::DelCurrVar()
{
    if ( m_CurrVarIndex < 0 || m_CurrVarIndex >= ( int )m_VarVec.size() )
    {
        return;
    }

    DesignVar* pl = m_VarVec[m_CurrVarIndex];

    m_VarVec.erase( m_VarVec.begin() +  m_CurrVarIndex );

    delete pl;

    m_CurrVarIndex = -1;
}

//==== Delete All Variables ====//
void DesignVarMgrSingleton::DelAllVars()
{
    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        delete m_VarVec[i];
    }

    m_VarVec.clear();
    m_CurrVarIndex = -1;
}

//==== Reset Working Variable ====//
void DesignVarMgrSingleton::ResetWorkingVar()
{
    m_CurrVarIndex = -1;

    m_WorkingParmID = string();
    Vehicle* veh = VehicleMgr.GetVehicle();
    veh->m_WorkingXDDMType = vsp::XDDM_VAR;
    veh->m_WorkingDVMin = 0;
    veh->m_WorkingDVMax = 0;
}

void DesignVarMgrSingleton::SetWorkingParmID( string parm_id )
{
    Parm *p = ParmMgr.FindParm( parm_id );

    if ( !p )
    {
        parm_id = string();
    }

    bool changed = ( parm_id != m_WorkingParmID );

    m_WorkingParmID = parm_id;

    // Seed the limit sliders from the parm the user just picked, so they start at its own
    // bounds and can be narrowed from there.  Left alone they would show the last
    // variable's numbers, or zero and zero in a fresh session, and AddCurrVar takes the
    // new variable's limits from them.
    if ( p && changed )
    {
        Vehicle* veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            veh->m_WorkingDVMin = p->GetLowerLimit();
            veh->m_WorkingDVMax = p->GetUpperLimit();
        }
    }
}

void DesignVarMgrSingleton::WriteDesVarsDES( const string &newfile )
{
    FILE *fp;
    fp = fopen( newfile.c_str(), "w" );

    fprintf( fp, "%d\n", ( int )m_VarVec.size() );

    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        string c_name, g_name, p_name;
        ParmMgr.GetNames( m_VarVec[i]->m_ParmID, c_name, g_name, p_name );

        Parm *p = ParmMgr.FindParm( m_VarVec[i]->m_ParmID );

        fprintf( fp, "%s:%s:%s:%s: %g\n", m_VarVec[i]->m_ParmID.c_str(), c_name.c_str(), g_name.c_str(), p_name.c_str(), p->Get() );
    }

    fprintf( fp, "#\n" );
    fprintf( fp, "#LIMITS\n" );
    fprintf( fp, "#\n" );

    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        string c_name, g_name, p_name;
        ParmMgr.GetNames( m_VarVec[i]->m_ParmID, c_name, g_name, p_name );

        Parm *p = ParmMgr.FindParm( m_VarVec[i]->m_ParmID );

        fprintf( fp, "%s:%s:%s:%s: %g %g\n", m_VarVec[i]->m_ParmID.c_str(), c_name.c_str(), g_name.c_str(), p_name.c_str(), m_VarVec[i]->m_LowerLimit, m_VarVec[i]->m_UpperLimit );
    }

    fclose( fp );
}

void DesignVarMgrSingleton::ReadDesVarsDES( const string &newfile )
{
    FILE *fp;
    fp = fopen( newfile.c_str(), "r" );
    char temp[255];

    fgets( temp, 255, fp );
    string line = temp;
    int nparm = atoi( line.c_str() );

    if( nparm > 0 )
    {
        DelAllVars();
        ResetWorkingVar();

        for ( int i = 0 ; i < nparm ; i++ )
        {
            fgets( temp, 255, fp );
            line = temp;

            unsigned int istart = 0;
            unsigned int iend = line.find( ':', istart );
            string id = line.substr( istart, iend - istart );

            istart = iend + 1;
            iend = line.find( ' ', istart );

            istart = iend + 1;
            iend = line.length();
            double val = atof( line.substr( istart, iend - istart ).c_str() );

            Parm *p = ParmMgr.FindParm( id );

            if ( p )
            {
                // Set with delayed updates.
                p->Set( val );
                AddVar( id, vsp::XDDM_VAR, p->GetLowerLimit(), p->GetUpperLimit() );
            }
        }

        // Read optional LIMITS section -- match lines by ParmID and apply lower/upper limits.
        while ( fgets( temp, 255, fp ) != nullptr )
        {
            line = temp;

            if ( line.empty() || line[0] == '#' )
            {
                continue;
            }

            unsigned int istart = 0;
            unsigned int iend = line.find( ':', istart );
            string id = line.substr( istart, iend - istart );

            istart = iend + 1;
            iend = line.find( ' ', istart );

            istart = iend + 1;
            iend = line.find( ' ', istart );
            double lower = atof( line.substr( istart, iend - istart ).c_str() );

            istart = iend + 1;
            iend = line.length();
            double upper = atof( line.substr( istart, iend - istart ).c_str() );

            for ( int i = 0 ; i < (int)m_VarVec.size() ; i++ )
            {
                if ( m_VarVec[i]->m_ParmID == id )
                {
                    m_VarVec[i]->m_LowerLimit = lower;
                    m_VarVec[i]->m_UpperLimit = upper;
                    break;
                }
            }
        }

        // Trigger update.
        VehicleMgr.GetVehicle()->Update();
    }
    fclose( fp );
}

void DesignVarMgrSingleton::WriteDesVarsXDDM( const string &newfile )
{
    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr model_node = xmlNewNode( nullptr, ( const xmlChar * )"Model" );
    xmlDocSetRootElement( doc, model_node );

    xmlSetProp( model_node, ( const xmlChar * )"ID", ( const xmlChar * ) VehicleMgr.GetVehicle()->GetVSP3FileName().c_str() );
    xmlSetProp( model_node, ( const xmlChar * )"Modeler", ( const xmlChar * )"OpenVSP" );
    xmlSetProp( model_node, ( const xmlChar * )"Wrapper", ( const xmlChar * )"wrap_vsp.csh" );

    for ( int i = 0 ; i < ( int )m_VarVec.size() ; i++ )
    {
        Parm *p = ParmMgr.FindParm( m_VarVec[i]->m_ParmID );

        xmlNodePtr var_node;

        if( m_VarVec[i]->m_XDDM_Type == vsp::XDDM_VAR )
        {
            var_node = xmlNewChild( model_node, nullptr, ( const xmlChar * )"Variable", nullptr );
        }
        else
        {
            var_node = xmlNewChild( model_node, nullptr, ( const xmlChar * )"Constant", nullptr );
        }

        string c_name, g_name, p_name;
        ParmMgr.GetNames( m_VarVec[i]->m_ParmID, c_name, g_name, p_name );

        char varname[255];
        snprintf( varname, sizeof( varname ), "%s:%s:%s", c_name.c_str(), g_name.c_str(), p_name.c_str() );

        xmlSetProp( var_node, ( const xmlChar * )"ID", ( const xmlChar * )varname );
        XmlUtil::SetDoubleProp( var_node, "Value", p->Get() );
        XmlUtil::SetDoubleProp( var_node, "Min", m_VarVec[i]->m_LowerLimit );
        XmlUtil::SetDoubleProp( var_node, "Max", m_VarVec[i]->m_UpperLimit );
        xmlSetProp( var_node, ( const xmlChar * )"VSPID", ( const xmlChar * )m_VarVec[i]->m_ParmID.c_str() );
    }

    //===== Save XML Tree and Free Doc =====//
    xmlSaveFormatFile( newfile.c_str(), doc, 1 );
    xmlFreeDoc( doc );
}

void DesignVarMgrSingleton::ReadDesVarsXDDM( const string &newfile )
{
    DelAllVars();
    ResetWorkingVar();

    //==== Read Xml File ====//
    xmlDocPtr doc;

    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    //==== Build an XML tree from a the file ====//
    doc = xmlReadFile( newfile.c_str(), nullptr, 0 );
//  if (doc == nullptr) return 0;

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == nullptr )
    {
        fprintf( stderr, "empty document\n" );
        xmlFreeDoc( doc );
//      return 0;
    }

    vector< xmlNodePtr > vlist;

    int num_v = XmlUtil::GetNumNames( root, "Variable" );
    for ( int i = 0 ; i < num_v ; i++ )
    {
        xmlNodePtr var_node = XmlUtil::GetNode( root, "Variable", i );
        vlist.push_back( var_node );
    }

    int num_c = XmlUtil::GetNumNames( root, "Constant" );
    for ( int i = 0 ; i < num_c ; i++ )
    {
        xmlNodePtr cst_node = XmlUtil::GetNode( root, "Constant", i );
        vlist.push_back( cst_node );
    }

    int num_tot = num_v + num_c;

    for ( int i = 0 ; i < num_tot ; i++ )
    {
        xmlNodePtr var_node = vlist[i];

        if ( var_node )
        {
            string varid = XmlUtil::FindStringProp( var_node, "VSPID", " " );


            Parm *p = ParmMgr.FindParm( varid );

            if ( p )
            {
                double val = XmlUtil::FindDoubleProp( var_node, "Value", p->Get() );
                double lowerlimit = XmlUtil::FindDoubleProp( var_node, "Min", p->GetLowerLimit() );
                double upperlimit = XmlUtil::FindDoubleProp( var_node, "Max", p->GetUpperLimit() );

                // Set with delayed updates.
                p->Set( val );

                const xmlChar* varstr = ( xmlChar* ) "Variable";


                if( !xmlStrcmp( var_node->name, varstr ) )
                {
                    AddVar( varid, vsp::XDDM_VAR, lowerlimit, upperlimit );
                }
                else
                {
                    AddVar( varid, vsp::XDDM_CONST, lowerlimit, upperlimit );
                }
            }
        }
    }

    // Trigger update.
    VehicleMgr.GetVehicle()->Update();

    //===== Free Doc =====//
    xmlFreeDoc( doc );

//  return 1;
}
