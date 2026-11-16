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
        TEST_ADD( ParmIndexTestSuite::TheTopOfANestedCurveIsItsGeom )
        TEST_ADD( ParmIndexTestSuite::AContainerWithNoParentIsItsOwnTop )
        TEST_ADD( ParmIndexTestSuite::EveryLiveContainerIsListed )
        TEST_ADD( ParmIndexTestSuite::ASubsurfacesParentIsItsOwner )
        TEST_ADD( ParmIndexTestSuite::AStructureIsTheParentOfWhatItHolds )
        TEST_ADD( ParmIndexTestSuite::AMaterialAndAnAssemblyAreTheParentsOfWhatTheyHold )
        TEST_ADD( ParmIndexTestSuite::AGeomIsTheParentOfItsSourcesTexturesAndColor )
        TEST_ADD( ParmIndexTestSuite::EveryNestedContainerSitsUnderItsOwner )
        TEST_ADD( ParmIndexTestSuite::NestedContainersSitUnderTheirOwnerAfterAFileRead )
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
    void TheTopOfANestedCurveIsItsGeom();
    void AContainerWithNoParentIsItsOwnTop();
    void EveryLiveContainerIsListed();
    void ASubsurfacesParentIsItsOwner();
    void AStructureIsTheParentOfWhatItHolds();
    void AMaterialAndAnAssemblyAreTheParentsOfWhatTheyHold();
    void AGeomIsTheParentOfItsSourcesTexturesAndColor();
    void EveryNestedContainerSitsUnderItsOwner();
    void NestedContainersSitUnderTheirOwnerAfterAFileRead();
};

#endif
