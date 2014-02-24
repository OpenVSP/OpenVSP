//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Parm.cpp: implementation of the Parm and Parm Container classes.
//
//////////////////////////////////////////////////////////////////////

#include "Parm.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VehicleMgr.h"
#include <float.h>
#include <time.h>
#include <algorithm>

#include "XmlUtil.h"
#include "StringUtil.h"

using std::string;

//==== Constructor ====//
Parm::Parm()
{
    m_Container = NULL;
    m_Name = string( "Default_Name" );
    m_GroupName = string( "Default_Group_Name" );
    m_GroupDisplaySuffix = -1;
    m_Descript = string( "Default Description" );
    m_Type = PARM_DOUBLE_TYPE;
    m_Val = 0.0;
    m_LastVal = 0.0;
    m_UpperLimit =  1.0e16;
    m_LowerLimit = -1.0e16;
    m_ActiveFlag = true;
    m_LinkableFlag = true;
    m_LinkedFlag = false;
    m_LinkUpdateFlag = false;
}

//==== Destructor ====//
Parm::~Parm()
{
    ParmMgr.RemoveParm( this );                 // Remove From Global Manager
    if ( m_Container )
    {
        m_Container->RemoveParm( m_ID );
    }

}

//==== Init ====//
void Parm::Init( const string& name, const string& group_name, ParmContainer* con, double val,
                 double lower, double upper, bool linkable )
{
    m_ID = GenerateID();
    Set( val );
    SetLowerUpperLimits( lower, upper );
    m_Name = name;
    m_GroupName = group_name;
    m_Container = con;
    m_LinkableFlag = linkable;

    if ( m_Container )
    {
        m_LinkContainerID = m_Container->GetID();
    }

    ParmMgr.AddParm( this );        // Add To Global Manager
    if ( m_Container )
    {
        m_Container->AddParm( m_ID );
    }
}

void Parm::ReSetLinkContainerID()
{
    if ( m_Container )
    {
        m_LinkContainerID = m_Container->GetID();
    }
}

//==== ChangeID ===//
// Changing the ID of a parameter requires finding references to the old ID and changing
// them to the new ID.  This routine does so in an incomplete manner.  It updates the ID held
// by the m_Container and ParmMgr only.  So long as a Geom was recently constructed, this is
// likely sufficient.
void Parm::ChangeID( const string& newID )
{
    if ( m_Container )
    {
        m_Container->RemoveParm( m_ID );
    }

    ParmMgr.RemoveParm( this );

    m_ID = newID;

    ParmMgr.AddParm( this );

    if ( m_Container )
    {
        m_Container->AddParm( newID );
    }
}

//==== Get Display Group Name ====//
string Parm::GetDisplayGroupName()
{
    string displayName = m_GroupName;

    if ( m_GroupDisplaySuffix >= 0 )
    {
        char str[256];
        sprintf( str, "_%d", m_GroupDisplaySuffix );
        displayName.append( str );
    }

    return displayName;
}

//==== Set Val And Check Limts ====//
bool Parm::SetValCheckLimits( double val )
{
    //==== Check If Val Has Changed ====//
    if ( fabs( val - m_Val ) < DBL_EPSILON )
    {
        return false;
    }

    //==== Store Last Val For Undo ====//
    m_LastVal = m_Val;

    //==== Keep Val Within Limits ====//
    if ( val < m_LowerLimit )
    {
        m_Val = m_LowerLimit;
    }
    else if ( val > m_UpperLimit )
    {
        m_Val = m_UpperLimit;
    }
    else
    {
        m_Val = val;
    }

    return true;
}

//==== Set Val ====//
double Parm::Set( double val )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    if ( m_Container )
    {
        m_Container->ParmChanged( this, SET );
    }

    LinkMgr.ParmChanged( m_ID, true );

    return m_Val;
}

//==== Set Val And Notify Container and Parm Link Mgr ====//
double Parm::SetFromLink( double val )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    LinkMgr.ParmChanged( m_ID, false );

    if ( m_Container )
    {
        m_Container->ParmChanged( this, SET_FROM_LINK );
    }

    return m_Val;
}

//==== Set Val And Notify Container, UnDo and Parm Link Mgr ====//
double Parm::SetFromDevice( double val )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    ParmMgr.AddToUndoStack( this );

    if ( m_Container )
    {
        m_Container->ParmChanged( this,  SET_FROM_DEVICE );
    }

    LinkMgr.ParmChanged( m_ID, true );

    return m_Val;
}

//==== Set Lower Limit ====//
void Parm::SetLowerLimit( double limit )
{
    m_LowerLimit = limit;
    if ( m_LowerLimit > m_UpperLimit )
    {
        m_LowerLimit = m_UpperLimit;
    }
    if ( m_Val < m_LowerLimit )
    {
        Set( m_LowerLimit );
    }
}
//==== Set Upper Limit ====//
void Parm::SetUpperLimit( double limit )
{
    m_UpperLimit = limit;
    if ( m_UpperLimit < m_LowerLimit  )
    {
        m_UpperLimit = m_LowerLimit;
    }
    if ( m_Val > m_UpperLimit )
    {
        Set( m_UpperLimit );
    }
}
//==== Set Both Lower and Upper Limit ====//
void Parm::SetLowerUpperLimits( double lower_limit, double upper_limit )
{
    SetLowerLimit( lower_limit );
    SetUpperLimit( upper_limit );
}

//==== Generate Unique ID ====//
string Parm::GenerateID()
{
    return ParmMgr.GenerateID( 11 );
}

string Parm::GetContainerID()
{
    if ( m_Container )
    {
        return m_Container->GetID();
    }
    return string();
}


//==== Encode Data To XML Data Structure ====//
void Parm::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr dnode = xmlNewChild( node, NULL, ( const xmlChar * )m_Name.c_str(), NULL );

    XmlUtil::SetDoubleProp( dnode, "Value", m_Val );
    XmlUtil::SetStringProp( dnode, "ID", m_ID );

//jrg think about adding more data - limits???
}

//==== Decode Data To XML Data Structure ====//
void Parm::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr n;

    double val = m_Val;
    string id = m_ID;

    n = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( n )
    {
        val = XmlUtil::FindDoubleProp( n, "Value", m_Val );

        string oldID = XmlUtil::FindStringProp( n, "ID", m_ID );
        string newID = ParmMgr.RemapID( oldID, m_ID );

        if( newID.compare( m_ID ) != 0 )        // they differ
        {
            ChangeID( newID );
        }
    }

    Set( val );

//jrg think about adding more data - limits???
}

ParmContainer* Parm::GetLinkContainer()
{
    string id = GetLinkContainerID();
    ParmContainer* pc = ParmMgr.FindParmContainer( id );

    if ( !pc )
    {
        pc = m_Container;
    }

    return pc;
}

//=========================================================================//
//=======================        IntParm       ============================//
//=========================================================================//

//==== Constructor ====//
IntParm::IntParm() : Parm()
{
    m_Type = PARM_INT_TYPE;
    m_UpperLimit = INT_MAX;
    m_LowerLimit = INT_MIN;

}
//==== Set Int Value ====//
int IntParm::Set( int val )
{
    Parm::Set( ( double )val );
    return ( int )( m_Val + 0.5 );
}


//=========================================================================//
//=======================        BoolParm       ============================//
//=========================================================================//

//==== Constructor ====//
BoolParm::BoolParm() : Parm()
{
    m_Type = PARM_BOOL_TYPE;
    m_UpperLimit = 1;
    m_LowerLimit = 0;

}

//==== Set Bool Value ====//
bool BoolParm::Set( bool val )
{
    if ( val )
    {
        m_Val = 1.0;
    }
    else
    {
        m_Val = 0.0;
    }

    return ( !!( int )m_Val );
}


//=========================================================================//
//==================        FractionParm       ============================//
//=========================================================================//

//==== Constructor ====//
FractionParm::FractionParm() : Parm()
{
    m_Type = PARM_FRACTION_TYPE;
    m_RefVal = 0.0;
    m_ResultVal = 0.0;

}

//==== Set Reference Val ====//
void FractionParm::SetRefVal( double val )
{
    if ( fabs( val - m_RefVal ) < DBL_EPSILON )
    {
        return;
    }

    m_RefVal = val;
    m_ResultVal = m_RefVal * m_Val;
}

//==== Set Val ====//
double FractionParm::Set( double val )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    UpdateResultVal();

    if ( m_Container )
    {
        m_Container->ParmChanged( this, SET );
    }

    LinkMgr.ParmChanged( m_ID, true );

    return m_Val;
}
//==== Set Val And Notify Container and Parm Link Mgr ====//
double FractionParm::SetFromLink( double val )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    UpdateResultVal();

    LinkMgr.ParmChanged( m_ID, false );

    if ( m_Container )
    {
        m_Container->ParmChanged( this, SET_FROM_LINK );
    }


    return m_Val;
}

//==== Set Val And Notify Container, UnDo and Parm Link Mgr ====//
double FractionParm::SetFromDevice( double val )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    ParmMgr.AddToUndoStack( this );

    UpdateResultVal();

    if ( m_Container )
    {
        m_Container->ParmChanged( this,  SET_FROM_DEVICE );
    }

    LinkMgr.ParmChanged( m_ID, true );

    return m_Val;
}

//==== Update Result Val ====//
void FractionParm::UpdateResultVal()
{
    m_ResultVal = m_RefVal * m_Val;
}

//==== Set Result ====//
double FractionParm::SetResult( double res_val )
{
    if ( fabs( m_RefVal ) > DBL_EPSILON )
    {
        Set( res_val / m_RefVal );
    }

    return m_ResultVal;
}

//==== Set Result From Link ====//
double FractionParm::SetResultFromLink( double res_val )
{
    if ( fabs( m_RefVal ) > DBL_EPSILON )
    {
        SetFromLink( res_val / m_RefVal );
    }

    return m_ResultVal;
}

//==== Set Result From Device ====//
double FractionParm::SetResultFromDevice( double res_val )
{
    if ( fabs( m_RefVal ) > DBL_EPSILON )
    {
        SetFromDevice( res_val / m_RefVal );
    }

    return m_ResultVal;
}

double FractionParm::GetResult()
{
    return m_ResultVal;
}

////=========================================================================//
////=======================        StringParm       ============================//
////=========================================================================//
//
////==== Constructor ====//
//StringParm::StringParm() : Parm()
//{
//  m_Type = PARM_STRING_TYPE;
//}
//
////==== Set String Value ====//
//string StringParm::Set( string & val )
//{
//  m_String = val;
//  return ( m_String );
//}

//=========================================================================//
//=======================      DriverGroup     ============================//
//=========================================================================//

DriverGroup::DriverGroup( int Nvar, int Nchoice )
{
    m_Nvar = Nvar;
    m_Nchoice = Nchoice;

    m_CurrChoices.resize( m_Nchoice );
}

DriverGroup::~DriverGroup()
{
}

void DriverGroup::SetChoice( int choice, int grpid )
{
    m_CurrChoices[choice] = grpid;
}
