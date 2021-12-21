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
    return NULL;
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
    AddVar( m_WorkingParmID, veh->m_WorkingXDDMType.Get() );

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

    if ( p == NULL )
    {
        return false;
    }

    DesignVar* dv = new DesignVar();

    dv->m_ParmID = parm_id;
    dv->m_XDDM_Type = xddmtype;

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
}

void DesignVarMgrSingleton::SetWorkingParmID( string parm_id )
{
    if ( !ParmMgr.FindParm( parm_id ) )
    {
        parm_id = string();
    }
    m_WorkingParmID = parm_id;
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
                AddVar( id, vsp::XDDM_VAR );
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

    xmlNodePtr model_node = xmlNewNode( NULL, ( const xmlChar * )"Model" );
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
            var_node = xmlNewChild( model_node, NULL, ( const xmlChar * )"Variable", NULL );
        }
        else
        {
            var_node = xmlNewChild( model_node, NULL, ( const xmlChar * )"Constant", NULL );
        }

        string c_name, g_name, p_name;
        ParmMgr.GetNames( m_VarVec[i]->m_ParmID, c_name, g_name, p_name );

        char varname[255];
        sprintf( varname, "%s:%s:%s", c_name.c_str(), g_name.c_str(), p_name.c_str() );

        xmlSetProp( var_node, ( const xmlChar * )"ID", ( const xmlChar * )varname );
        XmlUtil::SetDoubleProp( var_node, "Value", p->Get() );
        XmlUtil::SetDoubleProp( var_node, "Min", p->GetLowerLimit() );
        XmlUtil::SetDoubleProp( var_node, "Max", p->GetUpperLimit() );
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
    doc = xmlParseFile( newfile.c_str() );
//  if (doc == NULL) return 0;

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == NULL )
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

                // Set with delayed updates.
                p->Set( val );

                const xmlChar* varstr = ( xmlChar* ) "Variable";

                if( !xmlStrcmp( var_node->name, varstr ) )
                {
                    AddVar( varid, vsp::XDDM_VAR );
                }
                else
                {
                    AddVar( varid, vsp::XDDM_CONST );
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
