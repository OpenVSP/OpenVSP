//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(APIERRORMGR__INCLUDED_)
#define APIERRORMGR__INCLUDED_

#include "APIDefines.h"
#include "MessageMgr.h"

#include <string>
#include <stack>
#include <vector>

using std::string;
using std::stack;
using std::vector;

class Vehicle;

namespace vsp
{


//======================== Error Object ================================//
/*!
    \ingroup APIError
*/
/*!
    ErrorObj is defined by an error code enum and associated error string. 
*/
class ErrorObj
{
public:
/*!
    \internal
    An ErrorObj is built by the error manager, never by a caller, so the constructors are described
    with the class above rather than documented one by one.
*/
    ErrorObj();
/*!
    \internal
    Build an ErrorObj from a code and a description.
*/
    ErrorObj( ERROR_CODE err_code, const string & err_str );
/*!
    \internal
    Copy constructor.
*/
    ErrorObj( const ErrorObj& from );
    ~ErrorObj()         {}

/*!
    \ingroup APIError
*/
/*!
    Get the ERROR_CODE enum of the last raised error
    \forcpponly
    \code{.cpp}
    //==== Silence the errors so the bogus call below does not print ====//
    SilenceErrors();

    //==== Bogus call to raise an error ====//
    SetParmVal( "BogusParmID", 23.0 );

    PrintOnErrors();

    ErrorObj err = PopLastError();

    if ( err.GetErrorCode() != VSP_CANT_FIND_PARM )      { Print( "ERROR: GetErrorCode" ); __failure++; }

    //==== Leave the queue as it was found ====//
    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    #==== Silence the errors so the bogus call below does not print ====#
    err_mgr.SilenceErrors()

    #==== Bogus call to raise an error ====#
    SetParmVal( "BogusParmID", 23.0 )

    err_mgr.PrintOnErrors()

    err = err_mgr.PopLastError()

    assert err.GetErrorCode() == VSP_CANT_FIND_PARM, "GetErrorCode did not report the missing Parm"

    #==== Leave the queue as it was found ====#
    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \sa ERROR_CODE
    \return ERROR_CODE error code enum
*/

    ERROR_CODE GetErrorCode()

    {
        return m_ErrorCode;
    }
/*!
    \ingroup APIError
*/
/*!
    Get the error string of the last raised error
    \forcpponly
    \code{.cpp}
    SilenceErrors();

    SetParmVal( "BogusParmID", 23.0 );

    PrintOnErrors();

    ErrorObj err = PopLastError();

    if ( err.GetErrorString().length() == 0 )            { Print( "ERROR: GetErrorString" ); __failure++; }

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    err_mgr.SilenceErrors()

    SetParmVal( "BogusParmID", 23.0 )

    err_mgr.PrintOnErrors()

    err = err_mgr.PopLastError()

    assert len( err.GetErrorString() ) > 0, "GetErrorString returned nothing to report"

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \return Error string
*/

    string GetErrorString()

    {
        return m_ErrorString;
    }

    ERROR_CODE m_ErrorCode;
    string m_ErrorString;

/*!
    \internal
    Reset this object to the "no error" state.  Bookkeeping for the error manager.
*/
    void NoError()
    {
        m_ErrorCode = VSP_OK;
        m_ErrorString = "No Error";
    }
};


//======================== Error Mgr ================================//
/*!
    \ingroup APIError
*/
/*!
    ErrorMgrSingleton is the queue that API errors are reported through.  An API function that fails
    pushes an ErrorObj onto it and returns a default value rather than throwing, so a script that
    does not check anything keeps running.  Reach it with ErrorMgrSingleton.getInstance(), then use
    GetNumTotalErrors and PopLastError to find out what went wrong.
*/
class ErrorMgrSingleton : public MessageBase
{
public:

/*!
    \ingroup APIError
*/
/*!
    Check if there was an error on the last call to the API
    \forcpponly
    \code{.cpp}
    SilenceErrors();

    //==== Bogus call to raise an error ====//
    SetParmVal( "BogusParmID", 23.0 );

    if ( !GetErrorLastCallFlag() )                       { Print( "ERROR: GetErrorLastCallFlag" ); __failure++; }

    PrintOnErrors();

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    err_mgr.SilenceErrors()

    #==== Bogus call to raise an error ====#
    SetParmVal( "BogusParmID", 23.0 )

    assert err_mgr.GetErrorLastCallFlag(), "GetErrorLastCallFlag missed the error"

    err_mgr.PrintOnErrors()

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \return False if no error, true otherwise
*/

    bool GetErrorLastCallFlag();                // Did the last call have an error?

/*!
    \ingroup APIError
*/
/*!
    Count the total number of errors on the stack
    \forcpponly
    \code{.cpp}
    SilenceErrors();

    //==== Two bogus calls, so the count is something to check ====//
    SetParmVal( "BogusParmID", 23.0 );
    SetParmVal( "AnotherBogusParmID", 23.0 );

    if ( GetNumTotalErrors() != 2 )                      { Print( "ERROR: GetNumTotalErrors" ); __failure++; }

    PrintOnErrors();

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    err_mgr.SilenceErrors()

    #==== Two bogus calls, so the count is something to check ====#
    SetParmVal( "BogusParmID", 23.0 )
    SetParmVal( "AnotherBogusParmID", 23.0 )

    assert err_mgr.GetNumTotalErrors() == 2, "GetNumTotalErrors did not count both errors"

    err_mgr.PrintOnErrors()

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \return Number of errors
*/

    int  GetNumTotalErrors();                   // Total number of errors on stack

/*!
    \ingroup APIError
*/
/*!
    Pop (remove) and return the most recent error from the stack. Note, errors are printed on occurrence by default. 
    \forcpponly
    \code{.cpp}
    SilenceErrors();

    SetParmVal( "BogusParmID", 23.0 );

    int n0 = GetNumTotalErrors();

    ErrorObj err = PopLastError();

    //==== Pop takes the error off the queue ====//
    if ( GetNumTotalErrors() != n0 - 1 )                 { Print( "ERROR: PopLastError" ); __failure++; }

    if ( err.GetErrorCode() != VSP_CANT_FIND_PARM )      { Print( "ERROR: PopLastError" ); __failure++; }

    PrintOnErrors();

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    err_mgr.SilenceErrors()

    SetParmVal( "BogusParmID", 23.0 )

    n0 = err_mgr.GetNumTotalErrors()

    err = err_mgr.PopLastError()

    #==== Pop takes the error off the queue ====#
    assert err_mgr.GetNumTotalErrors() == n0 - 1, "PopLastError left the error on the queue"

    assert err.GetErrorCode() == VSP_CANT_FIND_PARM, "PopLastError returned the wrong error"

    err_mgr.PrintOnErrors()

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \return Error object
*/

    ErrorObj PopLastError();                    // Pop last error off stack

/*!
    \ingroup APIError
*/
/*!
    Return the most recent error from the stack (does NOT pop error off the stack)
    \forcpponly
    \code{.cpp}
    SilenceErrors();

    SetParmVal( "BogusParmID", 23.0 );

    int n0 = GetNumTotalErrors();

    ErrorObj err = GetLastError();

    //==== Unlike PopLastError, the error stays on the queue ====//
    if ( GetNumTotalErrors() != n0 )                     { Print( "ERROR: GetLastError" ); __failure++; }

    if ( err.GetErrorCode() != VSP_CANT_FIND_PARM )      { Print( "ERROR: GetLastError" ); __failure++; }

    PrintOnErrors();

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    err_mgr.SilenceErrors()

    SetParmVal( "BogusParmID", 23.0 )

    n0 = err_mgr.GetNumTotalErrors()

    err = err_mgr.GetLastError()

    #==== Unlike PopLastError, the error stays on the queue ====#
    assert err_mgr.GetNumTotalErrors() == n0, "GetLastError removed the error"

    assert err.GetErrorCode() == VSP_CANT_FIND_PARM, "GetLastError returned the wrong error"

    err_mgr.PrintOnErrors()

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \sa SilenceErrors, PrintOnErrors;
    \return Error object
*/

    ErrorObj GetLastError();                    // Get last error but leave on stack

/*!
    \internal
    Pop the most recent error and write it to an open C stream.  Takes a FILE*, which the bindings
    cannot supply, so it is a C++ convenience rather than part of the scripting API.
*/
    bool PopErrorAndPrint( FILE* stream );

/*!
    \ingroup APIError
*/
/*!
    Prevent errors from printing to stdout as they occur.
    \forcpponly
    \code{.cpp}
    //==== Errors are printed as they happen unless silenced ====//
    SilenceErrors();

    SetParmVal( "BogusParmID", 23.0 );

    //==== The error is still recorded, it is only the printing that stops ====//
    if ( GetNumTotalErrors() != 1 )                      { Print( "ERROR: SilenceErrors" ); __failure++; }

    PrintOnErrors();

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    #==== Errors are printed as they happen unless silenced ====#
    err_mgr.SilenceErrors()

    SetParmVal( "BogusParmID", 23.0 )

    #==== The error is still recorded, it is only the printing that stops ====#
    assert err_mgr.GetNumTotalErrors() == 1, "SilenceErrors also stopped the error being recorded"

    err_mgr.PrintOnErrors()

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \sa PrintOnErrors
*/

    void SilenceErrors()    { m_PrintErrors = false; };

/*!
    \ingroup APIError
*/
/*!
    Cause errors to be printed to stdout as they occur.
    \forcpponly
    \code{.cpp}
    SilenceErrors();

    SetParmVal( "BogusParmID", 23.0 );

    //==== Turn printing back on ====//
    PrintOnErrors();

    if ( GetNumTotalErrors() != 1 )                      { Print( "ERROR: PrintOnErrors" ); __failure++; }

    while ( GetNumTotalErrors() > 0 ) { PopLastError(); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    err_mgr.SilenceErrors()

    SetParmVal( "BogusParmID", 23.0 )

    #==== Turn printing back on ====#
    err_mgr.PrintOnErrors()

    assert err_mgr.GetNumTotalErrors() == 1, "PrintOnErrors lost the recorded error"

    while err_mgr.GetNumTotalErrors() > 0:
        err_mgr.PopLastError()
    \endcode
    \endPythonOnly
    \sa SilenceErrors
*/

    void PrintOnErrors()    { m_PrintErrors = true; };


/*!
    \internal
    Push an error onto the queue.  Called by the API functions themselves when something goes wrong;
    a script reads the queue rather than adding to it.
*/
    void AddError( ERROR_CODE code, const string & desc );
/*!
    \internal
    Clear the last-call error flag.
*/
    void NoError();

/*!
    \internal
    MessageMgr delivers errors raised elsewhere in OpenVSP through here.  Part of the MessageBase
    contract, not something an API caller invokes.
*/
    virtual void MessageCallback( const MessageBase* from, const MessageData& data );

/*!
    \ingroup APIError
*/
/*!
    Get the error manager.  There is one queue for the whole session, and this is how a script
    reaches it -- every other call in this group is made on the object returned here.
    \forcpponly
    \code{.cpp}
    //==== AngelScript reaches the same queue through global functions, so there is no instance to
    //==== fetch.  Call GetNumTotalErrors, PopLastError and the rest directly.
    if ( GetNumTotalErrors() < 0 )                       { Print( "ERROR: getInstance" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    err_mgr = ErrorMgrSingleton.getInstance()

    assert err_mgr.GetNumTotalErrors() >= 0, "getInstance did not return a usable error manager"

    \endcode
    \endPythonOnly
    \sa GetNumTotalErrors, PopLastError, GetErrorLastCallFlag
    \return ErrorMgrSingleton The one error manager for this session
*/

    static ErrorMgrSingleton& getInstance()
    {
        static ErrorMgrSingleton instance;
        return instance;
    }

private:

    bool m_PrintErrors;
    bool m_ErrorLastCallFlag;
    stack< ErrorObj > m_ErrorStack;

/*!
    \internal
    Private -- there is one error manager per session and getInstance is how to reach it.
*/
    ErrorMgrSingleton();
    ~ErrorMgrSingleton();
    ErrorMgrSingleton( ErrorMgrSingleton const& copy ) = delete;          // Not Implemented
    ErrorMgrSingleton& operator=( ErrorMgrSingleton const& copy ) = delete; // Not Implemented
};

#define ErrorMgr ErrorMgrSingleton::getInstance()

}

#endif // !defined(APIERRORMGR__INCLUDED_)

