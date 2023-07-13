#ifndef ALGORITHMS_RESIDUALCOSTS_RESIDUALCOSTCOMPUTERFREESATISFIEDEDGES
#define ALGORITHMS_RESIDUALCOSTS_RESIDUALCOSTCOMPUTERFREESATISFIEDEDGES

#include <cassert>
#include <ostream>
#include <vector>

#include "Algorithms/ResidualCosts/SpecializedResidualCosts.h"
#include "Algorithms/ResidualCosts/StandardResidualCostComputer.h"

#include "Auxiliary/Constants.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

/**
 * A struct to compute residual costs for the EscapingFreeCables strategy.
 * Edges with saturated cables are given a free upgrade which is used for
 * an iteration of the NCC algorithm. These upgrades are also reflected
 * in special residual costs that are kept for the next regular NCC iteration,
 * see method StealResidualCostCollectionInConstructionFrom
 * in EscapingResidualCostComputer.
 */
struct ResidualCostComputerFreeSatisfiedEdges : public StandardResidualCostComputer {
public:
    ResidualCostComputerFreeSatisfiedEdges(DataStructures::WindfarmResidual& windfarm)
      : StandardResidualCostComputer(windfarm),
        isSaturatedEdge(windfarm.NumberOfEdges(), false) {};

    void ComputeResidualCosts(int changeOfFlow) {
        ComputeStandardResidualCosts(changeOfFlow);

        for (edgeID id = 0; id < 2*windfarm_.NumberOfOriginalEdges(); id++) {
            assert(!windfarm_.IsSuperSubstation(windfarm_.StartVertex(id)));
            assert(!windfarm_.IsSuperSubstation(windfarm_.EndVertex(id)));

            // An edge is called saturated if sending changeOfFlow additional units along it is feasible, but requires a bigger cable type.
            // We want to give those bigger cable types away for free.
            if (windfarm_.FlowOnEdge(id) != 0 && windfarm_.ResidualCostOnEdge(id) > 0 && windfarm_.ResidualCostOnEdge(id) < Auxiliary::DINFTY) {
                windfarm_.ResidualCostOnEdge(id) = 0;
                isSaturatedEdge[id] = true;
            } else {
                isSaturatedEdge[id] = false;
            }
        }
    }

    void CreateNewCableTypeWhileCanceling(edgeID edge, int changeOfFlow, bool beVerbose_, std::ostream& verboseStream_) {
        assert(edge >= 0);
        assert(edge < windfarm_.NumberOfEdges());

        if (isSaturatedEdge[edge]) { // Build an adapted cost vector
            assert(!windfarm_.IsSuperSubstation(windfarm_.StartVertex(edge)));
            assert(!windfarm_.IsSuperSubstation(windfarm_.EndVertex(edge)));

            std::vector<int> newCostVector = windfarm_.Cabletypes().ExpandedCosts();
            int previousFlow = windfarm_.FlowOnEdge(windfarm_.ReverseEdgeIndex(edge));
            int previousCost = newCostVector[newCostVector.size() / 2 + previousFlow];

            int newFlow = previousFlow - changeOfFlow;
            int newCost = newCostVector[newCostVector.size() / 2 + newFlow];
            for (int k = newCostVector.size() / 2 + previousFlow - 1; k >= 0; k--) {
                if (newCostVector[k] == previousCost) {
                    continue;
                } else {
                    newCostVector[k] = newCostVector[k] - newCost + previousCost;
                }
            }
            desiredResidualCostsInConstruction_.addEdge(windfarm_.ReverseEdgeIndex(edge), newCostVector);

            std::vector<int> reverseNewCostVector(newCostVector.size());
            std::reverse_copy(newCostVector.begin(),newCostVector.end(),reverseNewCostVector.begin());
            desiredResidualCostsInConstruction_.addEdge(edge, reverseNewCostVector);
            if (beVerbose_) {
                verboseStream_ << "New residual cost function on edge " << windfarm_.ReverseEdgeIndex(edge) << " ";
                for (unsigned int k = 0; k < newCostVector.size(); k++) {
                    verboseStream_ << newCostVector[k] << " ";
                }
                verboseStream_ << '\n';
                verboseStream_ << "New residual cost function on reverse edge " << edge << " ";
                for (unsigned int k = 0; k < reverseNewCostVector.size(); k++) {
                    verboseStream_ << reverseNewCostVector[k] << " ";
                }
                verboseStream_ << '\n';
            }
        }
    }

    void ClearListOfResidualCostsInConstruction() {
        desiredResidualCostsInConstruction_.Clear();
    }

    void OutputAdaptations(std::ostream& os) {
        desiredResidualCostsInConstruction_.WriteToStream(os);
    }

    SpecialResidualCostCollection desiredResidualCostsInConstruction_;

protected:
    std::vector<bool> isSaturatedEdge;
    
};

} // namespace Algorithms

} // namespace wcp

#endif