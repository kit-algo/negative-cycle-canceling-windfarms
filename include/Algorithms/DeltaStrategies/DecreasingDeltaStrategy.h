#ifndef ALGORITHMS_DELTASTRATEGIES_DECREASINGDELTASTRATEGY
#define ALGORITHMS_DELTASTRATEGIES_DECREASINGDELTASTRATEGY

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"

namespace wcp {

namespace Algorithms {

/**
 * A delta strategy that decrements the delta until a negative cycle was canceled.
 * If a cycle was canceled the delta is reset to its maximum value.
 */
struct DecreasingDeltaStrategy : DeltaStrategy {
    DecreasingDeltaStrategy(int maximumFlowChange)
      : DeltaStrategy(maximumFlowChange)
    {}

    int FirstDelta() {
        return MaximumFlowChange();
    }

    int NextDelta(int currentDelta, bool canceledNegativeCycle) {
        return canceledNegativeCycle ? MaximumFlowChange() : (currentDelta - 1);
    }

    bool Continue(int upcomingDelta) {
        return upcomingDelta > 0;
    }
};

} // namespace Algorithms

} // namespace wcp

#endif