//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#include "APIErrorMgr.h"

using namespace vsp;

//===================================================================//
//======================== Error Object =============================//
//===================================================================//

ErrorObj::ErrorObj()
{
    NoError();
}

ErrorObj::ErrorObj( ERROR_CODE err_code, const string & err_str )
{
    m_ErrorCode = err_code;
    m_ErrorString = err_str;
}

ErrorObj::ErrorObj( const ErrorObj& from )
{
    m_ErrorCode = from.m_ErrorCode;
    m_ErrorString = from.m_ErrorString;
}

//===================================================================//
//======================== Error Mgr ================================//
//===================================================================//

ErrorMgrSingleton::ErrorMgrSingleton()
{
    m_ErrorLastCallFlag = false;
    m_PrintErrors = true;
    MessageBase::Register( string( "ErrorMgr" ) );
}

ErrorMgrSingleton::~ErrorMgrSingleton()
{
    while ( !m_ErrorStack.empty() )
        m_ErrorStack.pop();
    MessageMgr::getInstance().UnRegister( this );
}

//==== No Error For Last Call ====//
void ErrorMgrSingleton::NoError()
{
    m_ErrorLastCallFlag = false;
}

//==== Was There An Error On The Last API Call? ====//
bool ErrorMgrSingleton::GetErrorLastCallFlag()
{
    return m_ErrorLastCallFlag;
}

//==== How Many Total Errors on Stack? ====//
int ErrorMgrSingleton::GetNumTotalErrors()
{
    return m_ErrorStack.size();
}

//==== Return Error and Pop Off Stack =====//
ErrorObj ErrorMgrSingleton::PopLastError()
{
    ErrorObj ret_err;

    if ( m_ErrorStack.size() == 0 )         // Nothing To Undo
    {
        return ret_err;
    }

    ret_err = m_ErrorStack.top();
    m_ErrorStack.pop();

    return ret_err;

}

//==== Return Error =====//
ErrorObj ErrorMgrSingleton::GetLastError()
{
    ErrorObj ret_err;

    if ( m_ErrorStack.size() == 0 )         // Nothing To Undo
    {
        return ret_err;
    }

    ret_err = m_ErrorStack.top();

    return ret_err;
}



//==== Add Error To Stack And Set Last Call Flag ====//
void ErrorMgrSingleton::AddError( ERROR_CODE code, const string & desc )
{
    if ( code == VSP_OK )
    {
        m_ErrorLastCallFlag = false;
        return;
    }

    if ( m_PrintErrors )
    {
        printf( "Error Code: %d, Desc: %s\n", ( ERROR_CODE ) code, desc.c_str() );
    }

    m_ErrorLastCallFlag = true;
    m_ErrorStack.push( ErrorObj( code, desc ) );
}

//==== Check For Error and Print to Stream if Found ====//
bool ErrorMgrSingleton::PopErrorAndPrint( FILE* stream )
{
    if ( ! m_ErrorLastCallFlag || m_ErrorStack.size() == 0 )
    {
        return false;
    }

    ErrorObj err = m_ErrorStack.top();
    m_ErrorStack.pop();

    fprintf( stream, "Error Code: %d, Desc: %s\n", err.m_ErrorCode, err.m_ErrorString.c_str() );
    return true;

}

//==== Message Callbacks ====//
void ErrorMgrSingleton::MessageCallback( const MessageBase* from, const MessageData& data )
{
    if ( data.m_String == string( "Error" ) )
    {
        AddError( ( ERROR_CODE ) data.m_IntVec[0], data.m_StringVec[0] );
    }
}
