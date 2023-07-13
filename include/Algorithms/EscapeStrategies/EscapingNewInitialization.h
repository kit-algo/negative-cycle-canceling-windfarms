#ifndef ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGNEWINITIALIZATION
#define ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGNEWINITIALIZATION

#include "Algorithms/EscapeStrategies/EscapingStrategy.h"
#include "Algorithms/EscapingNCC.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"

#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

class EscapingNCC;

/**
 * Escaping strategy that computes a new initial flow.
 */
struct EscapingNewInitialization : EscapingStrategy {
public:
    EscapingNewInitialization(WindfarmResidual& windfarm, InitializationStrategy& initializationStrategy)
    : EscapingStrategy(windfarm, "NewInit"),
      initializationStrategy_(initializationStrategy) {}

    ReturnCode apply(EscapingNCC& ncc) {
        CommonStartForAllEscapingStrategies(ncc.beVerbose_, ncc.verboseStream_);

        for (int i = 0; i < Windfarm().NumberOfEdges(); i++) {
            Windfarm().SetFlowOnEdge(i, 0);
        }

        // Reset internal state of initialization strategy.
        if (numberOfCalls_ > 2){
            for (vertexID turbine : Windfarm().Turbines()) {
                initializationStrategy_.SetTurbineProductionCollected(turbine, false);
            }
        }

        ReturnCode initializationCode = initializationStrategy_.InitializeFlow();

#ifndef NDEBUG
        if (initializationCode == ReturnCode::SUCCESS){
            AssertFeasibleFlow(Windfarm());
        }
#endif
        return initializationCode;
    };

private:
    InitializationStrategy& initializationStrategy_;
};

} // namespace Algorithms

} // namespace wcp

#endif