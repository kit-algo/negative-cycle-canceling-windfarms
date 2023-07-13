#include <algorithm>
#include <cassert>
#include <cstdio>
#include <queue>
#include <string>

#include "Algorithms/EscapeStrategies/EscapingLeaves.h"

namespace wcp {

namespace Algorithms {

void EscapingLeaves::ExploreVertexWhenFindingSubstationAssignment(vertexID currentVertex, vertexID substation, std::queue<vertexID>& accessibleVertices) {
    for (edgeID edge : Windfarm().OutgoingEdges(currentVertex)) {
        if (Windfarm().FlowOnEdge(edge) != 0) {
            vertexID nextTurbine = Windfarm().EndVertex(edge);
            ProcessEndVertexWhenFindingSubstationAssignment(nextTurbine, substation, accessibleVertices);
        }
    }
}

bool EscapingLeaves::IsSubstationNotAtEndOfAssignmentForVertex(vertexID vertex, vertexID substation) {
    if (mappingTurbinesToSubstations_[vertex].empty()) { return true; }
    if (mappingTurbinesToSubstations_[vertex].back() != substation) { return true; }

    return false;
}

void EscapingLeaves::ProcessEndVertexWhenFindingSubstationAssignment(vertexID endVertex, vertexID substation, std::queue<vertexID>& accessibleVertices) {
    if (IsSubstationNotAtEndOfAssignmentForVertex(endVertex, substation)) {
        if (Windfarm().IsTurbine(endVertex)) {
            accessibleVertices.push(endVertex);
            mappingTurbinesToSubstations_[endVertex].push_back(substation);
        } else if (Windfarm().IsSubstation(endVertex)) {
            mappingTurbinesToSubstations_[endVertex].push_back(substation);
        } else {
            assert(Windfarm().IsSuperSubstation(endVertex)); // ignore supersubstation
        }
    } // else: substation was already assigned to endVertex
}

void EscapingLeaves::IdentifyTurbineSubstationAssignmentUsingBFS() {
    mappingTurbinesToSubstations_ = std::vector<std::vector<vertexID>>(Windfarm().NumberOfVertices(), std::vector<vertexID>());

    for (vertexID substation : Windfarm().Substations()) {
        mappingTurbinesToSubstations_[substation].push_back(substation);
        std::queue<vertexID> accessibleVertices;
        accessibleVertices.push(substation);

        while (!accessibleVertices.empty()) {
            vertexID currentVertex = accessibleVertices.front();
            accessibleVertices.pop();
            ExploreVertexWhenFindingSubstationAssignment(currentVertex, substation, accessibleVertices);
        }
    }
}

edgeID EscapingLeaves::FindUniqueOutgoingEdgeWithFlowFromVertex(vertexID vertex) {
    // If this unique edge does not exist, INVALID_EDGE is returned.
    edgeID uniqueEdgeWithFlow = INVALID_EDGE;
    for (edgeID outgoingEdge : Windfarm().OutgoingEdges(vertex)) {
        if (Windfarm().FlowOnEdge(outgoingEdge) != 0) {
            if (uniqueEdgeWithFlow == INVALID_EDGE) {
                uniqueEdgeWithFlow = outgoingEdge;
            } else {
                return INVALID_EDGE;
            }
        }
    }
    return uniqueEdgeWithFlow;
}

edgeID EscapingLeaves::FindShortestOutgoingEdgeWithoutFlow(vertexID vertex, edgeID edgeWithFlow) {
    assert(edgeWithFlow != INVALID_EDGE);
    assert(Windfarm().StartVertex(edgeWithFlow) == vertex);
    assert(Windfarm().FlowOnEdge(edgeWithFlow) == 1);

    edgeID shortestEdgeWithoutFlow = INVALID_EDGE;
    double shortestEdgeLength = Windfarm().Length(edgeWithFlow);
    for (edgeID outgoingEdge : Windfarm().OutgoingEdges(vertex)) {
        if (outgoingEdge != edgeWithFlow && Windfarm().Length(outgoingEdge) < shortestEdgeLength) {
            shortestEdgeWithoutFlow = outgoingEdge;
        } 
    }

    assert(shortestEdgeWithoutFlow == INVALID_EDGE || Windfarm().FlowOnEdge(shortestEdgeWithoutFlow) == 0);
    return shortestEdgeWithoutFlow;
}

void EscapingLeaves::InsertLeaf(vertexID vertex, edgeID edgeWithFlow, edgeID shortestEdgeWithoutFlow) {
    if (shortestEdgeWithoutFlow != INVALID_EDGE) {
        potentialLeaves_.emplace_back(vertex, edgeWithFlow, shortestEdgeWithoutFlow);
    }
}

void EscapingLeaves::IdentifyAllLeafTurbines() {
    potentialLeaves_.clear();

    for (vertexID turbine : Windfarm().Turbines()) {
        edgeID edgeWithFlow = FindUniqueOutgoingEdgeWithFlowFromVertex(turbine);
        if (edgeWithFlow != INVALID_EDGE) {
            edgeID shortestEdgeWithoutFlow = FindShortestOutgoingEdgeWithoutFlow(turbine, edgeWithFlow);
            InsertLeaf(turbine, edgeWithFlow, shortestEdgeWithoutFlow);
        } else {
            continue;
        }  
    }
}

bool EscapingLeaves::PotentiallyNewSubstationOfLeafHasFreeCapacity(LeafTurbine& leaf) {
    return std::any_of( mappingTurbinesToSubstations_[Windfarm().EndVertex(leaf.ShorterEdge())].begin(),
                        mappingTurbinesToSubstations_[Windfarm().EndVertex(leaf.ShorterEdge())].end(),
                        [&,this] (vertexID id) {
                            return this->Windfarm().HasFreeSubstationCapacity(id);
                        });
}

bool EscapingLeaves::MovingLeafMightNotChangeSubstationAssignment(LeafTurbine& leaf) {
    vertexID currentTurbine = leaf.TurbineID();
    for (vertexID assignedSubstation : mappingTurbinesToSubstations_[Windfarm().EndVertex(leaf.ShorterEdge())]) {
        if ( std::any_of(   mappingTurbinesToSubstations_[currentTurbine].begin(),
                            mappingTurbinesToSubstations_[currentTurbine].end(),
                            [&,this] (vertexID id) {
                                return id == assignedSubstation;
                            })) {
            return true;
        }
    }
    return false;
}

bool EscapingLeaves::IsMovingOfLeafFeasibleSubstationCapacitywise(LeafTurbine& leaf) {
    return MovingLeafMightNotChangeSubstationAssignment(leaf) || PotentiallyNewSubstationOfLeafHasFreeCapacity(leaf);
}

void EscapingLeaves::PrintAllLeaves(size_t originalNumberOfLeaves, bool beVerbose, std::ostream& verboseStream) {
    if (beVerbose) {
        verboseStream << "\tThere are " << originalNumberOfLeaves << " suitable leaves to start with.\n";
        for (LeafTurbine& leaf : potentialLeaves_) {
            leaf.Print(verboseStream);
        }
    }
}

edgeID EscapingLeaves::SingleDFSStepToFindPathToRepeatingEdge(edgeID edgeToRepeat, edgeID edgeToExplore, bool& hasEdgeToRepeatAlreadyBeenSeen, std::vector<edgeID>& NextEdgeToASubstation) {
    if (Windfarm().IsSubstation(Windfarm().EndVertex(edgeToExplore))) {
        return INVALID_EDGE;
    }

    if (edgeToExplore == edgeToRepeat) {
        if (hasEdgeToRepeatAlreadyBeenSeen) {
            return edgeToExplore;
        } else {
            hasEdgeToRepeatAlreadyBeenSeen = true;
        }
    }

    for (edgeID outgoingEdge : Windfarm().OutgoingEdges(Windfarm().EndVertex(edgeToExplore))) {
        if (outgoingEdge != Windfarm().ReverseEdgeIndex(edgeToExplore) &&
            Windfarm().FlowOnEdge(outgoingEdge) > 0) {
            edgeID NextEdgeOnWayToCycle = SingleDFSStepToFindPathToRepeatingEdge(edgeToRepeat, outgoingEdge, hasEdgeToRepeatAlreadyBeenSeen, NextEdgeToASubstation);
            if (NextEdgeOnWayToCycle == INVALID_EDGE) { continue; }
            assert(NextEdgeOnWayToCycle == outgoingEdge);
            NextEdgeToASubstation[edgeToExplore] = outgoingEdge;
            return edgeToExplore;
        }
    }
    return INVALID_EDGE;
}
std::vector<edgeID> EscapingLeaves::PopulateParentPointersUntilEdgeRepeats(edgeID edgeToRepeat, edgeID edgeToStartSearchWith) {
    std::vector<edgeID> NextEdgeToASubstation(Windfarm().NumberOfEdges(), INVALID_EDGE);
    bool hasEdgeToRepeatAlreadyBeenSeen = false;

#ifndef NDEBUG
    edgeID edgeThatShouldBeTheFirstEdge = SingleDFSStepToFindPathToRepeatingEdge(edgeToRepeat, edgeToStartSearchWith, hasEdgeToRepeatAlreadyBeenSeen, NextEdgeToASubstation);
    assert(edgeThatShouldBeTheFirstEdge == edgeToStartSearchWith);
#else
    (void) SingleDFSStepToFindPathToRepeatingEdge(edgeToRepeat, edgeToStartSearchWith, hasEdgeToRepeatAlreadyBeenSeen, NextEdgeToASubstation);
#endif

    return NextEdgeToASubstation;
}

void EscapingLeaves::CancelCycleWithAllPositiveFlow(edgeID edgeOnCycle, edgeID firstEdgeInSearch) {
    std::vector<edgeID> NextEdgeToASubstation = PopulateParentPointersUntilEdgeRepeats(edgeOnCycle, firstEdgeInSearch);

    std::vector<edgeID> edgesOnCycle;
    do {
        edgesOnCycle.push_back(edgeOnCycle);
        edgeOnCycle = NextEdgeToASubstation[edgeOnCycle];
    } while (edgeOnCycle != edgesOnCycle.front());

    int delta = Windfarm().FindMinimumFlowOnEdges(edgesOnCycle);

    Windfarm().AddFlowOnMultipleEdges(edgesOnCycle, -delta);
}

edgeID EscapingLeaves::ExploreEdgeWhenFindingPreviousSubstation(std::vector<bool>& hasEdgeAlreadyBeenExplored,
                                                                std::vector<edgeID>& invertedPathToPreviousSubstation,
                                                                edgeID edgeToExplore,
                                                                edgeID firstEdgeInSearch) {
    if (hasEdgeAlreadyBeenExplored[edgeToExplore]) {
        CancelCycleWithAllPositiveFlow(edgeToExplore, firstEdgeInSearch);
        invertedPathToPreviousSubstation.push_back(INVALID_EDGE);
        return INVALID_EDGE;
    }

    if (Windfarm().IsSubstation(Windfarm().EndVertex(edgeToExplore))) {
        invertedPathToPreviousSubstation.push_back(edgeToExplore);
        return edgeToExplore;
    }

    for (edgeID outgoingEdge : Windfarm().OutgoingEdges(Windfarm().EndVertex(edgeToExplore))) {
        if (outgoingEdge != Windfarm().ReverseEdgeIndex(edgeToExplore) &&
            Windfarm().FlowOnEdge(outgoingEdge) > 0) {
            edgeID NextEdgeOnPathToSubstation = ExploreEdgeWhenFindingPreviousSubstation(hasEdgeAlreadyBeenExplored, invertedPathToPreviousSubstation, outgoingEdge, firstEdgeInSearch);
            if (NextEdgeOnPathToSubstation == INVALID_EDGE) { continue; }
            assert(NextEdgeOnPathToSubstation == outgoingEdge);
            invertedPathToPreviousSubstation.push_back(edgeToExplore);
            return edgeToExplore;
        }
    }
    return INVALID_EDGE;
}

std::vector<edgeID> EscapingLeaves::IdentifyInvertedPathToPreviousSubstation(LeafTurbine& leaf) {
    std::vector<edgeID> invertedPathToPreviousSubstation;
    // The following do-while-loop catches the case that a cycle of positive flow was cancelled.
    // In that case we need to run the procedure again to identify the path to the substation.
    do {
        invertedPathToPreviousSubstation.clear();
        std::vector<bool> hasEdgeAlreadyBeenExplored(Windfarm().NumberOfEdges(), false);
        // We do not need the edge returned by the function but rather the vector entries in invertedPathToPreviousSubstation.
        (void) ExploreEdgeWhenFindingPreviousSubstation(hasEdgeAlreadyBeenExplored, invertedPathToPreviousSubstation, leaf.CurrentEdge(), leaf.CurrentEdge());
    } while (invertedPathToPreviousSubstation.front() == INVALID_EDGE);

    assert(invertedPathToPreviousSubstation.back() == leaf.CurrentEdge());
    return invertedPathToPreviousSubstation;
}

edgeID EscapingLeaves::ExploreEdgeWhenFindingNewSubstation( std::vector<bool>& hasEdgeAlreadyBeenExplored,
                                                            std::vector<edgeID>& invertedPathToNewSubstation,
                                                            edgeID edgeToExplore,
                                                            edgeID firstEdgeInSearch,
                                                            vertexID previousSubstation) {
    if (hasEdgeAlreadyBeenExplored[edgeToExplore]) {
        CancelCycleWithAllPositiveFlow(edgeToExplore, firstEdgeInSearch);
        return INVALID_EDGE;
    } else {
        hasEdgeAlreadyBeenExplored[edgeToExplore] = true;
    }

    if (Windfarm().IsSubstation(Windfarm().EndVertex(edgeToExplore))) {
        if (Windfarm().HasFreeSubstationCapacity(Windfarm().EndVertex(edgeToExplore)) || 
                Windfarm().EndVertex(edgeToExplore) == previousSubstation) {
            invertedPathToNewSubstation.push_back(edgeToExplore);
            return edgeToExplore;
        } else {
            return INVALID_EDGE;
        }
    }

    for (edgeID outgoingEdge : Windfarm().OutgoingEdges(Windfarm().EndVertex(edgeToExplore))) {
        if (outgoingEdge != Windfarm().ReverseEdgeIndex(edgeToExplore) &&
            Windfarm().HasFreeEdgeCapacity(outgoingEdge) &&
            Windfarm().FlowOnEdge(outgoingEdge) > 0) {
            // We might miss some opportunities here:
            // If outgoingEdge is saturated, we ignore it here. But, if reverseEdge(outgoingEdge)
            // is on the path to the previous substation, rerouting the flow gives us temporary free capacity.
            edgeID NextEdgeOnPathToSubstation = ExploreEdgeWhenFindingNewSubstation(hasEdgeAlreadyBeenExplored, invertedPathToNewSubstation, outgoingEdge, firstEdgeInSearch, previousSubstation);
            if (NextEdgeOnPathToSubstation != INVALID_EDGE) {
                assert(NextEdgeOnPathToSubstation == outgoingEdge);
                invertedPathToNewSubstation.push_back(edgeToExplore);
                return edgeToExplore;
            }
        }
    }
    return INVALID_EDGE;
}

std::vector<edgeID> EscapingLeaves::IdentifyInvertedPathToNewSubstation(LeafTurbine& leaf, vertexID previousSubstation) {
    std::vector<edgeID> invertedPathToNewSubstation;
    std::vector<bool> hasEdgeAlreadyBeenExplored(Windfarm().NumberOfEdges(), false);

#ifndef NDEBUG
    edgeID firstEdge = ExploreEdgeWhenFindingNewSubstation(hasEdgeAlreadyBeenExplored, invertedPathToNewSubstation, leaf.ShorterEdge(), leaf.ShorterEdge(), previousSubstation);
    if (firstEdge == INVALID_EDGE) {
        assert(invertedPathToNewSubstation.empty());
    } else {
        assert(invertedPathToNewSubstation.back() == leaf.ShorterEdge());
    }
#else
    (void) ExploreEdgeWhenFindingNewSubstation(hasEdgeAlreadyBeenExplored, invertedPathToNewSubstation, leaf.ShorterEdge(), leaf.ShorterEdge(), previousSubstation);
#endif

    return invertedPathToNewSubstation;
}

void EscapingLeaves::ChangeFlowOnPathToSuperSubstation(std::vector<edgeID>& invertedPathToNewSubstation, int delta, bool beVerbose, std::ostream& verboseStream) {
    IO::outputVectorToStream(  "\t\tFlow change for turbine " + std::to_string(Windfarm().StartVertex(invertedPathToNewSubstation.back())) + ": Add " + std::to_string(delta) + " units of flow to edges ",
                                    invertedPathToNewSubstation, "", true, beVerbose, verboseStream);
    invertedPathToNewSubstation.push_back(Windfarm().EdgeToSupersubstation(Windfarm().EndVertex(invertedPathToNewSubstation.front())));
    Windfarm().AddFlowOnMultipleEdges(invertedPathToNewSubstation, delta);
}

bool EscapingLeaves::RerouteFlowForLeafIfPathsCanBeFound(LeafTurbine& leaf, SpecialResidualCostCollection& newResidualCosts, bool beVerbose, std::ostream& verboseStream) {
    std::vector<edgeID> invertedPathToPreviousSubstation = IdentifyInvertedPathToPreviousSubstation(leaf);
    assert(!invertedPathToPreviousSubstation.empty() && invertedPathToPreviousSubstation.front() != INVALID_EDGE);
    edgeID lastEdgeToPreviousSubstation = invertedPathToPreviousSubstation.front();
    assert(Windfarm().IsSubstation(Windfarm().EndVertex(lastEdgeToPreviousSubstation)));

    std::vector<edgeID> invertedPathToNewSubstation = IdentifyInvertedPathToNewSubstation(leaf, Windfarm().EndVertex(lastEdgeToPreviousSubstation));

    if (!invertedPathToNewSubstation.empty()) {
        // That means we have found a substation with free capacity for the turbine associated to the current leaf-tuple
        ChangeFlowOnPathToSuperSubstation(invertedPathToPreviousSubstation, -1, beVerbose, verboseStream);
        ChangeFlowOnPathToSuperSubstation(invertedPathToNewSubstation,  1, beVerbose, verboseStream);
        AssertFeasibleFlow(Windfarm());

        BuildAdaptedResidualCostsForNewLeafEdge(newResidualCosts, leaf.ShorterEdge());

        leaf.MarkAsUsed();

        // Update assignments of substations to turbines:
        mappingTurbinesToSubstations_[Windfarm().StartVertex(leaf.ShorterEdge())] = mappingTurbinesToSubstations_[Windfarm().EndVertex(leaf.ShorterEdge())];

        return true;
    }

    return false;
}

void EscapingLeaves::DeleteUsedLeaves(bool aLeafHasBeenUsed) {
    if (aLeafHasBeenUsed) {
        potentialLeaves_.erase(std::remove_if(potentialLeaves_.begin(),
                                            potentialLeaves_.end(),
                                            [&] (LeafTurbine leaf) {
                                                return leaf.HasAlreadyBeenUsed();
                                            }),
                             potentialLeaves_.end());
    }
}

void EscapingLeaves::PrintRemainingNumberOfLeaves(bool beVerbose, std::ostream& verboseStream) {
    if (beVerbose) {
        verboseStream << "\tThere are " << potentialLeaves_.size() << " leaves remaining.\n";
    }
}

void EscapingLeaves::BuildAdaptedResidualCostsForNewLeafEdge(SpecialResidualCostCollection& newResidualCosts, edgeID newEdge) {
    std::vector<int> newCostVector = Windfarm().Cabletypes().ExpandedCosts();
    int newCost = newCostVector[newCostVector.size() / 2 - 1];
    for (int k = newCostVector.size() / 2 - 1; k >= 0; k--) {
        if (newCostVector[k] == 0) {
            continue;
        } else {
            newCostVector[k] -= newCost;
        }
    }

    newResidualCosts.addEdge(Windfarm().ReverseEdgeIndex(newEdge), newCostVector);

    std::vector<int> reverseNewCostVector(newCostVector.size());
    std::reverse_copy(newCostVector.begin(),newCostVector.end(),reverseNewCostVector.begin());
    newResidualCosts.addEdge(newEdge, reverseNewCostVector);
}

bool EscapingLeaves::TryToMoveALeaf(LeafTurbine& leaf, SpecialResidualCostCollection& newResidualCosts, bool beVerbose, std::ostream& verboseStream) {
    if (IsMovingOfLeafFeasibleSubstationCapacitywise(leaf)) {
        return RerouteFlowForLeafIfPathsCanBeFound(leaf, newResidualCosts, beVerbose, verboseStream);
    } else {
        return false;
    }
}

void EscapingLeaves::RepeatedlyConsiderAllLeavesForMoving(SpecialResidualCostCollection& newResidualCosts, bool beVerbose, std::ostream& verboseStream) {
    bool changedTheFlow = true; 
    
    while (changedTheFlow && !potentialLeaves_.empty()) {
        changedTheFlow = false;
        for (LeafTurbine& leaf : potentialLeaves_) {
            changedTheFlow |= TryToMoveALeaf(leaf, newResidualCosts, beVerbose, verboseStream);
        }

        DeleteUsedLeaves(changedTheFlow);
        PrintRemainingNumberOfLeaves(beVerbose, verboseStream);
    }
}

ReturnCode EscapingLeaves::DetermineReturnCodeAndCopyResidualCosts(EscapingNCC& ncc, size_t numberOfLeavesToStartWith, SpecialResidualCostCollection& newResidualCosts) {
    if (potentialLeaves_.size() == numberOfLeavesToStartWith) {
        return ReturnCode::NOTHING_CHANGED;
    } else {
        ncc.CopyAdaptedResidualCosts(newResidualCosts);
        return ReturnCode::SUCCESS;
    }
}

ReturnCode EscapingLeaves::SuccessivelyMoveLeaves(EscapingNCC& ncc) {
    size_t numberOfLeavesToStartWith = potentialLeaves_.size();
    PrintAllLeaves(numberOfLeavesToStartWith, ncc.beVerbose_, ncc.verboseStream_);

    SpecialResidualCostCollection newResidualCosts;
    RepeatedlyConsiderAllLeavesForMoving(newResidualCosts, ncc.beVerbose_, ncc.verboseStream_);
    return DetermineReturnCodeAndCopyResidualCosts(ncc, numberOfLeavesToStartWith, newResidualCosts);
}

ReturnCode EscapingLeaves::apply(EscapingNCC& ncc) {
    CommonStartForAllEscapingStrategies(ncc.beVerbose_, ncc.verboseStream_);

    IdentifyTurbineSubstationAssignmentUsingBFS();

    IdentifyAllLeafTurbines();

    return SuccessivelyMoveLeaves(ncc);
}

} // namespace Algorithms

} // namespace wcp
