#ifndef ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGLEAVES
#define ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGLEAVES

#include <iostream>
#include <queue>
#include <vector>

#include "Auxiliary/ReturnCodes.h"

#include "Algorithms/EscapeStrategies/EscapingStrategy.h"
#include "Algorithms/EscapingNCC.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

/**
 * This struct represents a turbine vertex with
 * the property that it has no incoming flow and that its
 * production is not routed via the shortest of its outgoing
 * edges.
 **/
struct LeafTurbine {
    LeafTurbine(vertexID turbine, edgeID currentEdge, edgeID shorterEdge)
      : turbine_(turbine),
        currentEdge_(currentEdge),
        shorterEdge_(shorterEdge) {}
    LeafTurbine() = delete;

    vertexID turbine_;
    edgeID currentEdge_;
    edgeID shorterEdge_;
    bool hasBeenUsed_ = false;

    vertexID    TurbineID()          const { return turbine_; }
    vertexID&   TurbineID()                { return turbine_; }
    edgeID      CurrentEdge()        const { return currentEdge_; }
    edgeID&     CurrentEdge()              { return currentEdge_; }
    edgeID      ShorterEdge()        const { return shorterEdge_; }
    edgeID&     ShorterEdge()              { return shorterEdge_; }
    bool        HasAlreadyBeenUsed() const { return hasBeenUsed_; }

    void        MarkAsUsed() { hasBeenUsed_ = true; }

    void        Print(std::ostream& verboseStream) {
        verboseStream << "\tTurbine " << TurbineID() << " has flow on edge " << CurrentEdge() << " and shorter edge " << ShorterEdge() << ".\n";
    }

    void        Print(bool beVerbose, std::ostream& verboseStream) {
        if (beVerbose) { Print(verboseStream); }
    }
};

/**
 * Escaping strategy that reroutes production from leaf turbines.
 */
struct EscapingLeaves : EscapingStrategy {
public:
    EscapingLeaves(WindfarmResidual& windfarm)
    : EscapingStrategy(windfarm, "Leaves") {}

    ReturnCode apply(EscapingNCC& ncc);

private:
    void ExploreVertexWhenFindingSubstationAssignment(vertexID currentVertex, vertexID substation, std::queue<vertexID>& accessibleVertices);

    bool IsSubstationNotAtEndOfAssignmentForVertex(vertexID vertex, vertexID substation);

    void ProcessEndVertexWhenFindingSubstationAssignment(vertexID endVertex, vertexID substation, std::queue<vertexID>& accessibleVertices);

    void IdentifyTurbineSubstationAssignmentUsingBFS();

    edgeID FindUniqueOutgoingEdgeWithFlowFromVertex(vertexID vertex);

    edgeID FindShortestOutgoingEdgeWithoutFlow(vertexID vertex, edgeID edgeWithFlow);

    void InsertLeaf(vertexID vertex, edgeID edgeWithFlow, edgeID shortestEdgeWithoutFlow);

    void IdentifyAllLeafTurbines();

    bool PotentiallyNewSubstationOfLeafHasFreeCapacity(LeafTurbine& leaf);

    bool MovingLeafMightNotChangeSubstationAssignment(LeafTurbine& leaf);

    bool IsMovingOfLeafFeasibleSubstationCapacitywise(LeafTurbine& leaf);

    void PrintAllLeaves(size_t originalNumberOfLeaves, bool beVerbose, std::ostream& verboseStream);

    edgeID SingleDFSStepToFindPathToRepeatingEdge(edgeID edgeToRepeat, edgeID edgeToExplore, bool& hasEdgeToRepeatAlreadyBeenSeen, std::vector<edgeID>& NextEdgeToASubstation);

    std::vector<edgeID> PopulateParentPointersUntilEdgeRepeats(edgeID edgeToRepeat, edgeID edgeToStartSearchWith);

    void CancelCycleWithAllPositiveFlow(edgeID edgeOnCycle, edgeID firstEdgeInSearch);

    edgeID ExploreEdgeWhenFindingPreviousSubstation(std::vector<bool>& hasEdgeAlreadyBeenExplored, std::vector<edgeID>& invertedPathToPreviousSubstation, edgeID edgeToExplore, edgeID firstEdgeInSearch);

    std::vector<edgeID> IdentifyInvertedPathToPreviousSubstation(LeafTurbine& leaf);

    edgeID ExploreEdgeWhenFindingNewSubstation(std::vector<bool>& hasEdgeAlreadyBeenExplored, std::vector<edgeID>& invertedPathToNewSubstation, edgeID edgeToExplore, edgeID firstEdgeInSearch, vertexID previousSubstation);

    std::vector<edgeID> IdentifyInvertedPathToNewSubstation(LeafTurbine& leaf, vertexID previousSubstation);

    void ChangeFlowOnPathToSuperSubstation(std::vector<edgeID>& invertedPathToNewSubstation, int delta, bool beVerbose, std::ostream& verboseStream);

    bool RerouteFlowForLeafIfPathsCanBeFound(LeafTurbine& leaf, SpecialResidualCostCollection& newResidualCosts, bool beVerbose, std::ostream& verboseStream);

    void DeleteUsedLeaves(bool aLeafHasBeenUsed);

    void PrintRemainingNumberOfLeaves(bool beVerbose, std::ostream& verboseStream);

    void BuildAdaptedResidualCostsForNewLeafEdge(SpecialResidualCostCollection& newResidualCosts, edgeID newEdge);

    bool TryToMoveALeaf(LeafTurbine& leaf, SpecialResidualCostCollection& newResidualCosts, bool beVerbose, std::ostream& verboseStream);

    void RepeatedlyConsiderAllLeavesForMoving(SpecialResidualCostCollection& newResidualCosts, bool beVerbose, std::ostream& verboseStream);

    ReturnCode DetermineReturnCodeAndCopyResidualCosts(EscapingNCC& ncc, size_t numberOfLeavesToStartWith, SpecialResidualCostCollection& newResidualCosts);

    ReturnCode SuccessivelyMoveLeaves(EscapingNCC& ncc);

    std::vector<std::vector<vertexID>> mappingTurbinesToSubstations_;
    std::vector<LeafTurbine> potentialLeaves_;
};

} // namespace Algorithms

} // namespace wcp

#endif