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

ErrorObj::ErrorObj( ErrorCode err_code, const string & err_str )
{
    m_ErrorCode = err_code;
    m_ErrorString = err_str;
}

//===================================================================//
//======================== Error Mgr ================================//
//===================================================================//

ErrorMgrSingleton::ErrorMgrSingleton()
{
    m_ErrorLastCallFlag = false;
}

ErrorMgrSingleton::~ErrorMgrSingleton()
{
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


//==== Add Error To Stack And Set Last Call Flag ====//
void ErrorMgrSingleton::AddError( ErrorCode code, const string & desc )
{
    if ( code == VSP_OK )
    {
        m_ErrorLastCallFlag = false;
        return;
    }

    m_ErrorLastCallFlag = true;
    m_ErrorStack.push( ErrorObj( code, desc ) );
}

//==== Check For Error and Print to Stream if Found ====//
bool ErrorMgrSingleton::PopErrorAndPrint( FILE* stream )
{
    if ( m_ErrorLastCallFlag == false || m_ErrorStack.size() == 0 )
    {
        return false;
    }

    ErrorObj err = m_ErrorStack.top();
    m_ErrorStack.pop();

    fprintf( stream, "Error Code: %d, Desc: %s\n", err.m_ErrorCode, err.m_ErrorString.c_str() );
    return true;

}
