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

#include <limits.h>

#include "XmlUtil.h"
#include "GuiDeviceEnums.h"

using std::string;
using std::map;

class ParmContainer;

enum PARM_TYPE { PARM_DOUBLE_TYPE = 0, PARM_INT_TYPE = 1, PARM_BOOL_TYPE = 2,
                 PARM_FRACTION_TYPE = 3, PARM_STRING_TYPE = 4, PARM_LIMITED_INT_TYPE = 5,
                 PARM_NOTEQ_TYPE = 6
               };

//==== Parm ====//
class Parm
{


public:
    Parm();
    virtual ~Parm();

    virtual void Init( const string& name, const string& group, ParmContainer* container,
                       double val, double lower, double upper, bool linkable = true );

    virtual string GetName()                             { return m_Name; }
    virtual void SetGroupDisplaySuffix( int num )        { m_GroupDisplaySuffix = num; }

    virtual string GetGroupName()
    {
        return m_GroupName;
    }
    virtual string GetDisplayGroupName();

    virtual void SetDescript( const string& d )
    {
        m_Descript = d;
    }
    virtual string GetDescript()
    {
        return m_Descript;
    }

    virtual ParmContainer* GetContainer()       { return m_Container; }
    virtual string GetContainerID();

    virtual string GetID()
    {
        return m_ID;
    }
    virtual void ChangeID( const string& newID );

    virtual int  GetType()
    {
        return m_Type;
    }

    enum { SET, SET_FROM_LINK, SET_FROM_DEVICE, };
    virtual double Set( double val );
    virtual double SetFromLink( double val );
    virtual double SetFromDevice( double val );

    virtual void SetLowerLimit( double limit );
    virtual void SetUpperLimit( double limit );
    virtual void SetLowerUpperLimits( double lower_limit, double upper_limit );
    virtual double GetLowerLimit()
    {
        return m_LowerLimit;
    }
    virtual double GetUpperLimit()
    {
        return m_UpperLimit;
    }

    double Get()
    {
        return m_Val;
    }
    double GetLastVal()
    {
        return m_LastVal;
    }
    double operator () ()
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
    virtual bool GetActiveFlag()
    {
        return m_ActiveFlag;
    }

    virtual void SetLinkedFlag( bool flag )
    {
        m_LinkedFlag = flag;
    }
    virtual bool GetLinkedFlag()
    {
        return m_LinkedFlag;
    }

    virtual void SetLinkableFlag( bool flag )
    {
        m_LinkableFlag = flag;
    }
    virtual bool IsLinkable()
    {
        return m_LinkableFlag;
    }

    virtual void SetLinkUpdateFlag( bool flag )
    {
        m_LinkUpdateFlag = flag;
    }
    virtual bool GetLinkUpdateFlag()
    {
        return m_LinkUpdateFlag;
    }

    virtual void SetLinkContainerID( const string & id )
    {
        m_LinkContainerID = id;
    }
    virtual void ReSetLinkContainerID();
    virtual string GetLinkContainerID()
    {
        return m_LinkContainerID;
    }
    virtual ParmContainer* GetLinkContainer();

    virtual void EncodeXml( xmlNodePtr & node );
    virtual void DecodeXml( xmlNodePtr & node );

protected:

    string m_ID;

    string m_Name;
    string m_GroupName;
    int m_GroupDisplaySuffix;
    ParmContainer* m_Container;

    std::string m_Descript;

    int m_Type;

    double m_Val;
    double m_LastVal;
    double m_LowerLimit;
    double m_UpperLimit;

    bool m_ActiveFlag;
    bool m_LinkedFlag;
    bool m_LinkableFlag;
    bool m_LinkUpdateFlag;
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

    double operator= ( double val )
    {
        return Set( val );
    }

    virtual double Set( double val );
    virtual double SetFromLink( double val );
    virtual double SetFromDevice( double val );

    virtual double SetResult( double val );
    virtual double SetResultFromLink( double val );
    virtual double SetResultFromDevice( double val );

    virtual double GetResult();

protected:

    double m_RefVal;
    double m_ResultVal;
    virtual void UpdateResultVal();

};

//==== Integer Parm ====//
class IntParm : public Parm
{
public:
    IntParm();

    int Set( int val );

    int Get()
    {
        return ( int )( m_Val + 0.5 );
    }
    int operator () ()
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

//==== Not Equal Parm ====//
class NotEqParm : public Parm
{
public:
    NotEqParm();
    virtual void ChangeID( const string& newID );

    virtual void SetOtherParmID( string pid, double tol )
    {
        m_OtherParmID = pid;
        m_Tol = tol;
    }

protected:
    string m_OtherParmID;
    double m_Tol;

    virtual bool SetValCheckLimits( double val );
};


//==== Bool Parm ====//
class BoolParm : public Parm
{
public:
    BoolParm();

    virtual bool Set( bool val );
    bool Get()
    {
        if ( m_Val > 0.5 )
        {
            return true;
        }
        return false;
    }
    bool operator () ()
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

    void SetChoices( vector< int > choices )      { m_CurrChoices = choices; }
    vector< int > GetChoices()                    { return m_CurrChoices; }
    int GetNchoice() const                        { return m_Nchoice; }
    int GetNvar() const                           { return m_Nvar; }

    void SetName( const string & name )           { m_Name = name; }
    string GetName()                              { return m_Name; }

    virtual void EncodeXml( xmlNodePtr & node );
    virtual void DecodeXml( xmlNodePtr & node );

    virtual void Test( vector< string > parmIDs, double tol );


protected:

    string m_Name;

    int m_Nvar;
    int m_Nchoice;
    vector< int > m_CurrChoices;
};

#endif // !defined(VSP_PARM__INCLUDED_)
