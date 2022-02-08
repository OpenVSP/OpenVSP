//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Parm.h: interface for the Parm and Parm Container classes.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_PARM__INCLUDED_)
#define VSP_PARM__INCLUDED_

#include <string>
#include <map>

#include <climits>

#include "XmlUtil.h"
#include "GuiDeviceEnums.h"

using std::string;
using std::map;

class ParmContainer;

//==== Parm ====//
class Parm
{


public:
    Parm();
    virtual ~Parm();

    virtual void Init( const string& name, const string& group, ParmContainer* container,
                       double val, double lower, double upper );

    virtual string GetName() const                       { return m_Name; }
    virtual void SetName( const string & name )          { m_Name = name; }

    virtual string GetGroupName() const                  { return m_GroupName; }
    virtual void SetGroupName( const string & name )     { m_GroupName = name; }
    virtual void SetGroupDisplaySuffix( int num )        { m_GroupDisplaySuffix = num; }

    virtual string GetDisplayGroupName();

    virtual void SetDescript( const string& d )
    {
        m_Descript = d;
    }
    virtual string GetDescript() const
    {
        return m_Descript;
    }

    virtual ParmContainer* GetContainer() const          { return m_Container; }
    virtual string GetContainerID();

    virtual string GetID() const
    {
        return m_ID;
    }
    virtual void ChangeID( const string& newID );

    virtual int  GetType() const
    {
        return m_Type;
    }
    virtual int GetChangeCnt() const            { return m_ChangeCnt; }

    enum { SET, SET_FROM_LINK, SET_FROM_DEVICE, };
    virtual double Set( double val );
    virtual double SetFromLink( double val );
    virtual double SetFromDevice( double val, bool drag_flag = false );

    virtual void SetLowerLimit( double limit );
    virtual void SetUpperLimit( double limit );
    virtual void SetLowerUpperLimits( double lower_limit, double upper_limit );
    virtual double GetLowerLimit() const
    {
        return m_LowerLimit;
    }
    virtual double GetUpperLimit() const
    {
        return m_UpperLimit;
    }

    double Get() const
    {
        return m_Val;
    }
    double GetLastVal() const
    {
        return m_LastVal;
    }
    double operator () () const
    {
        return Get();
    }

    double operator= ( double val )
    {
        return Set( val );
    }
    double operator*=( double rhs )
    {
        return Set( m_Val * rhs );
    }
    double operator+=( double rhs )
    {
        return Set( m_Val + rhs );
    }
    double operator-=( double rhs )
    {
        return Set( m_Val - rhs );
    }

    virtual void SetActiveFlag( bool flag )
    {
        m_ActiveFlag = flag;
    }
    virtual void Activate()
    {
        SetActiveFlag( true );
    }
    virtual void Deactivate()
    {
        SetActiveFlag( false );
    }
    virtual bool GetActiveFlag() const
    {
        return m_ActiveFlag;
    }

    virtual void SetLinkUpdateFlag( bool flag )
    {
        m_LinkUpdateFlag = flag;
    }
    virtual bool GetLinkUpdateFlag() const
    {
        return m_LinkUpdateFlag;
    }

    virtual void SetLinkContainerID( const string & id )
    {
        m_LinkContainerID = id;
    }
    virtual void ReSetLinkContainerID();
    virtual string GetLinkContainerID() const
    {
        return m_LinkContainerID;
    }
    virtual ParmContainer* GetLinkContainer() const;

    virtual void EncodeXml( xmlNodePtr & node, bool detailed = false );
    virtual void DecodeXml( xmlNodePtr & node, bool detailed = false );

protected:

    string m_ID;

    string m_Name;
    string m_GroupName;
    int m_GroupDisplaySuffix;
    ParmContainer* m_Container;
    int m_ChangeCnt;

    std::string m_Descript;

    int m_Type;

    double m_Val;
    double m_LastVal;
    double m_LowerLimit;
    double m_UpperLimit;

    bool m_ActiveFlag;
    bool m_LinkUpdateFlag;  // Used to identify actively updating Parms to prevent circular updates.
    string m_LinkContainerID;

    virtual string GenerateID();
    virtual bool SetValCheckLimits( double val );

};

//==== Fraction Parm ====//
class FractionParm : public Parm
{
public:
    FractionParm();

    virtual void SetRefVal( double val );
    virtual double GetRefVal()                      { return m_RefVal; }

    double operator= ( double val )
    {
        return Set( val );
    }

    virtual double Set( double val );
    virtual double SetFromLink( double val );
    virtual double SetFromDevice( double val, bool drag_flag = false );

    virtual double SetResult( double val );
    virtual double SetResultFromLink( double val );
    virtual double SetResultFromDevice( double val );

    virtual void SetDisplayResultFlag( bool flag )  { m_DisplayResultFlag = flag; }
    virtual bool GetDisplayResultsFlag()            { return m_DisplayResultFlag; }

    virtual double GetResult();

protected:

    double m_RefVal;
    double m_ResultVal;
    virtual void UpdateResultVal();

    // Flag indicating if m_Val or m_ResultVal should be displayed in the GUI. Note, the GUI
    // Device associated with the FractionParm must support this feature
    bool m_DisplayResultFlag;

};

//==== Integer Parm ====//
class IntParm : public Parm
{
public:
    IntParm();

    int Set( int val );

    int Get() const
    {
        return ( int )( m_Val + 0.5 );
    }
    int operator () () const
    {
        return Get();
    }

    virtual int operator= ( int val )
    {
        return Set( val );
    }
    virtual int operator*=( int rhs )
    {
        return Set( Get() * rhs );
    }
    virtual int operator+=( int rhs )
    {
        return Set( Get() + rhs );
    }
    virtual int operator-=( int rhs )
    {
        return Set( Get() - rhs );
    }

    virtual bool SetValCheckLimits( double val );

};

//==== Limited Integer Parm ====//
// IntParm restricted to values of m_Mult*n+m_Shift, where n is an integer
class LimIntParm : public IntParm
{
public:
    LimIntParm();

    virtual int operator= ( int val )
    {
        return IntParm::operator=( val );
    }

    virtual bool SetValCheckLimits( double val );

    virtual void SetMultShift( int mult, int shift );

protected:
    int m_Mult;
    int m_Shift;
};

//==== Power Integer Parm ====//
// IntParm restricted to values of pow(m_Base, n) + m_Shift, where n is an integer
class PowIntParm : public IntParm
{
public:
    PowIntParm();

    virtual int operator= ( int val )
    {
        return IntParm::operator=( val );
    }

    virtual bool SetValCheckLimits( double val );

    virtual void SetPowShift( int power, int shift ); // This must be called before Init 

protected:
    int m_Base;
    int m_Shift;
};

//==== Not Equal Parm ====//
class NotEqParm : public Parm
{
public:
    NotEqParm();
    virtual void ChangeID( const string& newID );

    virtual void SetOtherParmID( const string &pid, double tol )
    {
        m_OtherParmID = pid;
        m_Tol = tol;
    }

    virtual void DecodeXml( xmlNodePtr & node, bool detailed = false );

protected:
    string m_OtherParmID;
    double m_Tol;

    bool m_CheckFlag;

    virtual bool SetValCheckLimits( double val );
};


//==== Bool Parm ====//
class BoolParm : public Parm
{
public:
    BoolParm();

    virtual bool Set( bool val );
    bool Get() const
    {
        if ( m_Val > 0.5 )
        {
            return true;
        }
        return false;
    }
    bool operator () () const
    {
        return Get();
    }

    virtual bool operator= ( bool val )
    {
        return Set( val );
    }
};

// jrg not sure this makes any sense
////==== String Parm ====//
//class StringParm : public Parm
//{
//public:
//  StringParm();
//
//  virtual string Set( string & val );
//  string Get()                                    { return m_String; }
//  string operator () ()                           { return Get(); }
//
//  virtual string operator= (string val)           { return Set( val ); }
//
//protected:
//
//  string m_String;
//};

//==== Driver Group ====//
class DriverGroup
{
public:
    DriverGroup( int Nvar, int Nchoice );
    virtual ~DriverGroup();

    void SetChoice( int choice, int grpid );

    virtual void UpdateGroup( vector< string > parmIDs ) = 0;
    virtual bool ValidDrivers( vector< int > choices ) = 0;

    void SetChoices( const vector< int > &choices );
    vector< int > GetChoices()                    { return m_CurrChoices; }
    int GetNchoice() const                        { return m_Nchoice; }
    int GetNvar() const                           { return m_Nvar; }
    bool IsDriver( int dvar );

    void SetName( const string & name )           { m_Name = name; }
    string GetName()                              { return m_Name; }

    virtual void EncodeXml( xmlNodePtr & node );
    virtual void DecodeXml( xmlNodePtr & node );

    virtual void Test( vector< string > parmIDs, double tol );


protected:

    string m_Name;

    unsigned int m_Nvar;
    unsigned int m_Nchoice;
    vector< int > m_CurrChoices;
};

#endif // !defined(VSP_PARM__INCLUDED_)
