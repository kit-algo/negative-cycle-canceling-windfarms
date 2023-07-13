#ifndef ALGORITHMS_INITIALIZATIONSTRATEGIES_COLLECTING
#define ALGORITHMS_INITIALIZATIONSTRATEGIES_COLLECTING

#include <cassert>
#include <memory>

#include "Algorithms/Distance/Dijkstra.h"
#include "Algorithms/Distance/EdgeLength.h"

#include "Algorithms/InitializationStrategies/InitializationStrategy.h"

#include "Auxiliary/ReturnCodes.h"

namespace wcp {

namespace Algorithms {

/**
 * Initialization strategy that collects turbine production greedily
 * while traversing shortest paths.
 */
struct Collecting : public InitializationStrategy {
public:
    Collecting(WindfarmResidual& windfarm, std::unique_ptr<Dijkstra>&& dijkstra, vertexID targetSubstation)
    : InitializationStrategy(windfarm, std::move(dijkstra), targetSubstation) {
        // Adjust array length to wind farm, fill with 'false'
        haveTurbineProductionsBeenCollected.resize(windfarm.NumberOfOriginalVertices(), false);
    }

protected:
    bool HasTurbineProductionBeenCollected(vertexID turbineID) {
        assert(turbineID >= 0);
        assert(turbineID < Windfarm().NumberOfOriginalVertices());
        return haveTurbineProductionsBeenCollected[turbineID]; 
    }

    void SetTurbineProductionCollected(vertexID turbineID, bool isCollected) {
        assert(turbineID >= 0);
        assert(turbineID < Windfarm().NumberOfOriginalVertices());
        haveTurbineProductionsBeenCollected[turbineID] = isCollected; 
    }

    void UpdateFlowOnWalk(const std::vector<edgeID>& walk, vertexID substationID) {
        assert(Windfarm().EndVertex(walk.back()) == substationID);
        assert(!HasTurbineProductionBeenCollected(Windfarm().StartVertex(walk.front())));

        std::vector<int> freeCapacities(walk.size() + 1, 0);
        ComputeFreeCapacities(substationID, walk, freeCapacities);

        // Collect as much production on the walk as possible at any point
        int currentFlowValue = 0;
        for (unsigned int index = 0; index < walk.size(); index++) {
            vertexID startVertex = Windfarm().StartVertex(walk[index]);
            assert(Windfarm().IsTurbine(startVertex));
            if (!HasTurbineProductionBeenCollected(startVertex)
                    && currentFlowValue < freeCapacities[index]) {
                // Flow can be increased and turbine at the start of walk[index] has not been collected yet
                currentFlowValue++;
                haveTurbineProductionsBeenCollected[startVertex] = true;
            } // Otherwise do not increase current flow
            // Assign highest possible flow
            Windfarm().AddFlowOnEdge(walk[index], currentFlowValue);
        }

        Windfarm().AddFlowOnEdge(Windfarm().EdgeToSupersubstation(substationID),
                currentFlowValue);
    }

private:
    std::vector<bool> haveTurbineProductionsBeenCollected;

    /**
     * Computes, for each edge on the walk, the minimum of the free
     * substation capacity and the free capacities of all edges along the
     * subpath from this edge to the substation
     */
    void ComputeFreeCapacities(int substation,
                               const std::vector<edgeID>& walk,
                               std::vector<int>& freeCapacities) {
        assert(freeCapacities.size() == walk.size() + 1);

        int propagatedCapacity = Windfarm().FreeSubstationCapacity(substation);
        assert(propagatedCapacity > 0);

        freeCapacities.back() = propagatedCapacity;
        for (int index = walk.size() - 1; index >= 0; index--) {
            int freeCapacity = Windfarm().FreeEdgeCapacity(walk[index]);
            assert(freeCapacity > 0);
            if (freeCapacity < propagatedCapacity) {
                propagatedCapacity = freeCapacity;
            }
            freeCapacities[index] = propagatedCapacity;
        }
    }
};

} // namespace Algorithms

} // namespace wcp

#endif
