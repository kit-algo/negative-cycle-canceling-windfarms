#ifndef ALGORITHMS_INITIALIZATIONSTRATEGIES_NONCOLLECTING
#define ALGORITHMS_INITIALIZATIONSTRATEGIES_NONCOLLECTING

#include <cassert>
#include <memory>
#include <utility>

#include "Algorithms/Distance/Dijkstra.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

/**
 * Initialization strategy that considers one turbine at a time only.
 */
struct NonCollecting : public InitializationStrategy {
public:
    NonCollecting(WindfarmResidual& windfarm, std::unique_ptr<Dijkstra>&& dijkstra, vertexID targetSubstation)
    : InitializationStrategy(windfarm, std::move(dijkstra), targetSubstation) {}

protected:
    bool HasTurbineProductionBeenCollected(vertexID /* turbineID */ ) { return false; }

    /**
     * The unit of production of the turbine at beginning of walk is added on all
     * edges of walk leading to substation.
     */
    void UpdateFlowOnWalk(const std::vector<edgeID>& walk, vertexID substationID) {
        assert(Windfarm().EndVertex(walk.back()) == substationID);
        for (edgeID edge : walk) {
            assert(!Windfarm().IsSuperSubstation(Windfarm().StartVertex(edge)));
            assert(!Windfarm().IsSuperSubstation(Windfarm().EndVertex(edge)));
            Windfarm().AddFlowOnEdge(edge, 1);
        }
        Windfarm().AddFlowOnEdge(Windfarm().EdgeToSupersubstation(substationID), 1);
    }

};

} // namespace Algorithms

} // namespace wcp

#endif
