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
#include <cfloat>

#include "Combination.h"
#include "StlHelper.h"
#include "VspUtil.h"

using std::string;

//==== Constructor ====//
Parm::Parm()
{
    m_Container = NULL;
    m_Name = string( "Default_Name" );
    m_GroupName = string( "Default_Group_Name" );
    m_GroupDisplaySuffix = -1;
    m_Descript = string( "Default Description" );
    m_Type = vsp::PARM_DOUBLE_TYPE;
    m_Val = 0.0;
    m_LastVal = 0.0;
    m_UpperLimit =  1.0e16;
    m_LowerLimit = -1.0e16;
    m_ActiveFlag = true;
    m_LinkUpdateFlag = false;
    m_ChangeCnt = 0;
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
                 double lower, double upper )
{
    m_ID = GenerateID();
    Set( val );
    SetLowerUpperLimits( lower, upper );
    m_Name = name;
    m_GroupName = group_name;
    m_Container = con;

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

//==== Set Val And Check Limits ====//
bool Parm::SetValCheckLimits( double val )
{
    //==== Check If Val Has Changed ====//
    if ( std::abs( val - m_Val ) < DBL_EPSILON )
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

    m_ChangeCnt = ParmMgr.GetChangeCnt();

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
double Parm::SetFromDevice( double val, bool drag_flag )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    ParmMgr.AddToUndoStack( this, drag_flag );

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
    return GenerateRandomID( 11 );
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
void Parm::EncodeXml( xmlNodePtr & node, bool detailed )
{
    if ( !detailed )
    {
        xmlNodePtr dnode = xmlNewChild( node, NULL, ( const xmlChar * )m_Name.c_str(), NULL );
        XmlUtil::SetDoubleProp( dnode, "Value", m_Val );
        XmlUtil::SetStringProp( dnode, "ID", m_ID );
    }
    else
    {
        xmlNodePtr dnode = xmlNewChild( node, NULL, BAD_CAST "UserParm", NULL );
        XmlUtil::SetDoubleProp( dnode, "Value", m_Val );
        XmlUtil::SetStringProp( dnode, "ID", m_ID );
        XmlUtil::SetStringProp( dnode, "Name", m_Name );
        XmlUtil::SetStringProp( dnode, "GroupName", m_GroupName );
        XmlUtil::SetIntProp( dnode, "GroupDisplaySuffix", m_GroupDisplaySuffix );
        XmlUtil::SetStringProp( dnode, "Descript", m_Descript );
        XmlUtil::SetIntProp( dnode, "Type", m_Type );
        XmlUtil::SetDoubleProp( dnode, "UpperLimit", m_UpperLimit );
        XmlUtil::SetDoubleProp( dnode, "LowerLimit", m_LowerLimit );
    }
}

//==== Decode Data To XML Data Structure ====//
void Parm::DecodeXml( xmlNodePtr & node, bool detailed )
{
    xmlNodePtr n;

    double val = m_Val;

    if ( detailed )
    {
         n = node;
    }
    else
    {
         n = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    }

    if ( n )
    {
        val = XmlUtil::FindDoubleProp( n, "Value", m_Val );

        string oldID = XmlUtil::FindStringProp( n, "ID", m_ID );
        string newID = ParmMgr.RemapID( oldID, m_ID );

        if( newID.compare( m_ID ) != 0 )        // they differ
        {
            ChangeID( newID );
        }

        if ( detailed )
        {
            m_Name = XmlUtil::FindStringProp( n, "Name", m_Name );
            m_GroupName = XmlUtil::FindStringProp( n, "GroupName", m_GroupName );
            m_GroupDisplaySuffix = XmlUtil::FindIntProp( n, "GroupDisplaySuffix", m_GroupDisplaySuffix );
            m_Descript = XmlUtil::FindStringProp( n, "Descript", m_Descript );
            m_Type = XmlUtil::FindIntProp( n, "Type", m_Type );
            m_UpperLimit = XmlUtil::FindDoubleProp( n, "UpperLimit", m_UpperLimit );
            m_LowerLimit = XmlUtil::FindDoubleProp( n, "LowerLimit", m_LowerLimit );
        }
    }

    Set( val );
}

ParmContainer* Parm::GetLinkContainer() const
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
    m_Type = vsp::PARM_INT_TYPE;
    m_UpperLimit = INT_MAX;
    m_LowerLimit = INT_MIN;

}
//==== Set Int Value ====//
int IntParm::Set( int val )
{
    Parm::Set( ( double )val );
    return ( int )( m_Val + 0.5 );
}

//==== Set Val And Check Limits ====//
bool IntParm::SetValCheckLimits( double val )
{
    //==== Check If Val Has Changed ====//
    if ( std::abs( val - m_Val ) < 0.5 )
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
        m_Val = ( int )( val + 0.5 );
    }

    return true;
}


//=========================================================================//
//=======================      LimIntParm      ============================//
//=========================================================================//

//==== Constructor ====//
LimIntParm::LimIntParm() : IntParm()
{
    m_Type = vsp::PARM_LIMITED_INT_TYPE;

    m_Shift = 0;
    m_Mult = 1;
}

//==== Set Val And Check Limits ====//
bool LimIntParm::SetValCheckLimits( double val )
{
    if ( !IntParm::SetValCheckLimits( val ) )
    {
        return false;
    }

    // Check that value obeys the limits.
    m_Val = m_Mult * ceil( ( m_Val - m_Shift ) / m_Mult ) + m_Shift;

    return true;
}

void LimIntParm::SetMultShift( int mult, int shift )
{
    m_Mult = mult;
    m_Shift = shift;
}


//=========================================================================//
//=======================      PowIntParm      ============================//
//=========================================================================//

//==== Constructor ====//
PowIntParm::PowIntParm() : IntParm()
{
    m_Type = vsp::PARM_POWER_INT_TYPE;

    m_Shift = 0;
    m_Base = 1;
}

//==== Set Val And Check Limits ====//
bool PowIntParm::SetValCheckLimits( double val )
{
    if ( !IntParm::SetValCheckLimits( val ) )
    {
        return false;
    }
    
    // Check that value obeys the limits.
    if ( val < 0 )
    {
        m_Val = 0;
    }
    else
    {
        m_Val = pow( m_Base, round( log( val - m_Shift ) / log( m_Base ) ) ) + m_Shift;
    }

    return true;
}

void PowIntParm::SetPowShift( int base, int shift )
{
    m_Base = base;
    m_Shift = shift;
}


//=========================================================================//
//=======================       NotEqParm      ============================//
//=========================================================================//

//==== Constructor ====//
NotEqParm::NotEqParm() : Parm()
{
    m_Type = vsp::PARM_NOTEQ_TYPE;
    m_Tol = 1e-4;
    m_CheckFlag = true;
}

void NotEqParm::ChangeID( const string& newID )
{
    Parm::ChangeID( newID );

    NotEqParm* oparm = dynamic_cast< NotEqParm* > (ParmMgr.FindParm( m_OtherParmID ) );

    if( oparm )
    {
        oparm->SetOtherParmID( newID, m_Tol );
    }
}

bool NotEqParm::SetValCheckLimits( double val )
{
    double tmp = m_Val;
    double tmp2 = m_LastVal;
    if( Parm::SetValCheckLimits( val ) && m_CheckFlag ) // Passed in value different.
    {
        Parm* oparm = ParmMgr.FindParm( m_OtherParmID );
        if( oparm )
        {
            if( std::abs( oparm->Get() - val ) < m_Tol )
            {
                m_Val = tmp;
                m_LastVal = tmp2;

                MessageData errMsgData;
                errMsgData.m_String = "Error";
                errMsgData.m_IntVec.push_back( vsp::VSP_CANT_SET_NOT_EQ_PARM );
                char buf[255];
                sprintf( buf, "Error:  Attempt to set %s equal to %s.", m_Name.c_str(), oparm->GetName().c_str() );
                errMsgData.m_StringVec.emplace_back( string( buf ) );

                MessageMgr::getInstance().SendAll( errMsgData );

                return false;
            }
        }
    }
    else // Passed in value the same.
    {
        return false;
    }
    return true;
}

//==== Decode Data To XML Data Structure ====//
void NotEqParm::DecodeXml( xmlNodePtr & node, bool detailed )
{
    m_CheckFlag = false;
    Parm::DecodeXml( node, detailed );
    m_CheckFlag = true;
}

//=========================================================================//
//=======================        BoolParm       ============================//
//=========================================================================//

//==== Constructor ====//
BoolParm::BoolParm() : Parm()
{
    m_Type = vsp::PARM_BOOL_TYPE;
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
    m_Type = vsp::PARM_FRACTION_TYPE;
    m_RefVal = 0.0;
    m_ResultVal = 0.0;
    m_DisplayResultFlag = false;
}

//==== Set Reference Val ====//
void FractionParm::SetRefVal( double val )
{
    if ( std::abs( val - m_RefVal ) < DBL_EPSILON )
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
double FractionParm::SetFromDevice( double val, bool drag_flag )
{
    if ( !SetValCheckLimits( val ) )
    {
        return m_Val;
    }

    ParmMgr.AddToUndoStack( this, drag_flag );

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
    if ( std::abs( m_RefVal ) > DBL_EPSILON )
    {
        Set( res_val / m_RefVal );
    }

    return m_ResultVal;
}

//==== Set Result From Link ====//
double FractionParm::SetResultFromLink( double res_val )
{
    if ( std::abs( m_RefVal ) > DBL_EPSILON )
    {
        SetFromLink( res_val / m_RefVal );
    }

    return m_ResultVal;
}

//==== Set Result From Device ====//
double FractionParm::SetResultFromDevice( double res_val )
{
    if ( std::abs( m_RefVal ) > DBL_EPSILON )
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
    m_Name = "DriverGroup";

    m_CurrChoices.resize( m_Nchoice );
}

DriverGroup::~DriverGroup()
{
}

void DriverGroup::SetChoice( int choice, int grpid )
{
    if ( choice < m_CurrChoices.size() )
    {
        m_CurrChoices[choice] = grpid;
    }
}

void DriverGroup::SetChoices( const vector< int > &choices )
{
    for ( int i = 0; i < choices.size() && i < m_CurrChoices.size(); i++ )
    {
        m_CurrChoices[i] = choices[i];
    }
}

bool DriverGroup::IsDriver( int dvar )
{
    return vector_contains_val( m_CurrChoices, dvar );
}

//==== Encode Data To XML Data Structure ====//
void DriverGroup::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr dnode = xmlNewChild( node, NULL, ( const xmlChar *)m_Name.c_str(), NULL );

    XmlUtil::AddIntNode( dnode, "NumVar", m_Nvar );
    XmlUtil::AddIntNode( dnode, "NumChoices", m_Nchoice );
    XmlUtil::AddVectorIntNode( dnode, "ChoiceVec", m_CurrChoices );
}

//==== Decode Data To XML Data Structure ====//
void DriverGroup::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr n = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( n )
    {
        vector< int > tmp = XmlUtil::ExtractVectorIntNode( n, "ChoiceVec" );
        SetChoices( tmp );
    }
}

void DriverGroup::Test( vector< string > parmIDs, double tol )
{
    UpdateGroup( parmIDs );

    vector< double > ovals( m_Nvar );
    vector< int > vars( m_Nvar );
    for( int i = 0; i < m_Nvar; i++ )
    {
        Parm* p = ParmMgr.FindParm( parmIDs[ i ] );
        ovals[i] = p->Get();
        vars[i] = i;
    }
    vector< int > ochoice = GetChoices();
    bool failever = false;
    do
    {
        vector< int > checkchoices( m_Nchoice );
        for( int k = 0; k < m_Nchoice; k++ )
        {
            checkchoices[k] = vars[k];
        }

        if( ValidDrivers( checkchoices ) )
        {
            SetChoices( checkchoices );
            UpdateGroup( parmIDs );

            // Check for corrupted values.
            bool failonce = false;
            for( int k = 0; k < m_Nvar; k++ )
            {
                Parm* p = ParmMgr.FindParm( parmIDs[ k ] );
                if( std::abs( ovals[k] - p->Get() ) > tol )
                {
                    failonce = true;
                    failever = true;
                }
            }

            if( failonce )
            {
                printf( "Error, driver group mis-calculated parameters.\n" );
                printf( " Drivers: " );
                for( int k = 0; k < m_Nchoice; k++ )
                {
                    Parm* p = ParmMgr.FindParm( parmIDs[ checkchoices[k] ] );

                    printf( "%d %s ", checkchoices[k], p->GetName().c_str() );
                }
                printf( "\n" );

                for( int k = 0; k < m_Nvar; k++ )
                {
                    Parm* p = ParmMgr.FindParm( parmIDs[ k ] );
                    printf(" %s orig: %f this: %f\n", p->GetName().c_str(), ovals[k], p->Get() );
                }
                printf( "\n" );
            }

            // Restore values in case they were corrupted.
            for( int k = 0; k < m_Nvar; k++ )
            {
                Parm* p = ParmMgr.FindParm( parmIDs[ k ] );
                p->Set( ovals[k] );
            }
        }

    }while ( next_combination ( vars.begin(), vars.begin () + m_Nchoice, vars.end () ) );

    if( !failever )
    {
        printf( " Driver group test pass\n" );
    }

    SetChoices( ochoice );
    UpdateGroup( parmIDs );
}
