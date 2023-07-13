#ifndef ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGFREECABLES
#define ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGFREECABLES

#include "Algorithms/EscapeStrategies/EscapingStrategy.h"
#include "Algorithms/EscapingNCC.h"
#include "Algorithms/ResidualCosts/ResidualCostComputerFreeSatisfiedEdges.h"
#include "Algorithms/ResidualCosts/EscapingResidualCostComputer.h"

#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

#include "IO/DummyNCCLogger.h"

namespace wcp {

namespace Algorithms {

/**
 * Escaping strategy that performs an NCC iteration while granting free upgrades on saturated edges.
 */
struct EscapingFreeCables : EscapingStrategy {
public:
    EscapingFreeCables(WindfarmResidual& windfarm)
      : EscapingStrategy(windfarm, "FreeCables"),
        residualCostComputer_(windfarm) {}

    ReturnCode apply(EscapingNCC& ncc);

private:
    IO::DummyNCCLogger dummyLogger_;
    ResidualCostComputerFreeSatisfiedEdges residualCostComputer_;
};

} // namespace Algorithms

} // namespace wcp

#endif