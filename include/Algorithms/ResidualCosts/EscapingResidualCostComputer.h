#ifndef ALGORITHMS_RESIDUALCOSTS_ESCAPINGRESIDUALCOSTCOMPUTER
#define ALGORITHMS_RESIDUALCOSTS_ESCAPINGRESIDUALCOSTCOMPUTER

#include <algorithm>

#include "Algorithms/ResidualCosts/SpecializedResidualCosts.h"
#include "Algorithms/ResidualCosts/ResidualCostComputerFreeSatisfiedEdges.h"
#include "Algorithms/ResidualCosts/StandardResidualCostComputer.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

/**
 * A struct to compute residual costs for the EscapingNCC algorithm.
 * Some edges may have special residual costs due to adaptations from
 * escaping strategies. The goal of these adaptations is that changes
 * from escaping are not reversed right away.
 */
struct ResidualCostComputerWithAdaptedEdges : public StandardResidualCostComputer {
public:
    ResidualCostComputerWithAdaptedEdges(DataStructures::WindfarmResidual& windfarm)
      : StandardResidualCostComputer(windfarm) {};

    void ComputeResidualCosts(int changeOfFlow) {
        ComputeStandardResidualCosts(changeOfFlow);
        ApplyContentOfSpecialResidualCostCollection(changeOfFlow);        
    }

    void CopyAdaptedResidualCosts(const SpecialResidualCostCollection& newResidualCosts) {
        desiredResidualCosts_ = newResidualCosts;
    }

    void StealResidualCostCollectionInConstructionFrom(ResidualCostComputerFreeSatisfiedEdges& residualCostComputer) {
        std::swap(desiredResidualCosts_, residualCostComputer.desiredResidualCostsInConstruction_);
        residualCostComputer.ClearListOfResidualCostsInConstruction();
    }

    void ClearListOfAdaptedResidualCosts() {
        desiredResidualCosts_.Clear();
    }

    void OutputAdaptations(std::ostream& os) {
        desiredResidualCosts_.WriteToStream(os);
    }

protected:
    // Maintains all edges with non-standard residual costs.
    SpecialResidualCostCollection desiredResidualCosts_;

    void ApplyContentOfSpecialResidualCostCollection(int changeOfFlow) {
        for (auto&& desiredCostTuple : desiredResidualCosts_.showVector()) {
            edgeID currentEdge = desiredCostTuple.edgeWithSpecialCosts;
            windfarm_.ResidualCostOnEdge(currentEdge) = ResidualCostForNonSupersubstationEdge(  currentEdge,
                                                                                                windfarm_.StartVertex(currentEdge),
                                                                                                changeOfFlow,
                                                                                                desiredCostTuple.cables);
        }
    }

};

} // namespace Algorithms

} // namespace wcp

#endif
