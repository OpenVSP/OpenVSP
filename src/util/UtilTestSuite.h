//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// UtilTestSuite.h: Unit tests for util
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPUTILTESTSUITE__INCLUDED_)
#define VSPUTILTESTSUITE__INCLUDED_

#include "Defines.h"
#include "UsingCpp11.h"
#include "cpptest.h"
#include "MessageMgr.h"
#include "VspCurve.h"
#include "VspSurf.h"
#include <memory>


using std::shared_ptr;

class UtilTestSuite : public Test::Suite
{
public:
    UtilTestSuite()
    {
        TEST_ADD( UtilTestSuite::MessageMgrTest )
        TEST_ADD( UtilTestSuite::Vec2dUtilTest )
        TEST_ADD( UtilTestSuite::StringUtilTest )
        TEST_ADD( UtilTestSuite::StlHelperTest )
        TEST_ADD( UtilTestSuite::VspCurveTest )
        TEST_ADD( UtilTestSuite::VspSurfTest )
        TEST_ADD( UtilTestSuite::SharedPtrTest )
        TEST_ADD( UtilTestSuite::PointInPolyTest )
        TEST_ADD( UtilTestSuite::BilinearInterpTest )
        TEST_ADD( UtilTestSuite::FormatWidthTest )
    }

private:
    void MessageMgrTest();
    void Vec2dUtilTest();
    void StringUtilTest();
    void StlHelperTest();
    void VspCurveTest();
    void VspSurfTest();
    void SharedPtrTest();
    void PointInPolyTest();
    void BilinearInterpTest();
    void FormatWidthTest();

    static void WritePntVecs( vector< vector< vec3d > > & pnt_vecs,  string file_name );
    void WriteCurve( VspCurve& crv, string file_name );
//  void WriteSurf( vector< vector< PntNorm > > & pnt_norm_vec, string file_name );
    static void WriteControlPnts( const vector< vector< vec3d > > & control_pnts, string file_name );
};

//==== Create A Class To Test MessageMgr (MessageBase is Pure Virtual) ====//
class MessageBaseTest : public MessageBase
{
public:
    MessageBaseTest()
    {
        m_ValidFrom  = false;
    }
    bool m_ValidFrom;
    size_t m_FromPtrID;
    MessageData m_Data;
    void MessageCallback( const MessageBase* from, const MessageData& data )
    {
        m_ValidFrom = false;
        if ( from )
        {
            m_FromPtrID = ( size_t )from;
            m_ValidFrom = true;
        }
        m_Data = data;
    }

};

//==== Create Some Classes To Test Shared Ptrs ====//
class TestB;
typedef shared_ptr<TestB> TestBPtr;

class TestA
{
public:
    TestA( int v = 0 );
    ~TestA();

    int GetA()
    {
        return m_A;
    }
    int GetBA();

protected:

    int m_A;
    TestBPtr m_BPtr;
};

class TestB
{
public:
    TestB( TestA & a_ref );
    ~TestB();

    int GetA()
    {
        return m_ARef.GetA();
    }

protected:

    int m_B;
    TestA& m_ARef;
};



#endif // !defined(VSPUTILTESTSUITE__INCLUDED_)
