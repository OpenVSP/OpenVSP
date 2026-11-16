//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmIndexTestSuite.h: Tests of how containers and Parms are found under the containers that
// hold them.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPPARMINDEXTESTSUITE__INCLUDED_)
#define VSPPARMINDEXTESTSUITE__INCLUDED_

#include "cpptest.h"

class ParmIndexTestSuite : public Test::Suite
{
public:
    ParmIndexTestSuite()
    {
        TEST_ADD( ParmIndexTestSuite::ChildrenAreKeptForEveryKindOfContainer )
        TEST_ADD( ParmIndexTestSuite::ChildrenAreKeptThroughEdits )
        TEST_ADD( ParmIndexTestSuite::ChildrenAreKeptThroughAFileRead )
        TEST_ADD( ParmIndexTestSuite::ChildrenFollowAnExchangedIdentity )
        TEST_ADD( ParmIndexTestSuite::ADestroyedChildLeavesItsParent )
        TEST_ADD( ParmIndexTestSuite::AContainersChildrenHaveNoParentOnceItIsGone )
        TEST_ADD( ParmIndexTestSuite::AParentThatWouldMakeALoopIsRefused )
        TEST_ADD( ParmIndexTestSuite::ACustomGeomsCrossSectionsAreNotListed )
    }

private:
    void ChildrenAreKeptForEveryKindOfContainer();
    void ChildrenAreKeptThroughEdits();
    void ChildrenAreKeptThroughAFileRead();
    void ChildrenFollowAnExchangedIdentity();
    void ADestroyedChildLeavesItsParent();
    void AContainersChildrenHaveNoParentOnceItIsGone();
    void AParentThatWouldMakeALoopIsRefused();
    void ACustomGeomsCrossSectionsAreNotListed();
};

#endif
