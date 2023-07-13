#ifndef ALGORITHMS_INITIALIZATIONSTRATEGIES_INITIALIZATIONSTRATEGY
#define ALGORITHMS_INITIALIZATIONSTRATEGIES_INITIALIZATIONSTRATEGY

#include <cassert>
#include <memory>

#include "Algorithms/Distance/Dijkstra.h"

#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

/**
 * Base class from which to derive initialization strategies.
 */
class InitializationStrategy {
public:
    /**
     * Constructor
     *
     * The Dijkstra object takes care of the shortest path computations during
     * initialization and knows which edge lengths to use.
     *
     * The argument targetSubstation is passed on to the Dijkstra object.
     * While Dijkstra can make use of a vertexID representing an actual substation,
     * the idea of the initialization strategy is to specify a category of substations,
     * e.g., the first free substation.
     */
    InitializationStrategy(WindfarmResidual& windfarm, std::unique_ptr<Dijkstra>&& dijkstra, vertexID targetSubstation)
    : windfarm_(windfarm),
      dijkstra_(std::move(dijkstra)),
      targetSubstation_(targetSubstation) {}

    ReturnCode InitializeFlow() {
        for (vertexID turbine : Windfarm().Turbines()) {
            if ( HasTurbineProductionBeenCollected(turbine) ) { continue; }

            // Set up shortest path computation from turbine to a free substation
            dijkstra_->StartVertex() = turbine;
            dijkstra_->TargetVertex() = targetSubstation_;
            ReturnCode returnCode = dijkstra_->Run();
            // At this point, dijkstra_.TargetVertex() contains the ID of the closest free
            // substation and dijkstra_.ExtractWalk() yields a sequence of edges leading
            // from turbine to this substation. 

            // If no free substation was found, return now.
            if (returnCode == ReturnCode::INFEASIBLE) {
                return ReturnCode::NO_FEASIBLE_SOLUTION_FOUND;
            }

            // Extract closest substation with free capacities
            vertexID substation = dijkstra_->TargetVertex();
            assert(Windfarm().IsSubstation(substation));
            assert(Windfarm().HasFreeSubstationCapacity(substation));

            std::vector<edgeID> walk = dijkstra_->ExtractWalk();
            assert(Windfarm().EndVertex(walk.back()) == substation);

            // Update the flow in wind farm depending on the instantiated sub class
            UpdateFlowOnWalk(walk, substation);
        }

        return ReturnCode::SUCCESS;
    }

    virtual ~InitializationStrategy() = default;

protected:
    /**
     * Returns whether production of turbine has been directed to a substation
     */
    virtual bool HasTurbineProductionBeenCollected(vertexID turbineID) = 0;

    /**
     * Sets internal state of a turbine if applicable
     **/
    virtual void SetTurbineProductionCollected(vertexID turbineID, bool isCollected) {
        (void) turbineID;
        (void) isCollected;
    };

    virtual void UpdateFlowOnWalk(const std::vector<edgeID>& walk, vertexID substationID) = 0;

    WindfarmResidual& Windfarm() { return windfarm_; }


private:
    WindfarmResidual& windfarm_;
    std::unique_ptr<Dijkstra> dijkstra_;
    vertexID targetSubstation_;

};

} // namespace Algorithms

} // namespace wcp

#endif
