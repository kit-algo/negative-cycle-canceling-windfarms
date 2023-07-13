#include "Algorithms/EscapeStrategies/EscapingFreeCables.h"

namespace wcp {

namespace Algorithms {

ReturnCode EscapingFreeCables::apply(EscapingNCC& ncc) {
    CommonStartForAllEscapingStrategies(ncc.beVerbose_, ncc.verboseStream_);

    // This iteration of NCC used the residual costs from residualCostComputer_,
    // which includes the free upgrades. These residual costs are partially
    // carried over to the next NCC iteration(s).
    ReturnCode code = ncc.PerformNegativeCycleCancelingForSingleDelta(dummyLogger_, residualCostComputer_, 1, false);
    ncc.StealResidualCostCollectionInConstructionFrom(residualCostComputer_);
    return code;
};

} // namespace Algorithms

} // namespace wcp
