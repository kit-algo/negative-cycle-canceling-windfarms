#ifndef ALGORITHMS_DELTASTRATEGIES_INCREASINGDECREASINGSTRATEGY
#define ALGORITHMS_DELTASTRATEGIES_INCREASINGDECREASINGSTRATEGY

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"

namespace wcp {

namespace Algorithms {

/**
 * A delta strategy that increments the delta until a negative cycle was
 * canceled. Afterwards, this delta is decremented until 1 is reached.  When
 * delta is decremented to 1, it is increased again to the last value of delta
 * for which a negative cycle was found (as a speed-up).
 */
class IncreasingDecreasingStrategy : public DeltaStrategy {
public:
    IncreasingDecreasingStrategy(int maximumFlowChange)
      : DeltaStrategy(maximumFlowChange),
        lastDeltaWithChange_(1),
        increasing_(true)
      {}

    int FirstDelta() {
        return 1;
    }

    int NextDelta(int currentDelta, bool canceledNegativeCycle) {
        if (canceledNegativeCycle) {
            lastDeltaWithChange_ = currentDelta;
            increasing_ = false;
        }

        if (currentDelta == 1 && !increasing_ ) {
            increasing_ = true;
            return lastDeltaWithChange_;
        }
        return increasing_ ? (currentDelta + 1) : (currentDelta - 1);
    }

    bool Continue(int upcomingDelta) {
        return upcomingDelta <= MaximumFlowChange();
    }

private:
    int  lastDeltaWithChange_;
    
    // boolean value is true iff delta values are currently increasing between one iteration and the next.
    bool increasing_;
};

} // namespace Algorithms

} // namespace wcp

#endif
