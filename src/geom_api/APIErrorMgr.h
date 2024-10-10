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

#ifdef SWIG
%feature("autodoc", 1);
%feature("doxygen:ignore:forcpponly", range="end");
%feature("doxygen:ignore:beginPythonOnly", range="end:endPythonOnly", contents="parse");
#endif

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
    ErrorObj is defined by an error code enum and associated error string. 
*/
class ErrorObj
{
public:
    ErrorObj();
    ErrorObj( ERROR_CODE err_code, const string & err_str );
    ErrorObj( const ErrorObj& from );
    ~ErrorObj()         {}

/*!
    Get the ERROR_CODE enum of the last raised error
    \forcpponly
    \code{.cpp}
    ErrorObj err = PopLastError();

    if ( err.GetErrorCode() != VSP_CANT_FIND_PARM )            { Print( "---> Error: API PopLast" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        ErrorObj err = PopLastError()

    if  err.GetErrorCode() != VSP_CANT_FIND_PARM : Print( "---> Error: API PopLast" ); }

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
    Get the error string of the last raised error
    \forcpponly
    \code{.cpp}
    //==== Check For API Errors ====//
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
        Print( err.GetErrorString() );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Check For API Errors ====//
    while  GetNumTotalErrors() > 0 :
        ErrorObj err = PopLastError()
        Print( err.GetErrorString() )

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

    void NoError()
    {
        m_ErrorCode = VSP_OK;
        m_ErrorString = "No Error";
    }
};


//======================== Error Mgr ================================//
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
    //==== Force API to silence error messages ====//
    SilenceErrors();

    //==== Bogus Call To Create API Error ====//
    Print( string( "---> Test Error Handling" ) );

    SetParmVal( "BogusParmID", 23.0 );

    if ( !GetErrorLastCallFlag() )                        { Print( "---> Error: API GetErrorLastCallFlag " ); }

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Force API to silence error messages ====//
    SilenceErrors()

    #==== Bogus Call To Create API Error ====//
    Print( string( "---> Test Error Handling" ) )

    SetParmVal( "BogusParmID", 23.0 )

    if  not GetErrorLastCallFlag() : Print( "---> Error: API GetErrorLastCallFlag " ); }

    #==== Tell API to print error messages ====//
    PrintOnErrors()

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
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Check For API Errors ====//
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
        Print( err.GetErrorString() );
    }

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Force API to silence error messages ====//
    SilenceErrors()

    Print( "Creating an API error" )
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 )

    #==== Check For API Errors ====//
    while  GetNumTotalErrors() > 0 :
        ErrorObj err = PopLastError()
        Print( err.GetErrorString() )

    #==== Tell API to print error messages ====//
    PrintOnErrors()

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
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Check For API Errors ====//
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
        Print( err.GetErrorString() );
    }

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Force API to silence error messages ====//
    SilenceErrors()

    Print( "Creating an API error" )
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 )

    #==== Check For API Errors ====//
    while  GetNumTotalErrors() > 0 :
        ErrorObj err = PopLastError()
        Print( err.GetErrorString() )

    #==== Tell API to print error messages ====//
    PrintOnErrors()

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
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Check For API Errors ====//
    ErrorObj err = GetLastError();

    Print( err.GetErrorString() );

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Force API to silence error messages ====//
    SilenceErrors()

    Print( "Creating an API error" )
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 )

    #==== Check For API Errors ====//
    ErrorObj err = GetLastError()

    Print( err.GetErrorString() )

    #==== Tell API to print error messages ====//
    PrintOnErrors()

    \endcode
    \endPythonOnly
    \sa SilenceErrors, PrintOnErrors;
    \return Error object
*/

    ErrorObj GetLastError();                    // Get last error but leave on stack

    bool PopErrorAndPrint( FILE* stream );      // Check for error, pop and print to stream

/*!
	\ingroup APIError
*/
/*!
    Prevent errors from printing to stdout as they occur.
    \forcpponly
    \code{.cpp}
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Force API to silence error messages ====//
    SilenceErrors()

    Print( "Creating an API error" )
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 )

    #==== Tell API to print error messages ====//
    PrintOnErrors()

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
    //==== Force API to silence error messages ====//
    SilenceErrors();

    Print( "Creating an API error" );
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 );

    //==== Tell API to print error messages ====//
    PrintOnErrors();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
        #==== Force API to silence error messages ====//
    SilenceErrors()

    Print( "Creating an API error" )
    SetParmVal( "ABCDEFG", "Test_Name", "Test_Group", 123.4 )

    #==== Tell API to print error messages ====//
    PrintOnErrors()

    \endcode
    \endPythonOnly
    \sa SilenceErrors
*/

    void PrintOnErrors()    { m_PrintErrors = true; };


    void AddError( ERROR_CODE code, const string & desc );
    void NoError();

    virtual void MessageCallback( const MessageBase* from, const MessageData& data );

    static ErrorMgrSingleton& getInstance()
    {
        static ErrorMgrSingleton instance;
        return instance;
    }

private:

    bool m_PrintErrors;
    bool m_ErrorLastCallFlag;
    stack< ErrorObj > m_ErrorStack;

    ErrorMgrSingleton();
    ~ErrorMgrSingleton();
    ErrorMgrSingleton( ErrorMgrSingleton const& copy );          // Not Implemented
    ErrorMgrSingleton& operator=( ErrorMgrSingleton const& copy ); // Not Implemented
};

#define ErrorMgr ErrorMgrSingleton::getInstance()

}

#endif // !defined(APIERRORMGR__INCLUDED_)

