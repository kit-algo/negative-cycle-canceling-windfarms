#ifndef ALGORITHMS_DELTASTRATEGIES_INCREASINGDELTASTRATEGY
#define ALGORITHMS_DELTASTRATEGIES_INCREASINGDELTASTRATEGY

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"

namespace wcp {

namespace Algorithms {

/**
 * A delta strategy that increments the delta until a negative cycle was canceled.
 * If a cycle was canceled the delta is reset to 1.
 */
struct IncreasingDeltaStrategy : DeltaStrategy {
    IncreasingDeltaStrategy(int maximumFlowChange)
    : DeltaStrategy(maximumFlowChange) {}

    int FirstDelta() {
        return 1;
    }

    int NextDelta(int currentDelta, bool canceledNegativeCycle) {
        return canceledNegativeCycle ? 1 : (currentDelta + 1);
    }

    bool Continue(int upcomingDelta) {
        return upcomingDelta <= MaximumFlowChange();
    }
};

} // namespace Algorithms

} // namespace wcp

#endif
