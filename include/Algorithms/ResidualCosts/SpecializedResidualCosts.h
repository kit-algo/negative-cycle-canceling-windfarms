#ifndef ALGORITHMS_RESIDUALCOSTS_SPECIALIZEDRESIDUALCOSTS
#define ALGORITHMS_RESIDUALCOSTS_SPECIALIZEDRESIDUALCOSTS

#include <vector>

#include "DataStructures/Graphs/Windfarm.h" // for vertexID, edgeID, and INVALID_EDGE
#include "DataStructures/Cabletypes.h"

namespace wcp {

namespace Algorithms {

/**
 * Stores adapted residual costs for a single edge
 */
struct SpecialResidualCost {
    edgeID edgeWithSpecialCosts = INVALID_EDGE;
    DataStructures::Cabletypes cables;

    SpecialResidualCost(edgeID edge, const std::vector<int>& desiredCostVector)
      : edgeWithSpecialCosts(edge),
        cables(desiredCostVector) {}
};

/**
 * Stores adapted residual costs for multiple edges
 */
struct SpecialResidualCostCollection {
    std::vector<SpecialResidualCost> collection; 
    
    void Clear() {
        collection.clear();
    }

    void addEdge(edgeID newEdge, const std::vector<int>& desiredCostVector) {
        collection.emplace_back(newEdge, desiredCostVector);
    }

    const std::vector<SpecialResidualCost>& showVector() const {
        return collection;
    }

    bool isEmpty() const {
        return collection.empty();
    }

    void WriteToStream(std::ostream& os) {
        os << "\tEdges with special residual costs: ";
        if (!isEmpty()) {
            for (auto&& desiredCostTuple : collection) {
                os << desiredCostTuple.edgeWithSpecialCosts << " ";
            }
        } else {
            os << "None.";
        }
        os << '\n';
    }
};

} // namespace Algorithms

} // namespace wcp

#endif
