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
        TEST_ADD( ParmIndexTestSuite::ADatcomComponentKeepsItsSectionsWhenResized )
        TEST_ADD( ParmIndexTestSuite::TheTopOfANestedCurveIsItsGeom )
        TEST_ADD( ParmIndexTestSuite::AContainerWithNoParentIsItsOwnTop )
        TEST_ADD( ParmIndexTestSuite::EveryLiveContainerIsListed )
        TEST_ADD( ParmIndexTestSuite::ASubsurfacesParentIsItsOwner )
        TEST_ADD( ParmIndexTestSuite::AStructureIsTheParentOfWhatItHolds )
        TEST_ADD( ParmIndexTestSuite::AMaterialAndAnAssemblyAreTheParentsOfWhatTheyHold )
        TEST_ADD( ParmIndexTestSuite::AGeomIsTheParentOfItsSourcesTexturesAndColor )
        TEST_ADD( ParmIndexTestSuite::EveryNestedContainerSitsUnderItsOwner )
        TEST_ADD( ParmIndexTestSuite::NestedContainersSitUnderTheirOwnerAfterAFileRead )
        TEST_ADD( ParmIndexTestSuite::AParmIsUnderItsOwnerAtOnce )
        TEST_ADD( ParmIndexTestSuite::AStructuresPartsAreNumberedAsTheyChange )
        TEST_ADD( ParmIndexTestSuite::AGeomsSubsurfacesOfEveryKindAreNumbered )
        TEST_ADD( ParmIndexTestSuite::AGeomsSourcesAreNumbered )
        TEST_ADD( ParmIndexTestSuite::AGeomsTexturesAreNumbered )
        TEST_ADD( ParmIndexTestSuite::AGearsBogiesAreNumbered )
        TEST_ADD( ParmIndexTestSuite::ARoutesPointsAreNumbered )
        TEST_ADD( ParmIndexTestSuite::AStructuresConditionsAreNumbered )
        TEST_ADD( ParmIndexTestSuite::CpSlicesAreNumbered )
        TEST_ADD( ParmIndexTestSuite::UnsteadyGroupsAreNumbered )
        TEST_ADD( ParmIndexTestSuite::AMaterialsLayersAreNumbered )
        TEST_ADD( ParmIndexTestSuite::AnAssemblysConnectionsAreNumbered )
        TEST_ADD( ParmIndexTestSuite::PolySparPointsAreNumberedAcrossTheirStructure )
        TEST_ADD( ParmIndexTestSuite::TheParmsUnderAGeomAreItsOwnAndItsPartsInOrder )
        TEST_ADD( ParmIndexTestSuite::AContainerNotListedIsLeftOutOfTheWalkAboveIt )
        TEST_ADD( ParmIndexTestSuite::EveryContainerListsExactlyTheParmsUnderIt )
        TEST_ADD( ParmIndexTestSuite::LinkingAllOfAContainerDrivesEachParmOnce )
        TEST_ADD( ParmIndexTestSuite::TheChangeCountMovesWhenAContainerTakesAParent )
        TEST_ADD( ParmIndexTestSuite::EveryListingIsSorted )
        TEST_ADD( ParmIndexTestSuite::TheListingFollowsEachChange )
        TEST_ADD( ParmIndexTestSuite::ThePickerPicksBackTheParmItShows )
        TEST_ADD( ParmIndexTestSuite::EveryListedParmIsFoundByName )
        TEST_ADD( ParmIndexTestSuite::AContainersOwnParmIsFoundFirst )
        TEST_ADD( ParmIndexTestSuite::AContainersOwnParmIsFoundByItsGroupName )
        TEST_ADD( ParmIndexTestSuite::AParmIsFoundAsSoonAsItIsMade )
        TEST_ADD( ParmIndexTestSuite::TheContainerListFollowsTheModel )
        TEST_ADD( ParmIndexTestSuite::EveryTopWithParmsIsListed )
        TEST_ADD( ParmIndexTestSuite::NoCustomScriptIsRunningBetweenCalls )
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
    void ADatcomComponentKeepsItsSectionsWhenResized();
    void TheTopOfANestedCurveIsItsGeom();
    void AContainerWithNoParentIsItsOwnTop();
    void EveryLiveContainerIsListed();
    void ASubsurfacesParentIsItsOwner();
    void AStructureIsTheParentOfWhatItHolds();
    void AMaterialAndAnAssemblyAreTheParentsOfWhatTheyHold();
    void AGeomIsTheParentOfItsSourcesTexturesAndColor();
    void EveryNestedContainerSitsUnderItsOwner();
    void NestedContainersSitUnderTheirOwnerAfterAFileRead();
    void AParmIsUnderItsOwnerAtOnce();
    void AStructuresPartsAreNumberedAsTheyChange();
    void AGeomsSubsurfacesOfEveryKindAreNumbered();
    void AGeomsSourcesAreNumbered();
    void AGeomsTexturesAreNumbered();
    void AGearsBogiesAreNumbered();
    void ARoutesPointsAreNumbered();
    void AStructuresConditionsAreNumbered();
    void CpSlicesAreNumbered();
    void UnsteadyGroupsAreNumbered();
    void AMaterialsLayersAreNumbered();
    void AnAssemblysConnectionsAreNumbered();
    void PolySparPointsAreNumberedAcrossTheirStructure();
    void TheParmsUnderAGeomAreItsOwnAndItsPartsInOrder();
    void AContainerNotListedIsLeftOutOfTheWalkAboveIt();
    void EveryContainerListsExactlyTheParmsUnderIt();
    void LinkingAllOfAContainerDrivesEachParmOnce();
    void TheChangeCountMovesWhenAContainerTakesAParent();
    void EveryListingIsSorted();
    void TheListingFollowsEachChange();
    void ThePickerPicksBackTheParmItShows();
    void EveryListedParmIsFoundByName();
    void AContainersOwnParmIsFoundFirst();
    void AContainersOwnParmIsFoundByItsGroupName();
    void AParmIsFoundAsSoonAsItIsMade();
    void TheContainerListFollowsTheModel();
    void EveryTopWithParmsIsListed();
    void NoCustomScriptIsRunningBetweenCalls();
};

#endif
