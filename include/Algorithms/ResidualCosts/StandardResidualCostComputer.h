#ifndef ALGORITHMS_RESIDUALCOSTS_STANDARDRESIDUALCOSTCOMPUTER
#define ALGORITHMS_RESIDUALCOSTS_STANDARDRESIDUALCOSTCOMPUTER

#include <cassert>
#include <iostream>

#include "Algorithms/ResidualCosts/SpecializedResidualCosts.h"

#include "Auxiliary/Constants.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

#include "DataStructures/Cabletypes.h"

namespace wcp {

namespace Algorithms {

/**
 * A struct to compute residual costs for the standard NCC algorithm
 * corresponding to the definition in doi:10.4230/LIPIcs.ESA.2019.55, p. 5,
 * based on a set of cable types across all edges.
 */
struct StandardResidualCostComputer {
public:
    StandardResidualCostComputer(DataStructures::WindfarmResidual& windfarm)
      : windfarm_(windfarm) {};

    virtual void ComputeResidualCosts(int changeOfFlow) {
        ComputeStandardResidualCosts(changeOfFlow);
    }

    /**
     * This struct does not allow changes on single edges, but substructs may.
     * We need this method and others since methods in NegativeCycleCanceling
     * are templated with ResidualCostComputers.
     */
    virtual void CopyAdaptedResidualCosts(const SpecialResidualCostCollection& /*newResidualCosts*/) {}

    virtual void OutputAdaptations(std::ostream& /*os*/) {}

    virtual void ClearListOfAdaptedResidualCosts() {}

    virtual void ClearListOfResidualCostsInConstruction() {}

    virtual void CreateNewCableTypeWhileCanceling(edgeID /*edge*/, int /*changeOfFlow*/, bool /*beVerbose_*/, std::ostream& /*verboseStream_*/) {}

protected:
    void ComputeStandardResidualCosts(int changeOfFlow) {
        for (edgeID edge = 0; edge < windfarm_.NumberOfEdges(); ++edge) {
            windfarm_.ResidualCostOnEdge(edge) = ResidualCosts(edge, changeOfFlow);
        }
    }
    
    DataStructures::WindfarmResidual& windfarm_;

    double ResidualCostForNonSupersubstationEdge(edgeID edge, vertexID start, int changeOfFlow, const DataStructures::Cabletypes& cabletypes) const {
        assert(windfarm_.StartVertex(edge) == start);
        assert(!windfarm_.IsSuperSubstation(start));
        assert(!windfarm_.IsSuperSubstation(windfarm_.EndVertex(edge)));

        int remainingCapacity;
        if (windfarm_.IsSubstation(start)) {
            // edge goes from a substation to a turbine
            // At most the flow to start can be redirected
            remainingCapacity = -windfarm_.FlowOnEdge(edge);
        } else {
            // edge starts at a turbine (goes to turbine or substation)
            remainingCapacity = windfarm_.FreeEdgeCapacity(edge);
        }
        // Compare delta with free capacity to determine costs
        if (changeOfFlow <= remainingCapacity) {
            int flow = windfarm_.FlowOnEdge(edge);
            return (cabletypes.CostOfFlow(flow + changeOfFlow) - cabletypes.CostOfFlow(flow))
                    * windfarm_.Length(edge);
        } else {
            // Not enough capacity for delta
            return Auxiliary::DINFTY;
        }
    }

private:
    double ResidualCosts(edgeID edge, int changeOfFlow) const {
        vertexID start = windfarm_.StartVertex(edge);
        vertexID end = windfarm_.EndVertex(edge);
        // First two ifs: Is edge an edge to or from a super substation?
        if (windfarm_.IsSuperSubstation(start)) {
            assert(windfarm_.IsSubstation(end));
            int flowToSubstation = -windfarm_.FlowOnEdge(edge);
            assert(flowToSubstation >= 0);
            return (changeOfFlow <= flowToSubstation) ? 0 : Auxiliary::DINFTY;
        }
        if (windfarm_.IsSuperSubstation(end)) {
            assert(windfarm_.IsSubstation(start));
            int freeCapacity = windfarm_.FreeSubstationCapacity(start);
            return (changeOfFlow <= freeCapacity) ? 0 : Auxiliary::DINFTY;
        }

        return ResidualCostForNonSupersubstationEdge(edge, start, changeOfFlow);
    }

    double ResidualCostForNonSupersubstationEdge(edgeID edge, vertexID start, int changeOfFlow) const {
        return ResidualCostForNonSupersubstationEdge(edge, start, changeOfFlow, windfarm_.Cabletypes());
    }
};

} // namespace Algorithms

} // namespace wcp

#endif