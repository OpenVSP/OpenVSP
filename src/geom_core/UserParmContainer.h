//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// UserParmContainer.h: Parm Container classes.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_USERPARMCONTRAINER__INCLUDED_)
#define VSP_USERPARMCONTRAINER__INCLUDED_

#include <string>
#include <map>

#include <climits>

#include "XmlUtil.h"
#include "ParmContainer.h"

using std::string;
using std::map;

class Parm;

//==============================================================================//
//==============================================================================//
//==============================================================================//

//==== User Parms ====//
class UserParmContainer : public ParmContainer
{
public:
    UserParmContainer();
    virtual ~UserParmContainer();

    virtual void SetNumPredefined( int n )                  { m_NumPredefined = n; }
    virtual void Renew( int num_initial_parms );
    virtual void ParmChanged( Parm* parm_ptr, int type );

    int GetNumUserParms()                                   { return m_UserParmVec.size(); }
    string GetUserParmId( int index );
    vector < string > GetAllUserParms();
    virtual int GetUserParmIndex( const string & id );
    virtual string AddParm(int type, const string & name, const string & group );
    virtual void DeleteParm( int index );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual bool SortVars();

protected:

    int m_NumPredefined;
    vector< Parm* > m_UserParmVec;

    virtual void Init( int num_initial_parms );
    virtual void Wype();

};


#endif // !defined(VSP_USERPARMCONTRAINER__INCLUDED_)
